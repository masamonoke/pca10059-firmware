#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "boards.h"
#include "nrf.h"
#include "bsp.h"

#define ID 6608
#define ID_LEN 4

void fill_blink_times_from_id(int* arr, int id, size_t len) {
    int n;
    size_t i = len - 1;
    while (id != 0) {
        n = id % 10;
        id /= 10;
        arr[i--] = n;
    }
}

int avg_pause(int* arr, size_t len) {
    int s = 0;
    for (size_t i = 0; i < len; i++) {
        s += arr[i];
    }
    return s / len * 1000;
}

int main(void) {
    bsp_board_init(BSP_INIT_LEDS);

    int blink_times[ID_LEN];
    fill_blink_times_from_id(blink_times, ID, ID_LEN);
    //for zeros in id
    int avg_delay = avg_pause(blink_times, ID_LEN);

    while (true) {
        for (int i = 0; i < ID_LEN; i++) {
            if (blink_times[i] == 0) {
                nrf_delay_ms(avg_delay);
            }
            for (int j = 0; j < blink_times[i]; j++) {
                bsp_board_led_invert(i);
                nrf_delay_ms(500);
                bsp_board_led_invert(i);
                nrf_delay_ms(500);
            }
            nrf_delay_ms(500);
        }
    }
}
