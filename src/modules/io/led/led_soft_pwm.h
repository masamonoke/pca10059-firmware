#ifndef MODULE_IO_LED_LED_SOFT_PWM
#define MODULE_IO_LED_LED_SOFT_PWM

#include "modules/io/gpio_utils.h"
#include "app_timer.h"

//TODO: add getters and setters

typedef struct led_data_t led_data_t;

void led_soft_pwm_init(void);

void led_soft_pwm_blink(uint32_t led_id);

void led_soft_pwm_set_duty_cycling_state(bool flag);

void led_soft_pwm_set_duty_cycle_step(float step);

void led_soft_pwm_set_pwm_frequency_hz(uint32_t frequency);

void led_soft_pwm_set_global_duty_cycling_state(bool f);

#endif /* MODULE_IO_LED_LED_SOFT_PWM */
