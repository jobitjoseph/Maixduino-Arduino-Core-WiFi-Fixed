#ifndef __BITMAP_BUFFER_H
#define __BITMAP_BUFFER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "bmp/bm_alloc.h"
#include "bmp/bmpheader.h"

typedef struct {
  char FileHeader[BITMAP_FILEHEADER_SIZE];
  char Header[BITMAP_HEADER_SIZE];
  uint16_t Pixels[1];
} RGB565_BITMAP_T;

typedef struct {
  char _padding[6]; // avoid core dump: misaligned load
  RGB565_BITMAP_T bitmap;
} RGB565_DATA_T;

class Bitmap_Buffer {

public:
    Bitmap_Buffer();
    ~Bitmap_Buffer();
    
    virtual bool begin(uint16_t width, uint16_t height, uint8_t bpp = 16);
    virtual void end();
    bool reset();
    //bool setBitsPerPixel(uint8_t bpp);
    bool setFrameSize(uint16_t width, uint16_t height);
    /**
     * @return pixels 
     *         If pixels format is RGB565: return RGB565 pixels with every uint16_t one pixel, e.g. RED: 0xF800
     */
    virtual uint8_t* getPixelBuf();
    virtual uint16_t* getRGB565(){ return (uint16_t*)_dataBuffer; };
    virtual void setRotation(uint8_t rotation);
    virtual void setInvert(bool invert);

    // Swap the byte order for pushImage() - corrects endianness
    void swapBuffer();
    void setSwapBytes(bool swap);
    inline uint8_t *GetBitmapBuf(){ return (uint8_t*)&_data->bitmap; }
    inline size_t GetBitmapSize(){ return _data_size; }

private:
    int _width;
    int _height;
    RGB565_DATA_T* _data;
    uint32_t _data_size;
    uint8_t* _dataBuffer;    // put RGB565 data
    uint8_t  _resetPoliraty; // reset poliraty flag
    uint8_t  _pwdnPoliraty;  // PWDN poliraty flag
    uint8_t  _slaveAddr;     // camera address
    uint16_t  _id;
    uint32_t _freq;
    bool _swapBytes;

    int reverse_u16pixel(uint32_t* addr,uint32_t length);
    int reverse_u32pixel(uint32_t* addr,uint32_t length);
    bool setDataSize();

};

#endif
