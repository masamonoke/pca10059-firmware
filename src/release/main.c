#include "module/io/gpio_utils.h"

#define STRING_SEQUENCE "RYRRGGBGRRBB"

void modifed_blink(uint32_t led_id) {
    gpio_utils_turn_on_led(led_id);
    gpio_utils_pause();
    while(gpio_utils_is_button_released()) {
        //when button is pressed, the sequence continues
    }
    gpio_utils_turn_off_led(led_id);
    gpio_utils_pause();
}

void play_lights_sequence(const char* sequence) {
    size_t color_char_idx = 0;
    while (sequence[color_char_idx] != '\0') {
        if (gpio_utils_is_button_pressed()) {
            switch (sequence[color_char_idx]) {
                case 'R':
                    modifed_blink(LED_RED);
                    break;
                case 'G':
                    modifed_blink(LED_GREEN);
                    break;
                case 'B':
                    modifed_blink(LED_BLUE);
                    break;
                case 'Y':
                    modifed_blink(LED_YELLOW);
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
