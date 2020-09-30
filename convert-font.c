/*
 * Converts a gif file that contains a tile map into a vertical column
 * for the given cell_width and cell_height.
 *
 * Compile with cc -Wpedantic -std=c89 ../utils/convert-font.c -lgif -oconvert-font
 *
 * The target layout is useful for the CPC renderer.
 */

#include <stdlib.h>
#include <stdio.h>
#include <gif_lib.h>
#include <errno.h>
#include <assert.h>
#include <string.h>

typedef unsigned char u8;

void parse_u8(char *str, int *n)
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

void blit_rect(u8 *src, u8 *dst,
               int x1, int y1, int x2, int y2,
               int width, int height, int stride_src, int stride_dst)
{
  int line;

  for (line = 0; line < height; line++) {
    int offset_src;
    int offset_dst;

    offset_src = (y1 + line) * stride_src + x1;
    offset_dst = (y2 + line) * stride_dst + x2;

    memcpy(dst + offset_dst, src + offset_src, width);
  }
}

int main(int argc, char *argv[])
{
  GifFileType *input_gif;
  GifFileType *output_gif;
  u8 *input_data;
  int width;
  int height;
  int cell_width;
  int cell_height;
  int col_num;
  int row_num;
  int target_width;
  int target_height;
  int error_code;
  ColorMapObject *color_map_object;

  if (argc < 5) {
    printf("Usage: %s input.gif output.gif <cell_width> <cell_height>\n", argv[0]);
    return 0;
  }

  input_gif = DGifOpenFileName(argv[1], &error_code);

  if (input_gif == NULL) {
    fprintf(stderr, "Could not open file: %s\n", argv[1]);
    exit(1);
  }

  if (DGifSlurp(input_gif) == GIF_ERROR) {
    fprintf(stderr, "Unable to read gif file: %s\n", argv[1]);
    exit(1);
  }

  input_data = input_gif->SavedImages[0].RasterBits;

  parse_u8(argv[3], &cell_width);
  parse_u8(argv[4], &cell_height);

  width = input_gif->SWidth;
  height = input_gif->SHeight;

  col_num = width / cell_width;
  row_num = height / cell_height;

  target_width = cell_width;
  target_height = col_num * row_num * cell_width * cell_height;

  output_gif = EGifOpenFileName(argv[2], 0, &error_code);

  if (output_gif == NULL) {
    fprintf(stderr, "Could not open file: %s\n", argv[2]);
    exit(1);
  }

  color_map_object = input_gif->SColorMap;

  if (EGifPutScreenDesc(output_gif, cell_width,
                        col_num * row_num * cell_height,
                        input_gif->SColorResolution, 0,
                        color_map_object) == GIF_ERROR) {
    fprintf(stderr, "Failed to write screen desc\n");
    exit(1);
  }

  if (EGifPutImageDesc(output_gif, 0, 0, cell_width,
                       col_num * row_num * cell_height, 0,
                        color_map_object) == GIF_ERROR) {
    fprintf(stderr, "Failed to Put screen desc\n");
    exit(1);
  }

  GifMakeSavedImage(output_gif, &input_gif->SavedImages[0]);

  {
    u8 *src = input_gif->SavedImages[0].RasterBits;
    u8 *dst = malloc(width * height);
    int x, y, dest_y;

    dest_y = 0;

    for (y = 0; y < row_num; y++) {
      for (x = 0; x < col_num; x++) {
        int x1, y1, x2, y2;

        x1 = x * cell_width;
        y1 = y * cell_height;

        x2 = 0;
        y2 = dest_y * cell_width;

        blit_rect(src, dst,
                  x1, y1, x2, y2,
                  cell_width, cell_width, /* Remove the extra space, now it's 32x32 */
                  width, cell_width); /* src stride, dst stride */

        /* Rotate the target such that a horizontal line becomes a */
        /* vertical column. This will help when rendering a vertical */
        /* column on CPC as required by the sprite renderer. */

        /* This can be done by 90 degrees rotation and inverse matrices */
        /* Or with two loops */
        /* { */
        /*   int i, j; */
        /*   u8 *dst1; */
        /*   u8 *buffer; */

        /*   buffer = malloc(cell_width * cell_width); */
        /*   dst1 = dst + cell_width * dest_y * cell_width; */

        /*   for (j = 0; j < cell_width; j++) { */
        /*     for (i = 0; i < cell_width; i += 2) { */
        /*       buffer[(i + 0) * cell_width + j] = dst1[j * cell_width + i + 0]; */
        /*       buffer[(i + 1) * cell_width + j] = dst1[j * cell_width + i + 1]; */
        /*     } */
        /*   } */

        /*   memcpy(dst1, buffer, cell_width * cell_width); */

        /*   free(buffer); */
        /* } */
        dest_y += 1;
      }
    }

    for (y = 0; y < target_height; y++) {
      EGifPutLine(output_gif, dst + y * cell_width, target_width);
    }

    free(dst);
  }

  DGifCloseFile(input_gif, &error_code);
  EGifCloseFile(output_gif, &error_code);

  return 0;
}
