#ifndef __READER_TIME_H__
#define __READER_TIME_H__ 1

typedef struct _reader_time_tag {
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
} reader_time_t;

//show time info on screen
void reader_show_time_info(screen_t screen);

void reader_set_time_info(reader_time_t *tm);

//initialize and start the timer used by the reader
void reader_time_init(void);

//reset the reference time counter to current time
void reader_time_reset(void);

//return the elapsed time in ms after the last reset/init
unsigned int reader_time_elapse(void);

//return current time
void reader_time_get(reader_time_t *tm);

//increase the timer counter by seconds
void reader_time_inc(int seconds);

#endif
