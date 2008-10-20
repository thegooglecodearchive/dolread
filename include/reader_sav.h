#ifndef __READER_SAV_H__
#define __READER_SAV_H__ 1

typedef struct {
  int state;
  unsigned short uni_dir[READER_PATH_MAX]; //path name in unicode
  unsigned short uni_file[READER_FILENAME_MAX]; //file name in unicode
  int cursor;
  int info_visible;
  int font_size;
} reader_scenario_saver_t;

typedef struct {
  unsigned char lock_l;
  unsigned char enable_timer;
  unsigned char timer_interval;
  unsigned char enable_bg_pic;
  unsigned short pic_name[READER_FILENAME_MAX];
  unsigned short font_name[READER_FILENAME_MAX]; //reserved
  unsigned char brightness;
} reader_option_saver_t;

typedef union _tag_reader_file_saver {
  unsigned int all;
  struct {
    unsigned int layout: 1;
    unsigned int page_number: 31;
  } parts;
} reader_file_saver_t;

//this structure will be written as ".drb" file header in the preprocessor
typedef struct {
  char magic[4]; //"DRB\0"
  reader_file_saver_t file_saver;
  unsigned int reserved[2];
} reader_individual_saver_t;

void reader_save_page(unsigned short *filename, int page_number);

//return page number saved by reader_save()
int reader_load_page(unsigned short *filename);

void reader_save_color(void);
void reader_load_color(void);

void reader_save_option(void);
void reader_load_option(void);

int reader_load_layout(unsigned short *filename);
void reader_save_layout(unsigned short *filename, int layout);

void reader_save_scenario(reader_scenario_saver_t *data);
//return 0 means load successfully, -1 means no record saved yet
int reader_load_scenario(reader_scenario_saver_t *data);

//return total time for reading this ROM by user
unsigned int reader_load_time(void);

void reader_save_time(unsigned int t);

//return 0 means saver file already exists before this call
//               or is created in this call
//return -1 means create saver file failed
int reader_save_init(void);
#endif
