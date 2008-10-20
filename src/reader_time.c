#include <reader_core.h>

static unsigned char timer_id;
static reader_time_t rtm;
static unsigned int this_reading_time, total_reading_time;

enum {
  LONG_TIME_THRESHOLD = 3600 //1 hour
};

void reader_time_init(void) {
  StartTime(true);
  timer_id = NewTimer(true);
  total_reading_time = reader_load_time();
}

void reader_set_time_info(reader_time_t *tm) {
  rtm = *tm;
}

void reader_time_inc(int seconds) {
  this_reading_time += seconds;
  total_reading_time += seconds;
  if (total_reading_time % 60 == 0) {
    //save total reading time every minute
    reader_save_time(total_reading_time);
  }
}

void reader_time_get(reader_time_t *tm) {
#ifdef WIN32
  SYSTEMTIME st;
  GetSystemTime(&st);
  tm->year = st.wYear;
  tm->month = st.wMonth;
  tm->day = st.wDay;
  tm->hour = st.wHour;
  tm->minute = st.wMinute;
  tm->second = st.wSecond;
#else
  //refer to PALIB WIKI RTC part
  tm->year = PA_RTC.Year + 2000;
  tm->month = PA_RTC.Month;
  tm->day = PA_RTC.Day;
  tm->hour = PA_RTC.Hour;
  tm->minute = PA_RTC.Minutes;
  tm->second = PA_RTC.Seconds;
#endif
}

unsigned int reader_time_elapse(void) {
  return Tick(timer_id);
}

void reader_time_reset(void) {
  ResetTimer(timer_id);
}

static void format_integer(unsigned short *buf, int i) {
  assert(i >= 0 && i < 100);
  buf[0] = i / 10 + '0';
  buf[1] = i % 10 + '0';
}

static void format_time(unsigned short *buf, int hour, int minute, int second) {
  int i = 0;

  //hour may be greater than 100 (rarely)
  if (hour < 100) {    
    format_integer(buf, hour);
    i = 2;
  } else {
    i = reader_itoa(hour, buf);
  }
  buf[i++] = ':';
  format_integer(buf + i, minute);
  i += 2;
  buf[i++] = ':';
  format_integer(buf + i, second);
  i += 2;
  buf[i] = 0;
}

static void translate_time_counter(unsigned int t, int *hour, int *minute, int *second) {
  int left_seconds;

  *hour = t / 3600;
  left_seconds = t % 3600;
  *minute = left_seconds / 60;
  *second = left_seconds % 60;
}

void reader_show_time_info(screen_t screen) {
  int text_pos_y[] = {30, 100, 170, 230};
  unsigned char *strs[] = {current_time_str, this_time_str, total_time_str, return_prompt};
  int i;
  unsigned short buf[32];
  unsigned short *p;
  int hour, minute, second;

  reader_mmi_fill_screen(screen, READER_COLOR_BLACK);

  reader_caption(screen, READER_STR(time_info_str));

  //draw static text first
  for (i = 0; i < _dim(text_pos_y); i++) {
    p = READER_STR(strs[i]);
    reader_textout_ex(screen, 0, text_pos_y[i], p, reader_wcslen(p), DEFAULT_FONT_SIZE, READER_COLOR_WHITE, 0);
  }

  //show current time
  READER_TRACE(("time=%d:%d:%d\n", rtm.hour, rtm.minute, rtm.second));
  format_time(buf, rtm.hour, rtm.minute, rtm.second);
  reader_textout_ex(screen, 40, 50, buf, reader_wcslen(buf), DEFAULT_FONT_SIZE, READER_COLOR_THIN_BLUE, 0);

  //show this reading time
  translate_time_counter(this_reading_time, &hour, &minute, &second);
  format_time(buf, hour, minute, second);
  reader_textout_ex(screen, 40, 120, buf, reader_wcslen(buf), DEFAULT_FONT_SIZE, READER_COLOR_THIN_BLUE, 0);

  if (this_reading_time > LONG_TIME_THRESHOLD) {
    p = READER_STR(long_time_msg);
    reader_textout_ex(screen, 20, 140, p, reader_wcslen(p), 14, READER_COLOR_THIN_BLUE, 0);
  }

  //show total reading time
  translate_time_counter(total_reading_time, &hour, &minute, &second);
  format_time(buf, hour, minute, second);
  reader_textout_ex(screen, 40, 190, buf, reader_wcslen(buf), DEFAULT_FONT_SIZE, READER_COLOR_THIN_BLUE, 0);

  //show date
  READER_TRACE(("date=%d/%d/%d\n", rtm.month, rtm.day, rtm.year));
  format_integer(buf, rtm.month);
  buf[2] = '/';
  format_integer(buf + 3, rtm.day);
  buf[5] = '/';
  i = reader_itoa(rtm.year, buf + 6);
  assert(i == 4);
  buf[10] = 0;
  reader_textout_ex(screen, 100, 50, buf, reader_wcslen(buf), DEFAULT_FONT_SIZE, READER_COLOR_WHITE, 0);

  reader_mmi_trigger_screen_update(screen);
}
