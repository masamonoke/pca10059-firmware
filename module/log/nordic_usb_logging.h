#ifndef MODULE_LOG_NORDIC_USB_LOGGING
#define MODULE_LOG_NORDIC_USB_LOGGING

//adaptation of logger example https://github.com/DSRCorporation/esl-nsdk/blob/35e4f4e9b3663415e1c4e7fd4d1d32a760b5efb4/examples/peripheral/dongle_logs/main.c

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

void nordic_usb_logging_init(void) {
    ret_code_t ret = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(ret);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

#endif /* MODULE_LOG_NORDIC_USB_LOGGING */
