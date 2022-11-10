#include "button.h"

#define DEBOUNCING_TIME 10
#define DOUBLE_CLICK_TIME 500
#define CLICK_COUNT 3

void(* double_click_handler_func_)(void);
void(* press_handler_func_)(void);
void(* release_handler_func_)(void);

APP_TIMER_DEF(n_click_timer_id);
APP_TIMER_DEF(debouncing_timer_id);

static int _s_click;
static int _s_click_count;
static int _s_prev_state;
static bool _s_is_timer;
static bool _s_double_click_toggle_state;
static bool _s_is_double_click_check;

static void _s_n_click_handler(void* p_context) {
    if (_s_click_count == CLICK_COUNT) {
        double_click_handler_func_();
        _s_double_click_toggle_state = !_s_double_click_toggle_state;
    }
    _s_click_count = 0;
    _s_is_timer = false;
    _s_click = 0;
    _s_prev_state = BUTTON_UNDEFINED;
}

static bool _s_is_defined_button_state;

static void _s_debouncing_handler(void* context) {
    _s_is_defined_button_state = true;
}

static void _s_define_button_state(void) {
    if (gpio_utils_is_button_pressed()) {
        _s_prev_state = BUTTON_WAS_PRESSED;
    } else {
        _s_prev_state = BUTTON_WAS_RELEASED;
    }
}

static bool _s_is_clicked() {
    if (_s_prev_state == BUTTON_WAS_PRESSED && gpio_utils_is_button_released()) {
        _s_click = 1;
    }

    if (gpio_utils_is_button_pressed()) {
        _s_define_button_state();
    } else {
        _s_define_button_state();
    }

    if (_s_click) {
        _s_click = 0;
        return true;
    } 
    return false;
}

static void _s_input_button_toggle_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
    app_timer_start(debouncing_timer_id, APP_TIMER_TICKS(DEBOUNCING_TIME), NULL);
    
    if (!_s_is_defined_button_state) {
        return;
    }

    if (_s_is_clicked()) {
        if (!_s_is_timer && _s_is_double_click_check) {
            app_timer_start(n_click_timer_id, APP_TIMER_TICKS(DOUBLE_CLICK_TIME), NULL);
            _s_is_timer = true;
        }
        _s_click_count++;
        if (_s_click_count > CLICK_COUNT) {
            _s_click_count = CLICK_COUNT;
        }
    }

    if (gpio_utils_is_button_pressed()) {
        press_handler_func_();
    } else {
        release_handler_func_();
    }

    _s_is_defined_button_state = false;
}

static void _s_init_event_and_timer(void) {
    ret_code_t err_code;
    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);

    nrf_gpio_cfg_output(LED_YELLOW);
    gpio_utils_turn_off_led(LED_YELLOW);
    nrfx_gpiote_in_config_t in_cfg = GPIOTE_CONFIG_IN_SENSE_TOGGLE(false);
    in_cfg.pull = NRF_GPIO_PIN_PULLUP;

    err_code = nrfx_gpiote_in_init(BUTTON, &in_cfg, _s_input_button_toggle_handler);
    APP_ERROR_CHECK(err_code);
    
    nrfx_gpiote_in_event_enable(BUTTON, true);

    err_code = app_timer_create(&n_click_timer_id, APP_TIMER_MODE_SINGLE_SHOT, _s_n_click_handler);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&debouncing_timer_id, APP_TIMER_MODE_SINGLE_SHOT, _s_debouncing_handler);
    APP_ERROR_CHECK(err_code);
}

void button_init() {
    _s_init_event_and_timer();
    button_init_press_check(NULL);
    button_init_release_check(NULL);

    _s_click = 0;
    _s_click_count = 0;
    _s_prev_state = BUTTON_UNDEFINED;
    _s_is_timer = false;
    _s_double_click_toggle_state = false;
    _s_is_double_click_check = false;
    double_click_handler_func_ = NULL;
    press_handler_func_ = NULL;
    release_handler_func_ = NULL;
    //initially button is in defiend state that is not pressed
    _s_is_defined_button_state = true;
}

void button_init_double_click_check(void* handler) {
    _s_is_double_click_check = true;
    if (handler != NULL) {
        double_click_handler_func_ = handler;
    } 
}

static void _s_default_handler(void) {}

void button_init_press_check(void* handler) {
    if (handler == NULL) {
        press_handler_func_ = _s_default_handler;
    } else {
        press_handler_func_ = handler;
    }
}

void button_init_release_check(void* handler) {
    if (handler == NULL) {
        release_handler_func_ = _s_default_handler;
    } else {
        release_handler_func_ = handler;
    }
}