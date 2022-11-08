#ifndef MODULE_IO_GPIO_UTILS
#define MODULE_IO_GPIO_UTILS

#include "nrf_gpio.h"
#include "nrf_delay.h"

#define BUTTON      NRF_GPIO_PIN_MAP(1, 6)
#define LED_YELLOW  NRF_GPIO_PIN_MAP(0, 6)
#define LED_GREEN   NRF_GPIO_PIN_MAP(1, 9)
#define LED_RED     NRF_GPIO_PIN_MAP(0, 8)
#define LED_BLUE    NRF_GPIO_PIN_MAP(0, 12)

void gpio_utils_init(void);

void gpio_utils_turn_on_led(uint32_t led_id);

void gpio_utils_turn_off_led(uint32_t led_id);

bool gpio_utils_read_button_input(void);

bool gpio_utils_is_button_pressed(void);

bool gpio_utils_is_button_released(void);

void gpio_utils_led_invert(uint32_t led_id);

void gpio_utils_blink(uint32_t led_id);

bool gpio_utils_is_led_on(uint32_t pin_id);

void gpio_utils_pause(void);

#endif /* MODULE_IO_GPIO_UTILS */
