#ifndef __GA_H_
#define __GA_H_

typedef unsigned char u8;
typedef unsigned short u16;


/* (P)ixels (P)er (B)yte as per Gate Array screen modes */
#define GET_PPB(mode)                              \
    (mode == 2 ? 8 :                               \
     mode == 1 ? 4 :                               \
     mode == 0 ? 2 : -1)

/* Gate Array pixel format mappings: */

/* Mode 2: */
/* bit7 bit6 bit5 bit4 bit3 bit2 bit1 bit0 */
/*  p0   p1   p2   p3   p4   p5   p6   p7 */

/* Mode 1: */
/* bit7  bit6  bit5  bit4  bit3  bit2  bit1  bit0 */
/* p0(1) p1(1) p2(1) p3(1) p0(0) p1(0) p2(0) p3(0) */

/* Mode 0: */
/* bit7  bit6  bit5  bit4  bit3  bit2  bit1  bit0 */
/* p0(0) p1(0) p0(2) p1(2) p0(1) p1(1) p0(3) p1(3) */


/* Returns mode appropriate byte format for the given ink color */
#define MODE_1_P0(c) (((c & 2) >> 1) << 3) | ((c & 1) << 7)
#define MODE_1_P1(c) (((c & 2) >> 1) << 2) | ((c & 1) << 6)
#define MODE_1_P2(c) (((c & 2) >> 1) << 1) | ((c & 1) << 5)
#define MODE_1_P3(c) (((c & 2) >> 1) << 0) | ((c & 1) << 4)

#define MODE_0_P0(c) ((c & 8) >> 2) | ((c & 4) << 3) | ((c & 2) << 2) | ((c & 1) << 7)
#define MODE_0_P1(c) ((c & 8) >> 3) | ((c & 4) << 2) | ((c & 2) << 1) | ((c & 1) << 6)

#define MODE_2_PF(c, offset) (c << (7 - offset))

/* Shorthand for mode appropriate byte formats per given pixel offset
   in byte */
#define MODE_1_PF(c, offset)                    \
    (offset == 0 ? MODE_1_P0(c) :               \
     offset == 1 ? MODE_1_P1(c) :               \
     offset == 2 ? MODE_1_P2(c) :               \
     offset == 3 ? MODE_1_P3(c) : -1)

#define MODE_0_PF(c, offset)                    \
    (offset == 0 ? MODE_0_P0(c) :               \
     offset == 1 ? MODE_0_P1(c) : -1)

/* Byte mask for the mode per given pixel offset in byte */
#define MODE_2_MASK(offset) MODE_2_PF(1, offset)
#define MODE_1_MASK(offset) MODE_1_PF(3, offset)
#define MODE_0_MASK(offset) MODE_0_PF(15, offset)

/* Mode appropriate conversion to ink color index for the given byte and offset */
#define MODE_2_INK(byte, offset) ((byte & MODE_2_MASK(offset)) >> (7 - offset))

#define MODE_1_INK_P0(b) (((b >> 3) & 1) << 1) | ((b >> 7) & 1)
#define MODE_1_INK_P1(b) (((b >> 2) & 1) << 1) | ((b >> 6) & 1)
#define MODE_1_INK_P2(b) (((b >> 1) & 1) << 1) | ((b >> 5) & 1)
#define MODE_1_INK_P3(b) (((b >> 0) & 1) << 1) | ((b >> 4) & 1)

#define MODE_0_INK_P0(b)                                                \
    ((b & 2) << 2) | ((b & 0x20) >> 3) | ((b & 8) >> 2) | ((b & 0x80) >> 7)
#define MODE_0_INK_P1(b)                                                \
    ((b & 1) << 3) | ((b & 0x10) >> 2) | ((b & 4) >> 1) | ((b & 0x40) >> 6)

#define MODE_1_INK(byte, offset)                \
    (offset == 0 ? MODE_1_INK_P0(byte) :        \
     offset == 1 ? MODE_1_INK_P1(byte) :        \
     offset == 2 ? MODE_1_INK_P2(byte) :        \
     offset == 3 ? MODE_1_INK_P3(byte) : -1)


#define MODE_0_INK(byte, offset)                \
    (offset == 0 ? MODE_0_INK_P0(byte) :        \
     offset == 1 ? MODE_0_INK_P1(byte) : -1)

u8 ga_find_gate_array_color_code(u8 r, u8 g, u8 b);
u8 ga_find_gate_array_firmware_color_code(u8 r, u8 g, u8 b);
unsigned int ga_convert_col_to_rgb(int col);

#endif
