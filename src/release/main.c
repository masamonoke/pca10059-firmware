#include "module/log/nordic_usb_logging.h"
#include "module/cli/usb/usb_cli.h"
#include "module/io/led/nordic_rgb_pwm_utils.h"

static void s_cycle_with_com_acm_(void) {
    usb_cli_process();
    LOG_BACKEND_USB_PROCESS();
    NRF_LOG_PROCESS();
}

static void s_cycle_without_com_acm_(void) {
    LOG_BACKEND_USB_PROCESS();
    NRF_LOG_PROCESS();
}

int main(void) {
    nordic_usb_logging_init();
    void (*func)(void);
//to enable usb cli make clean && make dfu ESTC_USB_CLI_ENABLED=1 or 0 otherwise
//ESTC_USB_CLI_ENABLED=1 by default
#ifdef ESTC_USB_CLI_ENABLED
    if (ESTC_USB_CLI_ENABLED) {
        usb_cli_init();
        nrfx_pwm_t pwm_instance = NRFX_PWM_INSTANCE(0);
        nordic_rgb_pwm_utils_init(pwm_instance);
        func = s_cycle_with_com_acm_;
    } else {
        func = s_cycle_without_com_acm_;    
    }
#else
    func = s_cycle_without_com_acm_;
#endif


    while (true) {
        func();
    }
}
