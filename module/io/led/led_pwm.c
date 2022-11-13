#include "led_pwm.h"

#define LED_COUNT 4

struct led_data_t {
    uint32_t led_id;
    uint32_t period_us;
    uint32_t high_us_time;
    uint32_t low_us_time;
    float duty_cycle;
    float duty_cycle_step;
    bool is_rising;
    bool is_duty_cycling;
    bool is_high_time;
};

static led_data_t* _s_current_led_object;

static led_data_t _s_leds_data_array[LED_COUNT];

static bool _s_is_all_led_duty_cycling;

static uint32_t _s_count_high_us(uint32_t duty_cycle, uint32_t period_us) {
    return duty_cycle * period_us;
}

static uint32_t _s_count_low_us(uint32_t period_us, uint32_t high_us) {
    return period_us - high_us;
}

APP_TIMER_DEF(turn_off_timer_id);
APP_TIMER_DEF(turn_on_timer_id);

static void _s_turn_on_led_handler(void* p_context) {
    gpio_utils_turn_on_led(_s_current_led_object->led_id);
}

static void _s_turn_off_handler(void* p_context) {
    app_timer_stop(turn_on_timer_id);
    gpio_utils_turn_off_led(_s_current_led_object->led_id);

    if (!_s_is_all_led_duty_cycling) {
        return;
    }

    _s_current_led_object->high_us_time = _s_current_led_object->duty_cycle * _s_current_led_object->period_us;
    _s_current_led_object->low_us_time = _s_current_led_object->period_us - _s_current_led_object->high_us_time;

    if (!_s_current_led_object->is_duty_cycling) {
        return;
    }
    if (_s_current_led_object->is_rising) {
        _s_current_led_object->duty_cycle += _s_current_led_object->duty_cycle_step;
        if (_s_current_led_object->duty_cycle >= 1.f) {
            _s_current_led_object->is_rising = false;
            _s_current_led_object->duty_cycle = 1.f;
        }
    } else {
        _s_current_led_object->duty_cycle -= _s_current_led_object->duty_cycle_step;
        if (_s_current_led_object->duty_cycle <= 0.f) {
            _s_current_led_object->is_rising = true;
            _s_current_led_object->duty_cycle = 0.f;
        }
    }
}


static void _s_timers_init(void) {
    ret_code_t err_code;

    err_code = app_timer_create(&turn_off_timer_id, APP_TIMER_MODE_SINGLE_SHOT, _s_turn_off_handler);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&turn_on_timer_id, APP_TIMER_MODE_SINGLE_SHOT, _s_turn_on_led_handler);
    APP_ERROR_CHECK(err_code);  
}

static led_data_t _s_default_led_data(void) {
    led_data_t led_default_data;
    led_default_data.is_duty_cycling = true;
    led_default_data.period_us = 30000;
    led_default_data.duty_cycle = 1.f;
    led_default_data.duty_cycle_step = 0.04f;
    led_default_data.is_rising = false;
    led_default_data.high_us_time = _s_count_high_us(led_default_data.duty_cycle, led_default_data.period_us);
    led_default_data.low_us_time = _s_count_low_us(led_default_data.period_us, led_default_data.high_us_time);
    led_default_data.is_high_time = false;
    return led_default_data;
}

static void _s_init_leds_gpio(void) {
    nrf_gpio_cfg_output(LED_YELLOW);
    nrf_gpio_cfg_output(LED_BLUE);
    nrf_gpio_cfg_output(LED_RED);
    nrf_gpio_cfg_output(LED_GREEN);

    gpio_utils_turn_off_led(LED_RED);
    gpio_utils_turn_off_led(LED_BLUE);
    gpio_utils_turn_off_led(LED_GREEN);
    gpio_utils_turn_off_led(LED_YELLOW);
}

void led_pwm_init(void) {
    led_data_t tmp = _s_default_led_data();
    tmp.led_id = LED_RED;
    _s_leds_data_array[0] = tmp;
    tmp.led_id = LED_BLUE;
    _s_leds_data_array[1] = tmp;
    tmp.led_id = LED_GREEN;
    _s_leds_data_array[2] = tmp;
    tmp.led_id = LED_YELLOW;
    _s_leds_data_array[3] = tmp;

    _s_init_leds_gpio();

    _s_timers_init();
}

bool led_pwm_blink(uint32_t led_id) {
    led_data_t led;
    bool is_found = false;
    for (size_t i = 0; i < LED_COUNT; i++) {
        if (led_id == _s_leds_data_array[i].led_id) {
            led = _s_leds_data_array[i];
            is_found = true;
            break;
        }
    }

    if (!is_found) {
        return false;
    }

    _s_current_led_object = &led;

    float init_duty_cycle = _s_current_led_object->duty_cycle;
    uint8_t cycles = 0;
    while (cycles != 2) {
        if (_s_current_led_object->duty_cycle == !init_duty_cycle) {
            cycles++;
            init_duty_cycle = _s_current_led_object->duty_cycle;
        }
        uint32_t low_time = _s_current_led_object->low_us_time / 1000;
        uint32_t high_time = _s_current_led_object->high_us_time / 1000;
        app_timer_start(turn_on_timer_id, APP_TIMER_TICKS(high_time), NULL);
        app_timer_start(turn_off_timer_id, APP_TIMER_TICKS(low_time + high_time), NULL);
    }
    return true;
}

void led_pwm_set_global_duty_cycling_state(bool f) {
    _s_is_all_led_duty_cycling = f;
}