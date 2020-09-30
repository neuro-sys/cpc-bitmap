#ifndef __CRTC_C
#define __CRTC_C

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

struct crtc_s {
    u8 R0; /* Horizontal Total */
    u8 R1; /* Horizontal Displayed */
    u8 R6; /* Vertical Displayed */
    u8 R9; /* Maximum Raster Address */
    u8 R12; /* Display Start Address (High) */
    u8 R13; /* Display Start Address (Low) */
};

void crtc_init(struct crtc_s regs, u16 **lines, int *line_counter);

#endif
