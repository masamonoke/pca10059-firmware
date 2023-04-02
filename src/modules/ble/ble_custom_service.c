#include "ble_custom_service.h"
#include "sdk_common.h"
#include "nrf_gpio.h"
#include "boards.h"
#include "nrf_log.h"
#include <string.h>
#include "app_error.h"
#include "bsp.h"
#include "modules/io/gpio_utils.h"

static uint32_t custom_value_char_add(ble_cus_t* cus, const ble_cus_init_t* cus_init) {
	uint32_t err_code;
	
	ble_gatts_char_md_t char_md;
	memset(&char_md, 0, sizeof(char_md));
	char_md.char_props.read = 1;
    char_md.char_props.write = 1;
    char_md.char_props.notify = 0; 
    char_md.p_char_user_desc = NULL;
    char_md.p_char_pf = NULL;
    char_md.p_user_desc_md = NULL;
    char_md.p_cccd_md = NULL; 
    char_md.p_sccd_md = NULL;
	
	ble_gatts_attr_md_t attr_md;
	memset(&attr_md, 0, sizeof(attr_md));
	attr_md.read_perm  = cus_init->custom_value_char_attr_md.read_perm;
    attr_md.write_perm = cus_init->custom_value_char_attr_md.write_perm;
    attr_md.vloc = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen = 0;

	ble_uuid_t ble_uuid = {
		.type = cus->uuid_type,
		.uuid = CUSTOM_VALUE_CHAR_UUID
	};

	ble_gatts_attr_t attr_char_value;
	memset(&attr_char_value, 0, sizeof(attr_char_value));
	attr_char_value.p_uuid = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len = sizeof(uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len = sizeof(uint8_t);

	err_code = sd_ble_gatts_characteristic_add(
		cus->service_handle,
		&char_md, 
		&attr_char_value,
		&cus->custom_value_handles
	);

	return err_code;
}

uint32_t ble_cus_init(ble_cus_t* cus, const ble_cus_init_t* cus_init) {
	if (cus == NULL || cus_init == NULL) {
		return NRF_ERROR_NULL;
	}

	uint32_t err_code;
	cus->conn_handle = BLE_CONN_HANDLE_INVALID;
	ble_uuid128_t base_uuid = { CUSTOM_SERVICE_UUID_BASE };
	err_code = sd_ble_uuid_vs_add(&base_uuid, &cus->uuid_type);
	APP_ERROR_CHECK(err_code);
	ble_uuid_t ble_uuid = {
		.type = cus->uuid_type,
		.uuid = CUSTOM_SERVICE_UUID
	};
	err_code = sd_ble_gatts_service_add(
		BLE_GATTS_SRVC_TYPE_PRIMARY,
		&ble_uuid,
		&cus->service_handle
	);
	
	if (err_code != NRF_SUCCESS) {
		return err_code;
	}

	return custom_value_char_add(cus, cus_init);
}

static void on_connect(ble_cus_t* p_cus, ble_evt_t const* p_ble_evt) {
	NRF_LOG_INFO("Connected");
	ret_code_t err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
	APP_ERROR_CHECK(err_code);
	p_cus->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}

static void on_disconnect(ble_cus_t* p_cus, ble_evt_t const* p_ble_evt) {
	UNUSED_PARAMETER(p_ble_evt);
	NRF_LOG_INFO("Disconnected");
	p_cus->conn_handle = BLE_CONN_HANDLE_INVALID;
}

static void on_write(ble_cus_t* p_cus, ble_evt_t const* p_ble_evt) {
	ble_gatts_evt_write_t const* p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
	NRF_LOG_INFO("Data sent: %d", p_evt_write->data[0]);
	uint8_t led_number = p_evt_write->data[0];
	switch (led_number) {
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
}

void ble_cus_on_ble_evt(ble_evt_t const* p_ble_evt, void* p_ctx) {	

	ble_cus_t* p_cus = (ble_cus_t*) p_ctx;

	switch (p_ble_evt->header.evt_id) {
		case BLE_GAP_EVT_CONNECTED:
			on_connect(p_cus, p_ble_evt);
			break;
		case BLE_GAP_EVT_DISCONNECTED:
			on_disconnect(p_cus, p_ble_evt);
			break;
		case BLE_GATTS_EVT_WRITE:
			NRF_LOG_INFO("On write evt");
			on_write(p_cus, p_ble_evt);
			break;
		default:
			break;
	}
}

