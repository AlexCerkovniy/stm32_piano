#include "gfx8lib.h"

#define ABS(X)      ((X > 0)?(X):(-X))

/* Private functions ======================================================================================== */
static gfx8_plot_type _findMax(gfx8_plot_t *plot);
static gfx8_plot_type _findMin(gfx8_plot_t *plot);
static int32_t map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max);

void G8Lib_Plot_Init(gfx8_plot_t *plot){
    plot->pbuffer = plot->buffer;
    plot->buffer_count = 0;
    plot->buffer_max = 0;
    plot->buffer_min = 0;
}

/*
 *  @brief Add new value to plot.
 *  @param plot - plot handle pointer.
 *  @param value - new plot value, [gfx8_plot_type] type.
 */
void G8Lib_Plot_NewValue(gfx8_plot_t *plot, gfx8_plot_type value){
    gfx8_plot_type rewritten_value = *plot->pbuffer;
    *plot->pbuffer = value;

    /* Update max & min buffer values (auto range feature) */
    if(plot->scaling && plot->autorange){
        /* If buffer is empty - set new min & max values */
        if(plot->buffer_count == 0){
            plot->buffer_min = value;
            plot->buffer_max = value;
        }
        /* If value rewritten, in case of full circular buffer - update new min/max (auto range feature)  */
        else if(plot->buffer_count == plot->w){
            if(rewritten_value == plot->buffer_max){
                plot->buffer_max = _findMax(plot);
            }
            else if(rewritten_value == plot->buffer_min){
                plot->buffer_min = _findMin(plot);
            }
        }

        /* Update min/max by new value */
        if(value > plot->buffer_max){
            plot->buffer_max = value;
        }
        else if(value < plot->buffer_min){
            plot->buffer_min = value;
        }
    }

    /* Move pointer to next (empty) buffer slot */
    if(++plot->pbuffer >= plot->buffer + plot->w){
        plot->pbuffer = plot->buffer;
    }

    /* Update buffered values count */
    if(plot->buffer_count < plot->w){
        plot->buffer_count++;
    }
}

/*
 *  @brief Draw plot.
 *  @param plot - plot handle pointer.
 */
void G8Lib_Plot_Draw(gfx8_plot_t *plot){
    uint8_t point_x, point_y, y_zero;   /* Absolute "display" coordinates */
    gfx8_plot_type y_min, y_max;

    /* If buffer empty - nothing to draw */
    if(plot->buffer_count == 0){
        return;
    }

    /* Get last point */
    gfx8_plot_type *tmp = plot->pbuffer - 1;
    if(tmp < plot->buffer){
        tmp = plot->buffer + plot->w - 1;
    }

    /* Get points count */
    uint8_t point = plot->buffer_count;
    uint32_t offset_points = plot->w - plot->buffer_count;

    if(plot->scaling){
        if(plot->autorange){
            y_min = plot->buffer_min - plot->autorange_backing;
            y_max = plot->buffer_max + plot->autorange_backing;
        }
        else{
            y_min = plot->y_min;
            y_max = plot->y_max;
        }

        /* Calc zero line Y level (display Y coordinates) */
        if((y_min == 0) && (y_max == 0)){
            /* Both min & max is zero, something wrong - draw cannot be correct */
            return;
        }
        else if(y_min < 0 && y_max <= 0){
            y_zero = plot->y;
        }
        else if((y_min >= 0) && (y_max > 0)){
            y_zero = plot->y + plot->h - 1;
        }
        else{
            y_zero = map(ABS(y_min), 0, ABS(y_min) + ABS(y_max), plot->y + plot->h - 1, plot->y);
        }

        while(point--){
            /* Get absolute coordinate X of point */
            point_x = map(offset_points + point, 0, plot->w - 1, plot->x, plot->x + plot->w - 1);

            /* Get absolute coordinate of Y point */
            if(plot->autorange){
                if(*tmp > 0){
                    if((y_min >= 0) && (y_max > 0)){
                        point_y = map(*tmp, y_min, y_max, y_zero, plot->y);
                    }
                    else{
                        point_y = map(*tmp, 0, y_max, y_zero, plot->y);
                    }
                }
                else{
                    if((y_min < 0) && (y_max <= 0)){
                        point_y = map(*tmp, y_min, y_max, plot->y + plot->h - 1, y_zero);
                    }
                    else{
                        point_y = map(*tmp, y_min, 0, plot->y + plot->h - 1, y_zero);
                    }
                }
            }
            /* No autoscaling */
            else{
                if(*tmp > 0){
                    point_y = map(*tmp, 0, y_max, y_zero, plot->y);
                }
                else{
                    point_y = map(*tmp, y_min, 0, plot->y + plot->h - 1, y_zero);
                }
            }

            /* Draw Point */
            if(plot->line_style == GFX8_PLOT_FILLED_LINES){
                G8Lib_DrawVLine(point_x, point_y, y_zero);
            }
            else{
                G8Lib_PutPixel(point_x, point_y, GFX8_SET);
            }

            /* Next point and check bounds of buffer */
            if(--tmp < plot->buffer){
                tmp = plot->buffer + plot->w - 1;
            }
        }
    }
    else{
        /* TODO: Add fast non-scaling mode draw */
    }
}

static gfx8_plot_type _findMin(gfx8_plot_t *plot){
    gfx8_plot_type tmp = plot->buffer[0];
    uint8_t count = plot->buffer_count;

    while(count--){
        if(plot->buffer[count] < tmp){
            tmp = plot->buffer[count];
        }
    }

    return tmp;
}

static gfx8_plot_type _findMax(gfx8_plot_t *plot){
    gfx8_plot_type tmp = plot->buffer[0];
    uint8_t count = plot->buffer_count;

    while(count--){
        if(plot->buffer[count] > tmp){
            tmp = plot->buffer[count];
        }
    }

    return tmp;
}

static int32_t map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max){
    if(x < in_min){
        return out_min;
    }
    else if(x > in_max){
        return out_max;
    }

    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
