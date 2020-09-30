/*
 * Tool to take a gif file and convert it into the CPC screen format.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <gif_lib.h>
#include <errno.h>

#include "ga.h"

typedef unsigned char u8;
typedef unsigned short u16;

#define MASK_COL_INDEX 4

struct args_s {
    int mode;                    /* screen mode */
    int no_mask;                 /* 1 if mask data is to generate */
    int no_offsets;              /* 1 if offsetted sprites to be generated */
    char *inputfile;             /* input file argument */
};

struct gif_s {
    u8 *data;
    int color_count;
    GifColorType *colormap;
    int width;
    int height;

    GifFileType *_gif_file_type;
};

struct config_s {
    int ppb;                       /* pixels per byte for given mode */
    char filename[256];            /* output .bin file to create */
    char palname[256];             /* output .pal for palette data */
    char basename[256];            /* input file full path without extension  */
    char *basename_filename;       /* input file without path and extension */
    int buffer_size;
    u8 *buffer;                    /* in memory buffer to be used for data */
    int sub_byte_offset;           /* byte offset for the next offset buffer */
    int num_page;                  /* number of offset buffers */
    int mask_coef;                 /* 2 if there's mask, 1 if none */
};

void write_file(char *filename, u8* buffer, int buffer_size)
{
    FILE *file;

    file = fopen(filename, "wb");
    fwrite(buffer, sizeof(u8), buffer_size, file);
    fclose(file);
}

void write_palette(char *palname,
                   char *basename_filename,
                   GifColorType *colormap,
                   int color_count)
{
    FILE *file;
    int i;

    file = fopen(palname, "wb");

    fprintf(file, "pal_%s:     db ", basename_filename);
    for (i = 0; i < 16; i++) {
        u8 r = colormap[i].Red;
        u8 g = colormap[i].Green;
        u8 b = colormap[i].Blue;

        int c = i < color_count
            ? ga_find_gate_array_color_code(r, g, b)
            : 0x00;

        fprintf(file, "0x%x", c);

        if (i != 15) {
            fprintf(file, ", ");
        }
    }
    fprintf(file, "\n");

    printf("File %s is created.\n", palname);
}

void parse_args(int argc, char *argv[], struct args_s *args)
{
    int i;

    assert(args);

    args->mode = 1;
    args->no_mask = 0;
    args->no_offsets = 0;

    if (argc < 2) {
        printf("Usage: %s input.gif [--mode 1] [--no-mask] [--no-offsets]\n", argv[0]);
        printf("\n");
        printf("\t--no-mask\tDo not create interleaved mask data.\n");
        printf("\t--no-offsets\tDo not create byte offsets.\n");
        exit(0);
    }

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--mode") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Invalid arguments\n");
                exit(1);
            }

            args->mode = atoi(argv[i + 1]);
        }

        if (strcmp(argv[i], "--no-mask") == 0) {
            args->no_mask = 1;
        }

        if (strcmp(argv[i], "--no-offsets") == 0) {
            args->no_offsets = 1;
        }
    }

    args->inputfile = argv[1];
}

void gif_open(char *inputfile, struct gif_s *gif)
{
    int error_code;
    GifFileType *gif_file_type;

    assert(gif);

    memset(gif, 0, sizeof(*gif));

    gif_file_type = DGifOpenFileName(inputfile, &error_code);

    if (gif_file_type == NULL || error_code == E_GIF_ERR_OPEN_FAILED) {
        fprintf(stderr, "Could not open file: %s\n", inputfile);
        exit(1);
    }

    if (DGifSlurp(gif_file_type) == GIF_ERROR) {
        fprintf(stderr, "Unable to read gif file: %s\n", inputfile);
        exit(1);
    }


    gif->color_count = gif_file_type->SColorMap->ColorCount;
    gif->colormap = gif_file_type->SColorMap[0].Colors;

    gif->width = gif_file_type->SWidth;
    gif->height = gif_file_type->SHeight;

    gif->data = gif_file_type->SavedImages[0].RasterBits;

    gif->_gif_file_type = gif_file_type;
}

void gif_free(struct gif_s *gif)
{
    int error_code;

    DGifCloseFile(gif->_gif_file_type, &error_code);
}

void parse_config(struct config_s *config, struct args_s *args, struct gif_s *gif)
{
    int basename_len;

    assert(config);

    config->ppb = GET_PPB(args->mode);

    basename_len = strstr(args->inputfile, ".gif") - args->inputfile;

    sprintf(config->basename, "%.*s", basename_len, args->inputfile);

    config->basename_filename = config->basename;

    if (strrchr(config->basename, '/')) {
        config->basename_filename = strrchr(config->basename, '/') + 1;
    }

    if (strlen(config->basename_filename) > 8) {
        fprintf(stderr, "File base name cannot be longer than 8 characters: %s.\n", config->basename);
        exit(1);
    }

    sprintf(config->filename, "%s.bin", config->basename_filename);
    sprintf(config->palname, "%s.pal", config->basename_filename);

    config->buffer_size = gif->height * gif->width / config->ppb;

    if (!args->no_mask) {
        /* multiplied by 2 because we need mask data */
        config->buffer_size *= 2;
    }

    if (!args->no_offsets) {
        /* multiplied by ppb because we need a sprite for each sub-byte position */
        config->buffer_size *= config->ppb;
    }

    config->buffer = malloc(config->buffer_size);

    memset(config->buffer, 0, config->buffer_size);

    printf("width: %d, height: %d, color_count: %d\n",
           gif->width, gif->height, gif->color_count);

    /* For mask data we need twice the space */
    config->mask_coef = args->no_mask ? 1 : 2;

    /* If offsets included, how much to jump ahead for next offset buffer */
    config->sub_byte_offset = (gif->height * gif->width)
        / (config->ppb * config->mask_coef);

    /* Number of images for each offset */
    config->num_page = args->no_offsets ? 1 : config->ppb;
}

void config_free(struct config_s *config)
{
    assert(config);

    free(config->buffer);
}

void render(int width,
            int height,
            int mode,
            int num_page,
            int ppb,
            int sub_byte_offset,
            int no_mask,
            int mask_coef,
            u8 *data,
            u8 *buffer)
{
    int y, x, k;

    /* For each offset image */
    for (k = 0; k < num_page; k++) {
        for (y = 0; y < height; y++) {
            for (x = 0; x < width; x++) {
                int c = (x - k) < 0 ? MASK_COL_INDEX : data[y * width + x - k];
                int mask_it = c == MASK_COL_INDEX;
                int offset = x % ppb;
                int page = sub_byte_offset * k;
                int scanline_len;
                int x_byte_offset;
                u8 *pixel_addr;
                u8 *mask_addr;

                if (!no_mask) {
                    scanline_len = width / ppb * mask_coef;
                    x_byte_offset = x / ppb * mask_coef;
                    /* Interlace sprite with masked data one byte interleaved */
                    pixel_addr = &buffer[y * scanline_len + (x_byte_offset + 1) + page];
                    mask_addr = &buffer[y * scanline_len + (x_byte_offset + 0) + page];
                } else {
                    scanline_len = width / ppb;
                    x_byte_offset = x / ppb;
                    pixel_addr = &buffer[y * scanline_len + x_byte_offset + page];
                }

                if (mode == 2) {
                    int m = 1; /* mask color */

                    *pixel_addr |= MODE_2_PF(c, offset);
                    if (!no_mask && mask_it) {
                        *mask_addr |= MODE_2_PF(m, offset);
                    }
                } else if (mode == 1) {
                    int m = 3; /* mask color */

                    *pixel_addr |= MODE_1_PF(c, offset);
                    if (!no_mask && mask_it) {
                        *mask_addr |= MODE_1_PF(m, offset);
                    }
                } else if (mode == 0) {
                    int m = 15; /* mask color */

                    *pixel_addr |= MODE_0_PF(c, offset);
                    if (!no_mask && mask_it) {
                        *mask_addr |= MODE_0_PF(m, offset);
                    }
                }
            }
        }
    }
}

int main(int argc, char *argv[])
{
    struct args_s args;
    struct gif_s gif;
    struct config_s config;

    parse_args(argc, argv, &args);

    gif_open(args.inputfile, &gif);

    parse_config(&config, &args, &gif);

    render(gif.width,
           gif.height,
           args.mode,
           config.num_page,
           config.ppb,
           config.sub_byte_offset,
           args.no_mask,
           config.mask_coef,
           gif.data,
           config.buffer);

    write_file(config.filename, config.buffer, config.buffer_size);

    write_palette(config.palname, config.basename_filename, gif.colormap, gif.color_count);

    config_free(&config);

    gif_free(&gif);

    return 0;
}
