#ifndef _gfx8lib_plot
#define _gfx8lib_plot

/*
 *  1. Startup coordinates x, y describes upper-left corner, where drawn window starts - $.
 *     The rendering process starts from this point down by height, and to the right by width.
 *
 *      #######################################################
 *      # $ <- here                                           #
 *      #                                                     #
 *      #                                                     #
 *      #                                                     #
 *      #                                                     #
 *      #                                                     #
 *      #######################################################
 *
 *  2. [width, height] parameters describes plot window size in pixels. Width == plot draw points.
 *
 */

#include "gfx8lib_libraries.h"
#include "gfx8lib_config.h"

typedef enum{
  GFX8_PLOT_DOTS = 0,
  GFX8_PLOT_FILLED_LINES
}gfx8_plot_line_t;

typedef struct{
  /* Plot data storage */
  gfx8_plot_type *buffer;                   /* Values buffer */
  gfx8_plot_type *pbuffer;                  /* Pointer to first buffer value (last value) */
  uint8_t buffer_count;                     /* Currently buffered values */
  gfx8_plot_type buffer_min, buffer_max;    /* Min & max data values in buffer (autorange feature) */

  /* Plot size */
  uint8_t x, y, w, h;

  /* Plot scaling of input data to [height] dimension - [0 ... height] -> [y_min ... y_max] */
  bool scaling;
  bool autorange;
  gfx8_plot_type autorange_backing;         /* If autorange enabled, this value add some gap between min/max and graph top and bottom limits */
  gfx8_plot_type y_max;
  gfx8_plot_type y_min;

  /* Plot styles */
  gfx8_plot_line_t line_style;
}gfx8_plot_t;

void G8Lib_Plot_Init(gfx8_plot_t *plot);
void G8Lib_Plot_NewValue(gfx8_plot_t *plot, gfx8_plot_type value);
void G8Lib_Plot_Draw(gfx8_plot_t *plot);

#endif
