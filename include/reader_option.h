#ifndef __READER_OPTION_H__
#define __READER_OPTION_H__ 1

//---option UI control
void reader_option_draw_menu(screen_t screen);
void reader_option_init(void);

//return 0 means status changed, 1 means remained unchange.
int reader_option_cursor_up(void);
int reader_option_cursor_down(void);
int reader_option_cursor_left(void);
int reader_option_cursor_right(void);

void reader_option_select_ready(void);

void reader_option_select_cancel(void);

//---option data access
int reader_option_lkey_locked(void);
int reader_option_timer_enabled(void);
int reader_option_timer_interval(void);

//turn timer control
//return 1 means set timer expired, should trigger turn page
//return 0 means timer has not expired yet
int reader_option_turn_timer_check(unsigned int e);
//reset the turn page timer with the interval value defined in option
void reader_option_turn_timer_reset(void);

//background picture
int reader_option_bg_pic_enabled(void);
void reader_option_disable_bg_pic(void);
unsigned short *reader_option_bg_pic(void);

extern reader_option_saver_t reader_option, reader_option_def;

#endif
