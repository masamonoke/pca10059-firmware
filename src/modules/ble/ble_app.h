#ifndef BLE_APP_H
#define BLE_APP_H

#include "app_error.h"

#include "modules/color/converter.h"

ret_code_t ble_app_init(void);

void ble_app_idle_state_handle(void);

hsv_t* ble_app_get_color_data(void);

void ble_app_set_color_char_data(uint16_t h, uint8_t s, uint8_t v);

bool ble_app_is_color_char_updated(void);

void ble_app_set_is_color_char_updated(bool flag);

#endif // BLE_APP_H
