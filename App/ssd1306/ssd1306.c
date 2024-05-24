#include "ssd1306.h"
#include "ssd1306_port.h"
#include <string.h>

/* Default height & width in pixels */
#define SSD1306_BUFFER_SIZE                                 ((SSD1306_WIDTH * SSD1306_HEIGHT) / 8)
#define SSD1306_SCREEN_RAM_PAGES                            (SSD1306_HEIGHT / 8)

/* Local Variables */
static unsigned char screen_buffer[SSD1306_BUFFER_SIZE];
static unsigned char i2c_buffer[3];

static void _writeCmd(unsigned char cmd){
    i2c_buffer[0] = 0;
    i2c_buffer[1] = cmd;

    SSD1306_I2C_Write(SSD1306_I2C_ADDR, i2c_buffer, 2);
}

static void _writeCmdData(unsigned char cmd, unsigned char data){
    i2c_buffer[0] = 0;
    i2c_buffer[1] = cmd;
    i2c_buffer[2] = data;

    SSD1306_I2C_Write(SSD1306_I2C_ADDR, i2c_buffer, 3);
}

void SSD1306_Init(void) {
	SSD1306_I2C_Init();

  _writeCmd(SSD1306_DISPLAYOFF);
  _writeCmdData(SSD1306_SETDISPLAYCLOCKDIV, 0x80);
  //_writeCmd(SSD1306_SETMULTIPLEX);
  _writeCmdData(SSD1306_SETDISPLAYOFFSET, 0);
  _writeCmd(SSD1306_SETSTARTLINE);
  _writeCmdData(SSD1306_CHARGEPUMP, 0x14);
  _writeCmdData(SSD1306_MEMORYMODE, 0x00);

#if (SSD1306_MIRROR_HORIZONTAL)
  _writeCmd(SSD1306_SEGREMAP);
#else
  _writeCmd(SSD1306_SEGREMAP | 0x01);
#endif

#if (SSD1306_MIRROR_VERTICAL)
  _writeCmd(SSD1306_COMSCANINC);
#else
  _writeCmd(SSD1306_COMSCANDEC);
#endif
  
// Set multiplex ratio.
#if (SSD1306_HEIGHT == 128)
  _writeCmd(0xFF);
#else
  _writeCmd(0xA8);
#endif

#if (SSD1306_HEIGHT == 32)
  _writeCmd(0x1F);
#elif (SSD1306_HEIGHT == 64)
  _writeCmd(0x3F);
#elif (SSD1306_HEIGHT == 128)
  _writeCmd(0x3F);
#else
  #error "Only 32, 64, or 128 lines of height are supported!"
#endif

  _writeCmd(SSD1306_SETCOMPINS);
#if ((SSD1306_WIDTH == 128) && (SSD1306_HEIGHT == 32))
  _writeCmd(0x02);
#elif ((SSD1306_WIDTH == 128) && (SSD1306_HEIGHT == 64))
  _writeCmd(0x12);
#elif ((SSD1306_WIDTH == 96) && (SSD1306_HEIGHT == 16))
  _writeCmd(0x02);
#else
  #error "Only 16, 32 or 64 lines of height are supported!"
#endif
  SSD1306_SetContrast(SSD1306_STARTUP_CONTRAST);

  _writeCmdData(SSD1306_SETPRECHARGE, 0x22);
  _writeCmdData(SSD1306_SETVCOMDETECT, 0x20);
  _writeCmd(SSD1306_DISPLAYALLON_RESUME);
#if (SSD1306_INVERTED_COLORS)
  _writeCmd(SSD1306_INVERTDISPLAY);
#else
  _writeCmd(SSD1306_NORMALDISPLAY);
#endif
  _writeCmd(SSD1306_DEACTIVATE_SCROLL);
  _writeCmd(SSD1306_DISPLAYON);

  SSD1306_Update();
}

unsigned char *SSD1306_GetFramebuffer(void){
    return screen_buffer;
}

uint16_t SSD1306_GetFramebufferSize(void){
    return SSD1306_BUFFER_SIZE;
}

void SSD1306_Sleep(bool state){
    if(state){
        _writeCmd(SSD1306_DISPLAYON);
    }
    else{
        _writeCmd(SSD1306_DISPLAYOFF);
    }
}

void SSD1306_SetContrast(uint8_t contrast){
    _writeCmdData(SSD1306_SETCONTRAST, contrast);
}

void SSD1306_Clear(void){
    memset(screen_buffer, 0x00, sizeof(screen_buffer));
}

void SSD1306_Update(void){
	unsigned char tmp[SSD1306_WIDTH + 1];
	tmp[0] = 0x40;

    _writeCmd(0x00);
    _writeCmd(0x10);
    for(uint8_t i = 0; i < SSD1306_SCREEN_RAM_PAGES; i++) {
        _writeCmd(0xB0 + i); // Set the current RAM page address.
        memcpy(tmp + 1, &screen_buffer[SSD1306_WIDTH*i], SSD1306_WIDTH);
        SSD1306_I2C_Write(SSD1306_I2C_ADDR, tmp, SSD1306_WIDTH + 1);
    }
}
