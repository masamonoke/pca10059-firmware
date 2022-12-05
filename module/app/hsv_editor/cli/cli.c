#include "cli.h"
#include "module/io/led/nordic_rgb_pwm_utils.h"
#include "module/utils/string_utils.h"
#include "nrf_log.h"
#include "module/utils/math_utils.h"
#include "module/app/hsv_editor/hsv_editor.h"
#include "module/utils/array_utils.h"
#include "../hsv_editor_rgb_color_storage.h"
#include "string.h"
#include "../hsv_editor_nvm.h"

#define MESSAGE_SIZE 150
static char s_message_[MESSAGE_SIZE];
static bool s_is_message_ = false;

static void s_clear_message_(void) {
    for (size_t i = 0; i < MESSAGE_SIZE; i++) {
        s_message_[i] = '\0';
    }
}

//TODO: remove set len
void cli_set_message(char* str, uint16_t len) {
    s_clear_message_();
    size_t i;
    for (i = 0; i < len; i++) {
        s_message_[i] = str[i];
    }
    s_message_[i] = '\0';
    s_is_message_ = true;
}

void cli_get_message(char* str) {
    size_t i = 0;
    while (s_message_[i] != '\0') {
        str[i] = s_message_[i];
        i++;
    }
    s_is_message_ = false;
}

bool cli_is_there_message(void) {
    return s_is_message_;
}

static void s_cli_functions_undefined_command_(void) {
    cli_set_message("Undefined command\r\n", 19);
    NRF_LOG_INFO("Undefined command");
}

#define MAX_ARGS_STR_LEN 30
static bool s_get_args_(const char* input, uint16_t start_idx, uint16_t* args, uint16_t args_count) {
    char s[MAX_ARGS_STR_LEN];
    string_utils_substring_to_end(input, start_idx + 1, s);

    return string_utils_parse_string_get_nums(s, args, 3);
}

static void s_prepare_help_message_(void) {
    cli_set_message("RGB <red> <green> <blue>\r\nHSV <hur> <saturation> <value>\r\nhelp\r\n", 64);
}

static uint8_t s_get_num_len_(uint16_t num) {
    uint8_t len = 0;
    if (num == 0) {
        return 1;
    }
    while (num != 0) {
        num /= 10;
        len++;
    }
    return len;
}

#define MAX_DIGITS 5
static void s_prepare_message_(const uint16_t* vals, uint16_t vals_count,
         const char* start_of_message, const uint16_t mes_len, char* chars) {

    s_clear_message_();

    char message[130];
    for (size_t i = 0; i < mes_len; i++) {
        message[i] = start_of_message[i];
    }

    uint8_t len;
    char num[MAX_DIGITS];
    uint8_t cur_idx = mes_len;

    for (size_t val_idx = 0; val_idx < vals_count; val_idx++) {
        len = s_get_num_len_(vals[val_idx]);
        sprintf(num, "%d", vals[val_idx]);
        message[cur_idx++] = chars[val_idx];
        message[cur_idx++] = '=';
        for (size_t i = cur_idx, k = 0; i < cur_idx + len; i++, k++) {
            message[i] = num[k];
        }
        cur_idx += len;
        message[cur_idx++] = ' ';
    }

    message[cur_idx++] = '\r';
    message[cur_idx++] = '\n';
    message[cur_idx] = '\0';

    cli_set_message(message, strlen(message));
}

static bool s_cli_functions_rgb_proceed_(const char* input, uint8_t args_start_idx) {
    uint16_t args[3];
    if (!s_get_args_(input, args_start_idx, args, 3)) {
        return false;
    }
    args[0] = math_utils_clamp_int(args[0], 255, 0);
    args[1] = math_utils_clamp_int(args[1], 255, 0);
    args[2] = math_utils_clamp_int(args[2], 255, 0);
    uint8_t r = args[0];
    uint8_t g = args[1];
    uint8_t b = args[2];
    nordic_rgb_pwm_set_color(r, g, b);

    char chars[] = "RGB";
    uint16_t vals[] = { r, g, b };
    rgb_t rgb_obj = {
        .red = r,
        .blue = b,
        .green = g
    };
    hsv_t hsv_obj = converter_to_hsv_from_rgb(rgb_obj);
    hsv_editor_set_hsv(hsv_obj.hue, hsv_obj.saturation, hsv_obj.value);
    s_prepare_message_(vals, 3, "Color set to ", 13, chars);
    NRF_LOG_INFO("Color set to R=%d G=%d B=%d", r, g, b);

    return true;
}

static bool s_cli_functions_hsv_proceed_(const char* input, uint8_t args_start_idx) {
    uint16_t args[3];
    if (!s_get_args_(input, args_start_idx, args, 3)) {
        return false;
    }

    args[0] = math_utils_clamp_int(args[0], 360, 0);
    args[1] = math_utils_clamp_int(args[1], 100, 0);
    args[2] = math_utils_clamp_int(args[2], 100, 0);

    uint16_t h = args[0];
    uint8_t s = args[1];
    uint8_t v = args[1];

    nordic_rgb_pwm_set_hsv_color(h, s, v);

    
    char chars[] = "HSV";
    uint16_t vals[] = { h, s, v };
    hsv_editor_set_hsv(h, s, v);
    s_prepare_message_(vals, 3, "Color set to ", 13, chars);
    NRF_LOG_INFO("Color set to H=%d S=%d V=%d", h, s, v);

    return true;
}

static bool s_cli_functions_help_proceed_(const char* input, uint8_t args_start_idx) {
    if (input[args_start_idx] != '\0') {
        return false;
    }

    s_prepare_help_message_();
    NRF_LOG_INFO("\nRGB <red> <green> <blue>\nHSV <hur> <saturation> <value>\nhelp");

    return true;
}

static bool s_save_color_with_name_(uint8_t r, uint8_t g, uint8_t b, char* color_name) {
    bool res = hsv_editor_rgb_color_storage_add_color(r, g, b, color_name);
    if (!res) {
        char message[] = "Color with that name is already saved\r\n";
        cli_set_message(message, strlen(message));
        return false;
    }

    uint8_t saved_colors_count = hsv_editor_rgb_get_last_free_idx();
    if (saved_colors_count == 10) {
        char message[50] = "Saved colors count already at max count 10";
        cli_set_message(message, strlen(message));
        return false;
    }

    bool is_saved = hsv_editor_save_added_colors();
    if (!is_saved) {
        return false;
    } 

    return true;
}

static bool s_cli_functions_add_rgb_color_proceed_(const char* input, uint8_t args_start_idx) {

    //e.g. add_rgb_color 176 196 222 light_steel_blue is not allowed because of too long color name
    //shorten long names like lg_st_blue
    const uint8_t input_max_len = 36;
    if (strlen(input) > input_max_len) {
        char message[30] = "Color name is too long";
        cli_set_message(message, strlen(message));
        return false;
    }

    uint8_t str_len = strlen(input);
    uint8_t space_count = 0;
    size_t i;
    for (i = args_start_idx + 1; i < str_len; i++) {
        if (input[i] == ' ') {
            space_count++;
            if (space_count == 3) {
                break;
            }
        }
    }
    char num_args_substring[30];
    string_utils_substring(input, 0, i - 1, num_args_substring);
    num_args_substring[i] = '\0';
    uint16_t args[3];
    if (!s_get_args_(num_args_substring, args_start_idx, args, 3)) {
        return false;
    }

    uint8_t r = math_utils_clamp_int(args[0], 255, 0);
    uint8_t g = math_utils_clamp_int(args[1], 255, 0);
    uint8_t b = math_utils_clamp_int(args[2], 255, 0);

    char color_name[10];
    string_utils_substring_to_end(input, i + 1, color_name);

    bool is_saved = s_save_color_with_name_(r, g, b, color_name);
    if (!is_saved) {
        return true;
    }

    char message[100] = "Color with name " ;
    strcat(message, color_name);
    char tmp[] = " saved to storage\r\n";
    strcat(message, tmp);
    cli_set_message(message, strlen(message));

    return true;
}

static bool s_cli_functions_apply_color_(const char* input, uint8_t args_start_idx) {
    char color_name[10];
    string_utils_substring_to_end(input, args_start_idx + 1, color_name);
    rgb_t color = hsv_editor_rgb_color_get_color_by_name(color_name);
    if (color.red == 0 && color.green == 0 && color.blue == 0) {
        char message[50] = "There is no color with name ";
        strcat(message, color_name);
        char tmp[] = "\r\n";
        strcat(message, tmp);
        cli_set_message(message, strlen(message));
        return true;
    }
    hsv_t hsv_color = converter_to_hsv_from_rgb(color);
    hsv_editor_set_hsv(hsv_color.hue, hsv_color.saturation, hsv_color.value);
    char message[30] = "PWM color set to ";
    strcat(message, color_name);
    char tmp[] = "\r\n";
    strcat(message, tmp);
    cli_set_message(message, strlen(message));

    hsv_editor_set_is_nvm_write_time(true);

    return true;
}

static bool s_cli_functions_del_color_proceed_(const char* input, uint8_t args_start_idx) {
    char color_name[10];
    string_utils_substring_to_end(input, args_start_idx + 1, color_name);
    rgb_t color = hsv_editor_rgb_color_get_color_by_name(color_name);
    if (color.red == 0 && color.green == 0 && color.blue == 0) {
        char message[50] = "There is no color with name ";
        strcat(message, color_name);
        char tmp[] = "\r\n";
        strcat(message, tmp);
        cli_set_message(message, strlen(message));
        return true;
    }

    bool is_deleted = hsv_editor_nvm_delete_color(color_name);
    if (!is_deleted) {
        char message[30] = "NVM deletion error";
        cli_set_message(message, strlen(message));
        return true;
    }

    hsv_editor_rgb_color_storage_delete(color_name);
    char message[30] = "Color ";
    strcat(message, color_name);
    char tmp[] = " deleted\r\n";
    strcat(message, tmp);
    cli_set_message(message, strlen(message));
    return true;
}

static bool s_cli_functions_add_current_color_(const char* input, uint8_t args_start_idx) {

    const uint8_t input_max_len = 28;
    if (strlen(input) > input_max_len) {
        char message[30] = "Color name is too long";
        cli_set_message(message, strlen(message));
        return false;
    }

    char color_name[10];
    string_utils_substring_to_end(input, args_start_idx + 1, color_name);
    hsv_t hsv = hsv_editor_get_hsv_object();
    rgb_t rgb = converter_to_rgb_from_hsv(hsv);

    bool is_saved = s_save_color_with_name_(rgb.red, rgb.green, rgb.blue, color_name);
    if (!is_saved) {
        return true;
    }
    
    char message[30] = "Color ";
    strcat(message, color_name);
    char tmp[] = " saved\r\n";
    strcat(message, tmp);
    cli_set_message(message, strlen(message));
    return true;
}

#define COMMAND_STR_LEN 20
typedef struct {
    char command[COMMAND_STR_LEN];
    bool (* command_func)(const char*, uint8_t);
} command_obj_t;

static command_obj_t s_commands_[] = {
    {
        .command = "rgb",
        .command_func = s_cli_functions_rgb_proceed_
    },
    {
        .command = "hsv",
        .command_func = s_cli_functions_hsv_proceed_
    },
    {
        .command = "help",
        .command_func = s_cli_functions_help_proceed_
    },
    {
        .command = "add_rgb_color",
        .command_func = s_cli_functions_add_rgb_color_proceed_
    },
    {
        .command = "apply_color",
        .command_func = s_cli_functions_apply_color_
    },
    {
        .command = "del_color",
        .command_func = s_cli_functions_del_color_proceed_
    },
    {
        .command = "add_current_color",
        .command_func = s_cli_functions_add_current_color_
    }
};

static void s_define_command_(const char* command_str, const char* input, const size_t args_start_idx) {
    for (uint16_t com_idx = 0; com_idx < ARRAY_LEN(s_commands_); com_idx++) {
        if (string_utils_compare_string(s_commands_[com_idx].command, command_str)) {
            if (!s_commands_[com_idx].command_func(input, args_start_idx)) {
                break;
            } else {
                return;
            }
        }
    }
    s_cli_functions_undefined_command_();
}

void cli_proceed(char* input) {
    size_t i = 0;
    char command[COMMAND_STR_LEN];
    while (input[i] != ' ' && input[i] != '\0') {
        command[i] = input[i];
        i++;
    }

    if (i == 0) {
        return;
    }

    command[i] = '\0';
    string_utils_to_lower_case(command);

    s_define_command_(command, input, i);
}