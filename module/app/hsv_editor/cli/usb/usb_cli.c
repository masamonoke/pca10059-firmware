#include "usb_cli.h"
#include "module/io/gpio_utils.h"
#include "nrf_log.h"
#include "../cli.h"
#include "module/app/hsv_editor/hsv_editor.h"

#define READ_SIZE 1

#define CDC_ACM_COMM_INTERFACE  2
#define CDC_ACM_COMM_EPIN       NRF_DRV_USBD_EPIN3

#define CDC_ACM_DATA_INTERFACE  3
#define CDC_ACM_DATA_EPIN       NRF_DRV_USBD_EPIN4
#define CDC_ACM_DATA_EPOUT      NRF_DRV_USBD_EPOUT4

#define BUFF_SIZE 40

static void usb_ev_handler(app_usbd_class_inst_t const * p_inst, app_usbd_cdc_acm_user_event_t event);

APP_USBD_CDC_ACM_GLOBAL_DEF(usb_cdc_acm,
                            usb_ev_handler,
                            CDC_ACM_COMM_INTERFACE,
                            CDC_ACM_DATA_INTERFACE,
                            CDC_ACM_COMM_EPIN,
                            CDC_ACM_DATA_EPIN,
                            CDC_ACM_DATA_EPOUT,
                            APP_USBD_CDC_COMM_PROTOCOL_NONE);

static char s_rx_buffer_[READ_SIZE];
static bool s_is_init_ = false;

void usb_cli_init(app_usbd_cdc_acm_t instance) {
    if (!s_is_init_) {
        s_is_init_ = true;
        app_usbd_class_inst_t const* class_cdc_acm = app_usbd_cdc_acm_class_inst_get(&usb_cdc_acm);
        ret_code_t ret = app_usbd_class_append(class_cdc_acm);
        APP_ERROR_CHECK(ret);
    }
}

void usb_cli_process(void) {
    while (app_usbd_event_queue_process()) {}
}

static char s_buff_[BUFF_SIZE];
static uint8_t s_cur_buf_idx_ = 0;

static void s_clean_buffer_(void) {
    for (size_t k = 0; k < BUFF_SIZE; k++) {
        s_buff_[k] = '\0';
    }
    s_cur_buf_idx_ = 0;
}

static void usb_ev_handler(app_usbd_class_inst_t const * p_inst, app_usbd_cdc_acm_user_event_t event) {
    switch (event) {
    case APP_USBD_CDC_ACM_USER_EVT_PORT_OPEN: {
        NRF_LOG_INFO("Opened USB port");
        ret_code_t ret;
        ret = app_usbd_cdc_acm_read(&usb_cdc_acm, s_rx_buffer_, READ_SIZE);
        UNUSED_VARIABLE(ret);
        break;
    }
    case APP_USBD_CDC_ACM_USER_EVT_PORT_CLOSE: {
        NRF_LOG_INFO("Closed USB port");
        break;
    }
    case APP_USBD_CDC_ACM_USER_EVT_TX_DONE: {
        if (cli_is_there_message()) {
            char message[MESSAGE_SIZE];
            cli_get_message(message);
            uint16_t len = strlen(message);
            app_usbd_cdc_acm_write(&usb_cdc_acm, message, len);
        }
        break;
    }
    case APP_USBD_CDC_ACM_USER_EVT_RX_DONE: {
        ret_code_t ret;
        s_buff_[s_cur_buf_idx_] = s_rx_buffer_[0];
        s_cur_buf_idx_++;
        do {
            if (s_rx_buffer_[0] == '\r' || s_rx_buffer_[0] == '\n') {
                s_buff_[s_cur_buf_idx_ - 1] = '\0';
                NRF_LOG_INFO("%s", s_buff_);
#ifdef ESTC_IS_USB_CLI_LOCKABLE
                if (ESTC_IS_USB_CLI_LOCKABLE) {
                    if (hsv_editor_is_edit_completed()) {
                        cli_proceed(s_buff_);
                    } else {
                        cli_set_message("PWM module is locked by user\r\n");
                    }
                } else {
                    cli_proceed(s_buff_);
                }
#endif
                s_clean_buffer_();
                ret = app_usbd_cdc_acm_write(&usb_cdc_acm, "\r\n", 2);
            } else {
                ret = app_usbd_cdc_acm_write(&usb_cdc_acm, s_rx_buffer_, READ_SIZE);
            }

            ret = app_usbd_cdc_acm_read(&usb_cdc_acm, s_rx_buffer_, READ_SIZE);
        } while (ret == NRF_SUCCESS);

        break;
    }
    default:
        break;
    }
}