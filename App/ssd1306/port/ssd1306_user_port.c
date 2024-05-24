#include "ssd1306_port.h"
#include "ssd1306_config.h"

/* Port includes */
#include "main.h"

extern I2C_HandleTypeDef hi2c2;

void SSD1306_I2C_Init(void){
  
}

void SSD1306_I2C_Write(unsigned char addr, unsigned char *buffer, unsigned short length){
	HAL_I2C_Master_Transmit(&hi2c2, addr << 1, buffer, length, HAL_MAX_DELAY);
}
