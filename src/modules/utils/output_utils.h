#ifndef MODULE_UTILS_OUTPUT_UTILS
#define MODULE_UTILS_OUTPUT_UTILS

#include "nrf_delay.h"
#include "boards.h"
#include "bsp.h"

#define BSP_LED_GREEN 2
#define BSP_LED_RED 1
#define BSP_LED_YELLOW 0

void blink(int led_id);
void pause(int ms);
void blink_bytes(size_t const size, void const * const ptr);
void blink_ms(int led_id, uint32_t ms);
void blink_ms_ms(int led_id, uint32_t ms1, uint32_t ms2);

#endif /* MODULE_UTILS_OUTPUT_UTILS */
