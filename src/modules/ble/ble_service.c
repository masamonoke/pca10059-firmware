#include "ble_service.h"
#include <string.h>

ret_code_t ble_service_add_service(
	ble_custom_t* cus, 
	const ble_custom_init_t* cus_init, 
	ble_uuid128_t base_uuid, 
	uint16_t service_uuid
) 
{
	if (cus == NULL || cus_init == NULL) {
		return NRF_ERROR_NULL;
	}

	uint32_t err_code;
	cus->conn_handle = BLE_CONN_HANDLE_INVALID;
	err_code = sd_ble_uuid_vs_add(&base_uuid, &cus->uuid_type);
	APP_ERROR_CHECK(err_code);
	ble_uuid_t ble_uuid = {
		.type = cus->uuid_type,
		.uuid = service_uuid
	};
	err_code = sd_ble_gatts_service_add(
		BLE_GATTS_SRVC_TYPE_PRIMARY,
		&ble_uuid,
		&cus->service_handle
	);

	return err_code;
}

static char str[] = "user desc";

typedef enum {
	INITIALIZED = 0,
	NOT_INITIALIZED = 1
} char_init_status_t;

ret_code_t ble_service_add_characteristic(
	ble_custom_t *cus,
	const ble_custom_init_t *cus_init,
	ble_custom_characteristic_data_t* data
) {
	ret_code_t err_code;
	err_code = sd_ble_gatts_characteristic_add(
		cus->service_handle,
		&data->char_md, 
		&data->attr_char_value,
		&cus->custom_value_handles
	);

	return err_code;
}

void ble_service_setup_characteristic_rw_test(
	ble_custom_t *service_data,
	ble_custom_init_t *init,
	ble_custom_characteristic_data_t *characteristic_data,
	uint16_t char_uuid,
	uint8_t value,
	ble_service_rw_t rw
) {

	uint8_t read = rw & 0x1;
	uint8_t write = (rw >> 1) & 0x1;

	memset(&characteristic_data->char_md, 0, sizeof(characteristic_data->char_md));
	characteristic_data->char_md.char_props.read = read;
	characteristic_data->char_md.char_props.write = write;
	characteristic_data->char_md.char_user_desc_size = strlen(str);
	characteristic_data->char_md.char_user_desc_max_size = strlen(str);
	characteristic_data->char_md.p_char_user_desc = (uint8_t *) str;	

	memset(&characteristic_data->attr_md, 0, sizeof(characteristic_data->attr_md));
	characteristic_data->attr_md.read_perm  = init->custom_value_char_attr_md.read_perm;
    characteristic_data->attr_md.write_perm = init->custom_value_char_attr_md.write_perm;
    characteristic_data->attr_md.vloc = BLE_GATTS_VLOC_STACK;
    characteristic_data->attr_md.rd_auth = 0;
    characteristic_data->attr_md.wr_auth = 0;
    characteristic_data->attr_md.vlen = 0;

	characteristic_data->uuid = char_uuid;

	ble_uuid_t ble_uuid = {
		.type = service_data->uuid_type,
		.uuid = char_uuid
	};

	memset(&characteristic_data->attr_char_value, 0, sizeof(characteristic_data->attr_char_value));
	characteristic_data->attr_char_value.p_uuid = &ble_uuid;
    characteristic_data->attr_char_value.p_attr_md = &characteristic_data->attr_md;
    characteristic_data->attr_char_value.init_len = sizeof(uint8_t);
    characteristic_data->attr_char_value.init_offs = 0;
    characteristic_data->attr_char_value.max_len = sizeof(uint8_t);
	characteristic_data->attr_char_value.p_value = &value;
}
