#include "hsv_editor_rgb_color_storage.h"
#include "hsv_editor.h"
#include "module/color/converter.h"
#include "cli/cli.h"
#include <string.h>
#include "nrf_log.h"

static rgb_t s_colors_[COLORS_ENTRY_SIZE];
static char s_color_names_[COLORS_ENTRY_SIZE][10]; 
static uint8_t s_cur_free_idx_ = 0;

static bool s_is_name_unique_(char* color_name) {
    for (size_t i = 0; i < COLORS_ENTRY_SIZE; i++) {
        bool is_different = strcmp(s_color_names_[i], color_name);
        if (!is_different) {
            return false;
        }
    }
    return true;
}

bool hsv_editor_rgb_color_storage_add_color(uint8_t r, uint8_t g, uint8_t b, char* color_name) {
    if (s_cur_free_idx_ == COLORS_ENTRY_SIZE) {
        cli_set_message("Color storage is more than 10. Delete unwanted colors to free space\r\n");
        NRF_LOG_INFO("Color storage is more than 10. Delete unwanted colors to free space");
        return false;
    }

    uint8_t len = strlen(color_name);
    if (len > 10) {
        cli_set_message("Too long color name\r\n");
        NRF_LOG_INFO("Too long color name");
        return false;
    }

    if (!s_is_name_unique_(color_name)) {
        cli_set_message("There is already color with the same name\r\n");
        NRF_LOG_INFO("There is already color with the same name");
        return false;
    }

    rgb_t rgb_obj = converter_get_rgb_data(r, g, b);
    s_colors_[s_cur_free_idx_] = rgb_obj;
    strcpy(s_color_names_[s_cur_free_idx_], color_name);
    s_cur_free_idx_++;

    NRF_LOG_INFO("Color set to storage");
    return true;
}

rgb_t hsv_editor_rgb_color_storage_get_color_by_name(char* color_name) {
    size_t i = 0;
    while (i != COLORS_ENTRY_SIZE) {
        bool is_equal = !strcmp(s_color_names_[i], color_name);
        if (is_equal) {
            return s_colors_[i];
        }
        i++;
    }
    rgb_t zero_color = {
            .red = 0,
            .green = 0,
            .blue = 0
    };
    return zero_color;
}

void hsv_editor_rgb_color_storage_delete(char* color_name) {
    size_t i = 0;
    bool is_equal = false;
    while (i != COLORS_ENTRY_SIZE) {
        is_equal = !strcmp(s_color_names_[i], color_name);
        if (is_equal) {
            break;
        }
        i++;
    }
    if (!is_equal) {
        return;
    }
    for (i++; i < COLORS_ENTRY_SIZE; i++) {
        strcpy(s_color_names_[i - 1], s_color_names_[i]);
        s_colors_[i - 1] = s_colors_[i];
    }
    s_cur_free_idx_--;
}


void hsv_editor_rgb_color_storage_add_current_color_from_pwm(char* color_name) {
    hsv_t current_hsv = hsv_editor_get_hsv_object();
    rgb_t rgb = converter_to_rgb_from_hsv(current_hsv);
    hsv_editor_rgb_color_storage_add_color(rgb.red, rgb.green, rgb.blue, color_name);
}

void hsv_editor_rgb_color_storage_get_colors(rgb_t* colors) {
    memcpy(colors, s_colors_, COLORS_ENTRY_SIZE * sizeof(rgb_t));
}

void hsv_editor_rgb_color_storage_get_names(char names[10][10]) {
    for (size_t i = 0; i < COLORS_ENTRY_SIZE; i++) {
        strcpy(names[i], s_color_names_[i]);
    }
}

uint8_t hsv_editor_rgb_color_storage_get_last_free_idx(void) {
    return s_cur_free_idx_;
}

void hsv_editor_rgb_color_storage_set_names(char names[COLORS_ENTRY_SIZE][COLORS_ENTRY_SIZE], uint8_t len) {
    for (size_t i = 0; i < len; i++) {
        strcpy(s_color_names_[i], names[i]);
    }
}

void hsv_editor_rgb_color_storage_set_colors(rgb_t* colors, uint8_t len) {
    memcpy(s_colors_, colors, COLORS_ENTRY_SIZE * sizeof(rgb_t));
}

void hsv_editor_rgb_color_storage_set_last_free_idx(uint8_t data) {
    s_cur_free_idx_ = data;
}