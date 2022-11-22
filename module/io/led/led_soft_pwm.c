#include "led_soft_pwm.h"

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

static led_data_t s_current_led_object_;

static led_data_t s_leds_data_array_[LED_COUNT];

static bool s_is_all_led_duty_cycling_;

static uint32_t s_count_high_us_(uint32_t duty_cycle, uint32_t period_us) {
    return duty_cycle * period_us;
}

static uint32_t s_count_low_us_(uint32_t period_us, uint32_t high_us) {
    return period_us - high_us;
}

APP_TIMER_DEF(turn_off_timer_id);
APP_TIMER_DEF(turn_on_timer_id);

static void s_turn_on_led_handler_(void* p_context) {
    gpio_utils_turn_on_led(s_current_led_object_.led_id);
}

static void s_turn_off_handler_(void* p_context) {
    app_timer_stop(turn_on_timer_id);
    gpio_utils_turn_off_led(s_current_led_object_.led_id);

    if (!s_is_all_led_duty_cycling_) {
        return;
    }

    s_current_led_object_.high_us_time = s_current_led_object_.duty_cycle * s_current_led_object_.period_us;
    s_current_led_object_.low_us_time = s_current_led_object_.period_us - s_current_led_object_.high_us_time;

    if (!s_current_led_object_.is_duty_cycling) {
        return;
    }
    if (s_current_led_object_.is_rising) {
        s_current_led_object_.duty_cycle += s_current_led_object_.duty_cycle_step;
        if (s_current_led_object_.duty_cycle >= 1.f) {
            s_current_led_object_.is_rising = false;
            s_current_led_object_.duty_cycle = 1.f;
        }
    } else {
        s_current_led_object_.duty_cycle -= s_current_led_object_.duty_cycle_step;
        if (s_current_led_object_.duty_cycle <= 0.f) {
            s_current_led_object_.is_rising = true;
            s_current_led_object_.duty_cycle = 0.f;
        }
    }
}


static void s_timers_init_(void) {
    ret_code_t err_code;

    err_code = app_timer_create(&turn_off_timer_id, APP_TIMER_MODE_SINGLE_SHOT, s_turn_off_handler_);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&turn_on_timer_id, APP_TIMER_MODE_SINGLE_SHOT, s_turn_on_led_handler_);
    APP_ERROR_CHECK(err_code);  
}

static led_data_t s_default_led_data_(void) {
    led_data_t led_default_data;
    led_default_data.is_duty_cycling = true;
    led_default_data.period_us = 30000;
    led_default_data.duty_cycle = 1.f;
    led_default_data.duty_cycle_step = 0.04f;
    led_default_data.is_rising = false;
    led_default_data.high_us_time = s_count_high_us_(led_default_data.duty_cycle, led_default_data.period_us);
    led_default_data.low_us_time = s_count_low_us_(led_default_data.period_us, led_default_data.high_us_time);
    led_default_data.is_high_time = false;
    return led_default_data;
}

static void s_init_leds_gpio_(void) {
    nrf_gpio_cfg_output(LED_YELLOW);
    nrf_gpio_cfg_output(LED_BLUE);
    nrf_gpio_cfg_output(LED_RED);
    nrf_gpio_cfg_output(LED_GREEN);

    gpio_utils_turn_off_led(LED_RED);
    gpio_utils_turn_off_led(LED_BLUE);
    gpio_utils_turn_off_led(LED_GREEN);
    gpio_utils_turn_off_led(LED_YELLOW);
}

void led_soft_pwm_init(void) {
    led_data_t tmp = s_default_led_data_();
    tmp.led_id = LED_RED;
    s_leds_data_array_[0] = tmp;
    tmp.led_id = LED_BLUE;
    s_leds_data_array_[1] = tmp;
    tmp.led_id = LED_GREEN;
    s_leds_data_array_[2] = tmp;
    tmp.led_id = LED_YELLOW;
    s_leds_data_array_[3] = tmp;

    s_init_leds_gpio_();

    s_timers_init_();

    led_soft_pwm_set_global_duty_cycling_state(true);
}

static void s_find_and_set_led_object_by_id_(uint32_t led_id) {
    if (s_current_led_object_.led_id != led_id) {
        for (size_t i = 0; i < LED_COUNT; i++) {
            if (led_id == s_leds_data_array_[i].led_id) {
                s_current_led_object_ = s_leds_data_array_[i];
                break;
            }
        }
    }
} 

void led_soft_pwm_blink(uint32_t led_id) {
    s_find_and_set_led_object_by_id_(led_id);
    
    uint32_t low_time = s_current_led_object_.low_us_time / 1000;
    uint32_t high_time = s_current_led_object_.high_us_time / 1000;
    app_timer_start(turn_on_timer_id, APP_TIMER_TICKS(high_time), NULL);
    app_timer_start(turn_off_timer_id, APP_TIMER_TICKS(low_time + high_time), NULL);
}

void led_soft_pwm_set_global_duty_cycling_state(bool f) {
    s_is_all_led_duty_cycling_ = f;
}