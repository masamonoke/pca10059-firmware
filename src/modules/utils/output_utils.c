#include "output_utils.h"

void blink_bytes(size_t const size, void const * const ptr) {
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;

    for (i = size - 1; i >= 0; i--) {
        for (j = 7; j >= 0; j--) {
            byte = (b[i] >> j) & 1;
            if (byte == 0) {
                blink_ms_ms(BSP_LED_RED, 1000, 500);
            } else {
                blink_ms_ms(BSP_LED_GREEN, 1000, 500);
            }
        }
    }
    blink(BSP_LED_YELLOW);
    pause(500);
}

void pause(int ms) {
    nrf_delay_ms(ms);
}

void blink(int led_id) {
    blink_ms(led_id, 500);
}

void blink_ms(int led_id, uint32_t ms) {
    bsp_board_led_invert(led_id);
    pause(ms);
    bsp_board_led_invert(led_id);
    pause(ms);
}

void blink_ms_ms(int led_id, uint32_t ms1, uint32_t ms2) {
    bsp_board_led_invert(led_id);
    pause(ms1);
    bsp_board_led_invert(led_id);
    pause(ms2);
}