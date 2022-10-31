#include "../../module/io/gpio_utils.h"

void modifed_blink(uint32_t led_id) {
    gpio_utils_turn_on_led(led_id);
    nrf_delay_ms(500);
    while(gpio_utils_is_button_released()) {
        //when button is pressed, the sequence continues
    }
    gpio_utils_turn_off_led(led_id);
    nrf_delay_ms(500);
}

void play_lights_sequence_modified(const char* sequence) {
    size_t i = 0;
    while (sequence[i] != '\0') {
        if (gpio_utils_is_button_pressed()) {
            switch (sequence[i]) {
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
            i++;
        }
    }
}


int main(void) {
    while (true) {
        play_lights_sequence_modified("RRRGGGRRBB");
    }
}
