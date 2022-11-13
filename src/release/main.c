#include "module/log/nordic_usb_logging.h"
#include "module/io/gpio_utils.h"
#include "module/io/led/nordic_rgb_pwm_utils.h"
#include "module/io/button.h"

#define MODE_COUNT 4
#define MODE_DENOTING_LED LED_YELLOW
#define SLOW_DELAY 500
#define FAST_DELAY 200

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
static void _s_mode_led_blink_slow(void) {
    gpio_utils_blink_async(MODE_DENOTING_LED, SLOW_DELAY);
}

static bool _s_is_changing_color;
static hsv_t _s_hsv_data;

static void _s_increment_hsv_data(void) {
    static bool is_rising_h = true;
    static bool is_rising_s = true;
    static bool is_rising_v = true;

    if (_s_current_mode == HUE_MODIFY_MODE) {
        if (is_rising_h) {
            _s_hsv_data.hue++;
            if (_s_hsv_data.hue >= 360) {
                is_rising_h = false;
                _s_hsv_data.hue = 360;
            }
        } else {
            _s_hsv_data.hue--;
            if (_s_hsv_data.hue <= 0) {
                is_rising_h = true;
                _s_hsv_data.hue = 0;
            }
        }
    } else if (_s_current_mode == SATUR_MODIFY_MODE) {
        if (is_rising_s) {
            _s_hsv_data.saturation++;
            if (_s_hsv_data.saturation >= 100) {
                is_rising_s = false;
                _s_hsv_data.saturation = 100;
            }
        } else {
            _s_hsv_data.saturation--;
            if (_s_hsv_data.saturation <= 0) {
                is_rising_s = true;
                _s_hsv_data.saturation = 0;
            }
        }
    } else if (_s_current_mode == VALUE_MODIFY_MODE) {
        if (is_rising_v) {
            _s_hsv_data.value++;
            if (_s_hsv_data.value >= 100) {
                is_rising_v = false;
                _s_hsv_data.value = 100;
            }
        } else {
            _s_hsv_data.value--;
            if (_s_hsv_data.value <= 0) {
                is_rising_v = true;
                _s_hsv_data.value = 0;
            }
        }
    }

    NRF_LOG_INFO("MODE: %d; %d, %d, %d", _s_current_mode, (int)_s_hsv_data.hue, (int)_s_hsv_data.saturation, (int)_s_hsv_data.value);

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
            _s_mode_led_behavior = _s_mode_led_blink_slow;
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
    gpio_utils_led_invert(LED_YELLOW);
}

int main(void) {
    app_timer_init();
    nordic_usb_logging_init();
    gpio_utils_init();
    nordic_rgb_pwm_utils_init();
    button_init();

    button_init_press_check(press_handler);
    button_init_release_check(release_handler);
    button_set_n(2);
    button_init_n_click_check(double_click_handler);

    NRF_LOG_INFO("App start running");

    _s_timer_init();

    _s_current_mode = NO_INPUT_MODE;
    _s_mode_led_behavior = _s_mode_led_turn_off;
    _s_hsv_data = converter_get_hsv_data((float)301, (float)50, (float)50);
    _s_is_changing_color = false;

    nordic_rgb_pwm_set_hsv_color(301, 50, 50);

    while(true) {
        if (_s_is_changing_color) {
            app_timer_start(hsv_change_timer_id, APP_TIMER_TICKS(30), NULL);
        }
        _s_mode_led_behavior();
        LOG_BACKEND_USB_PROCESS();
        NRF_LOG_PROCESS();
    }
}