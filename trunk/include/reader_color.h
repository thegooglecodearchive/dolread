#ifndef __READER_COLOR_H__
#define __READER_COLOR_H__ 1

#include <reader_types.h>

typedef unsigned char reader_color_context_t[6];

void reader_color_show_window(screen_t screen);

void reader_color_set(void);

void reader_color_select_ready(void);

void reader_color_select_cancel(void);

//return 0 means status changed, 1 means remained unchange.
int reader_color_cursor_up(void);
int reader_color_cursor_down(void);
int reader_color_inc(void);
int reader_color_dec(void);
int reader_color_set_cursor(int pos);
int reader_color_set_default(void);

extern reader_color_context_t reader_color_context, reader_color_context_def;

#endif
