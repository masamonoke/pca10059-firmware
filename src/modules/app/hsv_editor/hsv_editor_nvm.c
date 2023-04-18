#include "hsv_editor.h"
#include "hsv_editor_nvm.h"
#include "hsv_editor_rgb_color_storage.h"
#include "modules/utils/string_utils.h"

#include "nrf_log.h"
#include "nrfx_nvmc.h"
#include <string.h>

#define SAVED_SET_COLOR_SPACE_ADDR START_ADDR
#define NAMED_COLORS_SPACE_ADDR (SAVED_SET_COLOR_SPACE_ADDR + PAGE_SIZE)

#define MAX_RGB_DATA_SIZE 130

static nvm_instance_t s_color_save_instance_;
static nvm_instance_t s_color_names_inst_;

static bool s_is_init_ = false;
void hsv_editor_nvm_init(void) {
    if (!s_is_init_) {
        nvm_reset_instance(&s_color_save_instance_, SAVED_SET_COLOR_SPACE_ADDR);
        nvm_reset_instance(&s_color_names_inst_, NAMED_COLORS_SPACE_ADDR);
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
	NRF_LOG_INFO("hsv_editor_nvm_write_hsv()");
    nvm_write_values(&s_color_save_instance_, values, SAVED_SET_COLOR_SPACE_DATA_COUNT);
}

static bool s_compare_entries_(rgb_t color1, char* name1, rgb_t color2, char* name2) {
    if (strcmp(name1, name2) == 0) {
        return true;
    }
    return false;
}

void hsv_editor_nvm_prepare_rgb_storage_to_write(uint32_t* array, uint8_t* len) {
    rgb_t restored_colors[COLORS_ENTRY_SIZE];
    char restored_color_names[COLORS_ENTRY_SIZE][COLORS_ENTRY_SIZE];
    uint8_t restored_entries_count;
    hsv_editor_nvm_restore_previous_rgb_storage(restored_colors,
                                                restored_color_names, &restored_entries_count);
    
    NRF_LOG_INFO("Preparing data. Restored entry count: %d", restored_entries_count);

    uint8_t curren_entries_count = hsv_editor_rgb_color_storage_get_last_free_idx();

    NRF_LOG_INFO("Preparing data. Current entry count: %d", curren_entries_count);

    rgb_t colors[COLORS_ENTRY_SIZE];
    hsv_editor_rgb_color_storage_get_colors(colors);
    char color_names[COLORS_ENTRY_SIZE][COLORS_ENTRY_SIZE];
    hsv_editor_rgb_color_storage_get_names(color_names);

    size_t i = 0;
    bool is_duplicate = false;
    for (size_t current_entry_idx = 0; current_entry_idx < curren_entries_count; current_entry_idx++) {
        for (size_t restored_entry_idx = 0; restored_entry_idx < restored_entries_count; restored_entry_idx++) {
            is_duplicate = s_compare_entries_(restored_colors[restored_entry_idx],
                                              restored_color_names[restored_entry_idx], colors[current_entry_idx], color_names[current_entry_idx]);
            if (is_duplicate) {
                break;
            }
        }
        if (!is_duplicate) {
            for (size_t c = 0; c < strlen(color_names[current_entry_idx]); c++) {
                array[i] = (uint32_t) color_names[current_entry_idx][c];
                i++;
            }
            array[i++] = colors[current_entry_idx].red;
            array[i++] = colors[current_entry_idx].green;
            array[i++] = colors[current_entry_idx].blue;
            array[i++] = ERASED_WORD;
        }
    }
    
    *len = i;
}

static void s_read_nvm_(uint32_t* ptr, uint32_t* buf) {
    size_t i = 0;

    while ((uint32_t) (ptr + i) != (NAMED_COLORS_SPACE_ADDR + PAGE_SIZE)) {
        if (*(ptr + i) == ERASED_WORD) {

            if (*(ptr + i + 1) == ERASED_WORD) {
                buf[i] = *(ptr + i);
                i++;
                buf[i] = *(ptr + i);
                break;
            }
        }

        buf[i] = *(ptr + i);
        i++;
    }
}

static void s_assemble_not_deleted_colors_data_(uint32_t* buf, uint32_t* restored_colors_data) {
    int k = 0;
    size_t z = 0;
    size_t last_stop = 0;
    NRF_LOG_INFO("Assembling not deleted colors data...");
    while (k != PAGE_SIZE - 2) {
        if (buf[k] == ERASED_WORD || buf[k] == LABEL_MARKED_FOR_DELETION) {

            if (buf[k + 1] == ERASED_WORD || buf[k + 1] == LABEL_MARKED_FOR_DELETION) {
                size_t tmp = last_stop;
                while (tmp != k + 1) {
                    restored_colors_data[z] = buf[tmp];
                    z++;
                    tmp++;
                }
                break;
            }

            size_t tmp = last_stop;
            while (tmp != k + 1) {
                restored_colors_data[z] = buf[tmp];
                z++;
                tmp++;
            }
        }

        if (buf[k] == ERASED_WORD || buf[k] == LABEL_MARKED_FOR_DELETION) {
            last_stop = k + 1;
        }

        k++;
    }
}

#define RESTORED_NON_ACTUAL_DATA 2000
static void s_parse_restored_data_(rgb_t* colors, char color_names[10][10], uint8_t* len, uint32_t* restored_colors_data) {
    int k = 0;
    size_t z = 0;
    size_t entry_idx = 0;
    for (size_t i = 0; i < MAX_RGB_DATA_SIZE - 1; i++) {

        if (restored_colors_data[i] == RESTORED_NON_ACTUAL_DATA) {
            if (restored_colors_data[i + 1] == RESTORED_NON_ACTUAL_DATA) {
                break;
            }
        }

        if (restored_colors_data[i] == ERASED_WORD) {
            if (i == 0) {
                break;
            }
            k = (int) (i - 1);
            colors[entry_idx].blue = restored_colors_data[k--];
            colors[entry_idx].green = restored_colors_data[k--];
            colors[entry_idx].red = restored_colors_data[k--];
            NRF_LOG_INFO("Restored color %d %d %d", colors[entry_idx].red, colors[entry_idx].green, colors[entry_idx].blue);
            z = 0;
            for (; k >= 0 && (restored_colors_data[k] != ERASED_WORD && restored_colors_data[k] != LABEL_MARKED_FOR_DELETION); k--, z++) {
                color_names[entry_idx][z] = (char) restored_colors_data[k];
            }
            color_names[entry_idx][z] = '\0';
            string_utils_reverse(color_names[entry_idx]);
            NRF_LOG_INFO("Restored color name %s", color_names[entry_idx]);
            entry_idx++;
        }
    }

    *len = entry_idx;
}

void hsv_editor_nvm_restore_previous_rgb_storage(rgb_t* colors, char color_names[10][10], uint8_t* len) {
    uint32_t* ptr = (uint32_t*) NAMED_COLORS_SPACE_ADDR;
    uint32_t buf[PAGE_SIZE];

    s_read_nvm_(ptr, buf);

    ptr = (uint32_t*) NAMED_COLORS_SPACE_ADDR;

    uint32_t restored_colors_data[MAX_RGB_DATA_SIZE];
    memset(restored_colors_data, RESTORED_NON_ACTUAL_DATA, sizeof(uint32_t) * MAX_RGB_DATA_SIZE);
    s_assemble_not_deleted_colors_data_(buf, restored_colors_data);

    s_parse_restored_data_(colors, color_names, len, restored_colors_data);
}

void hsv_editor_nvm_mark_for_deletion_rgb_storage_entry(char* color_name, uint16_t* idx) {
    uint32_t* ptr = (uint32_t*) NAMED_COLORS_SPACE_ADDR;
    size_t i = 0;

    while ((uint32_t) (ptr + i) != (NAMED_COLORS_SPACE_ADDR + PAGE_SIZE)) {
        bool is_potential_entry = false;
        if (*(ptr + i) == color_name[0]) {
            is_potential_entry = true;
            size_t ch_idx = 1;
            i++;
            while (!is_potential_entry || *(ptr + i + 4) != ERASED_WORD) {
                if (*(ptr + i) != color_name[ch_idx]) {
                    is_potential_entry = false;
                    ch_idx = 0;
                    break;
                }
                i++;
                ch_idx++;
            }
            if (is_potential_entry) {
                *idx = i + 4;
                return;
            }
        }
        i++;
    }
    //if not found such color_name in nvm
    *idx = -1;
}

bool hsv_editor_nvm_save_added_colors(void) {
    uint32_t array[130];
    uint8_t len;
    hsv_editor_nvm_prepare_rgb_storage_to_write(array, &len);

    if (len != 0) {
        rgb_t restored_colors[COLORS_ENTRY_SIZE];
        char restored_color_names[COLORS_ENTRY_SIZE][COLORS_ENTRY_SIZE];
        uint8_t restored_entries_count;
        hsv_editor_nvm_restore_previous_rgb_storage(restored_colors, restored_color_names, &restored_entries_count);

        if (restored_entries_count > 0 && *(s_color_names_inst_.p_addr - 1) != LABEL_MARKED_FOR_DELETION) {
            s_color_names_inst_.p_addr++;
        }

        NRF_LOG_INFO("Prepared data count %d", len);

        NRF_LOG_INFO("Pointer data before write: %d %d", *s_color_names_inst_.p_addr, (uint32_t) s_color_names_inst_.p_addr);
        
        bool is_erase_happened = nvm_write_values(&s_color_names_inst_, array, len);

        NRF_LOG_INFO("Pointer data after write: %d %d", *s_color_names_inst_.p_addr, (uint32_t) s_color_names_inst_.p_addr);

        if (is_erase_happened) {    
            NRF_LOG_INFO("Erased Named colors page");
            hsv_editor_nvm_save_added_colors();
        }

        return true;
        
    } else {
        NRF_LOG_INFO("No named color to write to NVM");
        return true;
    }
    
    return false;
}

bool hsv_editor_nvm_delete_color(char* color_name) {
    uint16_t delete_idx;
    hsv_editor_nvm_mark_for_deletion_rgb_storage_entry(color_name, &delete_idx);
    if (delete_idx == -1) {
        return false;
    }
    
    uint32_t* p = (uint32_t*) (NAMED_COLORS_SPACE_ADDR);
    p += delete_idx;
    uint32_t addr = (uint32_t) p;
    nrfx_nvmc_word_write(addr, LABEL_MARKED_FOR_DELETION);

    NRF_LOG_INFO("Deleted color \"%s\"", color_name);

    return true;
}
