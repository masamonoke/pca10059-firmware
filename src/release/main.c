#include "module/log/nordic_usb_logging.h"
#include "module/io/gpio_utils.h"
#include "module/io/led/nordic_rgb_pwm_utils.h"
#include "module/io/led/led_soft_pwm.h"
#include "module/io/button.h"
#include <math.h>
#include "nrf_nvmc.h"
#include "nrfx_nvmc.h"

#include "main_p.h"

void write_hsv_to_nvm(uint32_t, uint32_t, uint32_t, uint32_t);

int main(void) {
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

    _s_timer_init();

    _s_current_mode = NO_INPUT_MODE;
    _s_mode_led_behavior = _s_mode_led_turn_off;

    uint32_t app_data_start_addr = BOOTLOADER_ADDRESS - NRF_DFU_APP_DATA_AREA_SIZE;
    uint32_t* h_data = (uint32_t*) app_data_start_addr;
    uint32_t* s_data = (uint32_t*) (app_data_start_addr + 0x00001000);
    uint32_t* v_data = (uint32_t*) (app_data_start_addr + 0x00002000);

    uint16_t initial_hue;
    uint16_t initial_satur;
    uint16_t initial_value;

    if (*h_data == -1 || *s_data == -1 || *v_data == -1) {
        initial_hue = (uint16_t) ceilf(360.f * LAST_ID_DIGITS / 100.f);
        initial_satur = 100;
        initial_value = 100;
    } else {
        initial_hue = (uint16_t) *h_data;
        initial_satur = (uint16_t) *s_data;
        initial_value = (uint16_t) *v_data;
    }

    _s_hsv_data = converter_get_hsv_data(initial_hue, initial_satur, initial_value);
    _s_is_changing_color = false;

    nordic_rgb_pwm_set_hsv_color(_s_hsv_data.hue, _s_hsv_data.saturation, _s_hsv_data.value);

    while(true) {
        if (_s_is_changing_color) {
            app_timer_start(hsv_change_timer_id, APP_TIMER_TICKS(30), NULL);
        }

        _s_mode_led_behavior();

        if (s_is_nvm_write_time) {
            write_hsv_to_nvm(app_data_start_addr, (uint32_t) _s_hsv_data.hue, (uint32_t) _s_hsv_data.saturation,
                 (uint32_t) _s_hsv_data.value);
            s_is_nvm_write_time = false;
            NRF_LOG_INFO("Values have been written to NVM");
            NRF_LOG_INFO("%d %d %d", *h_data, *s_data, *v_data);
        }


        LOG_BACKEND_USB_PROCESS();
        NRF_LOG_PROCESS();
    }
}

void write_hsv_to_nvm(uint32_t start_addr, uint32_t h, uint32_t s, uint32_t v) {
    nrf_nvmc_page_erase(start_addr);
    if (nrfx_nvmc_word_writable_check(start_addr, h)) {
        nrf_nvmc_write_word(start_addr, h);
    }
    nrf_nvmc_page_erase(start_addr + 0x00001000);
    if (nrfx_nvmc_word_writable_check(start_addr + 0x00001000, s)) {
            nrf_nvmc_write_word(start_addr + 0x00001000, s);
    }

    nrf_nvmc_page_erase(start_addr + 0x00002000);

    if (nrfx_nvmc_word_writable_check(start_addr + 0x00002000, v)) {
        nrf_nvmc_write_word(start_addr + 0x00002000, v);
    }
}