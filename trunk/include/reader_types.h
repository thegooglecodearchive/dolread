#ifndef __READER_TYPES_H__
#define __READER_TYPES_H__ 1

//---for mmi
typedef enum {
  TOP = 1,
  BOTTOM = 0
} screen_t;

//16-bit Background
typedef unsigned short color_t;

//---for state saving
typedef enum {
  SELECT_TEXT,
  READ_TEXT,
  SELECT_COLOR,
  ABOUT_INFO,
  SHOW_TIME,
  CONFIG_OPTION,
  NO_TEXT //if no file has been found
} user_state_t;

//---for dir context saving
typedef struct _tag_dir_context {
  int cursor;
  unsigned int handle;
}
dir_context_t;

typedef struct _tag_point {
  int x;
  int y;
} point_t;

typedef struct _tag_gbk2uni_item {
  unsigned short gbk;
  unsigned short uni;
} gbk2uni_item_t;

enum {
  MAX_LAYER = 32,
  //size limitation in win32
  READER_FILENAME_MAX = 256,
  READER_PATH_MAX = 260
};

//--palib data type convention
#ifdef WIN32
typedef unsigned char u8;
typedef unsigned short u16;
typedef short s16;
typedef int s32;
typedef unsigned int u32;
#endif

#endif
