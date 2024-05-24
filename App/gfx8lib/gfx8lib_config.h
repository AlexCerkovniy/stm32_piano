#ifndef _gfx8lib_config
#define _gfx8lib_config

#include <stdint.h>

/* GENERAL LIBRARY OPTIONS */
#define GFX8_INVERT                             0
#define GFX8_VA_STRING_BUFFER_SIZE              (64)

/* FONTS INCLUDE OPTIONS */
//font_6x5
//font_6x8
//formplex_font_8x6
//hunter_font_8x8
//dos_font_8x8
//dos_font_8x16
//atari_font_8x16

#define GFX8_DEFAULT_FONT                       font_6x8

/* PLOT OPTIONS */
typedef int16_t                                 gfx8_plot_type;

#endif
