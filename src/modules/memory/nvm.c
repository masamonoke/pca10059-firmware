#include "nvm.h"
#include "nrfx_nvmc.h"
#include "nrf_log.h"

void nvm_reset_instance(nvm_instance_t* instance, uint32_t page) {
    instance->page = page;
    instance->cur_addr = instance->page;
    instance->p_addr = (uint32_t*) instance->cur_addr;
    nvm_set_ptr_to_last_elem(instance);
    instance->cur_addr = (uint32_t) instance->p_addr;
}

bool nvm_write_values(nvm_instance_t* instance, uint32_t* values, uint16_t len) {
    bool is_erased = false;
    instance->cur_addr = (uint32_t) instance->p_addr;
    if (instance->cur_addr + (4 * len) > instance->page + PAGE_SIZE) {
        nrf_nvmc_page_erase(instance->page);
        instance->cur_addr = instance->page;
        instance->p_addr = (uint32_t*) instance->cur_addr;
        is_erased = true;
    }

	NRF_LOG_INFO("DEBUG nvm modlue: before write");

    for (size_t i = 0; i < len; i++) {
        if (nrfx_nvmc_word_writable_check(instance->cur_addr, values[i])) {
            nrf_nvmc_write_word(instance->cur_addr, values[i]);
            if (nrfx_nvmc_write_done_check()) {
                NRF_LOG_INFO("NVM: Value is saved %d", values[i]);
            }
            instance->p_addr++;
            instance->cur_addr = (uint32_t) instance->p_addr;
        }
    }

	NRF_LOG_INFO("DEBUG nvm module: after write");

    return is_erased;
}

void nvm_read_last_data(nvm_instance_t* instance, uint32_t* buf, uint16_t len) {
	NRF_LOG_INFO("Reading last data from nvm");
    if (instance->cur_addr == instance->page) {
        buf = NULL;
        return;
    }

    if (instance->cur_addr - (4 * len) >= instance->page) {
        uint32_t tmp_addr = instance->cur_addr -  4;
        instance->p_addr = (uint32_t*) tmp_addr;
        for (int i = len - 1; i >= 0; i--) {
            buf[i] = *(instance->p_addr);
            instance->p_addr--;
        }
        instance->p_addr = (uint32_t*) instance->cur_addr;
    } else {
        buf = NULL;
    }
	NRF_LOG_INFO("Read last data from nvm");
}

void nvm_set_ptr_to_start(nvm_instance_t* instance) {
    instance->p_addr = (uint32_t*) instance->page;
}

void nvm_set_ptr_to_last_elem(nvm_instance_t* instance) {
    while (*(instance->p_addr) != ERASED_WORD) {
        instance->p_addr++;
        if (*(instance->p_addr + 1) != ERASED_WORD) {
            instance->p_addr++;
        }
    }
}

void nvm_set_ptr_to_last_label(nvm_instance_t* instance, uint32_t label) {
    while (*(instance->p_addr) != label) {
        instance->p_addr++;
    }
    instance->p_addr++;
}
