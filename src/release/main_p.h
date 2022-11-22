#ifndef SRC_RELEASE_MAIN_P
#define SRC_RELEASE_MAIN_P

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

static uint8_t s_current_mode_;

static void(* s_mode_led_behavior_)(void);

static void s_mode_led_turn_off_(void) {
    gpio_utils_turn_off_led(MODE_DENOTING_LED);
}
static void _s_mode_led_turn_on(void) {
    gpio_utils_turn_on_led(MODE_DENOTING_LED);
}
static void s_mode_led_blink_fast_(void) {
    gpio_utils_blink_async(MODE_DENOTING_LED, FAST_DELAY);
}

static void s_mode_led_pwm_blink_(void) {
    led_soft_pwm_blink(MODE_DENOTING_LED);
}

static bool s_is_changing_color_;
static hsv_t s_hsv_data_;

//function for moving component value from 0 to limit and back from limit to zero with step=1
static float s_inc_component_(float component, uint16_t high_limit, bool* is_rising) {
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

static void s_increment_hsv_data_(void) {
    if (s_current_mode_ == NO_INPUT_MODE) {
        return;
    }

    static bool is_rising_h = true;
    static bool is_rising_s = true;
    static bool is_rising_v = true;

    if (s_current_mode_ == HUE_MODIFY_MODE) {
        //moving around circle in one direction i.e. when passing 360 cycle starts from 0 not moving back
        if (is_rising_h) {
            s_hsv_data_.hue++;
            if (s_hsv_data_.hue >= 360) {
                is_rising_h = true;
                s_hsv_data_.hue = 0;
            }
        } else {
            s_hsv_data_.hue--;
            if (s_hsv_data_.hue <= 0) {
                is_rising_h = false;
                s_hsv_data_.hue = 360;
            }
        }
    } else if (s_current_mode_ == SATUR_MODIFY_MODE) {
        s_hsv_data_.saturation = s_inc_component_(s_hsv_data_.saturation, 100, &is_rising_s);
    } else if (s_current_mode_ == VALUE_MODIFY_MODE) {
        s_hsv_data_.value = s_inc_component_(s_hsv_data_.value, 100, &is_rising_v);
    }

    nordic_rgb_pwm_set_hsv_color(s_hsv_data_.hue, s_hsv_data_.saturation, s_hsv_data_.value);
}

APP_TIMER_DEF(hsv_change_timer_id);

static void s_change_hsv_handler_(void* p_context) {
    s_increment_hsv_data_();
}

static void s_timer_init_(void) {
    ret_code_t err_code;
    err_code = app_timer_create(&hsv_change_timer_id, APP_TIMER_MODE_SINGLE_SHOT, s_change_hsv_handler_);
    APP_ERROR_CHECK(err_code);
}

void press_handler(void) {
    s_is_changing_color_ = true;
}

void release_handler(void) {
    s_is_changing_color_ = false;
}

void _s_define_status_led_behavior(void) {
    switch (s_current_mode_) {
        case NO_INPUT_MODE:
            s_mode_led_behavior_ = s_mode_led_turn_off_;
            break;
        case HUE_MODIFY_MODE:
            s_mode_led_behavior_ = s_mode_led_pwm_blink_;
            break;
        case SATUR_MODIFY_MODE:
            s_mode_led_behavior_ = s_mode_led_blink_fast_;
            break;
        case VALUE_MODIFY_MODE:
            s_mode_led_behavior_ = _s_mode_led_turn_on;
            break;
        default:
            break;
    }
}

bool s_is_nvm_write_time_ = false;

void double_click_handler() {
    if (s_current_mode_ == VALUE_MODIFY_MODE) {
        s_current_mode_ = NO_INPUT_MODE;
        //if double click happened on the last mode so all values set and can be written to nvm
        s_is_nvm_write_time_ = true;
    } else {
        s_current_mode_++;
    }
    _s_define_status_led_behavior();
}

#endif /* SRC_RELEASE_MAIN_P */
