#include "gpio_utils.h"

#define PAUSE_TIME_MS 500
#define DEBOUNCE_TIME 10

void gpio_utils_turn_on_led(uint32_t led_id) {
    nrf_gpio_pin_write(led_id, 0);
}

void gpio_utils_turn_off_led(uint32_t led_id) {
    nrf_gpio_pin_write(led_id, 1);
}

bool gpio_utils_read_button_input(void) {
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
    if (gpio_utils_read_button_input() == 0) {
        nrf_delay_ms(DEBOUNCE_TIME);
        return gpio_utils_read_button_input() == 0;
    }
    return false;
}

bool gpio_utils_is_button_released(void) {
    return !gpio_utils_is_button_pressed();
}

bool gpio_utils_is_led_on(uint32_t led_id) {
    nrf_gpio_cfg_output(led_id);
    return !nrf_gpio_pin_out_read(led_id);
}

void gpio_utils_pause(void) {
    nrf_delay_ms(PAUSE_TIME_MS);
}

void gpio_utils_init(void) {
    nrf_gpio_cfg_output(LED_YELLOW);
    nrf_gpio_cfg_output(LED_BLUE);
    nrf_gpio_cfg_output(LED_RED);
    nrf_gpio_cfg_output(LED_GREEN);
    gpio_utils_turn_off_led(LED_RED);
    gpio_utils_turn_off_led(LED_BLUE);
    gpio_utils_turn_off_led(LED_GREEN);
    gpio_utils_turn_off_led(LED_YELLOW);
    nrf_gpio_cfg_input(BUTTON, NRF_GPIO_PIN_PULLUP);
}