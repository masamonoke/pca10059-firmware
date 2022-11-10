#include "timer_utils.h"

static void lfclk_config(void) {
    ret_code_t err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);
    nrf_drv_clock_lfclk_request(NULL);
}

void timer_utils_init(void) {
    lfclk_config();
    app_timer_init();
}