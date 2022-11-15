#include "module/log/nordic_usb_logging.h"
#include "module/io/gpio_utils.h"
#include "module/io/led/nordic_rgb_pwm_utils.h"
#include "module/io/led/led_soft_pwm.h"
#include "module/io/button.h"
#include <math.h>

#define MODE_COUNT 4
#define MODE_DENOTING_LED LED_YELLOW
#define FAST_DELAY 200
#define LAST_ID_DIGITS 8 //id 6608

enum {
    NO_INPUT_MODE,
    HUE_MODIFY_MODE,
    SATUR_MODIFY_MODE,
    VALUE_MODIFY_MODE
};

static uint8_t _s_current_mode;

static void(* _s_mode_led_behavior)(void);

static void _s_mode_led_turn_off(void) {
    gpio_utils_turn_off_led(MODE_DENOTING_LED);
}
static void _s_mode_led_turn_on(void) {
    gpio_utils_turn_on_led(MODE_DENOTING_LED);
}
static void _s_mode_led_blink_fast(void) {
    gpio_utils_blink_async(MODE_DENOTING_LED, FAST_DELAY);
}

static void _s_mode_led_pwm_blink(void) {
    led_soft_pwm_blink(MODE_DENOTING_LED);
}

static bool _s_is_changing_color;
static hsv_t _s_hsv_data;

//function for moving component value from 0 to limit and back from limit to zero with step=1
static float _s_inc_component(float component, uint16_t high_limit, bool* is_rising) {
    if (*is_rising) {
        component++;
        if (component >= high_limit) {
            *is_rising = false;
            component = high_limit;
        }
    } else {
        component--;
        if (component <= 0) {
            *is_rising = true;
            component = 0;
        }
    }
    return component;
}

static void _s_increment_hsv_data(void) {
    if (_s_current_mode == NO_INPUT_MODE) {
        return;
    }

    static bool is_rising_h = true;
    static bool is_rising_s = true;
    static bool is_rising_v = true;

    if (_s_current_mode == HUE_MODIFY_MODE) {
        //moving around circle in one direction i.e. when passing 360 cycle starts from 0 not moving back
        if (is_rising_h) {
            _s_hsv_data.hue++;
            if (_s_hsv_data.hue >= 360) {
                is_rising_h = true;
                _s_hsv_data.hue = 0;
            }
        } else {
            _s_hsv_data.hue--;
            if (_s_hsv_data.hue <= 0) {
                is_rising_h = false;
                _s_hsv_data.hue = 360;
            }
        }
    } else if (_s_current_mode == SATUR_MODIFY_MODE) {
        _s_hsv_data.saturation = _s_inc_component(_s_hsv_data.saturation, 100, &is_rising_s);
    } else if (_s_current_mode == VALUE_MODIFY_MODE) {
        _s_hsv_data.value = _s_inc_component(_s_hsv_data.value, 100, &is_rising_v);
    }

    NRF_LOG_INFO("%d, %d, %d", (int)_s_hsv_data.hue, (int)_s_hsv_data.saturation, (int)_s_hsv_data.value);

    nordic_rgb_pwm_set_hsv_color(_s_hsv_data.hue, _s_hsv_data.saturation, _s_hsv_data.value);
}

APP_TIMER_DEF(hsv_change_timer_id);

static void _s_change_hsv_handler(void* p_context) {
    _s_increment_hsv_data();
}

static void _s_timer_init(void) {
    ret_code_t err_code;
    err_code = app_timer_create(&hsv_change_timer_id, APP_TIMER_MODE_SINGLE_SHOT, _s_change_hsv_handler);
    APP_ERROR_CHECK(err_code);
}

void press_handler(void) {
    _s_is_changing_color = true;
}

void release_handler(void) {
    _s_is_changing_color = false;
}

void _s_define_status_led_behavior(void) {
    switch (_s_current_mode) {
        case NO_INPUT_MODE:
            _s_mode_led_behavior = _s_mode_led_turn_off;
            break;
        case HUE_MODIFY_MODE:
            _s_mode_led_behavior = _s_mode_led_pwm_blink;
            break;
        case SATUR_MODIFY_MODE:
            _s_mode_led_behavior = _s_mode_led_blink_fast;
            break;
        case VALUE_MODIFY_MODE:
            _s_mode_led_behavior = _s_mode_led_turn_on;
            break;
        default:
            break;
    }
}

void double_click_handler() {
    if (_s_current_mode == VALUE_MODIFY_MODE) {
        _s_current_mode = NO_INPUT_MODE;
    } else {
        _s_current_mode++;
    }
    _s_define_status_led_behavior();
}

int main(void) {
    app_timer_init();
    nordic_usb_logging_init();
    gpio_utils_init();
    nordic_rgb_pwm_utils_init();
    button_init();
    led_soft_pwm_init();

    button_init_press_check(press_handler);
    button_init_release_check(release_handler);
    button_set_n(2);
    button_init_n_click_check(double_click_handler);

    NRF_LOG_INFO("App start running");

    _s_timer_init();

    _s_current_mode = NO_INPUT_MODE;
    _s_mode_led_behavior = _s_mode_led_turn_off;
    uint16_t initial_hue = (uint16_t) ceilf(360.f * LAST_ID_DIGITS / 100.f);
    _s_hsv_data = converter_get_hsv_data(initial_hue, 100, 100);
    _s_is_changing_color = false;

    nordic_rgb_pwm_set_hsv_color(_s_hsv_data.hue, _s_hsv_data.saturation, _s_hsv_data.value);

    while(true) {
        if (_s_is_changing_color) {
            app_timer_start(hsv_change_timer_id, APP_TIMER_TICKS(30), NULL);
        }

        _s_mode_led_behavior();

        LOG_BACKEND_USB_PROCESS();
        NRF_LOG_PROCESS();
    }
}