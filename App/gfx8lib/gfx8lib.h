#ifndef _gfx8lib
#define _gfx8lib

#include "gfx8lib_libraries.h"
#include "gfx8lib_plot.h"
#include "gfx8lib_config.h"
#include "gfx8lib_fonts.h"
#include "gfx8lib_bitmap.h"

typedef enum{
  GFX8_RESET = 0,
  GFX8_SET,
  GFX8_ADAPTIVE
}gfx8_color_t;

typedef struct{
	uint16_t height, width;
	void (*init)(void);
	void (*clear)(void);
	void (*draw)(void);
	uint8_t* (*get_buffer)(void);
	void (*set_backlight)(bool enabled);
	bool (*get_backlight)(void);
	void (*sleep)(bool enabled);
}gfx8_display_driver_t;

/* LIBRARY API */
void G8Lib_Init(gfx8_display_driver_t *display_driver);
gfx8_display_driver_t *G8Lib_GetDisplayDrv(void);

/* BASIC */
void G8Lib_PutPixel(unsigned char x, unsigned char y, gfx8_color_t color);
bool G8Lib_GetPixel(uint8_t x, uint8_t y);
void G8Lib_PutByte(unsigned char x, unsigned char y, unsigned char byte, gfx8_color_t color);
void G8Lib_DrawVLine(unsigned char x, unsigned char y0, unsigned char y1);
void G8Lib_DrawDottedVLine(uint8_t y0, uint8_t y1, uint8_t x, uint8_t gap_width, uint8_t dot_width, bool dot_first);
void G8Lib_DrawHLine(unsigned char x0, unsigned char x1, unsigned char y, gfx8_color_t color);
void G8Lib_DrawDottedHLine(unsigned char x0, unsigned char x1, unsigned char y, uint8_t step_width_px);
void G8Lib_Rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, gfx8_color_t color);
void G8Lib_RectNoFill(uint8_t x, uint8_t y, uint8_t w, uint8_t h, gfx8_color_t color);

/* TEXT */
void G8Lib_SetCursor(unsigned char x, unsigned char y);
unsigned char G8Lib_GetCursorX(void);
unsigned char G8Lib_GetCursorY(void);
void G8Lib_SetFont(gfx8_font_t new_font);
gfx8_font_t *G8Lib_GetFont(void);
void G8Lib_PrintUnsignedInt(unsigned long number, gfx8_color_t color);
void G8Lib_PrintSignedInt(long number, gfx8_color_t color);
void G8Lib_String(char* str, gfx8_color_t color);
void G8Lib_Print(gfx8_color_t color, const char* fmt, ... );

/* MODELS/DRAWING */
void G8Lib_DrawBitmap(unsigned char x, unsigned char y, gfx8_bitmap_t bitmap, gfx8_color_t color);
void G8Lib_DrawBatteryStatus(unsigned char x, unsigned char y, unsigned char length, unsigned char status, gfx8_color_t color);

#endif
