#include "st7789.h"


static uint8_t g_dcx_pin;
static int8_t g_rst_pin;
static spi_device_num_t g_spi_num;
static dmac_channel_number_t g_dma_ch;
static uint8_t g_ss;
static uint8_t g_wmb;

static void init_dcx(int8_t dcx_pin)
{
    g_dcx_pin = dcx_pin;
    pinMode(g_dcx_pin, OUTPUT);
    digitalWrite(g_dcx_pin, HIGH);
}

static void set_dcx_control(void)
{
    digitalWrite(g_dcx_pin, LOW);
}

static void set_dcx_data(void)
{
    digitalWrite(g_dcx_pin, HIGH);
}

static void init_rst(int8_t rst_pin)
{
    g_rst_pin = rst_pin;
    if( rst_pin < 0)
        return ;
    pinMode(g_rst_pin, OUTPUT);
    digitalWrite(g_rst_pin, HIGH);
}

static void set_rst(uint8_t val)
{
    if(g_rst_pin < 0)
        return ;
    digitalWrite(g_rst_pin, val);
}

void tft_set_clk_freq(uint32_t freq)
{
    spi_set_clk_rate(g_spi_num, freq);
}

void tft_hard_init(uint8_t spi, uint8_t ss, uint8_t rst, uint8_t dcx, uint32_t freq, int8_t rst_pin, int8_t dcx_pin, uint8_t dma_ch)
{
    g_spi_num = spi;
    g_dma_ch = dma_ch;
    g_ss = ss;
    init_dcx(dcx_pin);
    init_rst(rst_pin);
    set_rst(0);
    spi_init(g_spi_num, SPI_WORK_MODE_0, SPI_FF_OCTAL, 8, 0);
    tft_set_clk_freq(freq);
    set_rst(1);
}

#define tft_work_mode(bits, ilen, alen)\
{\
    if (g_wmb != bits) {\
        spi_init(g_spi_num, SPI_WORK_MODE_0, SPI_FF_OCTAL, bits, 0);\
        spi_init_non_standard(g_spi_num, ilen /*instrction length*/, alen /*address length*/, 0 /*wait cycles*/,\
                          SPI_AITM_AS_FRAME_FORMAT /*spi address trans mode*/);\
        g_wmb = bits;\
    }\
}

void tft_write_command(uint8_t cmd)
{
    set_dcx_control();
    tft_work_mode(8, 8, 0);
    spi_send_data_normal_dma(g_dma_ch, g_spi_num, g_ss, (uint8_t *)(&cmd), 1, SPI_TRANS_CHAR);
    set_dcx_data();
}

void tft_write_byte(uint8_t *data_buf, uint32_t length)
{
    tft_work_mode(8, 8, 0);
    spi_send_data_normal_dma(g_dma_ch, g_spi_num, g_ss, data_buf, length, SPI_TRANS_CHAR);
}

void tft_write_half(uint16_t *data_buf, uint32_t length)
{
    tft_work_mode(16, 16, 0);
    spi_send_data_normal_dma(g_dma_ch, g_spi_num, g_ss, data_buf, length, SPI_TRANS_SHORT);
}

void tft_fill_half(uint16_t *data_buf, uint32_t length)
{
    tft_work_mode(16, 0, 16);
    spi_fill_data_dma(g_dma_ch, g_spi_num, g_ss, (uint32_t *)data_buf, length);
}

void tft_write_word(uint32_t *data_buf, uint32_t length)
{
    tft_work_mode(32, 0, 32);
    spi_send_data_normal_dma(g_dma_ch, g_spi_num, g_ss, data_buf, length, SPI_TRANS_INT);
}

void tft_fill_data(uint32_t *data_buf, uint32_t length)
{
    tft_work_mode(32, 0, 32);
    spi_fill_data_dma(g_dma_ch, g_spi_num, g_ss, data_buf, length);
}
