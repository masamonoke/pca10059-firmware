#include "button.h"

#define DEBOUNCING_TIME 10
#define DOUBLE_CLICK_TIME 500
#define CLICK_COUNT 2

void(* double_click_handler_func_)(void);
void(* press_handler_func_)(void);
void(* release_handler_func_)(void);

APP_TIMER_DEF(n_click_timer_id);
APP_TIMER_DEF(debouncing_timer_id);

static int s_click_;
static int s_click_count_;
static int s_prev_state_;
static bool s_is_timer_;
static bool s_double_click_toggle_state_;
static bool s_is_double_click_check_;

static void s_n_click_handler_(void* p_context) {
    if (s_click_count_ == CLICK_COUNT) {
        double_click_handler_func_();
        s_double_click_toggle_state_ = !s_double_click_toggle_state_;
    }
    s_click_count_ = 0;
    s_is_timer_ = false;
    s_click_ = 0;
    s_prev_state_ = BUTTON_UNDEFINED;
}

static bool s_is_defined_button_state_;

static void s_debouncing_handler_(void* context) {
    s_is_defined_button_state_ = true;
}

static void s_define_button_state_(void) {
    if (gpio_utils_is_button_pressed()) {
        s_prev_state_ = BUTTON_WAS_PRESSED;
    } else {
        s_prev_state_ = BUTTON_WAS_RELEASED;
    }
}

static bool s_is_clicked_() {
    if (s_prev_state_ == BUTTON_WAS_PRESSED && gpio_utils_is_button_released()) {
        s_click_ = 1;
    }

    if (gpio_utils_is_button_pressed()) {
        s_define_button_state_();
    } else {
        s_define_button_state_();
    }

    if (s_click_) {
        s_click_ = 0;
        return true;
    } 
    return false;
}

static void s_input_button_toggle_handler_(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
    app_timer_start(debouncing_timer_id, APP_TIMER_TICKS(DEBOUNCING_TIME), NULL);
    
    if (!s_is_defined_button_state_) {
        return;
    }

    if (s_is_clicked_()) {
        if (!s_is_timer_ && s_is_double_click_check_) {
            app_timer_start(n_click_timer_id, APP_TIMER_TICKS(DOUBLE_CLICK_TIME), NULL);
            s_is_timer_ = true;
        }
        s_click_count_++;
        if (s_click_count_ > CLICK_COUNT) {
            s_click_count_ = CLICK_COUNT;
        }
    }

    if (gpio_utils_is_button_pressed()) {
        press_handler_func_();
    } else {
        release_handler_func_();
    }

    s_is_defined_button_state_ = false;
}

static void s_init_event_and_timer_(void) {
    ret_code_t err_code;
    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);

    nrf_gpio_cfg_output(LED_YELLOW);
    gpio_utils_turn_off_led(LED_YELLOW);
    nrfx_gpiote_in_config_t in_cfg = GPIOTE_CONFIG_IN_SENSE_TOGGLE(false);
    in_cfg.pull = NRF_GPIO_PIN_PULLUP;

    err_code = nrfx_gpiote_in_init(BUTTON, &in_cfg, s_input_button_toggle_handler_);
    APP_ERROR_CHECK(err_code);
    
    nrfx_gpiote_in_event_enable(BUTTON, true);

    err_code = app_timer_create(&n_click_timer_id, APP_TIMER_MODE_SINGLE_SHOT, s_n_click_handler_);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&debouncing_timer_id, APP_TIMER_MODE_SINGLE_SHOT, s_debouncing_handler_);
    APP_ERROR_CHECK(err_code);
}

void button_init() {
    s_init_event_and_timer_();
    button_init_press_check(NULL);
    button_init_release_check(NULL);

    s_click_ = 0;
    s_click_count_ = 0;
    s_prev_state_ = BUTTON_UNDEFINED;
    s_is_timer_ = false;
    s_double_click_toggle_state_ = false;
    s_is_double_click_check_ = false;
    double_click_handler_func_ = NULL;
    press_handler_func_ = NULL;
    release_handler_func_ = NULL;
    //initially button is in defined state that is not pressed
    s_is_defined_button_state_ = true;
}

void button_init_n_click_check(void* handler) {
    s_is_double_click_check_ = true;
    if (handler != NULL) {
        double_click_handler_func_ = handler;
    } 
}

static void s_default_handler_(void) {}

void button_init_press_check(void* handler) {
    if (handler == NULL) {
        press_handler_func_ = s_default_handler_;
    } else {
        press_handler_func_ = handler;
    }
}

void button_init_release_check(void* handler) {
    if (handler == NULL) {
        release_handler_func_ = s_default_handler_;
    } else {
        release_handler_func_ = handler;
    }
}

void button_set_n(uint8_t click_count) {
    #undef CLICK_COUNT
    #define CLICK_COUNT (click_count);
}