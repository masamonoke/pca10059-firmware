#ifndef MODULE_APP_HSV_EDITOR
#define MODULE_APP_HSV_EDITOR

#include <stdint.h>
#include <stdbool.h>
#include "module/color/converter.h"

void hsv_editor_init(void);

void hsv_editor_change_color(void);

void hsv_editor_process_current_behavior(void);

void hsv_editor_set_init_hsv(uint16_t h, uint8_t s, uint8_t v);

bool hsv_editor_get_is_nvm_write_time(void);

void hsv_editor_set_is_nvm_write_time(bool);

hsv_t hsv_editor_get_hsv_object(void);

#endif /* MODULE_APP_HSV_EDITOR */
