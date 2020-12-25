#ifndef __CAMERA_H
#define __CAMERA_H

#include <stdint.h>
#include <stdbool.h>
#include "sensor.h"


class Camera{

public:
    Camera(framesize_t frameSize, pixformat_t pixFormat);
    Camera(int16_t width, uint16_t height, pixformat_t pixFormat);
    ~Camera();
    virtual bool begin( ) = 0;
    virtual void end() = 0;
    // virtual bool reset() = 0;
    // virtual bool setPixFormat(int pixFormat) = 0;
    // virtual bool setFrameSize(int frameSize) = 0;
    virtual bool run(bool run) = 0;
    virtual int id() = 0;
    /**
     * @return pixels 
     *         If pixels format is RGB565: return RGB565 pixels with every uint16_t one pixel, e.g. RED: 0xF800
     */
    virtual uint8_t* snapshot() = 0;
    /**
     * @return pixels with RGB565 format,  every uint16_t one pixel, e.g. RED: 0xF800, so two pixels: {0xF800, 0xF800}
     */
    virtual uint16_t* getRGB565(){ return nullptr; };
    /**
     * 
     * @return pixels with RGB888 format, for n pixels: {{R0,R1,...,Rn-1,},{G0,G1,...,Gn-1},{B0,B1,...,Bn-1}}
     *                                    e.g. two RED pixel: {0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00}
     */
    virtual uint8_t* getRGB888(){ return nullptr; };
    virtual void setRotation(uint8_t rotation) = 0;
    virtual void setInvert(bool invert) = 0;


    virtual int width(){ return _width; }
    virtual int height(){ return _height; }

protected:
    pixformat_t _pixFormat;
    framesize_t _frameSize;
    int _width;
    int _height;
 };



#endif

