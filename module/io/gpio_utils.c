#include "gpio_utils.h"
#include "app_timer.h"
#include "nrf_log.h"
#include "module/error/runtime_error.h"

#define PAUSE_TIME_MS 500
#define DEBOUNCE_TIME 10
#define LED_COUNT 4

enum state_t {
    LED_ON,
    LED_OFF
};

typedef struct {
    uint32_t led_id;
    enum state_t state;
} s_led_object_t_;

static s_led_object_t_ s_led_data_objects_[4] = {
        { .led_id = LED_RED, .state = LED_OFF },
        { .led_id = LED_GREEN, .state = LED_OFF },
        { .led_id = LED_BLUE, .state = LED_OFF },
        { .led_id = LED_YELLOW, .state = LED_OFF },
};

static s_led_object_t_* s_get_obj_by_led_id_(uint32_t led_id) {

    switch (led_id) {
        case LED_RED:
            return &s_led_data_objects_[0];
            break;
        case LED_GREEN:
            return &s_led_data_objects_[1];
            break;
        case LED_BLUE:
            return &s_led_data_objects_[2];
            break;
        case LED_YELLOW:
            return &s_led_data_objects_[3];
            break;
    }
    return NULL;
}

static void s_transition_ (uint32_t led_id) {
    s_led_object_t_* obj = s_get_obj_by_led_id_(led_id);
    if (obj == NULL) {
        RUNTIME_ERROR("Wrong LED id passed", led_id);
        return;
    }
    if (obj->state == LED_OFF) {
        obj->state = LED_ON;
        gpio_utils_turn_on_led(obj->led_id);
        NRF_LOG_INFO("Turned on LED: %d", led_id);
    } else {
        obj->state = LED_OFF;
        gpio_utils_turn_off_led(obj->led_id);
        NRF_LOG_INFO("Turned off LED: %d", led_id);
    }
}


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

APP_TIMER_DEF(green_async_blink_timer_id_);
APP_TIMER_DEF(yellow_async_blink_timer_id_);
APP_TIMER_DEF(red_async_blink_timer_id_);
APP_TIMER_DEF(blue_async_blink_timer_id_);

static void s_handle_(void* p_context) {
    uint32_t led_id = *((uint32_t*) p_context);
    s_transition_(led_id);
}

void s_green_blink_async_handler_(void* p_context) {
    s_handle_(p_context);
}

void s_yellow_blink_async_handler_(void* p_context) {
    s_handle_(p_context);
}

void s_blue_blink_async_handler_(void* p_context) {
    s_handle_(p_context);
}

void s_red_blink_async_handler_(void* p_context) {
    s_handle_(p_context);
}

static void s_timer_init_() {
    ret_code_t err_code;

    err_code = app_timer_create(&green_async_blink_timer_id_, APP_TIMER_MODE_SINGLE_SHOT, s_green_blink_async_handler_);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&yellow_async_blink_timer_id_, APP_TIMER_MODE_SINGLE_SHOT, s_yellow_blink_async_handler_);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&red_async_blink_timer_id_, APP_TIMER_MODE_SINGLE_SHOT, s_red_blink_async_handler_);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&blue_async_blink_timer_id_, APP_TIMER_MODE_SINGLE_SHOT, s_blue_blink_async_handler_);
    APP_ERROR_CHECK(err_code);
}

//TODO: add init flag and func to check is module init
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

    s_timer_init_();
}

static uint32_t s_context_green_led_;
static uint32_t s_context_yellow_led_;
static uint32_t s_context_red_led_;
static uint32_t s_context_blue_led_;

void gpio_utils_blink_async(uint32_t led_id, uint16_t delay_ms) {
    switch (led_id) {
        case LED_YELLOW:
            s_context_yellow_led_ = led_id;
            app_timer_start(yellow_async_blink_timer_id_, APP_TIMER_TICKS(delay_ms), &s_context_yellow_led_);
            break;
        case LED_GREEN:
            s_context_green_led_ = led_id;
            app_timer_start(green_async_blink_timer_id_, APP_TIMER_TICKS(delay_ms), &s_context_green_led_);
            break;
        case LED_BLUE:
            s_context_blue_led_ = led_id;
            app_timer_start(blue_async_blink_timer_id_, APP_TIMER_TICKS(delay_ms), &s_context_blue_led_);
            break;
        case LED_RED:
            s_context_red_led_ = led_id;
            app_timer_start(red_async_blink_timer_id_, APP_TIMER_TICKS(delay_ms), &s_context_red_led_);
            break;
    }
}