#ifndef MODULE_APP_HSV_EDITOR_NVM
#define MODULE_APP_HSV_EDITOR_NVM

#define SAVED_SET_COLOR_SPACE_DATA_COUNT 3

void hsv_editor_nvm_init(void);

bool hsv_editor_nvm_is_prev_set_color_saved(uint32_t* buf);

void hsv_editor_nvm_write_hsv(uint16_t h, uint8_t s, uint8_t v);

#endif /* MODULE_APP_HSV_EDITOR_NVM */
