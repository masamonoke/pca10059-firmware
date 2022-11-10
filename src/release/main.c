//#include "module/io/led.h"
#include "module/io/led_pwm.h"
#include "module/io/button.h"
#include "module/timer/timer_utils.h"

#define COLOR_SEQUENCE "RYRRGGBGRRBB"

void play_lights_sequence(const char* const sequence) {
    size_t color_char_idx = 0;
    while (sequence[color_char_idx] != '\0') {
        switch (sequence[color_char_idx]) {
            case 'R':
                led_pwm_blink(LED_RED);
                break;
            case 'G':
                led_pwm_blink(LED_GREEN);
                break;
            case 'B':
                led_pwm_blink(LED_BLUE);
                break;
            case 'Y':
                led_pwm_blink(LED_YELLOW);
                break;
        }
        color_char_idx++;
    }
}

static bool is_free_blinking_mode_ = false;

void press_handler(void) {
    if (is_free_blinking_mode_) {
        return;
    }
    //led_set_duty_cycling_state(true);
    led_pwm_set_global_duty_cycling_state(true);
}

void release_handler(void) {
    if (is_free_blinking_mode_) {
        return;
    }
    led_pwm_set_global_duty_cycling_state(false);
}

void double_click_handler() {
    is_free_blinking_mode_ = !is_free_blinking_mode_;
    if (is_free_blinking_mode_) {
        led_pwm_set_global_duty_cycling_state(true);
    } else {
        led_pwm_set_global_duty_cycling_state(false);
    }
}

int main(void) {
    timer_utils_init();
    led_pwm_init();
    button_init();
    button_init_press_check(press_handler);
    button_init_release_check(release_handler);
    button_init_double_click_check(double_click_handler);
    led_pwm_set_global_duty_cycling_state(false);

    while (true) {
        play_lights_sequence(COLOR_SEQUENCE);
    }
}