#include "fstorage_utils.h"
#include "nrf_nvmc.h"
#include "nrf_log.h"

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
	uint32_t addr = FSTORAGE_START_ADDR;
	uint32_t* p_addr = (uint32_t*) addr;
	while (fstorage_utils_read(addr) != -1) {
		p_addr++;
		addr = (uint32_t) p_addr;
	}
	
	if (addr <= FSTORAGE_START_ADDR) {
		return false;
	}

	p_addr--;
	addr = (uint32_t) p_addr;

	data->value = (uint8_t) fstorage_utils_read(addr);
	p_addr--;
	addr = (uint32_t) p_addr;
	data->saturation = (uint8_t) fstorage_utils_read(addr);
	p_addr--;
	addr = (uint32_t) p_addr;
	data-> hue = (uint16_t) fstorage_utils_read(addr);

	if (data->value == UINT8_MAX || data->hue == UINT16_MAX || data->saturation == UINT8_MAX) {
		return false;
	}
	
	return true;
}

uint32_t fstorage_utils_write_hsv(hsv_t hsv) {
	uint32_t addr = FSTORAGE_START_ADDR;
	uint32_t* p_addr = (uint32_t*) addr;
	while (fstorage_utils_read(addr) != -1) {
		p_addr++;
		addr = (uint32_t) p_addr;
	}
	
	if (addr >= FSTORAGE_START_ADDR + PAGE_SIZE - 1) {
		fstorage_utils_erase_page(FSTORAGE_START_ADDR);
		addr = FSTORAGE_START_ADDR;
		p_addr = (uint32_t*) addr;
	}

	NRF_LOG_INFO("fstorage_utils: Star writing starting from address %s");
	fstorage_utils_write(hsv.hue, addr);
	NRF_LOG_INFO("fstorage_utils: Wrote data %d to %d", hsv.hue, addr);
	p_addr++;
	addr = (uint32_t) p_addr;
	fstorage_utils_write(hsv.saturation, addr);
	p_addr++;
	addr = (uint32_t) p_addr;
	fstorage_utils_write(hsv.value, addr);

	return addr;
}

void fstorage_utils_erase_page(uint32_t addr) {
	nrf_fstorage_erase(&fstorage_s_, addr, 1, NULL);
}
