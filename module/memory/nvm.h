#ifndef MODULE_MEMORY_NVM
#define MODULE_MEMORY_NVM

#include <stdint.h>
#include <stdbool.h>

#define ERASED_WORD (-1)
#define PAGE_SIZE 0x00001000
#define START_ADDR (BOOTLOADER_ADDRESS - NRF_DFU_APP_DATA_AREA_SIZE)

typedef struct {
    uint32_t* p_addr;
    uint32_t cur_addr;
    uint32_t page;
} nvm_instance_t;

void nvm_reset_instance(nvm_instance_t* instance, uint32_t page);

bool nvm_write_values(nvm_instance_t* instance, uint32_t* values, uint16_t len);

void nvm_read_last_data(nvm_instance_t* instance, uint32_t* buf, uint16_t len);

void nvm_set_ptr_to_start(nvm_instance_t* instance);

void nvm_set_ptr_to_last_elem(nvm_instance_t* instance);

void nvm_set_ptr_to_last_label(nvm_instance_t* instance, uint32_t label);

void nvm_write_values_with_delimiter(nvm_instance_t* instance, uint32_t* values, uint16_t len,
    uint32_t delimiter);

#endif /* MODULE_MEMORY_NVM */
