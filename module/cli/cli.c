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
    size_t k = 0;
    uint8_t space_counts = 0;
    while (s[k] != '\0') {
        //TODO: fix space bug
        //workaround of e.g. rgb 123 123 123 there is space at the end but rgb 123 5 123 there isn't
        if (s[k] == ' ' && s[k + 1] != '\0') {
            space_counts++;
        }
        k++;
    }
    
    if (space_counts + 1 != args_count) {
        return false;
    }

    return string_utils_parse_string_get_nums(s, args, 3);
}

static void s_prepare_help_message_(void) {
    cli_set_message("RGB <red> <green> <blue>\r\nHSV <hur> <saturation> <value>\r\nhelp\r\n", 64);
}

static uint8_t s_get_num_len_(uint16_t num) {
    uint8_t len = 0;
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

    for (size_t z = 0; z < vals_count; z++) {
        len = s_get_num_len_(vals[z]);
        sprintf(num, "%u", vals[z]);
        s_message_[cur_idx++] = chars[z];
        s_message_[cur_idx++] = '=';
        size_t k = 0;
        for (size_t i = cur_idx; i < cur_idx + len; i++, k++) {
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
        s_cli_functions_undefined_command_();
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
        s_cli_functions_undefined_command_();
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

static void cli_functions_help_proceed(const char* input, uint8_t args_start_idx) {
    if (input[args_start_idx] != '\0') {
        s_cli_functions_undefined_command_();
        return;
    }
    s_prepare_help_message_();
    NRF_LOG_INFO("\nRGB <red> <green> <blue>\nHSV <hur> <saturation> <value>\nhelp");
}

void cli_proceed(char* input) {
    size_t i = 0;
    char command[10];
    while (input[i] != ' ' && input[i] != '\0') {
        command[i] = input[i];
        i++;
    }

    if (i == 0) {
        return;
    }

    command[i] = '\0';
    string_utils_to_lower_case(command);

    if (string_utils_compare_string(command, "rgb")) {
        cli_functions_rgb_proceed(input, i);
    } else if (string_utils_compare_string(command, "hsv")) {
        cli_functions_hsv_proceed(input, i);
    } else if (string_utils_compare_string(command, "help")) {
        cli_functions_help_proceed(input, i);
    } else {
        s_cli_functions_undefined_command_();
    }
}