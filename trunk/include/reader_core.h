#ifndef __READER_CORE_H__
#define __READER_CORE_H__ 1

#include <reader_types.h>

//be compatible with a bool conversion in PALIB
#ifndef __cplusplus
#ifdef WIN32
enum {false, true};
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __GNUC__
#define  __attribute__(x)  /*NOTHING*/
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

//---palib api support
#ifdef WIN32
#include <windows.h>
#ifdef PAFS
#include <PA_FS_sim.h>
#endif
#include <pa_timer_sim.h>
#include <pa_draw_sim.h>
#endif

#ifdef NDS
#include <PA9.h>                // PAlib include
#endif

#ifdef FAT
#include <fsal.h>
#endif

//debug options
#define TRACE_FILENAME 0
#define ENABLE_CONSOLE 0

#include <reader_util.h>
#include <reader_mmi.h>
#include <reader_gui.h>
#include <reader_sav.h>
#include <reader_file.h>
#include <reader_color.h>
#include <reader_str_res.h>
#include <reader_about.h>
#include <reader_time.h>
#include <reader_option.h>
#include <reader_path.h>
#include <reader_misc_dev.h>
#include <all_gfx.h>

//error status code
enum {
  ERR_OPEN_FILE = -1,
  ERR_FILE_FORMAT = -2,
  ERR_CHDIR = -3,
  ERR_SET_CURSOR_BY_NAME = -4
};

enum {
  TEXT_OUT_ALIGN_RIGHT = 1
};

//output a unicode string to screen s
//return how many wide chars are outputed
int reader_textout_ex(screen_t s, int x, int y, unsigned short *str, int n, int fs, color_t color, unsigned int flag);

#define reader_textout(s, x, y, str, n, sz) reader_textout_ex(s, x, y, str, n, sz, reader_text_color, 0)

//initialize the reader text with a well processed file
//dirname: directory name in utf-16
//filename: file name in utf-16
int reader_init(unsigned short *dirname, unsigned short *filename);
//initialize the reader text with a error message
void reader_init_content(unsigned short *text, int size);

int get_chr_width(unsigned short c, int fs);

int reader_get_layout(void);

//if reformat is 1 means refresh text immediately
void reader_set_layout(int compact, int reformat);
void reader_set_font_size(int fs, int reformat);
int reader_get_font_size(void);

//switch between font 14*14 and 16*16
//return new font size
int reader_switch_font_size(void);

//get max line number according to the font size
int reader_get_max_line(int fs);

unsigned short *reader_get_current_content(int *size);

typedef struct _font_tag {
  unsigned char width;
  //though 14*14 using only 14*16bits, 
  //however, keep all ascii font the same unsigned short[16] type for convenience
  //wasted 2 * 2 * 256 = 1K memory
  unsigned short matrix[16]; 
} font_t;

extern page_t reader_pages[MAX_PAGE];
extern int reader_total_pages;

extern int reader_last_page_lines;
extern int reader_last_line_chars;

//code tables
extern unsigned short uni_table[];
extern gbk2uni_item_t gbk2uni_table[];

//font tables
extern unsigned short chn_font_matrix_14[][14];
extern unsigned short chn_font_matrix_16[][16];
extern font_t asc_font_14[256];
extern font_t asc_font_16[256];

//---common util macro
#define _dim(a) (sizeof(a) / sizeof((a)[0]))

//---trace function---
#ifdef WIN32
#include <assert.h>
#include <stdio.h>

extern HWND reader_hwnd;
void msg_box(char *msg);

//must trace
#define READER_TRACEM(x) printf x
#define READER_TRACEWM(x) wprintf x

#ifdef READER_ENABLE_TRACE
#define READER_TRACE(x) printf x
#define READER_TRACEW(x) wprintf x
#else
#define READER_TRACE(x)
#define READER_TRACEW(x)
#endif // #ifdef READER_ENABLE_TRACE

#else
//disable assert on ds device
#define assert(exp)     ((void)0)

#define READER_TRACE(x)
#define READER_TRACEM(x)

#define READER_TRACEW(x)
#define READER_TRACEWM(x)

#endif // #ifdef WIN32s

#ifdef __cplusplus
}
#endif

#endif
