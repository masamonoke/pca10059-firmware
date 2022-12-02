#include "module/log/nordic_usb_logging.h"
#include "module/io/gpio_utils.h"
#include "module/io/led/nordic_rgb_pwm_utils.h"
#include "module/io/led/led_soft_pwm.h"
#include "module/io/button.h"
#include <math.h>
#include "nrfx_nvmc.h"
#include "nrf_log.h"
#include "module/app/hsv_editor.h"
#include "module/app/hsv_editor_nvm.h"

#define LAST_ID_DIGITS 8 //id 6608

int main(void) {
    hsv_editor_init();
    hsv_editor_nvm_init();

    uint16_t initial_hue;
    uint16_t initial_satur;
    uint16_t initial_value;

    uint32_t buf[SAVED_SET_COLOR_SPACE_DATA_COUNT];
    bool is_data_in_nvm = hsv_editor_nvm_is_prev_set_color_saved(buf);
    if (is_data_in_nvm) {
        initial_hue = buf[0];
        initial_satur = buf[1];
        initial_satur = buf[2];
    } else {
        initial_hue = (uint16_t) ceilf(360.f * LAST_ID_DIGITS / 100.f);
        initial_satur = 100;
        initial_value = 100;
    }

    hsv_editor_set_init_hsv(initial_hue, initial_satur, initial_value);

    while(true) {
        hsv_editor_nvm_is_prev_set_color_saved(buf);
        hsv_editor_change_color();
        hsv_editor_process_current_behavior();
        if (hsv_editor_get_is_nvm_write_time()) {
            NRF_LOG_INFO("%d %d %d", buf[0], buf[1], buf[2]);
            hsv_t cur_hsv_obj = hsv_editor_get_hsv_object();
            hsv_editor_nvm_write_hsv(cur_hsv_obj.hue, cur_hsv_obj.saturation, cur_hsv_obj.value);
            hsv_editor_set_is_nvm_write_time(false);
            NRF_LOG_INFO("Saved color %d %d %d", cur_hsv_obj.hue, cur_hsv_obj.saturation, cur_hsv_obj.value);
        }

        nordic_usb_logging_process();
    }
}