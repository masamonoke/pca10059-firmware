#include "led.h"

APP_TIMER_DEF(m_app_timer_id);
APP_TIMER_DEF(duty_timer_id);

static bool is_duty_cycling_ = true;
static uint32_t period_us_ = 1000;
static float duty_cycle_ = 1.f;
static float duty_cycle_step_ = 0.04f;
static bool is_rising_ = false;
static uint32_t led_id_;

static void change_duty_handler_(void* p_context) {
    if (!is_duty_cycling_) {
        return;
    }
    if (is_rising_) {
        duty_cycle_ += duty_cycle_step_;
        if (duty_cycle_ >= 1.f) {
            is_rising_ = false;
            duty_cycle_ = 1.f;
        }
    } else {
        duty_cycle_ -= duty_cycle_step_;
        if (duty_cycle_ <= 0.f) {
            is_rising_ = true;
            duty_cycle_ = 0.f;
        }
    }
}

static void led_blink_handler_(void* p_context) {
    uint32_t high_us = duty_cycle_ * period_us_;
    uint32_t low_us = period_us_ - high_us;
    gpio_utils_turn_on_led(led_id_);
    nrf_delay_us(high_us);
    gpio_utils_turn_off_led(led_id_);
    nrf_delay_us(low_us);
}

static void timer_init_(void) {
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
    err_code = app_timer_create(&m_app_timer_id, APP_TIMER_MODE_SINGLE_SHOT, led_blink_handler_);
    APP_ERROR_CHECK(err_code);
    err_code = app_timer_create(&duty_timer_id, APP_TIMER_MODE_SINGLE_SHOT, change_duty_handler_);
    APP_ERROR_CHECK(err_code);
}

void led_init(void) {
    nrf_gpio_cfg_output(LED_YELLOW);
    nrf_gpio_cfg_output(LED_BLUE);
    nrf_gpio_cfg_output(LED_RED);
    nrf_gpio_cfg_output(LED_GREEN);

    gpio_utils_turn_off_led(LED_RED);
    gpio_utils_turn_off_led(LED_BLUE);
    gpio_utils_turn_off_led(LED_GREEN);
    gpio_utils_turn_off_led(LED_YELLOW);

    timer_init_();

    led_set_duty_cycle_step(0.04f);
    led_set_pwm_frequency(1000);
}

void led_blink_pwm(uint32_t led_id) {
    float init_duty_cycle = duty_cycle_;
    uint8_t cycles = 0;
    led_id_ = led_id;
    while (cycles != 2) {
        if (duty_cycle_ == !init_duty_cycle) {
            cycles++;
            init_duty_cycle = duty_cycle_;
        }
        app_timer_start(m_app_timer_id, APP_TIMER_TICKS(10), NULL);
        app_timer_start(duty_timer_id, APP_TIMER_TICKS(50), NULL);
    }
}

void led_set_duty_cycling_state(bool flag) {
    is_duty_cycling_ = flag;
}

void led_set_duty_cycle_step(float step) {
    duty_cycle_step_ = step;
}

void led_set_pwm_frequency(uint32_t frequency) {
    //period_us_ = (int)(1.f / (frequency / 1000000.f));
    period_us_ = frequency;
}