#include "gpio_utils.h"
#include "app_timer.h"
#include "module/datastructures/queue.h"

static instance_t* _s_queue;

#define PAUSE_TIME_MS 500
#define DEBOUNCE_TIME 10
#define LED_COUNT 4
#define QUEUE_SIZE 100

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

APP_TIMER_DEF(blink_async_timer_id_);

void blink_async_handler(void* p_context) {
    uint16_t led_id = queue_ctx_instance_poll(_s_queue);
    gpio_utils_led_invert(led_id);
}

static void _s_timer_init() {
    ret_code_t err_code;

    err_code = app_timer_create(&blink_async_timer_id_, APP_TIMER_MODE_SINGLE_SHOT, blink_async_handler);
    APP_ERROR_CHECK(err_code);
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

    _s_timer_init();
    _s_queue = queue_ctx_alloc_instance(QUEUE_SIZE);
}

void gpio_utils_blink_async(uint32_t led_id, uint16_t delay_ms) {
    queue_ctx_instance_push(_s_queue, led_id);
    app_timer_start(blink_async_timer_id_, APP_TIMER_TICKS(delay_ms), NULL);
}