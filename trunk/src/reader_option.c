#include <reader_core.h>

enum {
  MAX_INTERVAL = 100,
  MIN_INTERVAL = 10,
  INTERVAL = 5,
  FILENAME_BUF_SIZE = 1024,
  MAX_FILES = 32,
  BMP_ID = 0x4d42 //'BM'
};

typedef enum _tag_cursor_pos_t {
  BRIGHTNESS_DR = 0,
  ENABLE_BG_PIC,
  BG_PIC_SETTING,
  ENABLE_TIMER,
  TIMER_SETTING,
  LOCK_L
} cursor_pos_t;

reader_option_saver_t reader_option, reader_option_bak;

reader_option_saver_t reader_option_def;

static cursor_pos_t cursor;
static void draw_cursor(screen_t screen);

/***************************************************************
The number N of displayed file names is limited by
the MAX_FILES(N <= MAX_FILES) and 
the FILENAME_BUF_SIZE (the total length of N files <= FILENAME_BUF_SIZE)
****************************************************************/
static int collect_bitmap_filenames(unsigned short *buf, unsigned short **pointers, int max_size);
static unsigned short *ptr_bitmap_filesnames[MAX_FILES];
static unsigned short bitmap_filenames[FILENAME_BUF_SIZE];

static int bg_pic_index;
static void dec_bg_pic_index(void);
static void inc_bg_pic_index(void);

static int bg_pic_num;

static int dec_brightness(void);
static int inc_brightness(void);

static int has_bg() {
  return (bg_pic_num > 0);
};

void reader_option_draw_menu(screen_t screen) {
  int text_pos_y[] = {30, 60, 110, 160, 230};
  unsigned char *strs[] = {lcd_brightness_str, enable_bg_pic_str, timer_turn, lock_l_str, ab_prompt};
  int i, ti;
  unsigned short *p, *p2;
  color_t time_color = READER_COLOR_WHITE, tc;
  unsigned short buf[4] = {0};

  reader_mmi_fill_screen(screen, READER_COLOR_BLACK);

  reader_caption(screen, READER_STR(option_str));

  //static text
  for (i = 0; i < _dim(text_pos_y); i++) {
    p = READER_STR(strs[i]);
    tc = READER_COLOR_WHITE;
    if ((unsigned char *)p == lcd_brightness_str) {
      //show brightness value
      ti = reader_option.brightness;
      assert(ti <= 3);
      reader_itoa(ti+1, buf);
      reader_textout_ex(screen, 164, text_pos_y[i], buf, 1, DEFAULT_FONT_SIZE, READER_COLOR_WHITE, 0);
    } else if ((unsigned char *)p == lock_l_str) {
      if (reader_option.lock_l) {
        p2 = READER_STR(yes_str);
      } else {
        p2 = READER_STR(no_str);
      }
      reader_textout_ex(screen, 160, text_pos_y[i], p2, reader_wcslen(p2), DEFAULT_FONT_SIZE, READER_COLOR_WHITE, 0);
    } else if ((unsigned char *)p == timer_turn) {
      if (reader_option.enable_timer) {
        p2 = READER_STR(yes_str);
      } else {
        p2 = READER_STR(no_str);
        time_color = READER_COLOR_GREY;
      }
      reader_textout_ex(screen, 160, text_pos_y[i], p2, reader_wcslen(p2), DEFAULT_FONT_SIZE, READER_COLOR_WHITE, 0);
    } else if ((unsigned char *)p == enable_bg_pic_str) {
      if (! has_bg()) {
        reader_option.enable_bg_pic = 0;
        tc = READER_COLOR_GREY;
      }
      if (reader_option.enable_bg_pic) {
        p2 = READER_STR(yes_str);
      } else {
        p2 = READER_STR(no_str);
      }
      reader_textout_ex(screen, 160, text_pos_y[i], p2, reader_wcslen(p2), DEFAULT_FONT_SIZE, 
        tc, 0);
    }
    reader_textout_ex(screen, 0, text_pos_y[i], p, reader_wcslen(p), DEFAULT_FONT_SIZE, tc, 0);
  }

  //show time option in grey if timer is disabled
  //else in white
  p = READER_STR(time_str);
  reader_textout_ex(screen, 0, 130, p, reader_wcslen(p), DEFAULT_FONT_SIZE, time_color, 0);
  p = READER_STR(second_str);
  reader_textout_ex(screen, 160, 130, p, reader_wcslen(p), DEFAULT_FONT_SIZE, time_color, 0);
  ti = reader_option.timer_interval;
  assert(ti >= MIN_INTERVAL && ti <= MAX_INTERVAL);
  i = reader_itoa(ti, buf);
  reader_textout_ex(screen, 140, 130, buf, i, DEFAULT_FONT_SIZE, time_color, TEXT_OUT_ALIGN_RIGHT);

  //show pic name or no_pic message
  if (! has_bg()) {
    p = READER_STR(no_bg_pic_msg);
    reader_textout_ex(screen, 0, 80, p, reader_wcslen(p), DEFAULT_FONT_SIZE, READER_COLOR_GREY, 0);
  } else {
    p = READER_STR(bg_pic_str);
    reader_textout_ex(screen, 0, 80, p, reader_wcslen(p), DEFAULT_FONT_SIZE, READER_COLOR_WHITE, 0);
    //use the first file if the bg pic is not set yet
    p = (*(reader_option.pic_name) ? reader_option.pic_name : ptr_bitmap_filesnames[bg_pic_index]);
    reader_textout_ex(screen, 60, 80, p, reader_wcslen(p), DEFAULT_FONT_SIZE, 
      (color_t)(reader_option.enable_bg_pic ? READER_COLOR_WHITE : READER_COLOR_GREY), 0);
  }

  draw_cursor(screen);

  reader_mmi_trigger_screen_update(screen);
}

void reader_option_init(void) {
  memset(&reader_option_def, 0, sizeof(reader_option_def));
  reader_option_def.timer_interval = 20;
  bg_pic_num = collect_bitmap_filenames(bitmap_filenames, ptr_bitmap_filesnames, MAX_FILES);
  if (bg_pic_num > 0) {
    reader_option_def.enable_bg_pic = 1;
    reader_wcscpy(reader_option_def.pic_name, ptr_bitmap_filesnames[0]);
  }
}

static void draw_cursor(screen_t screen) {
  point_t cursor_pos[] = {
    {150, 40}, {186, 40},
    {150, 70}, {186, 70},
    {50, 90}, {186, 90},
    {150, 120}, {186, 120},
    {110, 140}, {150, 140},
    {150, 170}, {186, 170},
  };
  point_t *p = cursor_pos + cursor*2;

  reader_arrow(screen, p[0].x, p[0].y, 5, LEFT_POINTER, READER_COLOR_YELLOW);
  reader_arrow(screen, p[1].x, p[1].y, 5, RIGHT_POINTER, READER_COLOR_YELLOW);
}

int reader_option_cursor_up(void) {
  switch (cursor) {
  case BRIGHTNESS_DR:
    return 1;
  case LOCK_L:
    if (reader_option.enable_timer) {
      cursor = TIMER_SETTING;
    } else {
      cursor = ENABLE_TIMER;
    }
    return 0;
  case ENABLE_TIMER:
    if (! has_bg()) {
      cursor = BRIGHTNESS_DR;
      return 0;
    }
    if (reader_option.enable_bg_pic) {
      cursor = BG_PIC_SETTING;
    } else {
      cursor = ENABLE_BG_PIC;
    }
    return 0;
  case TIMER_SETTING:
  case BG_PIC_SETTING:
  case ENABLE_BG_PIC:
    cursor --;
    return 0;
  default:
    assert(0);
  }
  return 0;
}

int reader_option_cursor_down(void) {
  switch (cursor) {
  case BRIGHTNESS_DR:
    if (has_bg()) {
      cursor = ENABLE_BG_PIC;
    } else {
      cursor = ENABLE_TIMER;
    }
    return 0;
  case BG_PIC_SETTING:
  case TIMER_SETTING:
    cursor ++;
    return 0;
  case ENABLE_TIMER:
    if (reader_option.enable_timer) {
      cursor = TIMER_SETTING;
    }
    else {
      cursor = LOCK_L;
    }
    return 0;
  case ENABLE_BG_PIC:
    if (reader_option.enable_bg_pic) {
      cursor = BG_PIC_SETTING;
    } else {
      cursor = ENABLE_TIMER;
    }
    return 0;
  case LOCK_L:
    return 1;
  default:
    assert(0);
  }
  return 0;
}

int reader_option_cursor_left(void) {
  switch (cursor) {
  case BRIGHTNESS_DR:
    return dec_brightness();
  case LOCK_L:
    if (reader_option.lock_l) {
      reader_option.lock_l = 0;
    } else {
      reader_option.lock_l = 1;
    }
    return 0;
  case ENABLE_TIMER:
    if (reader_option.enable_timer) {
      reader_option.enable_timer = 0;
    } else {
      reader_option.enable_timer = 1;
    }
    return 0;
  case TIMER_SETTING: {
    int t = reader_option.timer_interval;
    assert(t >= MIN_INTERVAL);
    if (t == MIN_INTERVAL) {
      return 1;
    }
    t -= INTERVAL;
    reader_option.timer_interval = t;
    return 0;
          }
  case ENABLE_BG_PIC:
    if (reader_option.enable_bg_pic) {
      reader_option.enable_bg_pic = 0;
    } else {
      reader_option.enable_bg_pic = 1;
      if (reader_option.pic_name[0] == 0) { //not initialized
        reader_wcscpy(reader_option.pic_name, ptr_bitmap_filesnames[bg_pic_index]);
      }
    }
    return 0;
  case BG_PIC_SETTING:
    dec_bg_pic_index();
    return 0;
  default:
    assert(0);
    break;
  }

  return 1;
}

int reader_option_cursor_right(void) {
  switch (cursor) {
  case BRIGHTNESS_DR:
    return inc_brightness();
  case LOCK_L:
  case ENABLE_TIMER:
  case ENABLE_BG_PIC:
    //the inverse logic is the same with cursor_left()
    return reader_option_cursor_left();
  case TIMER_SETTING: {
    int t = reader_option.timer_interval;
    assert(t <= MAX_INTERVAL);
    if (t == MAX_INTERVAL) {
      return 1;
    }
    t += INTERVAL;
    reader_option.timer_interval = t;
    return 0;
          }
  case BG_PIC_SETTING:
    inc_bg_pic_index();
    return 0;
  default:
    assert(0);
    break;
  }

  return 1;
}

static void check_cursor(void) {
  if (cursor == TIMER_SETTING && (! reader_option.enable_timer)) {
    cursor = ENABLE_TIMER;
  }

  if (cursor == BG_PIC_SETTING && (! reader_option.enable_bg_pic)) {
    cursor = ENABLE_BG_PIC;
  }

}

void reader_option_select_ready(void) {
  check_cursor();
  memcpy(&reader_option_bak, &reader_option, sizeof(reader_option));
}

void reader_option_select_cancel(void) {
  memcpy(&reader_option, &reader_option_bak, sizeof(reader_option));
  //restore brightness
  reader_dev_dsl_set_brightness(reader_option.brightness);
}

int reader_option_lkey_locked(void) {
  return reader_option.lock_l;
}

int reader_option_timer_enabled(void) {
  return reader_option.enable_timer;
}

int reader_option_timer_interval(void) {
  return reader_option.timer_interval;
}

int reader_option_bg_pic_enabled(void) {
  return reader_option.enable_bg_pic;
}

void reader_option_disable_bg_pic(void) {
  reader_option.enable_bg_pic = 0;
}

unsigned short *reader_option_bg_pic(void) {
  return reader_option.pic_name;
}

//-- turn timer control
static unsigned int turn_timer_v;
int reader_option_turn_timer_check(unsigned int e) {
  if (turn_timer_v < e) {
    return 1;
  }
  turn_timer_v -= e;
  return 0;
}

void reader_option_turn_timer_reset(void) {
  turn_timer_v = reader_option_timer_interval() * 1000; //in ms
}

//-- fs access helper
//return 0 means OK, 1 means invalid
static int check_pic_content(unsigned short *bmpfile) {
  fsal_file_handle_t fd = fsal_open(bmpfile, "rb");
  char buf[sizeof(BMPHeader0)+sizeof(BMP_Headers)];
  int nr;
  BMPHeader0 *pheader0;
  BMP_Headers *pheader;

  if (fd == 0) {
    goto err;
  }
  nr = fsal_read(buf, sizeof(buf), 1, fd);
  if (nr != 1) {
    goto err;
  }
  fsal_close(fd);
  pheader0 = (BMPHeader0 *)buf;
  READER_TRACEW((L"bmpfile=%s ", bmpfile));
  READER_TRACE(("id=%x\n", pheader0->Id));
  if (pheader0->Id != BMP_ID) {
    goto err;
  }
  pheader = (BMP_Headers *)(buf+sizeof(BMPHeader0));
  READER_TRACE(("colors=%d\n", pheader->NColors));
  if ((pheader->Width != READER_SCREEN_HEIGHT)
    ||(pheader->Height != READER_SCREEN_WIDTH)
    ||(pheader->Compression != 0 )) {
    goto err;
  }

  return 0;

err:
  if (fd != 0) {
    fsal_close(fd);
  }
  return 1;
}

//return 1 means the input file should be filtered
//return 0 means the input file is qualified
static int bg_pic_filter(unsigned short *filename, struct stat *st) {
  unsigned short ext1[5] = {'.', 'b', 'm', 'p', 0};
  unsigned short ext2[5] = {'.', 'B', 'M', 'P', 0};
  unsigned short *p;
  static unsigned short pic_name[READER_FILENAME_MAX];

  if (st->st_mode & S_IFDIR) {
    return 1;
  }
  p = reader_wcsrchr(filename, '.');
  if (p == NULL) {
    return 1;
  }
  if ((memcmp(p, ext1, 10) != 0)
    && (memcmp(p, ext2, 10) != 0)) {
    return 1;
  }
  reader_full_pic_name(pic_name, filename);
  return check_pic_content(pic_name);
}

static int collect_bitmap_filenames(unsigned short *buf, unsigned short **pointers, int max_size) {
  fsal_dir_handle_t dir_handle;
  int i, len;
  struct stat st;
  int cursor = 0;
  unsigned short *p;
  static unsigned short filename[READER_PATH_MAX]; //no stack overhead

  dir_handle = fsal_diropen(reader_path_bg);
  if (0 == dir_handle) {
    return 0;
  }
  i = 0;
  while ((i < MAX_FILES)
    && (fsal_dirnext(dir_handle, filename, &st) == 0)) {
    if (*filename == 0) {
      //no lfn is found
      continue;
    }
    if (0 == bg_pic_filter(filename, &st)) {
      len = reader_wcslen(filename);
      if (cursor+len+1 > FILENAME_BUF_SIZE) {
        break;
      }
      p = buf+cursor;
      reader_wcsncpy(p, filename, len);
      p[len] = 0;
      cursor += len+1; //move over last '\0\0'
      pointers[i ++] = p;
    }
  }

  return i;
}

//-- bg pic index operators
static void dec_bg_pic_index(void) {
  bg_pic_index --;
  if (bg_pic_index < 0) {
    bg_pic_index = bg_pic_num - 1;
  }
  reader_wcscpy(reader_option.pic_name, ptr_bitmap_filesnames[bg_pic_index]);
}

static void inc_bg_pic_index(void) {
  bg_pic_index ++;
  if (bg_pic_index >= bg_pic_num) {
    bg_pic_index = 0;
  }
  reader_wcscpy(reader_option.pic_name, ptr_bitmap_filesnames[bg_pic_index]);
}

//-- dsl brightness operators
static int dec_brightness(void) {
  if (reader_option.brightness > 0) {
    reader_option.brightness --;
    reader_dev_dsl_set_brightness(reader_option.brightness);
    return 0;
  }
  return 1;
}

static int inc_brightness(void) {
  if (reader_option.brightness < 3) {
    reader_option.brightness ++;
    reader_dev_dsl_set_brightness(reader_option.brightness);
    return 0;
  }
  return 1;
}
