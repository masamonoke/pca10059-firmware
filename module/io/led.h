#ifndef MODULE_IO_LED
#define MODULE_IO_LED

#include "gpio_utils.h"

#include "app_timer.h"

void init(void);

void led_init(void);
void led_blink_pwm(uint32_t led_id);
void led_blink(uint32_t led_id);
void led_set_duty_cycling_state(bool flag);
void led_set_duty_cycle_step(float step);
void led_set_pwm_frequency(uint32_t frequency);

#endif /* MODULE_IO_LED */
