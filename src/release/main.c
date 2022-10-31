#include "nrf_delay.h"
#include "boards.h"
#include "bsp.h"
#include "nrf_gpio.h"
#include "../../module/utils/output_utils.h"

#define BUTTON NRF_GPIO_PIN_MAP(1, 6)
#define PIN NRF_GPIO_PIN_MAP(0, 6)


int main(void) {
    bsp_board_init(BSP_INIT_LEDS);
    nrf_gpio_cfg_output(PIN);
    nrf_gpio_pin_write(PIN, 0);
    while (true) {
        
    }
}
