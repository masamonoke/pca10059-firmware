#ifndef MODULE_APP_HSV_EDITOR_NVM
#define MODULE_APP_HSV_EDITOR_NVM

#include "module/memory/nvm.h"
#include "module/color/converter.h"

#define SAVED_SET_COLOR_SPACE_DATA_COUNT 3

#define SAVED_SET_COLOR_SPACE_ADDR START_ADDR
#define NAMED_COLORS_SPACE_ADDR (SAVED_SET_COLOR_SPACE_ADDR + PAGE_SIZE)

#define LABEL_MARKED_FOR_DELETION 1000

void hsv_editor_nvm_init(void);

bool hsv_editor_nvm_is_prev_set_color_saved(uint32_t* buf);

void hsv_editor_nvm_write_hsv(uint16_t h, uint8_t s, uint8_t v);

void hsv_editor_nvm_prepare_rgb_storage_to_write(uint32_t* array, uint8_t* len);

void hsv_editor_nvm_restore_previous_rgb_storage(rgb_t* colors, char color_names[10][10], uint8_t* len);

void hsv_editor_nvm_mark_for_deletion_rgb_storage_entry(char* color_name, uint16_t* idx);

bool hsv_editor_nvm_save_added_colors(void);

bool hsv_editor_nvm_delete_color(char* color_name);

#endif /* MODULE_APP_HSV_EDITOR_NVM */
