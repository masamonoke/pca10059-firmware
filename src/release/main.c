#include "module/io/gpio_utils.h"

#define STRING_SEQUENCE "RYRRGGBGRRBB"

//delay modified to prevent cases when click happend during delay
//and enable ability to "click" the sequence
void modified_delay(uint32_t ms) {
    if (ms == 0) {
        return;
    }

    do {
        nrf_delay_us(1000);
        if (gpio_utils_is_button_released()) {
            return;
        }
    } while (--ms);
}

void modified_blink(uint32_t led_id) {
    gpio_utils_turn_on_led(led_id);
    modified_delay(500);
    while(gpio_utils_is_button_released()) {
        //when button is pressed, the sequence continues
    }
    gpio_utils_turn_off_led(led_id);
    modified_delay(500);
}

void play_lights_sequence(const char* sequence) {
    size_t color_char_idx = 0;
    while (sequence[color_char_idx] != '\0') {
        if (gpio_utils_is_button_pressed()) {
            switch (sequence[color_char_idx]) {
                case 'R':
                    modified_blink(LED_RED);
                    break;
                case 'G':
                    modified_blink(LED_GREEN);
                    break;
                case 'B':
                    modified_blink(LED_BLUE);
                    break;
                case 'Y':
                    modified_blink(LED_YELLOW);
                    break;
            }
            color_char_idx++;
        }
    }
}

int main(void) {
    while (true) {
        play_lights_sequence(STRING_SEQUENCE);
    }
}
