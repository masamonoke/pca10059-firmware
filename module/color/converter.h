#ifndef MODULE_COLOR_CONVERTER
#define MODULE_COLOR_CONVERTER

#include <stdint.h>

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} rgb_t;

typedef struct {
    uint8_t saturation;
    uint8_t value;
    uint16_t hue;
} hsv_t;

rgb_t converter_get_rgb_data(uint8_t r, uint8_t g, uint8_t b);

rgb_t converter_to_rgb_from_hsv(hsv_t hsv_data);

hsv_t converter_to_hsv_from_rgb(rgb_t rgb_data);

hsv_t converter_get_hsv_data(uint16_t h, uint8_t s, uint8_t v);

#endif /* MODULE_COLOR_CONVERTER */
