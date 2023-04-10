#include "ble_service.h"
#include <string.h>
#include "nrf_log.h"

ret_code_t ble_service_add_service(ble_service_data_t* service_data, ble_uuid128_t base_uuid, uint16_t service_uuid) {
	if (service_data == NULL) {
		return NRF_ERROR_NULL;
	}

	uint32_t err_code;
	service_data->conn_handle = BLE_CONN_HANDLE_INVALID;
	err_code = sd_ble_uuid_vs_add(&base_uuid, &service_data->uuid_type);
	APP_ERROR_CHECK(err_code);
	ble_uuid_t ble_uuid = {
		.type = service_data->uuid_type,
		.uuid = service_uuid
	};
	err_code = sd_ble_gatts_service_add(
		BLE_GATTS_SRVC_TYPE_PRIMARY,
		&ble_uuid,
		&service_data->service_handle
	);

	return err_code;
}

ret_code_t ble_service_add_characteristic(ble_service_data_t* service_data, ble_custom_characteristic_data_t* char_data) {
	ret_code_t err_code;
	err_code = sd_ble_gatts_characteristic_add(
		service_data->service_handle,
		&char_data->char_md, 
		&char_data->attr_char_value,
		&char_data->value_handles
	);

	return err_code;
}

void ble_service_setup_characteristic(
	ble_service_data_t* service_data,
	ble_custom_characteristic_data_t* char_data,
	uint16_t char_uuid,
	uint8_t value,
	ble_service_char_type_t rw,
	char* user_desc,
	uint8_t user_desc_len
) {

	uint8_t read = rw & 0x1;
	uint8_t write = (rw >> 1) & 0x1;
	uint8_t notify = (rw >> 2) & 0x1;	
	uint8_t indicate = (rw >> 3) & 0x1;

	memset(&char_data->char_md, 0, sizeof(char_data->char_md));
	char_data->char_md.char_props.read = read;
	char_data->char_md.char_props.write = write;
	char_data->char_md.p_char_user_desc = (uint8_t*) user_desc;
	char_data->char_md.char_user_desc_size = user_desc_len;
	char_data->char_md.char_user_desc_max_size = user_desc_len;


	ble_gatts_attr_md_t cccd_md;
	if (notify | indicate) {
		memset(&cccd_md, 0, sizeof(cccd_md));
		BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
		BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
		cccd_md.vloc = BLE_GATTS_VLOC_STACK;
		char_data->char_md.p_cccd_md = &cccd_md;
		char_data->char_md.char_props.notify = notify;
		char_data->char_md.char_props.indicate = indicate;
	}

	memset(&char_data->attr_md, 0, sizeof(char_data->attr_md));
	char_data->attr_md.read_perm = service_data->custom_value_char_attr_md.read_perm;
    char_data->attr_md.write_perm = service_data->custom_value_char_attr_md.write_perm;
    char_data->attr_md.vloc = BLE_GATTS_VLOC_STACK;
    char_data->attr_md.rd_auth = 0;
    char_data->attr_md.wr_auth = 0;
    char_data->attr_md.vlen = 0;

	char_data->uuid = char_uuid;

	ble_uuid_t ble_uuid = {
		.type = service_data->uuid_type,
		.uuid = char_uuid
	};

	memset(&char_data->attr_char_value, 0, sizeof(char_data->attr_char_value));
	char_data->attr_char_value.p_uuid = &ble_uuid;
    char_data->attr_char_value.p_attr_md = &char_data->attr_md;
    char_data->attr_char_value.init_len = sizeof(uint8_t);
    char_data->attr_char_value.init_offs = 0;
    char_data->attr_char_value.max_len = sizeof(uint8_t);
	char_data->attr_char_value.p_value = &value;
}

uint32_t ble_service_value_update_handler(ble_service_data_t* service_data, uint8_t value, ble_custom_characteristic_data_t* char_data, ble_service_char_type_t type) {	
	ble_gatts_value_t gatts_value;
	memset(&gatts_value, 0, sizeof(gatts_value));
	gatts_value.len = sizeof(value);
	gatts_value.offset = 0;
	gatts_value.p_value = &value;

	ret_code_t err_code = sd_ble_gatts_value_set(
		service_data->conn_handle,
		char_data->value_handles.value_handle,
		&gatts_value
	);

	if (err_code != NRF_SUCCESS) {
		NRF_LOG_INFO("Custom value update: error happened: %d", err_code);
		return err_code;
	}
	
	uint8_t notify = (type >> 2) & 0x1;	
	uint8_t indicate = (type >> 3) & 0x1;

	if ((notify && indicate) || !(notify || indicate)) {
		NRF_LOG_INFO("Custom value update: both notify and indicate passed. Return");
		return NRF_ERROR_INVALID_PARAM;
	}

	if (service_data->conn_handle != BLE_CONN_HANDLE_INVALID) {
		ble_gatts_hvx_params_t hvx_params;
		memset(&hvx_params, 0, sizeof(hvx_params));
		hvx_params.handle = char_data->value_handles.value_handle;
		hvx_params.type = notify ? BLE_GATT_HVX_NOTIFICATION : BLE_GATT_HVX_INDICATION;
		hvx_params.p_len = &gatts_value.len;
		hvx_params.p_data = gatts_value.p_value;
		err_code = sd_ble_gatts_hvx(service_data->conn_handle, &hvx_params);
		NRF_LOG_INFO("Custom value update: %d", value);
	} else {
		NRF_LOG_INFO("Custom value update: not connected");
		err_code = NRF_ERROR_INVALID_STATE;
	}

	return err_code;
}
