#ifndef MODULE_APP_HSV_EDITOR_HSV_EDITOR_RGB_COLOR_STORAGE
#define MODULE_APP_HSV_EDITOR_HSV_EDITOR_RGB_COLOR_STORAGE

#include <stdint.h>
#include <stdbool.h>
//TODO: remove
#include "module/color/converter.h"

bool hsv_editor_rgb_color_storage_add_color(uint8_t r, uint8_t g, uint8_t b, char* color_name);

void hsv_editor_rgb_color_storage_add_current_from_pwm(char* color_name);

void hsv_editor_rgb_color_storage_delete(char* color_name);

void hsv_editor_rgb_color_storage_apply_color(char* color_name);

void hsv_editor_rgb_color_storage_add_current_color_from_pwm(char* color_name);

//TODO: remove
void hsv_editor_rgb_color_get_colors(rgb_t* colors);
void hsv_editor_rgb_color_get_names(char** names);

rgb_t hsv_editor_rgb_color_get_color_by_name(char* color_name);

#endif /* MODULE_APP_HSV_EDITOR_HSV_EDITOR_RGB_COLOR_STORAGE */
