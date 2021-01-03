#include "bmpheader.h"

void bitmap_buf_swap_left_right_16bpp(uint16_t *pixBuf, uint16_t w, uint16_t h)
{
    uint32_t len = 0;  
    while(len<w*h) {
        for (int i = 0; i < w/2; i++) {
            uint16_t tmp = pixBuf[i];
            pixBuf[i] = pixBuf[w-i-1];
            pixBuf[w-i-1] = tmp;
        }
        pixBuf+=w;
        len+=w;
    }
}

void bitmap_buf_swap_top_down_16bpp(uint16_t *pixBuf, uint16_t w, uint16_t h)
{
    uint32_t len = 0;  
    uint16_t *botBuf=pixBuf;
    botBuf+=w*h;
    while(len<w*h/2) {
        botBuf-=w;
        for (int i = 0; i < w; i++) {
            uint16_t tmp = pixBuf[i];
            pixBuf[i] = botBuf[i];
            botBuf[i] = tmp;
        }
        pixBuf+=w;
        len+=w;
    }
}

void bitmap_buf_copy_rect_16bpp(uint8_t*src, uint32_t src_width, uint32_t src_height,
        uint8_t*dst, uint32_t dst_x, uint32_t dst_y, uint32_t dst_width, uint32_t dst_height)
{
    uint32_t len=0;
    src+=src_width*dst_y*2;
    while(len<dst_width*dst_height){
        src+=dst_x*2;
        memcpy(dst,src,dst_width*2);
        src+=(src_width-dst_x)*2;
        dst+=dst_width*2;
        len+=dst_width;
    }
}
