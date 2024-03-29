#include "ble_app.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "app_timer.h"
#include "fds.h"
#include "peer_manager.h"
#include "peer_manager_handler.h"
#include "sensorsim.h"
#include "ble_conn_state.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_log_backend_usb.h"

#include "ble_service.h"
#include "modules/app/hsv_editor/hsv_editor.h"

//generated uuid 56e9dab7-a61a-4cfe-8baa-22a0248a0e0c
#define CUSTOM_SERVICE_UUID_BASE { 0x0C, 0x0E, 0x8A, 0x24, 0xA0, 0x22, \
						           0xAA, 0x8B,                         \
							       0xFE, 0x4C,                         \
                                   0x1A, 0xA6,                         \
                                   0x00, 0x00, 0xE9, 0x56              \
                                 }								       

#define CUSTOM_SERVICE_UUID 0xABCD
#define CUSTOM_VALUE_CHAR_UUID 0xABCE
#define CUSTOM_VALUE_CHAR_UUID_SECOND 0xECBA

#define MIN_CONN_INTERVAL MSEC_TO_UNITS(100, UNIT_1_25_MS)
#define MAX_CONN_INTERVAL MSEC_TO_UNITS(200, UNIT_1_25_MS)
#define SLAVE_LATENCY 0
#define CONN_SUP_TIMEOUT MSEC_TO_UNITS(2000, UNIT_10_MS)
#define APP_BLE_OBSERVER_PRIO 3

static ble_service_data_t service_data_s_;
static hsv_t color_data_s_;
static ble_custom_characteristic_data_t char_data_s_;
static ble_custom_characteristic_data_t indicate_char_data_s_;

static void on_connect(ble_service_data_t* p_service_data, ble_evt_t const* p_ble_evt) {
	p_service_data->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
	if (p_service_data->conn_handle != BLE_CONN_HANDLE_INVALID) {
		NRF_LOG_INFO("Connected");
	}
}

static void on_disconnect(ble_service_data_t* p_service_data, ble_evt_t const* p_ble_evt) {
	UNUSED_PARAMETER(p_ble_evt);
	NRF_LOG_INFO("Disconnected");
	p_service_data->conn_handle = BLE_CONN_HANDLE_INVALID;
}

#define APP_BLE_OBSERVER_PRIORITY 3

NRF_BLE_QWR_DEF(qwr_);

#define APP_BLE_CONN_CFG_TAG 1

static void ble_stack_init(void) {
	ret_code_t err_code;
	err_code = nrf_sdh_enable_request();
	APP_ERROR_CHECK(err_code);
	uint32_t ram_start = 0;
	err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
	APP_ERROR_CHECK(err_code);
	err_code = nrf_sdh_ble_enable(&ram_start);
}

#define APP_ADV_INTERVAL 300
#define APP_ADV_DURATION 0
BLE_ADVERTISING_DEF(advertising_);

static void on_adv_event(ble_adv_evt_t ble_adv_evt) {
	switch (ble_adv_evt) {
		case BLE_ADV_EVT_FAST:
			NRF_LOG_INFO("Fast advertising");
			NRF_LOG_INFO("adv data: %s", advertising_.adv_data.adv_data.p_data);
			break;
		
		case BLE_ADV_EVT_IDLE:
			NRF_LOG_INFO("Idle");
			break;
		default:
			break;
	}
}

static void qwr_error_handler(uint32_t nrf_err) {
	APP_ERROR_HANDLER(nrf_err);
}

static void service_init(void) {
	ret_code_t err_code;
	

	ble_uuid128_t base_uuid = { .uuid128 = CUSTOM_SERVICE_UUID_BASE };

	service_data_s_.uuid_type = BLE_UUID_TYPE_BLE;
	
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&service_data_s_.custom_value_char_attr_md.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&service_data_s_.custom_value_char_attr_md.write_perm);
	err_code = ble_service_add_service(
		&service_data_s_,
		base_uuid,
		CUSTOM_SERVICE_UUID
	);
	APP_ERROR_CHECK(err_code);
	
	char notify_char_desc[] = "Color change";
	uint8_t notify_char_desc_len = strlen(notify_char_desc);

	ble_service_setup_characteristic (
		&service_data_s_,
		&char_data_s_,
		CUSTOM_VALUE_CHAR_UUID,
		WRITE,
		notify_char_desc,
		notify_char_desc_len
	);
	err_code = ble_service_add_characteristic(&service_data_s_, &char_data_s_);
	APP_ERROR_CHECK(err_code);

	char indicate_char_description[] = "Color read";
	uint8_t indicate_char_len = strlen(indicate_char_description);
	ble_service_setup_characteristic (
		&service_data_s_,
		&indicate_char_data_s_,
		CUSTOM_VALUE_CHAR_UUID_SECOND,
		NOTIFY | READ,
		indicate_char_description,
		indicate_char_len
	);
	err_code = ble_service_add_characteristic(&service_data_s_, &indicate_char_data_s_);
	APP_ERROR_CHECK(err_code);
}

#define FIRST_CONN_PARAMS_UPDATE_DELAY APP_TIMER_TICKS(5000)
#define NEXT_CONN_PARAMS_UPDATE_DELAY APP_TIMER_TICKS(300000)
#define MAX_CONN_PARAMS_UPDATE_COUNT 3

static void advertising_start(void) {
	ret_code_t err_code = ble_advertising_start(
		&advertising_,
		BLE_ADV_MODE_FAST
	);
	APP_ERROR_CHECK(err_code);
}

static ble_uuid_t adv_uuids_s_[] = { { CUSTOM_SERVICE_UUID, BLE_UUID_TYPE_VENDOR_BEGIN } };

static void advertising_init(void) {
	ret_code_t err_code;

	ble_advertising_init_t init;
	memset(&init, 0, sizeof(init));
	init.advdata.name_type = BLE_ADVDATA_FULL_NAME;
	
	init.srdata.uuids_complete.uuid_cnt = ARRAY_SIZE(adv_uuids_s_);
	init.srdata.uuids_complete.p_uuids = adv_uuids_s_; 

	init.config.ble_adv_fast_enabled = true;
	init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
	init.config.ble_adv_fast_timeout = APP_ADV_DURATION;
	init.evt_handler = on_adv_event;

	err_code = ble_advertising_init(&advertising_, &init);
	APP_ERROR_CHECK(err_code);

	ble_advertising_conn_cfg_tag_set(&advertising_, APP_BLE_CONN_CFG_TAG);
}

hsv_t* ble_app_get_color_data(void) {
	color_data_s_.hue = char_data_s_.value.h;
	color_data_s_.saturation = char_data_s_.value.s;
	color_data_s_.value = char_data_s_.value.v;
	return &color_data_s_;
}

static bool is_color_char_updated_s_ = false;

bool ble_app_is_color_char_updated(void) {
	return is_color_char_updated_s_;
}

void ble_app_set_is_color_char_updated(bool flag) {
	is_color_char_updated_s_ = flag;
}

static void on_write(ble_service_data_t* p_service_data, ble_evt_t const* p_ble_evt) {
	ble_gatts_evt_write_t const* p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
	uint16_t handle = p_evt_write->handle;
	if (handle == char_data_s_.value_handles.value_handle) {
		if (p_evt_write->len == 3) {
			NRF_LOG_INFO("Set HSV through BLE to %d %d %d", char_data_s_.value.h, char_data_s_.value.s, char_data_s_.value.v);
			char_data_s_.value.h = p_evt_write->data[0];
			char_data_s_.value.s = p_evt_write->data[1];
			char_data_s_.value.v = p_evt_write->data[2];

			ble_app_set_is_color_char_updated(true);
		}
	} else {
		NRF_LOG_INFO("Unknown handle");
	}
}

void ble_cus_on_ble_evt(ble_evt_t const* p_ble_evt, void* p_ctx) {	
	switch (p_ble_evt->header.evt_id) {
		case BLE_GAP_EVT_CONNECTED:
			on_connect(&service_data_s_, p_ble_evt);
			break;
		case BLE_GAP_EVT_DISCONNECTED:
			on_disconnect(&service_data_s_, p_ble_evt);
			break;
		case BLE_GATTS_EVT_WRITE:
			NRF_LOG_INFO("On write evt");
			on_write(&service_data_s_, p_ble_evt);
			break;
		case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
			NRF_LOG_INFO("SECURITY PARAMS REQUEST");
            break;
		default:
			break;
	}
}

APP_TIMER_DEF(notify_timer_);
#define NOTIFY_TIMER_INTERVAL APP_TIMER_TICKS(1000)

static void notify_timer_handler(void* p_ctx) {
	ble_service_value_update_handler(&service_data_s_, char_data_s_.value, &indicate_char_data_s_, NOTIFY);
}

static void timers_init(void) {
	app_timer_create(&notify_timer_, APP_TIMER_MODE_REPEATED, notify_timer_handler);
	app_timer_start(notify_timer_, NOTIFY_TIMER_INTERVAL, NULL);
}

#define SEC_PARAM_BOND                  1          
#define SEC_PARAM_MITM                  0     
#define SEC_PARAM_LESC                  0                                       
#define SEC_PARAM_KEYPRESS              0                                       
#define SEC_PARAM_IO_CAPABILITIES       BLE_GAP_IO_CAPS_NONE                    
#define SEC_PARAM_OOB                   0                                  
#define SEC_PARAM_MIN_KEY_SIZE          7                                       
#define SEC_PARAM_MAX_KEY_SIZE          16      

static void pm_evt_handler(pm_evt_t const * p_evt)
{
    ret_code_t err_code;

    switch (p_evt->evt_id) {
        case PM_EVT_BONDED_PEER_CONNECTED: {
            NRF_LOG_INFO("Connected to a previously bonded device.");
        } break;

        case PM_EVT_CONN_SEC_SUCCEEDED: {
            NRF_LOG_INFO("Connection secured: role: %d, conn_handle: 0x%x, procedure: %d.",
				ble_conn_state_role(p_evt->conn_handle),
				p_evt->conn_handle,
				p_evt->params.conn_sec_succeeded.procedure);
        } break;

        case PM_EVT_CONN_SEC_FAILED: {
			NRF_LOG_INFO("PV_EVT_CONN_SEC_FAILED evt");
        } break;

        case PM_EVT_CONN_SEC_CONFIG_REQ: {
            pm_conn_sec_config_t conn_sec_config = {.allow_repairing = true};
            pm_conn_sec_config_reply(p_evt->conn_handle, &conn_sec_config);
			NRF_LOG_INFO("PM_EVT_CONN_SEC_CONFIG_REQ evt");
        } break;

        case PM_EVT_STORAGE_FULL: {
            // Run garbage collection on the flash.
            err_code = fds_gc();
            if (err_code == FDS_ERR_NO_SPACE_IN_QUEUES) {
                // Retry.
            }
            else {
                APP_ERROR_CHECK(err_code);
            }
			NRF_LOG_INFO("PM_EVT_STORAGE_FULL evt");
        } break;

        case PM_EVT_PEERS_DELETE_SUCCEEDED: {
            advertising_start();
			NRF_LOG_INFO("PV_EVT_PEERS_DELETE_SUCCEEDED evt");
        } break;
		default:
			break;
    }
}

static void peer_manager_init(void) {
	ble_gap_sec_params_t sec_param;
	ret_code_t err_code;
	
	err_code = pm_init();
	APP_ERROR_CHECK(err_code);

	memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));
    sec_param.bond           = SEC_PARAM_BOND;
    sec_param.mitm           = SEC_PARAM_MITM;
    sec_param.lesc           = SEC_PARAM_LESC;
    sec_param.keypress       = SEC_PARAM_KEYPRESS;
    sec_param.io_caps        = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob            = SEC_PARAM_OOB;
    sec_param.min_key_size   = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size   = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc  = 1;
    sec_param.kdist_own.id   = 1;
    sec_param.kdist_peer.enc = 1;
    sec_param.kdist_peer.id  = 1;

    err_code = pm_sec_params_set(&sec_param);
    APP_ERROR_CHECK(err_code);

    err_code = pm_register(pm_evt_handler);
    APP_ERROR_CHECK(err_code);
}

static void power_management_init(void) {
	ret_code_t err_code = nrf_pwr_mgmt_init();
	APP_ERROR_CHECK(err_code);
}

#define MIN_CONN_INTERVAL MSEC_TO_UNITS(100, UNIT_1_25_MS)
#define MAX_CONN_INTERVAL MSEC_TO_UNITS(200, UNIT_1_25_MS)
#define SLAVE_LATENCY 0
#define CONN_SUP_TIMEOUT MSEC_TO_UNITS(2000, UNIT_10_MS)
#define APP_BLE_OBSERVER_PRIO 3

#define MAX_DEVICE_LEN 31
static uint8_t* device_name_s_ = (uint8_t*) "Custom BLE slave";
#define DEVICE_NAME_LEN strlen((char*) device_name_s_)


static void gap_parameters_init(void) {
	ret_code_t err_code;
	ble_gap_conn_params_t gap_conn_params;
	ble_gap_conn_sec_mode_t security_mode;
	
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&security_mode);
	
	err_code = sd_ble_gap_device_name_set(&security_mode, device_name_s_, DEVICE_NAME_LEN);
	APP_ERROR_CHECK(err_code);
	
	memset(&gap_conn_params, 0, sizeof(gap_conn_params));
	
	gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
	gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
	gap_conn_params.conn_sup_timeout = CONN_SUP_TIMEOUT;

	err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
	APP_ERROR_CHECK(err_code);
}

NRF_BLE_GATT_DEF(gatt_);

static void gatt_init(void) {
	ret_code_t err_code = nrf_ble_gatt_init(&gatt_, NULL);
	APP_ERROR_CHECK(err_code);
}

#define FIRST_CONN_PARAMS_UPDATE_DELAY APP_TIMER_TICKS(5000)
#define NEXT_CONN_PARAMS_UPDATE_DELAY APP_TIMER_TICKS(300000)
#define MAX_CONN_PARAMS_UPDATE_COUNT 3

static uint16_t conn_handle_s_ = BLE_CONN_HANDLE_INVALID;

static void on_conn_params_evt(ble_conn_params_evt_t* p_evt) {
	ret_code_t err_code;
	if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED) {
		err_code = sd_ble_gap_disconnect(
			conn_handle_s_,
			BLE_HCI_CONN_INTERVAL_UNACCEPTABLE
		);
		APP_ERROR_CHECK(err_code);
	}	
}

static void conn_params_error_handler(uint32_t nrf_error) {
	APP_ERROR_HANDLER(nrf_error);
}

static void conn_params_init(void) {
	ret_code_t err_code;
	ble_conn_params_init_t con_par_init;
	memset(&con_par_init, 0, sizeof(con_par_init));
	con_par_init.p_conn_params = NULL;
	con_par_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
	con_par_init.next_conn_params_update_delay = NEXT_CONN_PARAMS_UPDATE_DELAY;
	con_par_init.max_conn_params_update_count = MAX_CONN_PARAMS_UPDATE_COUNT;
	con_par_init.start_on_notify_cccd_handle = BLE_GATT_HANDLE_INVALID;
	con_par_init.disconnect_on_fail = false;
	con_par_init.error_handler = conn_params_error_handler;
	con_par_init.evt_handler = on_conn_params_evt;
	err_code = ble_conn_params_init(&con_par_init);
	APP_ERROR_CHECK(err_code);
}

void ble_app_idle_state_handle(void) {
	if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
	LOG_BACKEND_USB_PROCESS();
}

ret_code_t ble_app_init(void) {

	ble_stack_init();
	
	if (DEVICE_NAME_LEN > 31) {
		NRF_LOG_INFO("Device name exeeds limit %d", MAX_DEVICE_LEN);
		APP_ERROR_CHECK(NRF_ERROR_INVALID_PARAM);
		return NRF_ERROR_INVALID_STATE;
	}

	power_management_init();
	gap_parameters_init();
	gatt_init();
	conn_params_init();

	service_init();
	advertising_init();
	advertising_start();

	timers_init();
	NRF_SDH_BLE_OBSERVER(ble_observer_, APP_BLE_OBSERVER_PRIO, ble_cus_on_ble_evt, NULL);
	peer_manager_init();

	nrf_ble_qwr_init_t qwr_init = { 0 };
	qwr_init.error_handler = qwr_error_handler;
	ret_code_t err_code = nrf_ble_qwr_init(&qwr_, &qwr_init);
	APP_ERROR_CHECK(err_code);

	return NRF_SUCCESS;
}

void ble_app_set_color_char_data(uint16_t h, uint8_t s, uint8_t v) {
	char_data_s_.value.h = h;
	char_data_s_.value.s =s;
	char_data_s_.value.v = v;
}
