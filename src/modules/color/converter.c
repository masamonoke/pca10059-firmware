#include "converter.h"

#include <math.h>
#include <stdlib.h>
#include "nrf_log.h"

#define CONVERTER_MAX(x, y) (((x) > (y)) ? (x) : (y))
#define CONVERTER_MIN(x, y) (((x) < (y)) ? (x) : (y))

//formulas from here https://www.rapidtables.com/convert/color/rgb-to-hsv.html
hsv_t converter_to_hsv_from_rgb(rgb_t rgb_data) {
    float r_ = (float)rgb_data.red / 255;
    float g_ = (float)rgb_data.green / 255;
    float b_ = (float)rgb_data.blue / 255;


    float c_max = CONVERTER_MAX(CONVERTER_MAX(r_, g_), b_);
    float c_min = CONVERTER_MIN(CONVERTER_MIN(r_, g_), b_);

    float delta = c_max - c_min;

    float hue = 0;
    if (delta == 0) {
        hue = 0;
    } else {
        if (r_ == c_max) {
            float tmp = (g_ - b_) / delta;
            tmp = tmp < 0 ? -tmp : tmp;
            tmp = fmod(tmp, 6.0);
            tmp *= 60;
            hue = tmp;
        } else if (g_ == c_max) {
            hue = (60 * ((b_ - r_) / delta + 2));
        }

        if (b_ == c_max) {
            hue = (60 * ((r_ - g_) / delta + 4));
        }
    }

    float saturation = c_max == 0 ? 0 : (delta / c_max) * 100;

    float value = c_max * 100;

    hsv_t hsv_data = {
            .hue = (uint16_t) ceilf(hue),
            .saturation = (uint8_t) ceilf(saturation),
            .value = (uint8_t) ceilf(value),
    };

    return hsv_data;
}

rgb_t converter_get_rgb_data(uint8_t r, uint8_t g, uint8_t b) {
    rgb_t rgb_data = {
            .red = r,
            .green = g,
            .blue = b
    };

    return rgb_data;
}

//formula from here https://www.rapidtables.com/convert/color/hsv-to-rgb.html
rgb_t converter_to_rgb_from_hsv(hsv_t hsv_data) {
    float c = hsv_data.value / 100.f * hsv_data.saturation / 100.f;
    float x = c * (1 - fabs(fmod(hsv_data.hue / 60.f, 2) - 1));
    float m = hsv_data.value / 100.f - c;

    uint16_t h = hsv_data.hue;
    float r_;
    float g_;
    float b_;
    if (0 <= h && h < 60) {
        r_ = c;
        g_ = x;
        b_ = 0;
    } else if (60 <= h && h < 120) {
        r_ = x;
        g_ = c;
        b_ = 0;
    } else if (120 <= h && h < 180) {
        r_ = 0;
        g_ = c;
        b_ = x;
    } else if (180 <= h && h < 240) {
        r_ = 0;
        g_ = x;
        b_ = c;
    } else if (240 <= h && h < 300) {
        r_ = x;
        g_ = 0;
        b_ = c;
    } else {
        r_ = c;
        g_ = 0;
        b_ = x;
    }

    rgb_t rgb_data = {
            .red = (uint8_t) ceilf((r_ + m) * 255),
            .green = (uint8_t) ceilf((g_ + m) * 255),
            .blue = (uint8_t) ceilf((b_ + m) * 255),
    };

    return rgb_data;
}

hsv_t converter_get_hsv_data(uint16_t h, uint8_t s, uint8_t v) {
    h = h > 360 ? 360 : h;
    s = s > 100 ? 100 : s;
    v = v > 100 ? 100 : v;

    hsv_t hsv_data = {
            .hue = h,
            .saturation = s,
            .value = v
    };

    return hsv_data;
}