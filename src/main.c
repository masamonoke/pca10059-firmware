#include "modules/log/nordic_usb_logging.h"
#include "modules/io/gpio_utils.h"
#include "modules/io/led/nordic_rgb_pwm_utils.h"
#include "modules/io/led/led_soft_pwm.h"
#include "modules/io/button.h"
#include "modules/app/hsv_editor/hsv_editor.h"
#include "modules/app/hsv_editor/hsv_editor_nvm.h"
#include "modules/app/hsv_editor/cli/usb/usb_cli.h"
#include "modules/error/runtime_error_impl.h"

#include <math.h>
#include "nrf_log.h"

#define LAST_ID_DIGITS 8 //id 6608

static void s_dummy_func_(void) {}

int main(void) {
    hsv_editor_init();

    void (*usb_proceed)(void) = s_dummy_func_;
#ifdef ESTC_USB_CLI_ENABLED
    if (ESTC_USB_CLI_ENABLED) {
        usb_cli_init();
        usb_proceed = usb_cli_process;
    } else {
        usb_proceed = s_dummy_func_;    
    }
#endif

    uint16_t initial_hue;
    uint16_t initial_satur;
    uint16_t initial_value;

    uint32_t buf[SAVED_SET_COLOR_SPACE_DATA_COUNT];
    bool is_data_in_nvm = hsv_editor_nvm_is_prev_set_color_saved(buf);
    if (is_data_in_nvm) {
        initial_hue = buf[0];
        initial_satur = buf[1];
        initial_value = buf[2];
        NRF_LOG_INFO("Restored previous set color %d %d %d", initial_hue, initial_satur, initial_value);
    } else {
        initial_hue = (uint16_t) ceilf(360.f * LAST_ID_DIGITS / 100.f);
        initial_satur = 100;
        initial_value = 100;
    }

    hsv_editor_set_hsv_object(initial_hue, initial_satur, initial_value);

    while(true) {
        hsv_editor_change_color();
        hsv_editor_process_current_behavior();

        if (hsv_editor_get_is_nvm_write_time()) {
            hsv_t cur_hsv_obj = hsv_editor_get_hsv_object();
            hsv_editor_nvm_write_hsv(cur_hsv_obj.hue, cur_hsv_obj.saturation, cur_hsv_obj.value);
            hsv_editor_set_is_nvm_write_time(false);
            NRF_LOG_INFO("Saved HSV color to nvm: %d %d %d", cur_hsv_obj.hue, cur_hsv_obj.saturation, cur_hsv_obj.value);
        }

        usb_proceed();

        if (runtime_error_is_any_error()) {
            runtime_error_log_stacktrace();
        }

        nordic_usb_logging_process();
    }
}
