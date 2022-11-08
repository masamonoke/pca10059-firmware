#ifndef MODULE_IO_BUTTON
#define MODULE_IO_BUTTON

#include "gpio_utils.h"

#include "app_timer.h"
#include "drv_rtc.h"
#include "nrf_drv_clock.h"
#include "nrf_drv_gpiote.h"
#include "app_error.h"

typedef enum {
    BUTTON_WAS_PRESSED,
    BUTTON_WAS_RELEASED,
    BUTTON_WAS_CLICKED,
    BUTTON_UNDEFINED
} button_state_t;

void button_init();
void button_init_double_click_check(void* handler);
void button_init_press_check(void* handler);
void button_init_release_check(void* handler);
void button_set_handler(void* handler);

#endif /* MODULE_IO_BUTTON */
