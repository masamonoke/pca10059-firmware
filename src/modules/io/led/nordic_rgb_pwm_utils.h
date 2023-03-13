#ifndef MODULE_IO_LED_NORDIC_RGB_PWM_UTILS
#define MODULE_IO_LED_NORDIC_RGB_PWM_UTILS

#include "nrfx_pwm.h"
#include "modules/color/converter.h"

void nordic_rgb_pwm_utils_init(nrfx_pwm_t pwm_instance);

void nordic_rgb_pwm_update_duty_cycle(uint8_t duty_red, uint8_t duty_green, uint8_t duty_blue);

void nordic_rgb_pwm_set_color(uint8_t r, uint8_t g, uint8_t b);

void nordic_rgb_pwm_set_hsv_color(uint16_t h, uint8_t s, uint8_t v);

void nordic_rgb_pwm_blink(void);

void nordic_rgb_pwm_smooth_blink(void);

#endif /* MODULE_IO_LED_NORDIC_RGB_PWM_UTILS */
