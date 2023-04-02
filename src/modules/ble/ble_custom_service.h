#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"

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
} ble_cus_init_t;

typedef struct {
	uint16_t service_handle;
	ble_gatts_char_handles_t custom_value_handles;
	uint64_t conn_handle;
	uint8_t uuid_type;
} ble_cus_t;

uint32_t ble_cus_init(ble_cus_t* cus, const ble_cus_init_t* cus_init);

void ble_cus_on_ble_evt(ble_evt_t const* p_ble_evt, void* p_ctx);

uint32_t ble_custom_custom_value_update(ble_cus_t* p_cus, uint8_t custom_value);
