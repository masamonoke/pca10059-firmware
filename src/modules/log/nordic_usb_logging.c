#include "nordic_usb_logging.h"
#include <stdbool.h>
#include <stdint.h>

#include "nordic_common.h"
#include "boards.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrf_log_backend_usb.h"

#include "app_usbd.h"
#include "app_usbd_serial_num.h"

static bool s_is_init_ = false;

void nordic_usb_logging_init(void) {
    if (!s_is_init_) {
        ret_code_t ret = NRF_LOG_INIT(NULL);
        APP_ERROR_CHECK(ret);
        NRF_LOG_DEFAULT_BACKENDS_INIT();
        s_is_init_ = true;
    }
}

void nordic_usb_logging_process(void) {
    LOG_BACKEND_USB_PROCESS();
    NRF_LOG_PROCESS();
}
