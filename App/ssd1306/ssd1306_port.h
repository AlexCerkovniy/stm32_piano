#pragma once

void SSD1306_I2C_Init(void);
void SSD1306_I2C_WriteToRegister(unsigned char addr, unsigned char reg, unsigned char *buffer, unsigned short length);
void SSD1306_I2C_Write(unsigned char addr, unsigned char *buffer, unsigned short length);
