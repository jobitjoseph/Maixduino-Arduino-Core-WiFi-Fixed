#ifndef __SIPEED_OV2640_H
#define __SIPEED_OV2640_H

#include "Camera.h"
#include "sensor.h"


#define ON_CHIP_ID  (0x00)
#define OV_CHIP_ID  (0x0A)
#define OV_CHIP_ID2 (0x0B)
#define OV_CHIP_ID_16BIT  (0x300A)
#define OV_CHIP_ID2_16BIT (0x300B)

#define OV9650_ID       (0x96)
#define OV2640_ID       (0x2642)
#define OV5640_ID       (0x5640)
#define OV7725_ID       (0x77)
#define OV7740_ID       (0x7742)
#define OV3660_ID       (0x3660)
#define MT9V034_ID      (0x13)
#define LEPTON_ID       (0x54)


class Sipeed_OV2640 : public Camera{

public:
    Sipeed_OV2640(framesize_t frameSize = FRAMESIZE_QVGA, pixformat_t pixFormat = PIXFORMAT_RGB565);
    Sipeed_OV2640(uint16_t width, uint16_t height, pixformat_t pixFormat = PIXFORMAT_RGB565);
    ~Sipeed_OV2640();
    
    virtual bool begin();
    virtual void end();
    bool reset();
    bool setPixFormat(pixformat_t pixFormat);
    bool setFrameSize(framesize_t frameSize);
    virtual bool run(bool run);
    virtual int id();
    /**
     * @return pixels 
     *         If pixels format is RGB565: return RGB565 pixels with every uint16_t one pixel, e.g. RED: 0xF800
     */
    virtual uint8_t* snapshot();
    virtual uint16_t* getRGB565(){ return (uint16_t*)_dataBuffer; };
    virtual uint8_t* getRGB888(){ return _aiBuffer; };
    virtual void setRotation(uint8_t rotation);
    virtual void setInvert(bool invert);

private:
    uint8_t* _dataBuffer;    // put RGB565 data
    uint8_t* _aiBuffer;      // put RGB888 data
    uint8_t  _resetPoliraty; // reset poliraty flag
    uint8_t  _pwdnPoliraty;  // PWDN poliraty flag
    uint8_t  _slaveAddr;     // camera address
    uint16_t  _id;
    uint32_t _freq;
    sensor_t _sensor;

    int dvpInit(uint32_t freq = 24000000);
    int dvpInitIrq();
    
    int sensor_ov_detect();
    int sensro_gc_detect();
    int sensro_mt_detect(bool pwnd);

    int sensor_snapshot( );
    int reverse_u32pixel(uint32_t* addr,uint32_t length);

};

#endif

