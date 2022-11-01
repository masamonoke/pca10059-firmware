#include "gpio_utils.h"

#define PAUSE_TIME_MS 500

void gpio_utils_turn_on_led(uint32_t led_id) {
    nrf_gpio_cfg_output(led_id);
    nrf_gpio_pin_write(led_id, 0);
}

void gpio_utils_turn_off_led(uint32_t led_id) {
    nrf_gpio_cfg_output(led_id);
    nrf_gpio_pin_write(led_id, 1);
}

bool gpio_utils_listen_button_input() {
    nrf_gpio_cfg_input(BUTTON, NRF_GPIO_PIN_PULLUP);
    return nrf_gpio_pin_read(BUTTON);
}

void gpio_utils_led_invert(uint32_t led_id) {
    if (nrf_gpio_pin_out_read(led_id)) {
        gpio_utils_turn_on_led(led_id);
    } else {
        gpio_utils_turn_off_led(led_id);
    }
}

void gpio_utils_blink(uint32_t led_id) {
    gpio_utils_turn_on_led(led_id);
    gpio_utils_pause();
    gpio_utils_turn_off_led(led_id);
    gpio_utils_pause();
}

bool gpio_utils_is_button_pressed(void) {
    return gpio_utils_listen_button_input() == 0;
}

bool gpio_utils_is_button_released(void) {
    return gpio_utils_listen_button_input() == 1;
}

bool gpio_utils_is_pin_on(uint32_t pin_id) {
    nrf_gpio_cfg_output(pin_id);
    return !nrf_gpio_pin_out_read(pin_id);
}

void gpio_utils_pause(void) {
    nrf_delay_ms(PAUSE_TIME_MS);
}