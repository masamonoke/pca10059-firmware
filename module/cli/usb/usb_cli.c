#include "usb_cli.h"
#include "module/io/gpio_utils.h"
#include "nrf_log.h"
#include "../cli.h"

#define READ_SIZE 1

#define CDC_ACM_COMM_INTERFACE  2
#define CDC_ACM_COMM_EPIN       NRF_DRV_USBD_EPIN3

#define CDC_ACM_DATA_INTERFACE  3
#define CDC_ACM_DATA_EPIN       NRF_DRV_USBD_EPIN4
#define CDC_ACM_DATA_EPOUT      NRF_DRV_USBD_EPOUT4

static void usb_ev_handler(app_usbd_class_inst_t const * p_inst, app_usbd_cdc_acm_user_event_t event);

APP_USBD_CDC_ACM_GLOBAL_DEF(usb_cdc_acm,
                            usb_ev_handler,
                            CDC_ACM_COMM_INTERFACE,
                            CDC_ACM_DATA_INTERFACE,
                            CDC_ACM_COMM_EPIN,
                            CDC_ACM_DATA_EPIN,
                            CDC_ACM_DATA_EPOUT,
                            APP_USBD_CDC_COMM_PROTOCOL_NONE);

static char m_rx_buffer[READ_SIZE];
static bool s_is_init_ = false;

void usb_cli_init(app_usbd_cdc_acm_t instance) {
    if (!s_is_init_) {
        gpio_utils_init();
        s_is_init_ = true;
        app_usbd_class_inst_t const* class_cdc_acm = app_usbd_cdc_acm_class_inst_get(&usb_cdc_acm);
        ret_code_t ret = app_usbd_class_append(class_cdc_acm);
        APP_ERROR_CHECK(ret);
    }
}

void usb_cli_process(void) {
    while (app_usbd_event_queue_process()) {
    }
}

static char buff[40];
static uint8_t i = 0;

static void s_clean_buffer_(void) {
    for (size_t k = 0; k < 40; k++) {
        buff[k] = '\0';
    }
    i = 0;
}

static void usb_ev_handler(app_usbd_class_inst_t const * p_inst, app_usbd_cdc_acm_user_event_t event) {
    switch (event) {
    case APP_USBD_CDC_ACM_USER_EVT_PORT_OPEN: {
        ret_code_t ret;
        gpio_utils_turn_on_led(LED_YELLOW);
        ret = app_usbd_cdc_acm_read(&usb_cdc_acm, m_rx_buffer, READ_SIZE);
        UNUSED_VARIABLE(ret);
        break;
    }
    case APP_USBD_CDC_ACM_USER_EVT_PORT_CLOSE: {
        gpio_utils_turn_off_led(LED_YELLOW);
        break;
    }
    case APP_USBD_CDC_ACM_USER_EVT_TX_DONE: {
        break;
    }
    case APP_USBD_CDC_ACM_USER_EVT_RX_DONE: {
        ret_code_t ret;
        buff[i] = m_rx_buffer[0];
        i++;
        do {
            if (m_rx_buffer[0] == '\r' || m_rx_buffer[0] == '\n') {
                buff[i - 1] = '\0';
                cli_proceed(buff);
                s_clean_buffer_();
                ret = app_usbd_cdc_acm_write(&usb_cdc_acm, "\r\n", 2);
            } else {
                ret = app_usbd_cdc_acm_write(&usb_cdc_acm, m_rx_buffer, READ_SIZE);
            }

            ret = app_usbd_cdc_acm_read(&usb_cdc_acm, m_rx_buffer, READ_SIZE);
        } while (ret == NRF_SUCCESS);

        break;
    }
    default:
        break;
    }
}