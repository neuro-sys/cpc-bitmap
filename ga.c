#include <assert.h>
#include <stdio.h>

#include "ga.h"

struct color_code_s {
    u8 rgb[3];
    u8 GA_code;
};

/*
  Palette sorted by Firmware number

  Hardware Number  Colour Name    R % G % B % Hexadecimal RGB values
  54h              Black          0   0   0   #000000     0/0/0
  44h (or 50h)     Blue           0   0   50  #000080     0/0/128
  55h              Bright Blue    0   0   100 #0000FF     0/0/255
  5Ch              Red            50  0   0   #800000     128/0/0
  58h              Magenta        50  0   50  #800080     128/0/128
  5Dh              Mauve          50  0   100 #8000FF     128/0/255
  4Ch              Bright Red     100 0   0   #FF0000     255/0/0
  45h (or 48h)     Purple         100 0   50  #ff0080     255/0/128
  4Dh              Bright Magenta 100 0   100 #FF00FF     255/0/255
  56h              Green          0   50  0   #008000     0/128/0
  46h              Cyan           0   50  50  #008080     0/128/128
  57h              Sky Blue       0   50  100 #0080FF     0/128/255
  5Eh              Yellow         50  50  0   #808000     128/128/0
  40h (or 41h)     White          50  50  50  #808080     128/128/128
  5Fh              Pastel Blue    50  50  100 #8080FF     128/128/255
  4Eh              Orange         100 50  0   #FF8000     255/128/0
  47h              Pink           100 50  50  #FF8080     255/128/128
  4Fh              Pastel Magenta 100 50  100 #FF80FF     255/128/255
  52h              Bright Green   0   100 0   #00FF00     0/255/0
  42h (or 51h)     Sea Green      0   100 50  #00FF80     0/255/128
  53h              Bright Cyan    0   100 100 #00FFFF     0/255/255
  5Ah              Lime           50  100 0   #80FF00     128/255/0
  59h              Pastel Green   50  100 50  #80FF80     128/255/128
  5Bh              Pastel Cyan    50  100 100 #80FFFF     128/255/255
  4Ah              Bright Yellow  100 100 0   #FFFF00     255/255/0
  43h (or 49h)     Pastel Yellow  100 100 50  #FFFF80     255/255/128
  4Bh              Bright White   100 100 100 #FFFFFF     255/255/255
*/

struct color_code_s color_mapping[27] = {
                                         { .rgb = { 0x00, 0x00, 0x00 }, .GA_code = 0x54 },
                                         { .rgb = { 0x00, 0x00, 0x80 }, .GA_code = 0x44 },
                                         { .rgb = { 0x00, 0x00, 0xFF }, .GA_code = 0x55 },
                                         { .rgb = { 0x80, 0x00, 0x00 }, .GA_code = 0x5C },
                                         { .rgb = { 0x80, 0x00, 0x80 }, .GA_code = 0x58 },
                                         { .rgb = { 0x80, 0x00, 0xFF }, .GA_code = 0x5D },
                                         { .rgb = { 0xFF, 0x00, 0x00 }, .GA_code = 0x4C },
                                         { .rgb = { 0xFF, 0x00, 0x80 }, .GA_code = 0x45 },
                                         { .rgb = { 0xFF, 0x00, 0xFF }, .GA_code = 0x4D },
                                         { .rgb = { 0x00, 0x80, 0x00 }, .GA_code = 0x56 },
                                         { .rgb = { 0x00, 0x80, 0x80 }, .GA_code = 0x46 },
                                         { .rgb = { 0x00, 0x80, 0xFF }, .GA_code = 0x57 },
                                         { .rgb = { 0x80, 0x80, 0x00 }, .GA_code = 0x5E },
                                         { .rgb = { 0x80, 0x80, 0x80 }, .GA_code = 0x40 },
                                         { .rgb = { 0x80, 0x80, 0xFF }, .GA_code = 0x5F },
                                         { .rgb = { 0xFF, 0x80, 0x00 }, .GA_code = 0x4E },
                                         { .rgb = { 0xFF, 0x80, 0x80 }, .GA_code = 0x47 },
                                         { .rgb = { 0xFF, 0x80, 0xFF }, .GA_code = 0x4F },
                                         { .rgb = { 0x00, 0xFF, 0x00 }, .GA_code = 0x52 },
                                         { .rgb = { 0x00, 0xFF, 0x80 }, .GA_code = 0x42 },
                                         { .rgb = { 0x00, 0xFF, 0xFF }, .GA_code = 0x53 },
                                         { .rgb = { 0x80, 0xFF, 0x00 }, .GA_code = 0x5A },
                                         { .rgb = { 0x80, 0xFF, 0x80 }, .GA_code = 0x59 },
                                         { .rgb = { 0x80, 0xFF, 0xFF }, .GA_code = 0x5B },
                                         { .rgb = { 0xFF, 0xFF, 0x00 }, .GA_code = 0x4A },
                                         { .rgb = { 0xFF, 0xFF, 0x80 }, .GA_code = 0x43 },
                                         { .rgb = { 0xFF, 0xFF, 0xFF }, .GA_code = 0x4B },
};

u8 ga_find_gate_array_color_code(u8 r, u8 g, u8 b)
{
    int i;
    int len;

    len = sizeof(color_mapping) / sizeof(color_mapping[0]);

    for (i = 0; i < len; i++) {
        struct color_code_s c = color_mapping[i];

        if (c.rgb[0] == r && c.rgb[1] == g && c.rgb[2] == b) {
            return c.GA_code;
        }
    }

    fprintf(stderr, "Color not found: %.2x %.2x %.2x\n", r, g, b);
    assert(0);
}

u8 ga_find_gate_array_firmware_color_code(u8 r, u8 g, u8 b)
{
    int i;
    int len;

    len = sizeof(color_mapping) / sizeof(color_mapping[0]);

    for (i = 0; i < len; i++) {
        struct color_code_s c = color_mapping[i];

        if (c.rgb[0] == r && c.rgb[1] == g && c.rgb[2] == b) {
            return i;
        }
    }

    fprintf(stderr, "Color not found: %.2x %.2x %.2x\n", r, g, b);
    assert(0);
}

unsigned int ga_convert_col_to_rgb(int col)
{
    int i;
    int len;
    unsigned int rgb;
    struct color_code_s c;

    c = color_mapping[col];

    rgb = 0;

    rgb |= c.rgb[0] << 16;
    rgb |= c.rgb[1] <<  8;
    rgb |= c.rgb[2] <<  0;

    return rgb;
}

#ifdef TEST
int main(int argc, char *argv[])
{
    int c, offset;

    /* Test mode 0 two-way conversion */
    for (offset = 0; offset < 2; offset++) {
        for (c = 0; c < 16; c++) {
            assert(MODE_0_INK(MODE_0_PF(c, offset), offset) == c);
            printf("MODE_0_P%d_%d equ 0x%.2X\n", offset, c, MODE_0_PF(c, offset));
        }
        printf("MODE_0_P%d equ 0x%.2X\n", offset, MODE_0_MASK(offset));
    }

    /* Test mode 1 two-way conversion */
    for (offset = 0; offset < 4; offset++) {
        for (c = 0; c < 4; c++) {
            assert(MODE_1_INK(MODE_1_PF(c, offset), offset) == c);
            printf("MODE_1_P%d_%d equ 0x%.2X\n", offset, c, MODE_1_PF(c, offset));
        }
        printf("MODE_1_P%d equ 0x%.2X\n", offset, MODE_1_MASK(offset));
    }

    /* Test mode 2 two-way conversion */
    for (offset = 0; offset < 8; offset++) {
        for (c = 0; c < 1; c++) {
            assert(MODE_2_INK(MODE_2_PF(c, offset), offset) == c);
            printf("MODE_2_P%d_%d equ 0x%.2X\n", offset, c, MODE_2_PF(c, offset));
        }
        printf("MODE_2_P%d equ 0x%.2X\n", offset, MODE_2_MASK(offset));
    }

    return 0;
}

#endif
