#include <math.h>

#include "nrf_log.h"

#include "modules/memory/fstorage_utils.h"
#include "modules/ble/ble_service.h"
#include "modules/io/gpio_utils.h"
#include "modules/ble/ble_app.h"
#include "modules/app/hsv_editor/hsv_editor.h"
#include "modules/app/hsv_editor/cli/usb/usb_cli.h"
#include "modules/io/led/led_soft_pwm.h"
#include "modules/io/button.h"
#include "modules/app/hsv_editor/hsv_editor_nvm.h"
#include "modules/error/runtime_error_impl.h"

#define LAST_ID_DIGITS 8 //id 6608

static void s_dummy_func_(void) {}

int main(void) {
	hsv_editor_nvm_set_nvm_enabled(false);
	hsv_editor_init(); 

	fstorage_utils_init();
	hsv_t data;
	bool is_data_in_nvm = fstorage_utils_read_hsv(&data);
	if (!is_data_in_nvm) {
		data.hue = (uint16_t) ceilf(360.f * LAST_ID_DIGITS / 100.f);
		data.saturation = 100;
		data.value = 100;
	}

	hsv_editor_set_hsv_object(data.hue, data.saturation, data.value);

	void (*usb_proceed)(void) = s_dummy_func_;
#ifdef ESTC_USB_CLI_ENABLED
    if (ESTC_USB_CLI_ENABLED) {
        usb_cli_init();
        usb_proceed = usb_cli_process;
    } else {
        usb_proceed = s_dummy_func_;    
    }
#endif	

	ble_app_set_color_char_data(data.hue, data.saturation, data.value);
	ble_app_init();

	while (true) {
		hsv_editor_change_color();
        hsv_editor_process_current_behavior();

		if (ble_app_is_color_char_updated()) {
            hsv_t* hsv_obj = ble_app_get_color_data();
			hsv_editor_set_hsv_object(hsv_obj->hue, hsv_obj->saturation, hsv_obj->value);
			hsv_editor_set_is_nvm_write_time(true);
		}

        if (hsv_editor_get_is_nvm_write_time()) {
            hsv_t* hsv_obj = hsv_editor_get_hsv_object();

			if (ble_app_is_color_char_updated()) {
				ble_app_set_is_color_char_updated(false);
			} else {
				ble_app_set_color_char_data(hsv_obj->hue, hsv_obj->saturation, hsv_obj->value);
			}

			fstorage_utils_write_hsv(*hsv_obj);
            hsv_editor_set_is_nvm_write_time(false);
            NRF_LOG_INFO("Saved HSV color to nvm: %d %d %d",
				hsv_obj->hue, hsv_obj->saturation, hsv_obj->value);
        }

        usb_proceed();

        if (runtime_error_is_any_error()) {
            runtime_error_log_stacktrace();
        }

		ble_app_idle_state_handle();
	}
}
