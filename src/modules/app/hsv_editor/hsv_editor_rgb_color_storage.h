#ifndef MODULE_APP_HSV_EDITOR_HSV_EDITOR_RGB_COLOR_STORAGE
#define MODULE_APP_HSV_EDITOR_HSV_EDITOR_RGB_COLOR_STORAGE

#include <stdint.h>
#include <stdbool.h>

#include "modules/color/converter.h"

#define COLORS_ENTRY_SIZE 10

bool hsv_editor_rgb_color_storage_add_color(uint8_t r, uint8_t g, uint8_t b, char* color_name);

void hsv_editor_rgb_color_storage_add_current_from_pwm(char* color_name);

void hsv_editor_rgb_color_storage_delete(char* color_name);

void hsv_editor_rgb_color_storage_apply_color(char* color_name);

void hsv_editor_rgb_color_storage_add_current_color_from_pwm(char* color_name);

void hsv_editor_rgb_color_storage_prepare_data_to_nvm_write(uint32_t* array, uint8_t len);

uint8_t hsv_editor_rgb_color_storage_get_last_free_idx(void);

void hsv_editor_rgb_color_storage_get_names(char names[COLORS_ENTRY_SIZE][COLORS_ENTRY_SIZE]);

void hsv_editor_rgb_color_storage_get_colors(rgb_t* colors);

rgb_t hsv_editor_rgb_color_storage_get_color_by_name(char* color_name);

uint8_t hsv_editor_rgb_color_storage_get_last_free_idx(void);

void hsv_editor_rgb_color_storage_set_last_free_idx(uint8_t);

void hsv_editor_rgb_color_storage_set_names(char names[COLORS_ENTRY_SIZE][COLORS_ENTRY_SIZE], uint8_t len);

void hsv_editor_rgb_color_storage_set_colors(rgb_t* colors, uint8_t len);

#endif /* MODULE_APP_HSV_EDITOR_HSV_EDITOR_RGB_COLOR_STORAGE */
