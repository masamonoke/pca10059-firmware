#include "ble.h"
#include "ble_advertising.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "app_timer.h"
#include "nrf_ble_qwr.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_log_backend_usb.h"

#include "modules/ble/ble_service.h"
#include "modules/io/gpio_utils.h"
#include "modules/ble/ble_starter.h"
#include "modules/app/hsv_editor/hsv_editor.h"
#include "modules/app/hsv_editor/cli/usb/usb_cli.h"
#include "modules/io/led/led_soft_pwm.h"
#include "modules/io/button.h"
#include "modules/app/hsv_editor/hsv_editor_nvm.h"
#include "modules/error/runtime_error_impl.h"
#include <math.h>
#include "fds.h"
#include "nrf_fstorage.h"
#include "nrf_fstorage_sd.h"
#include "modules/memory/fstorage_utils.h"

#include "peer_manager.h"
#include "peer_manager_handler.h"
#include "ble_conn_state.h"
#include "nrf_nvmc.h"

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

static void ble_enable(void) {
	ble_stack_init();
	ble_starter_init();
	service_init();
	advertising_init();
	advertising_start();
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

			hsv_editor_set_hsv_object(char_data_s_.value.h, char_data_s_.value.s, char_data_s_.value.v);
			hsv_editor_set_is_nvm_write_time(true);
			NRF_LOG_INFO("Saved HSV color to nvm: %d %d %d", char_data_s_.value.h, char_data_s_.value.s, char_data_s_.value.v);
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
            if (err_code == FDS_ERR_NO_SPACE_IN_QUEUES)
            {
                // Retry.
            }
            else
            {
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

#define LAST_ID_DIGITS 8 //id 6608

static void s_dummy_func_(void) {}

int main(void) {
	hsv_editor_init(); 

	fstorage_utils_init();
	hsv_t data;
	bool is_data_in_nvm = fstorage_utils_read_hsv(&data);
	if (!is_data_in_nvm) {
		data.hue = (uint16_t) ceilf(360.f * LAST_ID_DIGITS / 100.f);
		data.saturation = 100;
		data.value = 100;
	}

	char_data_s_.value.h = data.hue;
	char_data_s_.value.s = data.value;
	char_data_s_.value.s = data.saturation;

	hsv_editor_set_hsv_object(data.hue, data.saturation, data.value);

	void (*usb_proceed)(void) = s_dummy_func_;
#ifdef ESTC_USB_CLI_ENABLED
    if (ESTC_USB_CLI_ENABLED) {
        usb_cli_init();
        usb_proceed = usb_cli_process;
    } else {
        usb_proceed = s_dummy_func_;    
    }
#endif

	ble_enable();
	timers_init();
	NRF_SDH_BLE_OBSERVER(ble_observer_, APP_BLE_OBSERVER_PRIO, ble_cus_on_ble_evt, NULL);
	peer_manager_init();

	nrf_ble_qwr_init_t qwr_init = { 0 };
	qwr_init.error_handler = qwr_error_handler;
	ret_code_t err_code = nrf_ble_qwr_init(&qwr_, &qwr_init);
	APP_ERROR_CHECK(err_code);

	while (true) {
		hsv_editor_change_color();
        hsv_editor_process_current_behavior();

        if (hsv_editor_get_is_nvm_write_time()) {
            hsv_t cur_hsv_obj = hsv_editor_get_hsv_object();
			char_data_s_.value.h = cur_hsv_obj.hue;
			char_data_s_.value.s = cur_hsv_obj.saturation;
			char_data_s_.value.v = cur_hsv_obj.value;

			fstorage_utils_write_hsv(cur_hsv_obj);
            hsv_editor_set_is_nvm_write_time(false);
            NRF_LOG_INFO("Saved HSV color to nvm: %d %d %d", cur_hsv_obj.hue, cur_hsv_obj.saturation, cur_hsv_obj.value);
        }


		if (!nrf_sdh_is_enabled()) {
			ble_enable();
			NRF_SDH_BLE_OBSERVER(ble_observer_, APP_BLE_OBSERVER_PRIO, ble_cus_on_ble_evt, NULL);
		}


        usb_proceed();

        if (runtime_error_is_any_error()) {
            runtime_error_log_stacktrace();
        }

		ble_starter_idle_state_handle();
	}
}
