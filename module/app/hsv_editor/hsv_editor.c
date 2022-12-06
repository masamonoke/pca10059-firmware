#include "hsv_editor.h"

#include "module/log/nordic_usb_logging.h"
#include "module/io/gpio_utils.h"
#include "module/io/led/nordic_rgb_pwm_utils.h"
#include "module/io/led/led_soft_pwm.h"
#include "module/io/button.h"
#include "module/app/hsv_editor/hsv_editor_rgb_color_storage.h"
#include "module/app/hsv_editor/hsv_editor_nvm.h"

#define MODE_COUNT 4
#define MODE_DENOTING_LED LED_YELLOW
#define FAST_DELAY 200

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

void s_define_status_led_behavior_(void) {
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
bool s_is_edit_ = false;

void double_click_handler() {
    if (s_current_mode_ == VALUE_MODIFY_MODE) {
        s_current_mode_ = NO_INPUT_MODE;
        //if double click happened on the last mode so all values set and can be written to nvm
        s_is_nvm_write_time_ = true;
        s_is_edit_ = false;
    } else {
        s_is_edit_ = true;
        s_current_mode_++;
    }
    s_define_status_led_behavior_();
}

static void s_restore_previous_session_(void) {
    rgb_t restored_colors[COLORS_ENTRY_SIZE];
    char restored_color_names[COLORS_ENTRY_SIZE][COLORS_ENTRY_SIZE];
    uint8_t restored_entries_count;
    hsv_editor_nvm_restore_previous_rgb_storage(restored_colors, restored_color_names, &restored_entries_count);
    NRF_LOG_INFO("Restored entries count %d", restored_entries_count);
    if (restored_entries_count != 0) {
        hsv_editor_rgb_color_storage_set_names(restored_color_names, restored_entries_count);
        hsv_editor_rgb_color_storage_set_colors(restored_colors, restored_entries_count);
        hsv_editor_rgb_color_storage_set_last_free_idx(restored_entries_count);
    }
}

static void init(void) {
    app_timer_init();
    nordic_usb_logging_init();
    gpio_utils_init();
    nrfx_pwm_t pwm_instance = NRFX_PWM_INSTANCE(0);
    nordic_rgb_pwm_utils_init(pwm_instance);
    button_init();
    led_soft_pwm_init();

    button_init_press_check(press_handler);
    button_init_release_check(release_handler);
    button_set_n(2);
    button_init_n_click_check(double_click_handler);

    NRF_LOG_INFO("App start running");

    s_timer_init_();

    s_current_mode_ = NO_INPUT_MODE;
    s_mode_led_behavior_ = s_mode_led_turn_off_;

    s_is_changing_color_ = false;
    
    s_restore_previous_session_();
} 

static bool s_is_init_ = false;
void hsv_editor_init(void) {
    if (!s_is_init_) {
        init();
        hsv_editor_nvm_init();
        s_is_init_ = true;
    }
}

void hsv_editor_process_current_behavior(void) {
    s_mode_led_behavior_();
}

void hsv_editor_change_color(void) {
    if (s_is_changing_color_) {
        app_timer_start(hsv_change_timer_id, APP_TIMER_TICKS(30), NULL);
    }
}

void hsv_editor_set_hsv_object(uint16_t h, uint8_t s, uint8_t v) {
    s_hsv_data_ = converter_get_hsv_data(h, s, v);
    nordic_rgb_pwm_set_hsv_color(s_hsv_data_.hue, s_hsv_data_.saturation, s_hsv_data_.value);
}

bool hsv_editor_get_is_nvm_write_time(void) {
    return s_is_nvm_write_time_;
}

hsv_t hsv_editor_get_hsv_object(void) {
    return s_hsv_data_;
}

void hsv_editor_set_is_nvm_write_time(bool is_write_time) {
    s_is_nvm_write_time_ = is_write_time;
}

bool hsv_editor_is_edit_completed(void) {
    return s_is_edit_ == false;
}