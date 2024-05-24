#ifndef _gfx8lib_bitmap
#define _gfx8lib_bitmap

#include "gfx8lib_config.h"

typedef struct {
  unsigned char width;          /*!< Bitmap width in pixels */
  unsigned char height;         /*!< Bitmap height in pixels */
  const unsigned char *data;    /*!< Pointer to bitmap data array */
}gfx8_bitmap_t;

extern const gfx8_bitmap_t mute_icon;
extern const gfx8_bitmap_t temperature_icon;
extern const gfx8_bitmap_t humidity_icon;
extern const gfx8_bitmap_t co2_icon;
extern const gfx8_bitmap_t backlight_icon;
extern const gfx8_bitmap_t antenna_icon;

#endif

