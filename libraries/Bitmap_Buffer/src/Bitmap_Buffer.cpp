#include "Bitmap_Buffer.h"

//////////// HAL ///////////////
#include "sysctl.h"
#include "fpioa.h"
#include "sleep.h"
#include "stdlib.h"
#include "utils.h"
#include "plic.h"
#include "math.h"
#include "Arduino.h" // millis


Bitmap_Buffer::Bitmap_Buffer()
{
    _width = 0;
    _height = 0;
    _data = NULL;
    _data_size = 0;
    _dataBuffer = NULL;
}


Bitmap_Buffer::~Bitmap_Buffer()
{
    end();
}

bool Bitmap_Buffer::begin(uint16_t width, uint16_t height, uint8_t bpp)
{
    _width = width;
    _height = height;
    _data = NULL;
    _data_size = 0;
    _dataBuffer = NULL;

    setDataSize();

    //_dataBuffer = (uint8_t*)malloc(_width*_height*2); //RGB565
    if(!_dataBuffer)
    {
        _width = 0;
        _height = 0;
        return false;
    }
    if(!reset())
        return false;
    if(!setFrameSize(_width, _height))
        return false;
    return true;
}

void Bitmap_Buffer::end()
{
    if(_data)
        free(_data);
    _data = NULL;
}

bool Bitmap_Buffer::reset()
{
    return true;
}

bool Bitmap_Buffer::setDataSize()
{
    uint32_t _old_size = _data_size;
    _data_size = BITMAP_FILEHEADER_SIZE+BITMAP_HEADER_SIZE;
    _data_size += (_width*_height*2);

    if ((_old_size < _data_size) && _data) {
        free(_data);
	_data = NULL;
    }

    if (!_data)
        _data = (RGB565_DATA_T*)bm_malloc(6+_data_size);

    if (!_data) {
        _width = 0;
        _height = 0;
        _data_size = 0;
        return false;
    }

    bitmap_buf_fill_header(_data->bitmap.FileHeader, _data->bitmap.Header, _width, _height, 16);

    _dataBuffer = (uint8_t*)&_data->bitmap.Pixels[0];
    return true;
}

bool Bitmap_Buffer::setFrameSize(uint16_t width, uint16_t height)
{
    // Set framebuffer size
    _width = width;
    _height = height;

    setDataSize();

    return true;
}  

/**
 * @return pixels 
 *         If pixels format is RGB565: return RGB565 pixels with every uint16_t one pixel, e.g. RED: 0xF800
 */
uint8_t* Bitmap_Buffer::getPixelBuf()
{
    return _dataBuffer;
}

void Bitmap_Buffer::setRotation(uint8_t rotation)
{
    //FIXME
}

void Bitmap_Buffer::setInvert(bool invert)
{
    //FIXME
    return;
}

int Bitmap_Buffer::reverse_u16pixel(uint32_t* addr,uint32_t length)
{
  if(NULL == addr)
    return -1;

  uint32_t data;
  uint32_t* pend = addr+length;
  for(;addr<pend;addr++)
  {
          data = *(addr);
          *(addr) = ((data & 0x000000FF) << 8) | ((data & 0x0000FF00) >> 8) |
                ((data & 0x00FF0000) << 8) | ((data & 0xFF000000) >> 8) ;
  }  //1.7ms


  return 0;
}

int Bitmap_Buffer::reverse_u32pixel(uint32_t* addr,uint32_t length)
{
  if(NULL == addr)
    return -1;

  uint32_t data;
  uint32_t* pend = addr+length;
  for(;addr<pend;addr++)
  {
	  data = *(addr);
	  *(addr) = ((data & 0x000000FF) << 24) | ((data & 0x0000FF00) << 8) | 
                ((data & 0x00FF0000) >> 8) | ((data & 0xFF000000) >> 24) ;
  }  //1.7ms
  
  
  return 0;
}


void Bitmap_Buffer::swapBuffer() {
    reverse_u16pixel((uint32_t*)_dataBuffer, _width*_height/2);
}

/***************************************************************************************
** Function name:           setSwapBytes
** Description:             Used by 16 bit pushImage() to swap byte order in colours
***************************************************************************************/
void Bitmap_Buffer::setSwapBytes(bool swap) {
    _swapBytes = swap;
}
