#include "cli.h"
#include "module/io/led/nordic_rgb_pwm_utils.h"
#include "module/utils/string_utils.h"
#include "nrf_log.h"
#include "module/utils/math_utils.h"

static void cli_functions_rgb_proceed(uint8_t r, uint8_t g, uint8_t b) {
    nordic_rgb_pwm_set_color(r, g, b);
    NRF_LOG_INFO("Color set to R=%d G=%d B=%d", r, g, b);
}

static void cli_functions_hsv_proceed(uint16_t h, uint8_t s, uint8_t v) {
    nordic_rgb_pwm_set_hsv_color(h, s, v);
    NRF_LOG_INFO("Color set to H=%d S=%d V=%d", h, s, v);
}

static void cli_functions_help_proceed(void) {
    NRF_LOG_INFO("RGB <red> <green> <blue>\nHSV <hur> <saturation> <value>\nhelp");
}

static void cli_functions_undefined_command(void) {
    NRF_LOG_INFO("Undefined command");
}

static bool s_get_args_(const char* input, uint16_t start_idx, uint16_t* args) {
    char s[12];
    string_utils_substring_to_end(input, start_idx + 1, s);
    return string_utils_parse_string_get_nums(s, args, 3);
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
        uint16_t args[3];
        if (!s_get_args_(input, i, args)) {
            cli_functions_undefined_command();
            return;
        }
        args[0] = math_utils_clamp_int(args[0], 255, 0);
        args[1] = math_utils_clamp_int(args[1], 255, 0);
        args[2] = math_utils_clamp_int(args[2], 255, 0);
        cli_functions_rgb_proceed(args[0], args[1], args[2]);
        return;
    } else if (string_utils_compare_string(command, "hsv")) {
        uint16_t args[3];
        if (!s_get_args_(input, i, args)) {
            cli_functions_undefined_command();
            return;
        }
        args[0] = math_utils_clamp_int(args[0], 360, 0);
        args[1] = math_utils_clamp_int(args[1], 100, 0);
        args[2] = math_utils_clamp_int(args[2], 100, 0);
        cli_functions_hsv_proceed(args[0], args[1], args[2]);
        return;
    } else if (string_utils_compare_string(command, "help")) {
        uint16_t args[3];
        s_get_args_(input, i, args);
        if (args[0] != 0) {
            cli_functions_undefined_command();
            return;
        }
        cli_functions_help_proceed();
        return;
    } else {
        cli_functions_undefined_command();
        return;
    }
}