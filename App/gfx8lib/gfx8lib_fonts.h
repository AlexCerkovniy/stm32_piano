#ifndef _gfx8lib_fonts
#define _gfx8lib_fonts

#include "gfx8lib_libraries.h"
#include "gfx8lib_config.h"

typedef struct {
  uint8_t width;          /*!< Font width in pixels */
  uint8_t height;         /*!< Font height in pixels */
  uint8_t height_pages;   /*!< Font height in pages (1 page = 8 pixels) */
  uint8_t ascii_start;    /*!< Font ASCII start symbol */
  uint8_t ascii_end;      /*!< Font ASCII end symbol */
  const uint8_t *data;    /*!< Pointer to data font data array */
} gfx8_font_t;

extern const gfx8_font_t font_5x4;
extern const gfx8_font_t font_5x7;
extern const gfx8_font_t digital_font_5x7;
extern const gfx8_font_t font_6x5;
extern const gfx8_font_t font_6x8;
extern const gfx8_font_t formplex_font_8x6;
extern const gfx8_font_t hunter_font_8x8;
extern const gfx8_font_t dos_font_8x8;
extern const gfx8_font_t font_8x16;
extern const gfx8_font_t dos_font_8x16;
extern const gfx8_font_t atari_font_8x16;
extern const gfx8_font_t pixel_operator_8x16;
extern const gfx8_font_t pixel_operator_bold_8x16;
extern const gfx8_font_t courier_font_digits_11x16;


#endif
