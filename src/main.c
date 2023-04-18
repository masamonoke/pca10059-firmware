#include "ble.h"
#include "ble_advertising.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "app_timer.h"
#include "bsp_btn_ble.h"
#include "nrf_ble_qwr.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_log_backend_usb.h"

#include "modules/ble/ble_service.h"
#include "modules/io/gpio_utils.h"
#include "modules/ble/ble_starter.h"

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
	ret_code_t err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
	APP_ERROR_CHECK(err_code);
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

static uint8_t notify_value = 0;

static void on_write(ble_service_data_t* p_service_data, ble_evt_t const* p_ble_evt) {
	
	ble_gatts_evt_write_t const* p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
	
	NRF_LOG_INFO("Data sent: %d", p_evt_write->data[0]);
	uint8_t value = p_evt_write->data[0];
	
	switch (value) {
		case 0:
			NRF_LOG_INFO("Turn off leds");
			gpio_utils_turn_off_led(LED_GREEN);
			gpio_utils_turn_off_led(LED_RED);
			gpio_utils_turn_off_led(LED_BLUE);
			break;
		case 1:
			NRF_LOG_INFO("Toggled green led");
			gpio_utils_led_invert(LED_GREEN);
			break;
		case 2:
			NRF_LOG_INFO("Toggled blue led");
			gpio_utils_led_invert(LED_BLUE);
			break;
		case 3:
			NRF_LOG_INFO("Toggled red led");
			gpio_utils_led_invert(LED_RED);
			break;
		default:
			NRF_LOG_INFO("Cannot map this data to led");
			break;
	}

	notify_value = value;
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
		default:
			break;
	}
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
	NRF_SDH_BLE_OBSERVER(ble_observer_, APP_BLE_OBSERVER_PRIO, ble_cus_on_ble_evt, NULL);
}

#define APP_ADV_INTERVAL 300
#define APP_ADV_DURATION 0
BLE_ADVERTISING_DEF(advertising_);

static void on_adv_event(ble_adv_evt_t ble_adv_evt) {
	ret_code_t err_code;

	switch (ble_adv_evt) {
		case BLE_ADV_EVT_FAST:
			NRF_LOG_INFO("Fast advertising");
			err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
			APP_ERROR_CHECK(err_code);	
			NRF_LOG_INFO("adv data: %s", advertising_.adv_data.adv_data.p_data);
			break;
		
		case BLE_ADV_EVT_IDLE:
			NRF_LOG_INFO("Idle");
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
	
	char notify_char_desc[] = "Notify char";
	uint8_t notify_char_desc_len = strlen(notify_char_desc);
	uint8_t init_value_1 = 18;
	ble_service_setup_characteristic (
		&service_data_s_,
		&char_data_s_,
		CUSTOM_VALUE_CHAR_UUID,
		init_value_1,
		NOTIFY | WRITE,
		notify_char_desc,
		notify_char_desc_len
	);
	err_code = ble_service_add_characteristic(&service_data_s_, &char_data_s_);
	APP_ERROR_CHECK(err_code);

	char indicate_char_description[] = "Indicate char";
	uint8_t indicate_char_len = strlen(indicate_char_description);
	uint8_t init_value_2 = 17;
	ble_service_setup_characteristic (
		&service_data_s_,
		&indicate_char_data_s_,
		CUSTOM_VALUE_CHAR_UUID_SECOND,
		init_value_2,
		INDICATE | READ,
		indicate_char_description,
		indicate_char_len
	);
	err_code = ble_service_add_characteristic(&service_data_s_, &indicate_char_data_s_);
	APP_ERROR_CHECK(err_code);
}

APP_TIMER_DEF(char_timer_id_);
#define TIMER_INTERVAL APP_TIMER_TICKS(1000)

static void timeout_handler(void* p_ctx) {
	notify_value += 1;
	ble_service_value_update_handler(&service_data_s_, notify_value, &char_data_s_, NOTIFY);
}


APP_TIMER_DEF(char_indicate_timer_id_);
#define TIMER_INTERVAL_FASTER APP_TIMER_TICKS(500)

static uint8_t indicate_value = 99;

static void timeout_handler_faster(void* p_ctx) {
	indicate_value -= 1;
	ble_service_value_update_handler(&service_data_s_, indicate_value, &indicate_char_data_s_, INDICATE);
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

static void timers_init(void) {
	ret_code_t err_code = app_timer_init();
	APP_ERROR_CHECK(err_code);
	app_timer_create(&char_timer_id_, APP_TIMER_MODE_REPEATED, timeout_handler);
	app_timer_start(char_timer_id_, TIMER_INTERVAL, NULL);
	app_timer_create(&char_indicate_timer_id_, APP_TIMER_MODE_REPEATED, timeout_handler_faster);
	app_timer_start(char_indicate_timer_id_, TIMER_INTERVAL_FASTER, NULL);
}

static void log_init(void) {
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

int main(void) {
	log_init();	
	timers_init();
	ble_stack_init();
	ble_starter_init();
	
	service_init();
	advertising_init();

	NRF_LOG_INFO("App started");

	advertising_start();

	while (true) {
		ble_starter_idle_state_handle();
	}
}
