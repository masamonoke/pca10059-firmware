#ifndef FSTORAGE_UTILS_H
#define FSTORAGE_UTILS_H

#include "fds.h"
#include "nrf_fstorage.h"
#include "nrf_fstorage_sd.h"
#include "modules/color/converter.h"

#define FSTORAGE_START_ADDR 0x3e000
#define FSTORAGE_END_ADDR 0x3ffff

void fstorage_utils_init(void);

uint32_t fstorage_utils_read(uint32_t address);

void fstorage_utils_write(uint32_t data, uint32_t address);

void fstorage_utils_erase(void);

bool fstorage_utils_read_hsv(hsv_t* data);

void fstorage_utils_write_hsv(hsv_t hsv);

#endif // FSTORAGE_UTILS_H
