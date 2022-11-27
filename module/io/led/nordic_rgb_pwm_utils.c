#include "nordic_rgb_pwm_utils.h"
#include "app_timer.h"
#include "module/io/gpio_utils.h"
#include "module/color/converter.h"
#include <math.h>

#define PWM_TOP 100

static nrfx_pwm_t s_pwm_instance_;
static nrf_pwm_values_individual_t s_seq_values_[] = {{ 0, 0, 0, 0 }};

static const nrf_pwm_sequence_t seq = {
    .values.p_individual = s_seq_values_,
    .length = NRF_PWM_VALUES_LENGTH(s_seq_values_),
    .repeats = 0,
    .end_delay = 0
};

static void pmw_update_duty_cycle(uint8_t duty_red, uint8_t duty_green, uint8_t duty_blue) {
    if (duty_red >= 100) {
        s_seq_values_->channel_0 = 100;
    } else {
        s_seq_values_->channel_0 = duty_red;
    }
    if (duty_green >= 100) {
        s_seq_values_->channel_1 = 100;
    } else {
        s_seq_values_->channel_1 = duty_green;
    }
    if (duty_blue >= 100) {
        s_seq_values_->channel_2 = 100;
    } else {
        s_seq_values_->channel_2 = duty_blue;
    }

    nrfx_pwm_simple_playback(&s_pwm_instance_, &seq, 1, NRFX_PWM_FLAG_LOOP);
}

APP_TIMER_DEF(change_duty_cycle_timer_id);

#define DUTY_CYCLE_STEP 1

static uint8_t s_current_duty_cycle_r_;
static uint8_t s_current_duty_cycle_g_;
static uint8_t s_current_duty_cycle_b_;
static uint8_t s_current_duty_cycle_step_ = DUTY_CYCLE_STEP;

static void s_change_duty_cycle_handler_(void* p_context) {
    s_current_duty_cycle_r_ += s_current_duty_cycle_step_;
    s_current_duty_cycle_g_ += s_current_duty_cycle_step_;
    s_current_duty_cycle_b_ += s_current_duty_cycle_step_;
    if (s_current_duty_cycle_r_  >= 100 || 
            s_current_duty_cycle_g_  >= 100 || s_current_duty_cycle_b_  >= 100) {

        s_current_duty_cycle_step_ = -s_current_duty_cycle_step_;

    } else if (s_current_duty_cycle_r_  <= 0 || 
            s_current_duty_cycle_g_  <= 0 || s_current_duty_cycle_b_  <= 0) {

        s_current_duty_cycle_step_ = -s_current_duty_cycle_step_;

    }
    
    nordic_rgb_pwm_update_duty_cycle(s_current_duty_cycle_r_, s_current_duty_cycle_g_,
         s_current_duty_cycle_b_);
}

APP_TIMER_DEF(invert_leds_timer_id);

static uint8_t s_remembered_duty_r_;
static uint8_t s_remembered_duty_g_;
static uint8_t s_remembered_duty_b_;
static bool s_is_to_turn_on_ = false;

 static void s_invert_leds_handler_(void* p_context) {
    if (s_is_to_turn_on_) {
        nordic_rgb_pwm_update_duty_cycle(s_remembered_duty_r_,
             s_remembered_duty_g_, s_remembered_duty_b_);
    } else {
        nordic_rgb_pwm_update_duty_cycle(0, 0, 0);
    }
    s_is_to_turn_on_ = !s_is_to_turn_on_;
}

static void s_timer_init_(void) {
    ret_code_t err_code;

    err_code = app_timer_create(&invert_leds_timer_id, APP_TIMER_MODE_SINGLE_SHOT, s_invert_leds_handler_);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&change_duty_cycle_timer_id, APP_TIMER_MODE_SINGLE_SHOT,
         s_change_duty_cycle_handler_);
    APP_ERROR_CHECK(err_code);
}

void pwm_init(void) {
    const nrfx_pwm_config_t config = {
        .output_pins = {
            LED_RED, 
            LED_GREEN,
            LED_BLUE,
            NRFX_PWM_PIN_NOT_USED
        },
        .irq_priority = APP_IRQ_PRIORITY_LOWEST,
        .base_clock = NRF_PWM_CLK_1MHz,
        .count_mode = NRF_PWM_MODE_UP,
        .top_value = PWM_TOP,
        .load_mode = NRF_PWM_LOAD_INDIVIDUAL,
        .step_mode = NRF_PWM_STEP_AUTO
    };

    APP_ERROR_CHECK(nrfx_pwm_init(&s_pwm_instance_, &config, NULL));
    s_timer_init_();
    gpio_utils_init();
}

static bool s_is_init_ = false;

void nordic_rgb_pwm_utils_init(nrfx_pwm_t pwm_instance) {
    if (!s_is_init_) {
        s_pwm_instance_ = pwm_instance;
        pwm_init();
        s_is_init_ = true; 
    }
}

void nordic_rgb_pwm_update_duty_cycle(uint8_t duty_red, uint8_t duty_green, uint8_t duty_blue) {
    pmw_update_duty_cycle(duty_red, duty_green, duty_blue);
}

static void s_set_duty_values_(uint8_t r_duty, uint8_t g_duty, uint8_t b_duty) {
    s_remembered_duty_r_ = r_duty;
    s_remembered_duty_g_ = g_duty;
    s_remembered_duty_b_ = b_duty;
    s_current_duty_cycle_r_ = r_duty;
    s_current_duty_cycle_g_ = g_duty;
    s_current_duty_cycle_b_ = b_duty;
}

void nordic_rgb_pwm_set_color(uint8_t r, uint8_t g, uint8_t b) {
    rgb_t rgb_data = converter_get_rgb_data(r, g, b);
    uint8_t r_duty = (uint8_t)ceilf((float)rgb_data.red / 255.f * 100.f);
    uint8_t g_duty = (uint8_t)ceilf((float)rgb_data.green / 255.f * 100.f);
    uint8_t b_duty = (uint8_t)ceilf((float)rgb_data.blue / 255.f * 100.f);
    s_set_duty_values_(r_duty, g_duty, b_duty);
    nordic_rgb_pwm_update_duty_cycle(r_duty, g_duty, b_duty);
}

void nordic_rgb_pwm_set_hsv_color(uint16_t h, uint8_t s, uint8_t v) {
    hsv_t hsv_data = converter_get_hsv_data(h, s, v);
    rgb_t rgb_data = converter_to_rgb_from_hsv(hsv_data);
    nordic_rgb_pwm_set_color(rgb_data.red, rgb_data.green, rgb_data.blue);
}

#define BLINK_DELAY_TIME 500

void nordic_rgb_pwm_blink(void) {
    app_timer_start(invert_leds_timer_id, APP_TIMER_TICKS(BLINK_DELAY_TIME), NULL);
}

void nordic_rgb_pwm_smooth_blink(void) {
    app_timer_start(change_duty_cycle_timer_id, APP_TIMER_TICKS(10), NULL);
}