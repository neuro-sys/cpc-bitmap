#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <gif_lib.h>
#include <errno.h>

#include "ga.h"

#include "crtc.h"

struct crtc_s regs = { 63, 40, 25, 7, 0x0c, 00 };

void parse_num(char *str, unsigned char *n)
{
    errno = 0;

    assert(str);
    assert(n);

    if (strchr(str, 'x') || strchr(str, '&')) {
        sscanf(str, "%x", n);
    } else {
        sscanf(str, "%d", n);
    }

    if (errno) {
        fprintf(stderr, "%s it not a number", str);
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    FILE *file;
    GifFileType *gif_file_type;
    GifColorType *colormap;
    int width;
    int height;
    int y, x;
    int i, j;
    u8 *buffer;
    int basename_len;
    char basename[256];
    char *basename_begin;
    char filename[256];
    char filename1[15];
    char filename2[15];
    char palname[256];
    char pabname[256]; /* This is a binary file containing palette ink numbers */
    u16 *lines;
    u8 *data;
    int line_counter;
    int error_code;
    int color_count;
    int mode;
    int ppb;
    int two_files;
    int total_address_space;
    u8 palette[16][2]; /* 0: hardware number, 1: firmware number */

    two_files = 0;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s input.gif [--mode 1] [--crtc (R0) (R1) (R6) (R9) (R12) (R13)] [-2]\n", argv[0]);
        exit(1);
    }

    mode = 1;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-2") == 0) {
            two_files = 1;
        }

        if (strcmp(argv[i], "--mode") == 0) {
            u8 n;

            if (i + 1 >= argc) {
                fprintf(stderr, "Invalid arguments\n");
                exit(1);
            }

            parse_num(argv[i + 1], &n);
            mode = n;
        }

        if (strcmp(argv[i], "--crtc") == 0) {
            if (i + 6 >= argc) {
                fprintf(stderr, "Invalid arguments\n");
                exit(1);
            }

            parse_num(argv[i + 1], &regs.R0);
            parse_num(argv[i + 2], &regs.R1);
            parse_num(argv[i + 3], &regs.R6);
            parse_num(argv[i + 4], &regs.R9);
            parse_num(argv[i + 5], &regs.R12);
            parse_num(argv[i + 6], &regs.R13);
        }
    }

    ppb = GET_PPB(mode);

    printf("R0: %d, R1: %d, R6: %d, R9: %d, R12: %d, R13: %d\n",
           regs.R0, regs.R1, regs.R6, regs.R9, regs.R12, regs.R13);

    printf("Mode %d\n", mode);

    if (!strstr(argv[1], ".gif")) {
        fprintf(stderr, "File should have .gif extension.\n");
        exit(1);
    }

    basename_len = strstr(argv[1], ".gif") - argv[1];

    sprintf(basename, "%.*s", basename_len, argv[1]);

    basename_begin = basename;

    if (strrchr(basename, '/')) {
        basename_begin = strrchr(basename, '/') + 1;
    }

    sprintf(palname, "%s.pal", basename_begin);
    sprintf(pabname, "%s.pab", basename_begin);

    if (strlen(basename_begin) > 8) {
        fprintf(stderr, "File base name cannot be longer than 8 characters: %s.\n", basename);
        exit(1);
    }

    if (two_files) {
        sprintf(filename1, "%s1.bin", basename_begin);
        sprintf(filename2, "%s2.bin", basename_begin);
    } else {
        sprintf(filename, "%s.bin", basename_begin);
    }

    gif_file_type = DGifOpenFileName(argv[1], &error_code);

    if (gif_file_type == NULL) {
        fprintf(stderr, "Could not open file: %s\n", argv[1]);
        exit(1);
    }

    if (DGifSlurp(gif_file_type) == GIF_ERROR) {
        fprintf(stderr, "Unable to read gif file: %s\n", argv[1]);
        exit(1);
    }

    color_count = gif_file_type->SColorMap->ColorCount;
    colormap = gif_file_type->SColorMap[0].Colors;

    width = gif_file_type->SWidth;
    height = gif_file_type->SHeight;

    data = gif_file_type->SavedImages[0].RasterBits;

    crtc_init(regs, &lines, &line_counter);

    printf("width: %d, height: %d, color_count: %d\n", width, height, color_count);

    if (height - 1 < 0) {
        fprintf(stderr, "Invalid data\n");
        exit(1);
    }

    printf("%.4x\n", lines[height - 1]);

    total_address_space = lines[height - 1] + (regs.R1 * 2);
    buffer = malloc(total_address_space);
    memset(buffer, 0, total_address_space);

    printf("total_address_space: %d (0x%.4x)\n", total_address_space, total_address_space);

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            const int c = data[y * width + x];
            const int offset = x % ppb;
            u8 *pixel_addr = &buffer[lines[y] + (x / ppb)];

            if (mode == 2) {
                *pixel_addr |= MODE_2_PF(c, offset);
            } else if (mode == 1) {
                *pixel_addr |= MODE_1_PF(c, offset);
            } else if (mode == 0) {
                *pixel_addr |= MODE_0_PF(c, offset);
            }
        }
    }

    if (two_files) {
        file = fopen(filename1, "wb");
        fwrite(buffer, sizeof(u8), total_address_space / 2, file);
        fclose(file);
        printf("File %s is created.\n", filename1);

        file = fopen(filename2, "wb");
        fwrite(buffer + total_address_space / 2, sizeof(u8), total_address_space / 2, file);
        fclose(file);
        printf("File %s is created.\n", filename2);
    } else {
        file = fopen(filename, "wb");
        fwrite(buffer, sizeof(u8), total_address_space, file);
        fclose(file);
        printf("File %s is created.\n", filename);
    }

    for (i = 0; i < 16; i++) {
        u8 r = colormap[i].Red;
        u8 g = colormap[i].Green;
        u8 b = colormap[i].Blue;

        int c = i < color_count
            ? ga_find_gate_array_color_code(r, g, b)
            : 0x00;

        int fc = i < color_count
            ? ga_find_gate_array_firmware_color_code(r, g, b)
            : 0x00;

        palette[i][0] = c;
        palette[i][1] = fc;
    }

    /* Print palette */
    file = fopen(palname, "wb");

    fprintf(file, "pal_%s:     db ", basename_begin);

    for (i = 0; i < 16; i++) {
        fprintf(file, "0x%x", palette[i][0]);

        if (i != 15) {
            fprintf(file, ", ");
        }
    }

    fprintf(file, "\n");
    fclose(file);
    printf("File %s is created.\n", palname);

    file = fopen(pabname, "wb");
    for (i = 0; i < 16; i++) {
        u8 c = palette[i][1];
        fwrite(&c, 1, 1, file);
    }
    fclose(file);
    printf("File %s is created.\n", pabname);

    free(lines);
    free(buffer);
    DGifCloseFile(gif_file_type, &error_code);

    return 0;
}
