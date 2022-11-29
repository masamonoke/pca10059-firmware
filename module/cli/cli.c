#include "cli.h"
#include "module/io/led/nordic_rgb_pwm_utils.h"
#include "module/utils/string_utils.h"
#include "nrf_log.h"
#include "module/utils/math_utils.h"

#define MESSAGE_SIZE 150
static char s_message_[MESSAGE_SIZE];
static uint16_t s_len_;
static bool s_is_message_ = false;

static void s_clear_message_(void) {
    for (size_t i = 0; i < MESSAGE_SIZE; i++) {
        s_message_[i] = '\0';
    }
}

void cli_set_message(char* str, uint16_t len) {
    s_clear_message_();
    size_t i;
    for (i = 0; i < len; i++) {
        s_message_[i] = str[i];
    }
    s_message_[i] = '\0';
    s_is_message_ = true;
    s_len_ = len;
}

void cli_get_message(char* str, uint16_t* len) {
    size_t i = 0;
    while (s_message_[i] != '\0') {
        str[i] = s_message_[i];
        i++;
    }
    *len = s_len_;
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

    for (size_t i = 0; i < mes_len; i++) {
        s_message_[i] = start_of_message[i];
    }

    uint8_t len;
    char num[MAX_DIGITS];
    uint8_t cur_idx = mes_len;

    for (size_t val_idx = 0; val_idx < vals_count; val_idx++) {
        len = s_get_num_len_(vals[val_idx]);
        sprintf(num, "%d", vals[val_idx]);
        s_message_[cur_idx++] = chars[val_idx];
        s_message_[cur_idx++] = '=';
        for (size_t i = cur_idx, k = 0; i < cur_idx + len; i++, k++) {
            s_message_[i] = num[k];
        }
        cur_idx += len;
        s_message_[cur_idx++] = ' ';
    }

    s_message_[cur_idx++] = '\r';
    s_message_[cur_idx++] = '\n';
    s_message_[cur_idx] = '\0';

    s_len_ = cur_idx;
    s_is_message_ = true;
}


static bool cli_functions_rgb_proceed(const char* input, uint8_t args_start_idx) {
    uint16_t args[3];
    if (!s_get_args_(input, args_start_idx, args, 3)) {
        return false;
    }
    args[0] = math_utils_clamp_int(args[0], 255, 0);
    args[1] = math_utils_clamp_int(args[1], 255, 0);
    args[2] = math_utils_clamp_int(args[2], 255, 0);
    uint32_t r = args[0];
    uint32_t g = args[1];
    uint32_t b = args[2];
    nordic_rgb_pwm_set_color(r, g, b);

    char chars[] = "RGB";
    uint16_t vals[] = { r, g, b };
    s_prepare_message_(vals, 3, "Color set to ", 13, chars);
    NRF_LOG_INFO("Color set to R=%d G=%d B=%d", r, g, b);

    return true;
}

static bool cli_functions_hsv_proceed(const char* input, uint8_t args_start_idx) {
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
    s_prepare_message_(vals, 3, "Color set to ", 13, chars);
    NRF_LOG_INFO("Color set to H=%d S=%d V=%d", h, s, v);

    return true;
}

static bool cli_functions_help_proceed(const char* input, uint8_t args_start_idx) {
    if (input[args_start_idx] != '\0') {
        return false;
    }

    s_prepare_help_message_();
    NRF_LOG_INFO("\nRGB <red> <green> <blue>\nHSV <hur> <saturation> <value>\nhelp");

    return true;
}

#define COMMAND_STR_LEN 10
typedef struct {
    char command[COMMAND_STR_LEN];
    bool (* command_func)(const char*, uint8_t);
} command_obj_t;

#define COMMAND_LIST_LEN 3
static command_obj_t s_commands_[] = {
    {
        .command = "rgb",
        .command_func = cli_functions_rgb_proceed
    },
    {
        .command = "hsv",
        .command_func = cli_functions_hsv_proceed
    },
    {
        .command = "help",
        .command_func = cli_functions_help_proceed
    }
};

static void s_define_command_(const char* command_str, const char* input, const size_t args_start_idx) {
    for (uint16_t com_idx = 0; com_idx < COMMAND_LIST_LEN; com_idx++) {
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