#include <reader_core.h>

void reader_lineto(screen_t screen, int x1, int y1, int x2, int y2, color_t color) {
  int from, to, x, y;

  if (x1 == x2) {
    if (y1 > y2) {
      from = y2;
      to = y1;
    } else {
      from = y1;
      to = y2;
    }

    for (y = from; y <= to; y++) {
      reader_setpixel(screen, x1, y, color);
    }
  }

  if (y1 == y2) {
    if (x1 > x2) {
      from = x2;
      to = x1;
    } else {
      from = x1;
      to = x2;
    }

    for (x = from; x <= to; x++) {
      reader_setpixel(screen, x, y1, color);
    }
  }
}

void reader_rectangle(screen_t screen, int x1, int y1, int x2, int y2, color_t color, int fill) {
  int left_x, right_x, up_y, down_y;

  //draw border
  reader_lineto(screen, x1, y1, x2, y1, color);
  reader_lineto(screen, x1, y1, x1, y2, color);
  reader_lineto(screen, x2, y1, x2, y2, color);
  reader_lineto(screen, x1, y2, x2, y2, color);

  if (fill) {
    if (x1 > x2) {
      left_x = x2;
      right_x = x1;
    } else {
      right_x = x2;
      left_x = x1;
    }

    if (y1 > y2) {
      up_y = y2;
      down_y = y1;
    } else {
      down_y = y2;
      up_y = y1;
    }

    //borader already drawn
    left_x ++;
    right_x --;
    while (up_y ++ < down_y) {
      reader_lineto(screen, left_x, up_y, right_x, up_y, color);
    }
  }
}

void reader_caption(screen_t screen, unsigned short *str) {
  //draw caption bg
  reader_rectangle(screen, 0, DEFAULT_HEADER_BLANK, READER_SCREEN_WIDTH - 1,
                   DEFAULT_FONT_SIZE + DEFAULT_HEADER_BLANK, READER_COLOR_BLUE, 1);

  //draw caption text
  //prompt
  reader_textout_ex(screen, 0, DEFAULT_HEADER_BLANK, str, reader_wcslen(str),
                    DEFAULT_FONT_SIZE, READER_COLOR_YELLOW, 0);
}

void reader_arrow(screen_t screen, int x, int y, int size, arrow_pointer_t ap, color_t color) {
  int i = 0, x0, y0, y_lim;
  if (ap == LEFT_POINTER) {
    for (i = 0; i < size; i ++) {
      y_lim = y + i;
      x0 = x + i;
      for (y0 = y - i; y0 < y_lim; y0 ++) {
        reader_setpixel(screen, x0, y0, color);
      }
    }
  } else {
    for (i = 0; i < size; i ++) {
      y_lim = y + i;
      x0 = x - i;
      for (y0 = y - i; y0 < y_lim; y0 ++) {
        reader_setpixel(screen, x0, y0, color);
      }
    }
  }
}
