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
#include "bsp_btn_ble.h"
#include "sensorsim.h"
#include "ble_conn_state.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "nrf_pwr_mgmt.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_log_backend_usb.h"

#define MAX_DEVICE_LEN 31
static uint8_t* device_name = (uint8_t*) "ThisIsAReallyLongNameAndLonger1";
#define DEVICE_NAME_LEN strlen((char*) device_name)

#define MIN_CONN_INTERVAL MSEC_TO_UNITS(100, UNIT_1_25_MS)
#define MAX_CONN_INTERVAL MSEC_TO_UNITS(200, UNIT_1_25_MS)
#define SLAVE_LATENCY 0
#define CONN_SUP_TIMEOUT MSEC_TO_UNITS(2000, UNIT_10_MS)

static void gap_parameters_init(void) {
	ret_code_t err_code;
	ble_gap_conn_params_t gap_conn_params;
	ble_gap_conn_sec_mode_t security_mode;
	
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&security_mode);
	
	err_code = sd_ble_gap_device_name_set(&security_mode, device_name, DEVICE_NAME_LEN);
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

#define APP_BLE_OBSERVER_PRIORITY 3

NRF_BLE_QWR_DEF(qwr_);

static uint16_t conn_handle_ = BLE_CONN_HANDLE_INVALID;

static void ble_evt_handler(ble_evt_t const* p_ble_evt, void* p_context) {
	ret_code_t err_code = NRF_SUCCESS;
	switch (p_ble_evt->header.evt_id) {
		
		case BLE_GAP_EVT_DISCONNECTED:
			NRF_LOG_INFO("Disconnected");
			break;
		
		case BLE_GAP_EVT_CONNECTED:
			NRF_LOG_INFO("Connected");
			err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
			APP_ERROR_CHECK(err_code);
			conn_handle_ = p_ble_evt->evt.gap_evt.conn_handle;
			err_code = nrf_ble_qwr_conn_handle_assign(&qwr_, conn_handle_);
			break;
		
		case BLE_GAP_EVT_PHY_UPDATE:
			NRF_LOG_INFO("PHY update request");
			ble_gap_phys_t const phys = {
				.rx_phys = BLE_GAP_PHY_AUTO,
				.tx_phys = BLE_GAP_PHY_AUTO
			};
			err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
			APP_ERROR_CHECK(err_code);
			break;
		
		default:
			break;
	}
}

#define APP_BLE_CONN_CFG_TAG 1

static void ble_stack_init(void) {
	ret_code_t err_code;
	err_code = nrf_sdh_enable_request();
	APP_ERROR_CHECK(err_code);
	uint32_t ram_start = 0;
	err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
	APP_ERROR_CHECK(err_code);
	err_code = nrf_sdh_ble_enable(&ram_start);
	NRF_SDH_BLE_OBSERVER(ble_observer_, APP_BLE_OBSERVER_PRIORITY, ble_evt_handler, NULL);
}

#define APP_ADV_INTERVAL 300
#define APP_ADV_DURATION 0
BLE_ADVERTISING_DEF(advertising_);

ble_advdata_t g_adv_data;

static void on_adv_event(ble_adv_evt_t ble_adv_evt) {
	ret_code_t err_code;

	switch (ble_adv_evt) {
		case BLE_ADV_EVT_FAST:
			NRF_LOG_INFO("fast advertising");
			err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
			APP_ERROR_CHECK(err_code);	
			NRF_LOG_INFO("adv data: %s", advertising_.adv_data.adv_data.p_data);
			break;
		
		case BLE_ADV_EVT_IDLE:
			NRF_LOG_INFO("idle");
			err_code = bsp_indication_set(BSP_INDICATE_IDLE);
			APP_ERROR_CHECK(err_code);
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
	nrf_ble_qwr_init_t qwr_init = { 0 };
	qwr_init.error_handler = qwr_error_handler;
	err_code = nrf_ble_qwr_init(&qwr_, &qwr_init);
	APP_ERROR_CHECK(err_code);
}

#define FIRST_CONN_PARAMS_UPDATE_DELAY APP_TIMER_TICKS(5000)
#define NEXT_CONN_PARAMS_UPDATE_DELAY APP_TIMER_TICKS(300000)
#define MAX_CONN_PARAMS_UPDATE_COUNT 3

static void on_conn_params_evt(ble_conn_params_evt_t* p_evt) {
	ret_code_t err_code;
	if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED) {
		err_code = sd_ble_gap_disconnect(conn_handle_, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
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

static void advertising_start(void) {
	ret_code_t err_code = ble_advertising_start(&advertising_, BLE_ADV_MODE_FAST);
	APP_ERROR_CHECK(err_code);
}

static void advertising_init(void) {
	ret_code_t err_code;

	ble_advertising_init_t init;
	memset(&init, 0, sizeof(init));
	init.advdata.name_type = BLE_ADVDATA_FULL_NAME;

	size_t device_name_size = DEVICE_NAME_LEN;
	size_t scan_res_flags_count = 4;
	size_t scan_res_free_memory = abs(device_name_size - MAX_DEVICE_LEN);
	size_t scan_flags_count_to_drop;
	if (scan_res_free_memory > scan_res_flags_count) {
		scan_flags_count_to_drop = 0;
	} else {
		scan_flags_count_to_drop = scan_res_flags_count - scan_res_free_memory;
	}
	uint16_t scan_res_data_size = device_name_size - scan_flags_count_to_drop;
	ble_advdata_manuf_data_t manuf_data_response = {
		.data.p_data = device_name,
		.data.size = scan_res_data_size,
		.company_identifier = 1
	};
	init.srdata.name_type = BLE_ADVDATA_NO_NAME;
	init.srdata.p_manuf_specific_data = &manuf_data_response;

	init.config.ble_adv_fast_enabled = true;
	init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
	init.config.ble_adv_fast_timeout = APP_ADV_DURATION;
	init.evt_handler = on_adv_event;

	err_code = ble_advertising_init(&advertising_, &init);
	APP_ERROR_CHECK(err_code);

	ble_advertising_conn_cfg_tag_set(&advertising_, APP_BLE_CONN_CFG_TAG);
}

static void power_management_init(void) {
	ret_code_t err_code = nrf_pwr_mgmt_init();
	APP_ERROR_CHECK(err_code);
}

static void idle_state_handle(void) {
	if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
	LOG_BACKEND_USB_PROCESS();
}

static void leds_init(void) {
	ret_code_t err_code = bsp_init(BSP_INIT_LEDS, NULL);
	APP_ERROR_CHECK(err_code);
}

static void timers_init(void) {
	ret_code_t err_code = app_timer_init();
	APP_ERROR_CHECK(err_code);
}

static void log_init(void) {
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

int main(void) {
	log_init();	
	timers_init();
	if (DEVICE_NAME_LEN > 31) {
		NRF_LOG_INFO("Device name exeeds limit %d", MAX_DEVICE_LEN);
		APP_ERROR_CHECK(NRF_ERROR_INVALID_PARAM);
		return 1;
	}
	leds_init();
	power_management_init();
	ble_stack_init();
	gap_parameters_init();
	gatt_init();
	advertising_init();
	service_init();
	conn_params_init();

	NRF_LOG_INFO("app started");

	advertising_start();

	while (true) {
		idle_state_handle();
	}
}
