#ifndef __READER_GUI_H__
#define __READER_GUI_H__ 1

#include <reader_types.h>

#ifdef WIN32
#define PA_RGB(r,g,b) ((1 << 15) + (r) + ((g)<<5) + ((b)<<10))
#endif
#define READER_RGB(r, g, b) PA_RGB((r), (g), (b))

#define READER_COLOR_WHITE ((color_t)READER_RGB(28, 28, 28))
#define READER_COLOR_BLACK ((color_t)READER_RGB(0, 0, 0))
#define READER_COLOR_RED ((color_t)READER_RGB(31, 0, 0))
#define READER_COLOR_GREEN ((color_t)READER_RGB(0, 31, 0))
#define READER_COLOR_BLUE ((color_t)READER_RGB(0, 0, 31))
#define READER_COLOR_YELLOW ((color_t)READER_RGB(31, 31, 0))
#define READER_COLOR_THIN_BLUE ((color_t)READER_RGB(2, 22, 28))
#define READER_COLOR_GREY ((color_t)READER_RGB(13, 13, 7))

//only draw horizontally or vertically
void reader_rectangle(screen_t screen, int x1, int y1, int x2, int y2, color_t color, int fill);

//only draw horizontally or vertically
void reader_lineto(screen_t screen, int x1, int y1, int x2, int y2, color_t color);

//show caption
void reader_caption(screen_t screen, unsigned short *str);

//draw arrow
typedef enum {
  LEFT_POINTER,
  RIGHT_POINTER
} arrow_pointer_t;

void reader_arrow(screen_t screen, int x, int y, int size, arrow_pointer_t ap, color_t color);

#endif
