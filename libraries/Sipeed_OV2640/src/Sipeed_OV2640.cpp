
#include "Sipeed_OV2640.h"

//////////// HAL ///////////////
#include "sysctl.h"
#include "fpioa.h"
#include "dvp.h"
#include "sleep.h"
#include "ov2640_regs.h"
#include "stdlib.h"
#include "utils.h"
#include "plic.h"
#include "math.h"
#include "cambus.h"
#include "gc0328.h"
#include "gc2145.h"
#include "ov2640.h"
#include "mt9d111.h"
#include "ov3660.h"
#include "ov5640.h"
#include "ov7740.h"
#include "Arduino.h" // millis

volatile static uint8_t g_dvp_finish_flag = 0;

struct sensor_config_t {
    uint8_t cmos_pclk;
    uint8_t cmos_xclk;
    uint8_t cmos_href;
    uint8_t cmos_pwdn;
    uint8_t cmos_vsync;
    uint8_t cmos_rst;

    uint8_t reg_width;
    uint8_t i2c_num;
    uint8_t pin_clk;
    uint8_t pin_sda;
    uint8_t gpio_clk;
    uint8_t gpio_sda;
};

/* 8 or 16 */
#define SENSOR_REG_WIDTH 16
/* 2 or -2 */
#define SENSOR_I2C_NUM   2

#define SENSOR_PINS_SWAPPED

#ifdef SENSOR_PINS_SWAPPED
static struct sensor_config_t sensor_config = {
    47, 46, 45, 44, 43, 42,
    SENSOR_REG_WIDTH, SENSOR_I2C_NUM, 41, 40, 0, 0
};
#else
static struct sensor_config_t sensor_config = {
    40, 41, 42, 43, 44, 45,
    SENSOR_REG_WIDTH, SENSOR_I2C_NUM, 46, 47, 0, 0
};
#endif


#define DCMI_RESET_LOW()      dvp->cmos_cfg &= ~DVP_CMOS_RESET
#define DCMI_RESET_HIGH()     dvp->cmos_cfg |= DVP_CMOS_RESET
#define DCMI_PWDN_LOW()       dvp->cmos_cfg |= DVP_CMOS_POWER_DOWN
#define DCMI_PWDN_HIGH()      dvp->cmos_cfg &= ~DVP_CMOS_POWER_DOWN



Sipeed_OV2640::Sipeed_OV2640( framesize_t frameSize, pixformat_t pixFormat)
:Camera(frameSize, pixFormat),
_dataBuffer(NULL), _aiBuffer(NULL),
_resetPoliraty(ACTIVE_HIGH), _pwdnPoliraty(ACTIVE_HIGH),
_slaveAddr(0x00),
_id(0),
_swapBytes(true)
{
    configASSERT(pixFormat == PIXFORMAT_RGB565 || pixFormat==PIXFORMAT_YUV422);
}



Sipeed_OV2640::Sipeed_OV2640(uint16_t width, uint16_t height, pixformat_t pixFormat)
:Camera(width, height, pixFormat),
_dataBuffer(NULL), _aiBuffer(NULL),
_resetPoliraty(ACTIVE_HIGH), _pwdnPoliraty(ACTIVE_HIGH),
_slaveAddr(0x00),
_id(0)
{
    configASSERT(pixFormat == PIXFORMAT_RGB565 || pixFormat==PIXFORMAT_YUV422);
}


Sipeed_OV2640::~Sipeed_OV2640()
{
    end();
}

bool Sipeed_OV2640::begin()
{
    if(_dataBuffer)
        free(_dataBuffer);
    if(_aiBuffer)
        free(_aiBuffer);

    _data_size = BITMAP_FILEHEADER_SIZE+BITMAP_HEADER_SIZE;
    _data_size += (_width*_height*2);
    _data = (RGB565_DATA*)bm_malloc(6+_data_size);

    if (!_data) {
        _width = 0;
        _height = 0;
        _data_size = 0;
        Serial.println("Failed!");
        return false;
    }

    bitmap_fill_header(_data->bitmap.FileHeader, _data->bitmap.Header, _width, _height, 16);

    _dataBuffer = (uint8_t*)&_data->bitmap.Pixels[0];
    //_dataBuffer = (uint8_t*)malloc(_width*_height*2); //RGB565
    if(!_dataBuffer)
    {
        _width = 0;
        _height = 0;
        return false;
    }
    _aiBuffer = (uint8_t*)malloc(_width*_height*3);   //RGB888
    if(!_aiBuffer)
    {
        _width = 0;
        _height = 0;
        free(_dataBuffer);
        return false;
    }
    if(!reset())
        return false;
    if( !setPixFormat(_pixFormat))
        return false;
    if(!setFrameSize(_frameSize))
        return false;
    return true;
}

void Sipeed_OV2640::end()
{
    if(_dataBuffer)
        free(_dataBuffer);
    if(_aiBuffer)
        free(_aiBuffer);
    _dataBuffer = nullptr;
    _aiBuffer   = nullptr;
}

bool Sipeed_OV2640::reset()
{
    if(dvpInit() != 0)
        return false;
    if(_sensor.reset(&_sensor) != 0)
        return false;
    if(dvpInitIrq() != 0)
        return false;
    return true;
}

bool Sipeed_OV2640::setPixFormat(pixformat_t pixFormat)
{
    bool set_regs = true;

    switch (pixFormat)
    {
    case PIXFORMAT_RGB565:
        // monkey patch about ide default sensor.set_pixformat(sensor.RGB565) but it need sensor.YUV422
        if (_sensor.chip_id != OV7740_ID && _sensor.chip_id != GC0328_ID && _sensor.chip_id != OV2640_ID)
        {
            dvp_set_image_format(DVP_CFG_RGB_FORMAT);
            break;
        } // to next yuv422
    case PIXFORMAT_YUV422:
        dvp_set_image_format(DVP_CFG_YUV_FORMAT);
        break;
    case PIXFORMAT_GRAYSCALE:
        dvp_set_image_format(DVP_CFG_Y_FORMAT);
        break;
    // case PIXFORMAT_JPEG:
    //     dvp_set_image_format(DVP_CFG_RGB_FORMAT);
    //     break;
    // case PIXFORMAT_BAYER:
    //     break;
    default:
        return false;
    }

    if (set_regs)
    {
        if (_sensor.set_pixformat == NULL || _sensor.set_pixformat(&_sensor, pixFormat) != 0)
        {
            // Operation not supported
            return false;
        }
    }
    // Set pixel format
    _sensor.pixformat = pixFormat;

    return true;
}

bool Sipeed_OV2640::setFrameSize(framesize_t frameSize)
{
    bool set_regs = true;
    _sensor.size_set = false;

    // Call the sensor specific function
    if (set_regs)
    {
        if (_sensor.set_framesize == NULL || _sensor.set_framesize(&_sensor, frameSize) != 0)
        {
            // Operation not supported
            return false;
        }
    }
    // Set framebuffer size
    _sensor.framesize = frameSize;

    _sensor.size_set = true;
    return true;
}  

bool Sipeed_OV2640::run(bool run)
{
	if(run)
	{
		dvp_clear_interrupt(DVP_STS_FRAME_START | DVP_STS_FRAME_FINISH);
		plic_irq_enable(IRQN_DVP_INTERRUPT);
		dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 1);
	}
	else{
		plic_irq_disable(IRQN_DVP_INTERRUPT);
		dvp_clear_interrupt(DVP_STS_FRAME_START | DVP_STS_FRAME_FINISH);
		dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 0);
	}
    return true;
}

int Sipeed_OV2640::id()
{
    return _id;
}

/**
 * @return pixels 
 *         If pixels format is RGB565: return RGB565 pixels with every uint16_t one pixel, e.g. RED: 0xF800
 */
uint8_t* Sipeed_OV2640::snapshot()
{
    if ( sensor_snapshot(_swapBytes) != 0)
        return nullptr;
    return _dataBuffer;
}

void Sipeed_OV2640::setRotation(uint8_t rotation)
{
    //FIXME
}

void Sipeed_OV2640::setInvert(bool invert)
{
    //FIXME
    _sensor.set_hmirror(&_sensor, !invert);
    //_sensor.set_vflip(&_sensor, 1);
    return;
}



int Sipeed_OV2640::dvpInit(uint32_t freq)
{
    // just support RGB565 and YUV442 on k210
    configASSERT(_pixFormat==PIXFORMAT_RGB565 || _pixFormat==PIXFORMAT_YUV422);
    _freq  = freq;

    fpioa_set_function(sensor_config.cmos_pclk, FUNC_CMOS_PCLK);
    fpioa_set_function(sensor_config.cmos_xclk, FUNC_CMOS_XCLK);
    fpioa_set_function(sensor_config.cmos_href, FUNC_CMOS_HREF);
    fpioa_set_function(sensor_config.cmos_pwdn, FUNC_CMOS_PWDN);
    fpioa_set_function(sensor_config.cmos_vsync, FUNC_CMOS_VSYNC);
    fpioa_set_function(sensor_config.cmos_rst, FUNC_CMOS_RST);

    /* Do a power cycle */
    DCMI_PWDN_HIGH();
    msleep(10);

    DCMI_PWDN_LOW();
    msleep(10);

    // Initialize the camera bus, 8bit reg
    // Initialize the camera bus, 8bit reg
    cambus_init(
        sensor_config.reg_width, // 16 //  8
        sensor_config.i2c_num,   //  2 // -2
        sensor_config.pin_clk,   // 46 // 41
        sensor_config.pin_sda,   // 47 // 40
        sensor_config.gpio_clk,  // 0
        sensor_config.gpio_sda   // 0
    );
	 // Initialize dvp interface
	dvp_set_xclk_rate(freq);
	dvp->cmos_cfg |= DVP_CMOS_CLK_DIV(3) | DVP_CMOS_CLK_ENABLE;
	dvp_enable_burst();
	dvp_disable_auto();
	dvp_set_output_enable(DVP_OUTPUT_AI, 1);	//enable to AI
	dvp_set_output_enable(DVP_OUTPUT_DISPLAY, 1);	//enable to lcd
        dvp_set_image_format(DVP_CFG_YUV_FORMAT);
	dvp_set_image_size(_width, _height);	//set QVGA default
	dvp_set_ai_addr( (uint32_t)((long)_aiBuffer), (uint32_t)((long)(_aiBuffer+_width*_height)), (uint32_t)((long)(_aiBuffer+_width*_height*2)));
	dvp_set_display_addr( (uint32_t)((long)_dataBuffer) );

    _sensor.chip_id = 0;
    if(0 == sensor_ov_detect()){//find ov sensor
        // printf("find ov sensor\n");
    }
    else if(0 == sensro_gc_detect()){//find gc sensor
        // printf("find gc\n");
    }
    else if(0 == sensro_mt_detect(true)){//find mt sensor
        // printf("find mt\n");
    }

    return 0;
}



int Sipeed_OV2640::sensor_ov_detect()
{
    int init_ret = 0;
    sensor_t *sensor = &_sensor;

    /* Reset the sensor */
    DCMI_RESET_HIGH();
    msleep(10);

    DCMI_RESET_LOW();
    msleep(10);

    /* Probe the ov sensor */
    _slaveAddr = cambus_scan();
    if (_slaveAddr == 0) {
        /* Sensor has been held in reset,
           so the reset line is active low */
        _resetPoliraty = ACTIVE_LOW;

        /* Pull the sensor out of the reset state,systick_sleep() */
        DCMI_RESET_HIGH();
        msleep(10);

        /* Probe again to set the slave addr */
        _slaveAddr = cambus_scan();
        if (_slaveAddr == 0) {
            _pwdnPoliraty = ACTIVE_LOW;

            DCMI_PWDN_HIGH();
            msleep(10);

            _slaveAddr = cambus_scan();
            if (_slaveAddr == 0) {
                _resetPoliraty = ACTIVE_HIGH;

                DCMI_RESET_LOW();
                msleep(10);

                _slaveAddr = cambus_scan();
                if(_slaveAddr == 0) {
                    //should do something?
                    return -2;
                }
            }
        }
    }

    // Clear sensor chip ID.
    _id = 0;

    if (_slaveAddr == LEPTON_ID) {
        _id = LEPTON_ID;
		/*set LEPTON xclk rate*/
		/*lepton_init*/
    } else {
        // Read ON semi sensor ID.
        cambus_readb(_slaveAddr, ON_CHIP_ID, (uint8_t *)&_id);
        Serial.printf("[MAIX]: on id = %x\n", _id);
        if (_id == MT9V034_ID) {
			/*set MT9V034 xclk rate*/
			/*mt9v034_init*/
        } else { // Read OV sensor ID.
            uint8_t tmp;
            uint8_t reg_width = cambus_reg_width();
            uint16_t reg_addr, reg_addr2;
            if (reg_width == 8)
            {
                reg_addr = OV_CHIP_ID;
                reg_addr2 = OV_CHIP_ID2;
            }
            else
            {
                reg_addr = OV_CHIP_ID_16BIT;
                reg_addr2 = OV_CHIP_ID2_16BIT;
            }
            cambus_readb(_slaveAddr, reg_addr, &tmp);
            _id = tmp << 8;
            cambus_readb(_slaveAddr, reg_addr2, &tmp);
            _id |= tmp;
            Serial.printf("[MAIX]: ov id = %x\n", _id);
            // Initialize sensor struct.
            sensor->chip_id = _id;
            switch (_id) {
                case OV9650_ID:
					/*ov9650_init*/
                    break;
                case OV2640_ID:
                    sensor->slv_addr = _slaveAddr;
                    Serial.printf("detect ov2640, id:%x\n", _slaveAddr);
                    init_ret = ov2640_init(sensor);
                    break;
                case OV5640_ID:
                    sensor->slv_addr = _slaveAddr;
                    Serial.printf("[MAIX]: find ov5640\n");
                    init_ret = ov5640_init(sensor);
                    break;
                case OV7725_ID:
					/*ov7725_init*/
                    break;
                case OV7740_ID:
                    sensor->slv_addr = _slaveAddr;
                    Serial.printf("[MAIX]: find ov7740\n");
                    init_ret = ov7740_init(sensor);
                    break;
                case OV3660_ID:
                    sensor->slv_addr = _slaveAddr;
                    Serial.printf("[MAIX]: find ov3660\n");
                    init_ret = ov3660_init(sensor);
                    break;
                default:
                    // Sensor is not supported.
                    return -3;
            }
        }
    }

    if (init_ret != 0 ) {
        // Sensor init failed.
        return -4;
    }
    return 0;
}

int Sipeed_OV2640::sensro_gc_detect()
{
    sensor_t *sensor = &_sensor;

    DCMI_PWDN_HIGH();//enable gc0328 要恢复 normal 工作模式，需将 PWDN pin 接入低电平即可，同时写入初始化寄存器即可
    DCMI_RESET_LOW();//reset gc3028
    msleep(10);
    DCMI_RESET_HIGH();
    msleep(10);
    uint8_t id = cambus_scan_gc0328();
    if(0 == id)
    {
        return -3;
    }
    else
    {
        Serial.printf("[MAIX]: gc id = %x\n",id);
        _id = id;
        sensor->chip_id = id;
        switch (sensor->chip_id)
        {
        case GC0328_ID:
            _slaveAddr = GC0328_ADDR;
            sensor->slv_addr = _slaveAddr;
            Serial.printf("[MAIX]: find gc0328\n");
            gc0328_init(sensor);
            break;
        case GC2145_ID:
            _slaveAddr = GC2145_ADDR;
            sensor->slv_addr = _slaveAddr;
            Serial.printf("[MAIX]: find gc2145\n");
            gc2145_init(sensor);
        default:
            break;
        }
    }
    return 0;
}

int Sipeed_OV2640::sensro_mt_detect(bool pwnd)
{
    sensor_t *sensor = &_sensor;

    if (pwnd)
        DCMI_PWDN_LOW();
    DCMI_RESET_LOW();
    msleep(10);
    DCMI_RESET_HIGH();
    msleep(10);
    uint16_t id = cambus_scan_mt9d111();
    if (0 == id)
    {
        return -3;
    }
    else
    {
        _id = id;
        _slaveAddr = MT9D111_CONFIG_I2C_ID;
        sensor->chip_id = id;
        sensor->slv_addr = _slaveAddr;
        Serial.printf("[MAIX]: find mt9d111\n");
        mt9d111_init(sensor);
    }
    return 0;
}

#ifdef __cplusplus
extern "C" {
#endif

static int sensor_irq(void *ctx)
{
	if (dvp_get_interrupt(DVP_STS_FRAME_FINISH)) {	//frame end
		dvp_clear_interrupt(DVP_STS_FRAME_FINISH);
		g_dvp_finish_flag = 1;
	} else {	//frame start
        if(g_dvp_finish_flag == 0)  //only we finish the convert, do transmit again
            dvp_start_convert();	//so we need deal img ontime, or skip one framebefore next
		dvp_clear_interrupt(DVP_STS_FRAME_START);
	}

	return 0;
}

#ifdef __cplusplus
}
#endif



int Sipeed_OV2640::dvpInitIrq()
{
	dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 0);
	plic_set_priority(IRQN_DVP_INTERRUPT, 2);
    /* set irq handle */
	plic_irq_register(IRQN_DVP_INTERRUPT, sensor_irq, (void*)NULL);

	plic_irq_disable(IRQN_DVP_INTERRUPT);
	dvp_clear_interrupt(DVP_STS_FRAME_START | DVP_STS_FRAME_FINISH);
	dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 1);

	return 0;
}




int Sipeed_OV2640::reverse_u16pixel(uint32_t* addr,uint32_t length)
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

int Sipeed_OV2640::reverse_u32pixel(uint32_t* addr,uint32_t length)
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


int Sipeed_OV2640::sensor_snapshot( bool swap )
{	
    //wait for new frame
    g_dvp_finish_flag = 0;
    uint32_t start =  millis();
    while (g_dvp_finish_flag == 0)
    {
        usleep(50);
        if(millis() - start > 300)//wait for 300ms
            return -1;
    }
    if (swap)
        reverse_u32pixel((uint32_t*)_dataBuffer, _width*_height/2);
    return 0;
}

void Sipeed_OV2640::swapBuffer() {
    reverse_u16pixel((uint32_t*)_dataBuffer, _width*_height/2);
}

/***************************************************************************************
** Function name:           setSwapBytes
** Description:             Used by 16 bit pushImage() to swap byte order in colours
***************************************************************************************/
void Sipeed_OV2640::setSwapBytes(bool swap) {
    _swapBytes = swap;
}
