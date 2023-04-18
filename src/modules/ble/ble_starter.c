#include "ble_starter.h"
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

void ble_starter_idle_state_handle(void) {
	if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
	LOG_BACKEND_USB_PROCESS();
}

ret_code_t ble_starter_init(void) {
	if (DEVICE_NAME_LEN > 31) {
		NRF_LOG_INFO("Device name exeeds limit %d", MAX_DEVICE_LEN);
		APP_ERROR_CHECK(NRF_ERROR_INVALID_PARAM);
		return NRF_ERROR_INVALID_STATE;
	}
	power_management_init();
	gap_parameters_init();
	gatt_init();
	conn_params_init();

	return NRF_SUCCESS;
}
