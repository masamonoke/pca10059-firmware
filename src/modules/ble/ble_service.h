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

typedef struct {
	uint8_t initial_custom_value;
	ble_srv_cccd_security_mode_t custom_value_char_attr_md;
} ble_custom_init_t;

typedef struct {
	uint16_t service_handle;
	ble_gatts_char_handles_t custom_value_handles;
	uint64_t conn_handle;
	uint8_t uuid_type;
} ble_custom_t;

typedef struct {
	bool init_status;
	uint16_t uuid;
	ble_gatts_char_md_t char_md;
	ble_gatts_attr_md_t attr_md;
	ble_gatts_attr_t attr_char_value;
} ble_custom_characteristic_data_t;

typedef struct {
	uint16_t value_handle;
	uint16_t user_desc_handle;
	uint16_t client_char_desc_handle;
	uint16_t server_char_desc_handle;
} ble_custom_char_handles_t;


ret_code_t ble_service_add_service(
	ble_custom_t* cus, 
	const ble_custom_init_t* cus_init, 
	ble_uuid128_t base_uuid, 
	uint16_t service_uuid
);


ret_code_t ble_service_add_characteristic(
	ble_custom_t *cus,
	const ble_custom_init_t *cus_init,
	ble_custom_characteristic_data_t* data 
);


typedef enum {
	READ = 0b01,
	WRITE = 0b10
} ble_service_rw_t;

void ble_service_setup_characteristic_rw_test(
	ble_custom_t *service_data,
	ble_custom_init_t *init,
	ble_custom_characteristic_data_t *characteristic_data,
	uint16_t char_uuid,
	uint8_t value,
	ble_service_rw_t rw
);

