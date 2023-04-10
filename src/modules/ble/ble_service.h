#ifndef BLE_SERVICE_H
#define BLE_SERVICE_H

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"
#include "app_error.h"

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

typedef enum {
	EMPTY_MODE = 0b0000,
	READ = 0b0001,
	WRITE = 0b0010,
	NOTIFY = 0b0100,
	INDICATE = 0b1000
} ble_service_char_type_t;

typedef struct {
	uint16_t service_handle;
	uint64_t conn_handle;
	uint8_t uuid_type;
	ble_srv_cccd_security_mode_t custom_value_char_attr_md;
} ble_service_data_t;

typedef struct {
	uint16_t uuid;
	ble_gatts_char_md_t char_md;
	ble_gatts_attr_md_t attr_md;
	ble_gatts_attr_t attr_char_value;
	ble_gatts_char_handles_t value_handles;
} ble_custom_characteristic_data_t;

ret_code_t ble_service_add_service(
	ble_service_data_t* service_data, 
	ble_uuid128_t base_uuid, 
	uint16_t service_uuid
);


ret_code_t ble_service_add_characteristic(ble_service_data_t* service_data, ble_custom_characteristic_data_t* char_data);

void ble_service_setup_characteristic (
	ble_service_data_t* service_data,
	ble_custom_characteristic_data_t* char_data,
	uint16_t char_uuid, 
	uint8_t value, ble_service_char_type_t type, 
	char* user_desc,
	uint8_t user_desc_len
);

uint32_t ble_service_value_update_handler(
	ble_service_data_t* service_data,
	uint8_t value,
	ble_custom_characteristic_data_t* char_data,
	ble_service_char_type_t type
);

#endif // BLE_SERVICE_H
