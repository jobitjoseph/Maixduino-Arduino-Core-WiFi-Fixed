#ifndef _BMPHEADER_H
#define _BMPHEADER_H

#include <stdint.h>
#include <string.h>

#define BITMAP_FILEHEADER_SIZE 14

#define BITMAP_HEADER_SIZE 40+68

void bitmap_buf_fill_header(char* bfh,  char* bh, unsigned int w, unsigned int h, unsigned short int bits);

unsigned int bitmap_buf_fileheader_size(char* bfh);

#ifdef __cplusplus
extern "C" {
#endif

void bitmap_buf_swap_left_right_16bpp(uint16_t *pixBuf, uint16_t w, uint16_t h);
void bitmap_buf_swap_top_down_16bpp(uint16_t *pixBuf, uint16_t w, uint16_t h);
void bitmap_buf_copy_rect_16bpp(uint8_t*src, uint32_t src_width, uint32_t src_height,
        uint8_t*dst, uint32_t dst_x, uint32_t dst_y, uint32_t dst_width, uint32_t dst_height);

#ifdef __cplusplus
}
#endif
#endif
