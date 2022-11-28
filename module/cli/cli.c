#include "cli.h"
#include "module/io/led/nordic_rgb_pwm_utils.h"
#include "module/utils/string_utils.h"
#include "nrf_log.h"
#include "module/utils/math_utils.h"

//TODO: bug when type 00 instead of 0

static char s_message_[150];
static uint16_t s_len_;
static bool s_is_message_ = false;

static void s_clear_message_(void) {
    for (size_t i = 0; i < 100; i++) {
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

static void cli_functions_undefined_command(void) {
    cli_set_message("Undefined command\r\n\n", 20);
    NRF_LOG_INFO("Undefined command");
}

static bool s_get_args_(const char* input, uint16_t start_idx, uint16_t* args) {
    char s[12];
    string_utils_substring_to_end(input, start_idx + 1, s);
    return string_utils_parse_string_get_nums(s, args, 3);
}

static void s_prepare_help_message_(void) {
    cli_set_message("RGB <red> <green> <blue>\r\nHSV <hur> <saturation> <value>\r\nhelp\r\n\n", 65);
}

static void s_prepare_rgb_message_(uint32_t r, uint32_t g, uint32_t b) {
    s_clear_message_();
    char s[13] = "Color set to ";
    for (size_t i = 0; i < 13; i++) {
        s_message_[i] = s[i];
    }
    uint8_t len;
    char num[3];
    if (r / 100 == 0) {
        if (r / 10 == 0) {
            len = 1;
        } else {
            len = 2;
        }
    } else {
        len = 3;
    }
    sprintf(num, "%ld", r);
    uint8_t cur_idx = 13;
    s_message_[cur_idx++] = 'R';
    s_message_[cur_idx++] = '=';
    size_t k = 0;
    for (size_t i = cur_idx; i < cur_idx + len; i++, k++) {
        s_message_[i] = num[k];
    }
    cur_idx += len;
    s_message_[cur_idx++] = ' ';
    s_message_[cur_idx++] = 'G';
    s_message_[cur_idx++] = '=';
    if (g / 100 == 0) {
        if (g / 10 == 0) {
            len = 1;
        } else {
            len = 2;
        }
    } else {
        len = 3;
    }
    sprintf(num, "%ld", g);
    k = 0;
    for (size_t i = cur_idx; i < cur_idx + len; i++, k++) {
        s_message_[i] = num[k];
    }
    cur_idx += len;
    s_message_[cur_idx++] = ' ';
    s_message_[cur_idx++] = 'B';
    s_message_[cur_idx++] = '=';
    if (b / 100 == 0) {
        if (b / 10 == 0) {
            len = 1;
        } else {
            len = 2;
        }
    } else {
        len = 3;
    }
    sprintf(num, "%ld", b);
    k = 0;
    for (size_t i = cur_idx; i < cur_idx + len; i++, k++) {
        s_message_[i] = num[k];
    }
    cur_idx += len;

    s_message_[cur_idx++] = '\r';
    s_message_[cur_idx++] = '\n';
    s_message_[cur_idx++] = '\0';
    s_len_ = cur_idx;
    s_is_message_ = true;
}

static void s_prepare_hsv_message_(uint32_t h, uint32_t s, uint32_t v) {
    s_clear_message_();
    char str[13] = "Color set to ";
    for (size_t i = 0; i < 13; i++) {
        s_message_[i] = str[i];
    }
    uint8_t len;
    char num[3];
    if (h / 100 == 0) {
        if (h / 10 == 0) {
            len = 1;
        } else {
            len = 2;
        }
    } else {
        len = 3;
    }
    sprintf(num, "%ld", h);
    uint8_t cur_idx = 13;
    s_message_[cur_idx++] = 'H';
    s_message_[cur_idx++] = '=';
    size_t k = 0;
    for (size_t i = cur_idx; i < cur_idx + len; i++, k++) {
        s_message_[i] = num[k];
    }
    cur_idx += len;
    s_message_[cur_idx++] = ' ';
    s_message_[cur_idx++] = 'S';
    s_message_[cur_idx++] = '=';
    if (s / 100 == 0) {
        if (s / 10 == 0) {
            len = 1;
        } else {
            len = 2;
        }
    } else {
        len = 3;
    }
    sprintf(num, "%ld", s);
    k = 0;
    for (size_t i = cur_idx; i < cur_idx + len; i++, k++) {
        s_message_[i] = num[k];
    }
    cur_idx += len;
    s_message_[cur_idx++] = ' ';
    s_message_[cur_idx++] = 'V';
    s_message_[cur_idx++] = '=';
    if (v / 100 == 0) {
        if (v / 10 == 0) {
            len = 1;
        } else {
            len = 2;
        }
    } else {
        len = 3;
    }
    sprintf(num, "%ld", v);
    k = 0;
    for (size_t i = cur_idx; i < cur_idx + len; i++, k++) {
        s_message_[i] = num[k];
    }
    cur_idx += len;

    s_message_[cur_idx++] = '\r';
    s_message_[cur_idx++] = '\n';
    s_message_[cur_idx++] = '\0';
    s_len_ = cur_idx;
    s_is_message_ = true;
}

static bool cli_functions_rgb_proceed(const char* input, uint8_t i) {
    uint16_t args[3];
    if (!s_get_args_(input, i, args)) {
        cli_functions_undefined_command();
        return false;
    }
    args[0] = math_utils_clamp_int(args[0], 255, 0);
    args[1] = math_utils_clamp_int(args[1], 255, 0);
    args[2] = math_utils_clamp_int(args[2], 255, 0);
    uint32_t r = args[0];
    uint32_t g = args[1];
    uint32_t b = args[2];
    nordic_rgb_pwm_set_color(r, g, b);

    s_prepare_rgb_message_(r, g, b);
    NRF_LOG_INFO("Color set to R=%d G=%d B=%d", r, g, b);
    return true;
}

static bool cli_functions_hsv_proceed(const char* input, uint8_t i) {
    uint16_t args[3];
    if (!s_get_args_(input, i, args)) {
        cli_functions_undefined_command();
        return false;
    }

    args[0] = math_utils_clamp_int(args[0], 360, 0);
    args[1] = math_utils_clamp_int(args[1], 100, 0);
    args[2] = math_utils_clamp_int(args[2], 100, 0);

    uint16_t h = args[0];
    uint8_t s = args[1];
    uint8_t v = args[1];

    nordic_rgb_pwm_set_hsv_color(h, s, v);
    s_prepare_hsv_message_(h, s, v);
    NRF_LOG_INFO("Color set to H=%d S=%d V=%d", h, s, v);
    return true;
}

static void cli_functions_help_proceed(const char* input, uint8_t i) {
    uint16_t args[3];
    s_get_args_(input, i, args);
    if (args[0] != 0) {
        cli_functions_undefined_command();
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
        cli_functions_undefined_command();
        return;
    }
    command[i] = '\0';
    string_utils_to_lower_case(command);
    if (string_utils_compare_string(command, "rgb")) {
        if (!cli_functions_rgb_proceed(input, i)) {
            return;
        }
    } else if (string_utils_compare_string(command, "hsv")) {
        if (!cli_functions_hsv_proceed(input, i)) {
            return;
        }
    } else if (string_utils_compare_string(command, "help")) {
        cli_functions_help_proceed(input, i);
    } else {
        cli_functions_undefined_command();
        return;
    }
}