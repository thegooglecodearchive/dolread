#ifndef __READER_MMI_H__
#define __READER_MMI_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <reader_mmi_dev.h>

extern color_t reader_text_color;
extern color_t reader_bg_color;

enum {
  READER_SCREEN_HEIGHT = 256,
  READER_SCREEN_WIDTH = 192,
  MAX_PAGE = 10240,
  DEFAULT_FONT_SIZE = 16,
  FONT_TOTAL = 22026,
  LINE_INTERVAL = 2,
  DEFAULT_HEADER_BLANK = 3,
  MAX_LINE = 15
};

typedef struct {
  unsigned int page_offset;
  unsigned short line_offest[MAX_LINE];
}page_t;

void reader_mmi_update_screen(screen_t screen, unsigned short *content, int page_number);
#ifdef WIN32
extern void sim_setpixel(screen_t screen, int x, int y, color_t color);
extern void sim_setpixel2(screen_t screen, int x, int y, color_t color1, color_t color2);
extern unsigned int reader_palette[];
#endif


//current page changed return 0, failed return 1
int reader_turn_next_page();
int reader_turn_prev_page();
int reader_turn_next_page10();
int reader_turn_prev_page10();

int reader_get_current_page(void);
//return the old page number
int reader_set_current_page(int pg);

//functions to clear screen
  //erase screen in vram, trigger to LCD after invoked reader_mmi_trigger_screen_update()
void reader_mmi_fill_screen(screen_t screen, color_t color);
  //erase screen immediately
void reader_mmi_fill_screen_trigger(screen_t screen, color_t color);

void reader_mmi_trigger_screen_update(screen_t screen);

void reader_mmi_select_color(screen_t s, int index, unsigned int color);

//function to hide/show info bar
int reader_info_visible(void);
void reader_set_info_visible(int v);

//return X start position according to font size
int reader_get_start_x(int fs);

//return a static buffer with given bitmap loaded
unsigned short *reader_load_bmp(unsigned short *bmpfile);

#ifdef __cplusplus
}
#endif

#endif
