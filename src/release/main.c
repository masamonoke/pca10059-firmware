#include "module/log/nordic_usb_logging.h"
#include "module/io/gpio_utils.h"
#include "module/io/led/nordic_rgb_pwm_utils.h"
#include "module/io/led/led_soft_pwm.h"
#include "module/io/button.h"
#include <math.h>
#include "nrf_nvmc.h"
#include "nrfx_nvmc.h"

#include "main_p.h"

#define PAGE_SIZE 0x00001000
#define DATA_COUNT 3

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

    s_timer_init_();

    s_current_mode_ = NO_INPUT_MODE;
    s_mode_led_behavior_ = s_mode_led_turn_off_;

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

    s_hsv_data_ = converter_get_hsv_data(initial_hue, initial_satur, initial_value);
    s_is_changing_color_ = false;

    nordic_rgb_pwm_set_hsv_color(s_hsv_data_.hue, s_hsv_data_.saturation, s_hsv_data_.value);

    while(true) {
        if (s_is_changing_color_) {
            app_timer_start(hsv_change_timer_id, APP_TIMER_TICKS(30), NULL);
        }

        s_mode_led_behavior_();

        if (s_is_nvm_write_time_) {
            write_hsv_to_nvm(app_data_start_addr, (uint32_t) s_hsv_data_.hue, (uint32_t) s_hsv_data_.saturation,
                 (uint32_t) s_hsv_data_.value);
            s_is_nvm_write_time_ = false;
            NRF_LOG_INFO("Values have been written to NVM");
            NRF_LOG_INFO("%d %d %d", *h_data, *s_data, *v_data);
        }


        LOG_BACKEND_USB_PROCESS();
        NRF_LOG_PROCESS();
    }
}

void write_hsv_to_nvm(uint32_t start_addr, uint32_t h, uint32_t s, uint32_t v) {
    uint32_t addr = start_addr;
    uint32_t values[] = { h, s, v };
    for (uint32_t page = 0; page < DATA_COUNT; page++) {
        addr = start_addr + PAGE_SIZE * page;
        nrf_nvmc_page_erase(addr);
        if (nrfx_nvmc_word_writable_check(addr, values[page])) {
            nrf_nvmc_write_word(addr, values[page]);
            if (nrfx_nvmc_write_done_check()) {
                NRF_LOG_INFO("Value is saved");
            }
        }
    }
}