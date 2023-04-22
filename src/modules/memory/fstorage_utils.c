#include "fstorage_utils.h"
#include "nrf_nvmc.h"

/* NRF_FSTORAGE_DEF(nrf_fstorage_t fstorage) = { */
/*     .start_addr = FSTORAGE_START_ADDR, */
/*     .end_addr   = FSTORAGE_END_ADDR, */
/* }; */

NRF_FSTORAGE_DEF(nrf_fstorage_t fstorage_s_) = {
    .start_addr = 0,
    .end_addr   = 0,
};

void wait_for_flash_ready(nrf_fstorage_t const * p_fstorage) {
    while (nrf_fstorage_is_busy(p_fstorage))
    {
    	(void) sd_app_evt_wait();
    }
}

static nrf_fstorage_api_t* p_fs_api;
void fstorage_utils_init() {
	fstorage_s_.start_addr = FSTORAGE_START_ADDR;
	fstorage_s_.end_addr = FSTORAGE_END_ADDR;
    p_fs_api = &nrf_fstorage_sd;
	nrf_fstorage_init(&fstorage_s_, p_fs_api, NULL);
}

void fstorage_utils_erase(void) {
	nrf_fstorage_erase(&fstorage_s_, FSTORAGE_START_ADDR, 3, NULL);
}

void fstorage_utils_write(uint32_t data, uint32_t address) {
	nrf_fstorage_write(&fstorage_s_, address, &data, sizeof(data), NULL);
	wait_for_flash_ready(&fstorage_s_);
}

uint32_t fstorage_utils_read(uint32_t address) {
	uint32_t data = -1;
	nrf_fstorage_read(&fstorage_s_, address, &data, sizeof(data));
	return data;
}

bool fstorage_utils_read_hsv(hsv_t* data) {
	uint32_t address;
	address = FSTORAGE_START_ADDR;
	uint32_t h = fstorage_utils_read(address);
	if (h == -1) {
		return false;
	}
	uint32_t* p_addr;
	p_addr = (uint32_t*) address;
	p_addr++;
	address = (uint32_t) p_addr;
	uint32_t s = fstorage_utils_read(address);
	p_addr = (uint32_t*) address;
	p_addr++;
	address = (uint32_t) p_addr;
	uint32_t v = fstorage_utils_read(address);
	
	data->hue = h;
	data->saturation = s;
	data->value = v;

	return true;
}

void fstorage_utils_write_hsv(hsv_t hsv) {
	fstorage_utils_erase();
	uint32_t address;
	address = FSTORAGE_START_ADDR;
	fstorage_utils_write((uint32_t) hsv.hue, address);
	wait_for_flash_ready(&fstorage_s_);
	uint32_t* p_addr;
	p_addr = (uint32_t*) address;
	p_addr++;
	address = (uint32_t) p_addr;
	fstorage_utils_write((uint32_t) hsv.saturation, address);
	wait_for_flash_ready(&fstorage_s_);
	p_addr = (uint32_t*) address;
	p_addr++;
	address = (uint32_t) p_addr;
	fstorage_utils_write((uint32_t) hsv.value, address);
	wait_for_flash_ready(&fstorage_s_);
}

