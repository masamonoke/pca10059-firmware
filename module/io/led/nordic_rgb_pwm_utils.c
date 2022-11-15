#include "nordic_rgb_pwm_utils.h"
#include "nrfx_pwm.h"
#include "app_timer.h"
#include "module/io/gpio_utils.h"
#include "module/color/converter.h"
#include <math.h>

#define PWM_TOP 100

static nrfx_pwm_t _s_pwm_instance = NRFX_PWM_INSTANCE(0);
static nrf_pwm_values_individual_t _s_seq_values[] = {{ 0, 0, 0, 0 }};

const nrf_pwm_sequence_t seq = {
    .values.p_individual = _s_seq_values,
    .length = NRF_PWM_VALUES_LENGTH(_s_seq_values),
    .repeats = 0,
    .end_delay = 0
};

static void pmw_update_duty_cycle(uint8_t duty_red, uint8_t duty_green, uint8_t duty_blue) {
    if (duty_red >= 100) {
        _s_seq_values->channel_0 = 100;
    } else {
        _s_seq_values->channel_0 = duty_red;
    }
    if (duty_green >= 100) {
        _s_seq_values->channel_1 = 100;
    } else {
        _s_seq_values->channel_1 = duty_green;
    }
    if (duty_blue >= 100) {
        _s_seq_values->channel_2 = 100;
    } else {
        _s_seq_values->channel_2 = duty_blue;
    }

    nrfx_pwm_simple_playback(&_s_pwm_instance, &seq, 1, NRFX_PWM_FLAG_LOOP);
}

APP_TIMER_DEF(change_duty_cycle_timer_id);

#define DUTY_CYCLE_STEP 1

static uint8_t _s_current_duty_cycle_r;
static uint8_t _s_current_duty_cycle_g;
static uint8_t _s_current_duty_cycle_b;
static uint8_t _s_current_duty_cycle_step = DUTY_CYCLE_STEP;

void change_duty_cycle_handler(void* p_context) {
    _s_current_duty_cycle_r += _s_current_duty_cycle_step;
    _s_current_duty_cycle_g += _s_current_duty_cycle_step;
    _s_current_duty_cycle_b += _s_current_duty_cycle_step;
    if (_s_current_duty_cycle_r  >= 100 || 
            _s_current_duty_cycle_g  >= 100 || _s_current_duty_cycle_b  >= 100) {

        _s_current_duty_cycle_step = -_s_current_duty_cycle_step;

    } else if (_s_current_duty_cycle_r  <= 0 || 
            _s_current_duty_cycle_g  <= 0 || _s_current_duty_cycle_b  <= 0) {

        _s_current_duty_cycle_step = -_s_current_duty_cycle_step;

    }
    
    nordic_rgb_pwm_update_duty_cycle(_s_current_duty_cycle_r, _s_current_duty_cycle_g,
         _s_current_duty_cycle_b);
}

APP_TIMER_DEF(invert_leds_timer_id);

static uint8_t _s_rembebered_duty_r;
static uint8_t _s_rembebered_duty_g;
static uint8_t _s_rembebered_duty_b;
static bool _s_is_to_turn_on = false;

 void invert_leds_handler(void* p_context) {
    if (_s_is_to_turn_on) {
        nordic_rgb_pwm_update_duty_cycle(_s_rembebered_duty_r,
             _s_rembebered_duty_g, _s_rembebered_duty_b);
    } else {
        nordic_rgb_pwm_update_duty_cycle(0, 0, 0);
    }
    _s_is_to_turn_on = !_s_is_to_turn_on;
}

static void _s_timer_init(void) {
    ret_code_t err_code;

    err_code = app_timer_create(&invert_leds_timer_id, APP_TIMER_MODE_SINGLE_SHOT, invert_leds_handler);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&change_duty_cycle_timer_id, APP_TIMER_MODE_SINGLE_SHOT,
         change_duty_cycle_handler);
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

    APP_ERROR_CHECK(nrfx_pwm_init(&_s_pwm_instance, &config, NULL));
    _s_timer_init();
    gpio_utils_init();
}

void nordic_rgb_pwm_utils_init(void) {
    pwm_init();
}

void nordic_rgb_pwm_update_duty_cycle(uint8_t duty_red, uint8_t duty_green, uint8_t duty_blue) {
    pmw_update_duty_cycle(duty_red, duty_green, duty_blue);
}

static void _s_set_duty_values(uint8_t r_duty, uint8_t g_duty, uint8_t b_duty) {
    _s_rembebered_duty_r = r_duty;
    _s_rembebered_duty_g = g_duty;
    _s_rembebered_duty_b = b_duty;
    _s_current_duty_cycle_r = r_duty;
    _s_current_duty_cycle_g = g_duty;
    _s_current_duty_cycle_b = b_duty;
}

void nordic_rgb_pwm_set_color(uint8_t r, uint8_t g, uint8_t b) {
    rgb_t rgb_data = converter_get_rgb_data(r, g, b);
    uint8_t r_duty = (uint8_t)ceilf((float)rgb_data.red / 255.f * 100.f);
    uint8_t g_duty = (uint8_t)ceilf((float)rgb_data.green / 255.f * 100.f);
    uint8_t b_duty = (uint8_t)ceilf((float)rgb_data.blue / 255.f * 100.f);
    _s_set_duty_values(r_duty, g_duty, b_duty);
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