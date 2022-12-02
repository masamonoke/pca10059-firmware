#include "hsv_editor.h"
#include "hsv_editor_nvm.h"
#include "module/memory/nvm.h"
#include "nrfx_nvmc.h"

#define SAVED_SET_COLOR_SPACE_ADDR START_ADDR
#define NAMED_COLORS_SPACE_ADDR (SAVED_SET_COLOR_SPACE_ADDR + PAGE_SIZE)

static nvm_instance_t s_color_save_instance_;

static bool s_is_init_ = false;
void hsv_editor_nvm_init(void) {
    if (!s_is_init_) {
        nvm_init_instance(&s_color_save_instance_, SAVED_SET_COLOR_SPACE_ADDR);
        s_is_init_ = true;
    }
}

bool hsv_editor_nvm_is_prev_set_color_saved(uint32_t* buf) {
    nvm_read_last_data(&s_color_save_instance_, buf, SAVED_SET_COLOR_SPACE_DATA_COUNT);
    if (buf == NULL) {
        return false;
    } else {
        return true;
    }
}

void hsv_editor_nvm_write_hsv(uint16_t h, uint8_t s, uint8_t v) {
    uint32_t values[] = { h, s, v };
    nvm_write_values(&s_color_save_instance_, values, SAVED_SET_COLOR_SPACE_DATA_COUNT);
}