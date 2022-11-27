#include "module/log/nordic_usb_logging.h"
#include "module/cli/usb/usb_cli.h"
#include "module/io/led/nordic_rgb_pwm_utils.h"



int main(void) {
    nordic_usb_logging_init();
    usb_cli_init();
    nrfx_pwm_t pwm_instance = NRFX_PWM_INSTANCE(0);
    nordic_rgb_pwm_utils_init(pwm_instance);


    while (true) {
        usb_cli_process();
        LOG_BACKEND_USB_PROCESS();
        NRF_LOG_PROCESS();
    }
}
