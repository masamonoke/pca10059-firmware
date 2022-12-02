#include "nvm.h"
#include "nrfx_nvmc.h"
#include "nrf_log.h"

#define ERASED_WORD -1

void nvm_init_instance(nvm_instance_t* instance, uint32_t page) {
    instance->page = page;
    instance->cur_addr = instance->page;
    instance->p_addr = (uint32_t*) instance->cur_addr;
    nvm_set_ptr_to_last_elem(instance);
    instance->cur_addr = (uint32_t) instance->p_addr;
}

void nvm_write_values(nvm_instance_t* instance, uint32_t* values, uint16_t len) {
    if (instance->cur_addr + (4 * len) > instance->page + PAGE_SIZE) {
        nrf_nvmc_page_erase(instance->page);
        instance->cur_addr = instance->page;
        instance->p_addr = (uint32_t*) instance->cur_addr;
    }

    if (instance->cur_addr >= instance->page + PAGE_SIZE) {
        instance->cur_addr = instance->page;
        instance->p_addr = (uint32_t*) instance->cur_addr;
        nrf_nvmc_page_erase(instance->cur_addr);
    } else {
        for (size_t i = 0; i < len; i++) {
            if (nrfx_nvmc_word_writable_check(instance->cur_addr, values[i])) {
                nrf_nvmc_write_word(instance->cur_addr, values[i]);
                if (nrfx_nvmc_write_done_check()) {
                    NRF_LOG_INFO("Value is saved");
                }
                instance->p_addr++;
                instance->cur_addr = (uint32_t) instance->p_addr;
            }
        }
    }
}

void nvm_read_last_data(nvm_instance_t* instance, uint32_t* buf, uint16_t len) {
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
}

void nvm_set_ptr_to_start(nvm_instance_t* instance) {
    instance->p_addr = (uint32_t*) instance->page;
}

void nvm_set_ptr_to_last_elem(nvm_instance_t* instance) {
    while (*(instance->p_addr) != ERASED_WORD) {
        instance->p_addr++;
    }
}