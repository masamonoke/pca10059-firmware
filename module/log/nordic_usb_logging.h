#ifndef MODULE_LOG_NORDIC_USB_LOGGING
#define MODULE_LOG_NORDIC_USB_LOGGING

//adaptation of logger example 
//https://github.com/DSRCorporation/esl-nsdk/blob/35e4f4e9b3663415e1c4e7fd4d1d32a760b5efb4/examples/peripheral/dongle_logs/main.c

#include "nrf_log.h"

void nordic_usb_logging_init(void);

void nordic_usb_logging_process(void);

#endif /* MODULE_LOG_NORDIC_USB_LOGGING */
