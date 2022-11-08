#include "button.h"

void(* double_click_handler_func_)(void);
void(* press_handler_func_)(void);
void(* release_handler_func_)(void);

APP_TIMER_DEF(double_click_timer_id);

static int click_;
static int click_count_;
static int prev_state_;
static bool is_timer_;
static bool double_click_toggle_state_;
static bool is_double_click_check_;

static void double_click_handler_(void* p_context) {
    if (click_count_ == 2) {
        double_click_handler_func_();
        double_click_toggle_state_ = !double_click_toggle_state_;
    }
    click_count_ = 0;
    is_timer_ = false;
    click_ = 0;
    prev_state_ = BUTTON_UNDEFINED;
}

static void define_button_state_(void) {
    if (gpio_utils_is_button_pressed()) {
        prev_state_ = BUTTON_WAS_PRESSED;
    } else {
        prev_state_ = BUTTON_WAS_RELEASED;
    }
}

static bool is_clicked() {
    if (prev_state_ == BUTTON_WAS_PRESSED && gpio_utils_is_button_released()) {
        click_ = 1;
    }

    if (gpio_utils_is_button_pressed()) {
        define_button_state_();
    } else {
        define_button_state_();
    }

    if (click_) {
        click_ = 0;
        return true;
    } 
    return false;
}

static void input_button_toggle_handler_(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
    if (is_clicked()) {
        if (!is_timer_ && is_double_click_check_) {
            app_timer_start(double_click_timer_id, APP_TIMER_TICKS(500), NULL);
            is_timer_ = true;
        }
        click_count_++;
        if (click_count_ > 2) {
            click_count_ = 2;
        }
    }

    if (gpio_utils_is_button_pressed()) {
        press_handler_func_();
    } else {
        release_handler_func_();
    }
}

static void init_event_and_timer_(void) {
    ret_code_t err_code;
    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);

    nrf_gpio_cfg_output(LED_YELLOW);
    gpio_utils_turn_off_led(LED_YELLOW);
    nrfx_gpiote_in_config_t in_cfg = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    in_cfg.pull = NRF_GPIO_PIN_PULLUP;

    err_code = nrfx_gpiote_in_init(BUTTON, &in_cfg, input_button_toggle_handler_);
    APP_ERROR_CHECK(err_code);
    
    nrfx_gpiote_in_event_enable(BUTTON, true);

    err_code = app_timer_create(&double_click_timer_id, APP_TIMER_MODE_SINGLE_SHOT, double_click_handler_);
    APP_ERROR_CHECK(err_code);
}

void button_init() {
    init_event_and_timer_();
    button_init_press_check(NULL);
    button_init_release_check(NULL);

    click_ = 0;
    click_count_ = 0;
    prev_state_ = BUTTON_UNDEFINED;
    is_timer_ = false;
    double_click_toggle_state_ = false;
    is_double_click_check_ = false;
    double_click_handler_func_ = NULL;
    press_handler_func_ = NULL;
    release_handler_func_ = NULL;
}

void button_init_double_click_check(void* handler) {
    is_double_click_check_ = true;
    if (handler != NULL) {
        double_click_handler_func_ = handler;
    } 
}

static void default_handler(void) {}

void button_init_press_check(void* handler) {
    if (handler == NULL) {
        press_handler_func_ = default_handler;
    } else {
        press_handler_func_ = handler;
    }
}

void button_init_release_check(void* handler) {
    if (handler == NULL) {
        release_handler_func_ = default_handler;
    } else {
        release_handler_func_ = handler;
    }
}