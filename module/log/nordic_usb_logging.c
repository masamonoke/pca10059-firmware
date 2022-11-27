#include "nordic_usb_logging.h"

static bool s_is_init_ = false;

void nordic_usb_logging_init(void) {
    if (!s_is_init_) {
        ret_code_t ret = NRF_LOG_INIT(NULL);
        APP_ERROR_CHECK(ret);
        NRF_LOG_DEFAULT_BACKENDS_INIT();
        s_is_init_ = true;
    }
}