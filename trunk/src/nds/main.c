#include <reader_core.h>

unsigned short *reader_text;
int reader_text_size;

user_state_t user_state;
static reader_scenario_saver_t saver;

enum {
  Start_TIME_SLOW = 50,
  Start_TIME_FAST = 25
};

static int already_draw_left;
static void on_paint(void);
//return a bool value which means need refresh or not
static int on_timer(void);

//help function to handle file errors
static unsigned short *get_reader_text(unsigned short *dirname, unsigned short *filename);

int main(void)
{
  static int magic_state;
  static unsigned short name[READER_FILENAME_MAX+1]; //less stack overhead

  // PAlib Inits
  PA_Init();
  PA_InitVBL();

  // initialize background
  PA_Init16bitBg(0, 3);
  PA_Init16bitBg(1, 3);

  //FAT and saving system initialization
  fsal_init();
  reader_save_init();

  //initialize the timer used by the reader
  reader_time_init();

  //load color configuration
  reader_load_color();
  reader_color_set();

  //load user options
  reader_option_init();
  reader_load_option();

  //set font size to default value in case no saved value
  reader_set_font_size(DEFAULT_FONT_SIZE, 0);

  if (reader_load_scenario(&saver) < 0) {
    user_state = SELECT_TEXT;
    reader_set_info_visible(1); //status info is visible by default
    reader_file_init();
  } else {
    //load global states
    user_state = (user_state_t) saver.state;
    reader_set_info_visible(saver.info_visible);
    reader_set_font_size(saver.font_size, 0);
    switch (user_state) {
    case READ_TEXT:{
        //directories information is also initialized in reader_init()
        if (0 != reader_init(saver.uni_dir, saver.uni_file)) {
          //something wrong in the saved data, ignore it
          user_state = SELECT_TEXT;
          reader_file_init();
          break;
        }
        reader_text = reader_get_current_content(NULL);
        user_state = READ_TEXT;
      }
      break;
    case SELECT_TEXT:{
        if (reader_file_set_dir(saver.uni_dir)
            || reader_file_menu_set_cursor(saver.cursor)) {
          //something wrong in the saved data, ignore it
          user_state = SELECT_TEXT;
          reader_file_init();
          break;
        }
      }
      break;
    default:
      break;
    }
  }                             //end load scenario data
  on_paint();

  while (1) {                   // Inifinite loop
    int need_refresh = 0;
    int need_save = 0;
    static unsigned int down_cnt, up_cnt, left_cnt, right_cnt;

    //background light management, turn off the light no matter what the user state is
    PA_CheckLid();

    switch (user_state) {
    case READ_TEXT:
      if (Pad.Newpress.Up) {    //left after turned
        if (0 == reader_turn_prev_page()) {
          need_refresh = 1;
        }
      } else if (Pad.Newpress.Down) {   //right after turned
        if (0 == reader_turn_next_page()) {
          need_refresh = 1;
        }
      } else if (Pad.Newpress.Right) {  //up after turned
        if (0 == reader_turn_prev_page10()) {
          need_refresh = 1;
        } else {
          //switch to the previous file
          if (0 == reader_file_menu_up()) {
            if (0 == reader_file_current_item(name)) {
              reader_text = get_reader_text(NULL, name);
              need_save = 1;
              need_refresh = 1;
            } else {
              //the previous item is not a file, move back cursor 
              reader_file_menu_down();
            }
          }
        }
      } else if (Pad.Newpress.Left) {   //down after turned
        if (0 == reader_turn_next_page10()) {
          need_refresh = 1;
        } else {
          //switch to the next file
          if (0 == reader_file_menu_down()) {
            if (0 == reader_file_current_item(name)) {
              reader_text = get_reader_text(NULL, name);
              need_save = 1;
              need_refresh = 1;
            } else {
              //the next item is not a file, move back cursor 
              reader_file_menu_up();
            }
          }
        }
      } else if (Pad.Newpress.A) {
        user_state = CONFIG_OPTION;
        reader_option_select_ready();
        need_refresh = 1;
      } else if (Pad.Newpress.B
                 || ((!reader_option_lkey_locked()) && Pad.Newpress.L)) {
        user_state = SELECT_TEXT;
        need_refresh = 1;
        need_save = 1;
      } else if (Pad.Newpress.Y) {
        user_state = SELECT_COLOR;
        reader_color_select_ready();
        need_refresh = 1;
      } else if (Pad.Newpress.X) {
        int layout = (!reader_get_layout());
        reader_set_layout(layout, 1);
        reader_save_layout(reader_get_current_file(), layout);
        need_refresh = 1;
      } else if (Pad.Newpress.R) {
        if (reader_info_visible()) {
          reader_set_info_visible(0);
        } else {
          reader_set_info_visible(1);
        }
        need_refresh = 1;
        need_save = 1;
      } else if (Pad.Newpress.Select) {
        reader_switch_font_size();
        need_refresh = 1;
        need_save = 1;
      } else if (Pad.Newpress.Start) {
        //set timer info to current time
        reader_time_t tm;
        user_state = SHOW_TIME;
        reader_time_get(&tm);
        reader_set_time_info(&tm);
        need_refresh = 1;
      }
      //for stylus control
      //one touch in full screen mode
      else if (Stylus.Newpress) {
        int y = READER_SCREEN_HEIGHT - 1 - Stylus.X;
        int x = Stylus.Y;
        if (!reader_info_visible()) {   //full screen
          if (y < 32) {
            //touch top 2 lines, back to file menu
            user_state = SELECT_TEXT;
            need_refresh = 1;
            need_save = 1;
          } else if (x < 32) {
            //touch leftmost 2 columns, turn to the previous page
            if (0 == reader_turn_prev_page()) {
              need_refresh = 1;
            }
          } else if ((x > 50) && (y > 50)) {
            //touch right-bottom part, turn to the next page
            if (0 == reader_turn_next_page()) {
              need_refresh = 1;
              need_save = 1;
            }
          }
        }
      }
      //drag the sliding bar
      else if (Stylus.Held) {
        if (reader_info_visible() && Stylus.X > 0 && Stylus.X < 25) {
          int old_pg = reader_get_current_page();
          int x_cur = Stylus.Y;
          int page_number =
              x_cur * reader_total_pages / READER_SCREEN_WIDTH;
          if (page_number > reader_total_pages - 1) {
            page_number = reader_total_pages - 1;
          }
          if (page_number != old_pg) {
            reader_set_current_page(page_number);
            need_refresh = 1;
          }
        }
      }
      //for fast turnning pages
      else if (Pad.Held.Up) {
        up_cnt++;
        //held for a while to begin fast turnning
        if (up_cnt > Start_TIME_SLOW) {
          if (0 == reader_turn_prev_page()) {
            need_refresh = 1;
          }
        }
      } else if (Pad.Held.Down) {
        down_cnt++;
        //held for a while to begin fast turnning
        if (down_cnt > Start_TIME_SLOW) {
          if (0 == reader_turn_next_page()) {
            need_refresh = 1;
          }
        }
      } else if (Pad.Held.Left) {
        left_cnt++;
        //held for a while to begin fast turnning
        if (left_cnt > Start_TIME_SLOW) {
          if (0 == reader_turn_next_page10()) {
            need_refresh = 1;
          }
        }
      } else if (Pad.Held.Right) {
        right_cnt++;
        //held for a while to begin fast turnning
        if (right_cnt > Start_TIME_SLOW) {
          if (0 == reader_turn_prev_page10()) {
            need_refresh = 1;
          }
        }
      }

      if (!Pad.Held.Up) {
        up_cnt = 0;
      }
      if (!Pad.Held.Down) {
        down_cnt = 0;
      }
      if (!Pad.Held.Left) {
        left_cnt = 0;
      }
      if (!Pad.Held.Right) {
        right_cnt = 0;
      }
      break;
    case SELECT_TEXT:
      if (Pad.Newpress.Up) {
        if (0 == reader_file_menu_prev_page()) {
          need_refresh = 1;
          need_save = 1;
        }
      } else if (Pad.Newpress.Down) {
        if (0 == reader_file_menu_next_page()) {
          need_refresh = 1;
          need_save = 1;
        }
      } else if (Pad.Newpress.Left) {
        if (0 == reader_file_menu_down()) {
          need_refresh = 1;
          need_save = 1;
        }
      } else if (Pad.Newpress.Right) {
        if (0 == reader_file_menu_up()) {
          need_refresh = 1;
          need_save = 1;
        }
      } else if (Pad.Newpress.A || Pad.Newpress.L) {
        //read the file or go into the dir
        if (0 == reader_file_current_item(name)) {
          reader_text = get_reader_text(NULL, name);
          user_state = READ_TEXT;
          already_draw_left = 0;
        } else {
          reader_file_into_dir(name);
        }
        need_refresh = 1;
        need_save = 1;
      } else if (Pad.Newpress.B) {
        if (0 == reader_file_upper_dir()) {
          need_refresh = 1;
          need_save = 1;
        }
      } else if (Pad.Newpress.Start) {
        user_state = ABOUT_INFO;
        magic_state = 0;
        reader_about_hide_magic();
        need_refresh = 1;
      }
      //the stylus in palib070615 works well!
      else if (Stylus.Newpress) {       //touch screen to select file
        int y = READER_SCREEN_HEIGHT - 1 - Stylus.X;
        int top_lim = DEFAULT_FONT_SIZE + LINE_INTERVAL;
        int bottom_lim = READER_SCREEN_HEIGHT - top_lim;
        if (y < top_lim) {
          //turn to perv page
          if (0 == reader_file_menu_prev_page()) {
            need_refresh = 1;
            need_save = 1;
          }
        } else if (y >= top_lim && y < bottom_lim) {
          int cur = reader_file_menu_map_cursor(y);
          if (cur < 0) {
            break;
          }
          if (cur != reader_file_menu_get_cursor()) {
            reader_file_menu_set_cursor(cur);
          } else {
            //read the file or go into the dir
            if (0 == reader_file_current_item(name)) {
              reader_text = get_reader_text(NULL, name);
              user_state = READ_TEXT;
              already_draw_left = 0;
            } else {
              reader_file_into_dir(name);
            }
          }
          need_refresh = 1;
          need_save = 1;
        } else {
          //turn to next page
          if (0 == reader_file_menu_next_page()) {
            need_refresh = 1;
            need_save = 1;
          }
        }
      }
      break;
    case SELECT_COLOR:
      if (Pad.Newpress.Up) {
        if (0 == reader_color_dec()) {
          need_refresh = 1;
        }
      } else if (Pad.Newpress.Down) {
        if (0 == reader_color_inc()) {
          need_refresh = 1;
        }
      } else if (Pad.Newpress.Left) {
        if (0 == reader_color_cursor_down()) {
          need_refresh = 1;
        }
      } else if (Pad.Newpress.Right) {
        if (0 == reader_color_cursor_up()) {
          need_refresh = 1;
        }
      } else if (Pad.Newpress.A) {
        reader_color_set();
        reader_save_color();
        user_state = READ_TEXT;
        already_draw_left = 0;
        need_refresh = 1;
      } else if (Pad.Newpress.B) {
        reader_color_select_cancel();
        user_state = READ_TEXT;
        already_draw_left = 0;
        need_refresh = 1;
      } else if (Pad.Newpress.X) {
        if (0 == reader_color_set_default()) {
          need_refresh = 1;
        }
      }
      //fast move when holding
      else if (Pad.Held.Up) {
        up_cnt++;
        if (up_cnt > Start_TIME_FAST) {
          if (0 == reader_color_dec()) {
            need_refresh = 1;
          }
        }
      } else if (Pad.Held.Down) {
        down_cnt++;
        if (down_cnt > Start_TIME_FAST) {
          if (0 == reader_color_inc()) {
            need_refresh = 1;
          }
        }
      }

      if (!Pad.Held.Up) {
        up_cnt = 0;
      }
      if (!Pad.Held.Down) {
        down_cnt = 0;
      }
      break;
    case ABOUT_INFO:
      if (Pad.Newpress.Start) {
        user_state = SELECT_TEXT;
        need_refresh = 1;
      } else if (Pad.Newpress.X) {
        //XXXY
        if (magic_state < 3) {
          magic_state++;
        } else {
          magic_state = 0;
        }
      } else if (Pad.Newpress.Y) {
        if (magic_state == 3) {
          reader_about_show_magic();
          need_refresh = 1;
        } else {
          magic_state = 0;
        }
      } else if (Pad.Newpress.Anykey) {
        magic_state = 0;
      }
      break;
    case SHOW_TIME:
      if (Pad.Newpress.Start) {
        user_state = READ_TEXT;
        already_draw_left = 0;
        need_refresh = 1;
      }
      break;
    case CONFIG_OPTION:
      if (Pad.Newpress.Up) {
        if (0 == reader_option_cursor_left()) {
          need_refresh = 1;
        }
      } else if (Pad.Newpress.Down) {
        if (0 == reader_option_cursor_right()) {
          need_refresh = 1;
        }
      } else if (Pad.Newpress.Left) {
        if (0 == reader_option_cursor_down()) {
          need_refresh = 1;
        }
      } else if (Pad.Newpress.Right) {
        if (0 == reader_option_cursor_up()) {
          need_refresh = 1;
        }
      } else if (Pad.Newpress.A) {
        reader_save_option();
        user_state = READ_TEXT;
        already_draw_left = 0;
        need_refresh = 1;
      } else if (Pad.Newpress.B) {
        reader_option_select_cancel();
        user_state = READ_TEXT;
        already_draw_left = 0;
        need_refresh = 1;
      }
      break;
    case NO_TEXT:              //ignore all input
      break;
    }                           //switch (user_state)

    //check status reset that triggered by user input
    //check turn page timer defined in option
    if (need_refresh && user_state == READ_TEXT
        && reader_option_timer_enabled()) {
      reader_option_turn_timer_reset();
    } else if (need_refresh && user_state == SELECT_TEXT) {
      //control rolling file name
      reader_file_reset_name_cursor();
    }
    //check timer periodically
    if (on_timer()) {
      need_refresh = 1;
    }

    if (need_refresh) {
      on_paint();
    }

    if (need_save) {
      reader_wcsncpy(saver.uni_dir, reader_file_get_dir(), READER_PATH_MAX);
      reader_wcsncpy(saver.uni_file, reader_get_current_file(), READER_FILENAME_MAX);
      saver.state = user_state;
      saver.info_visible = reader_info_visible();
      saver.font_size = reader_get_font_size();
      reader_save_scenario(&saver);
    }

    PA_WaitForVBL();
  }                             //end while (1)

  return 0;
}

static void show_reader_theme_pic(screen_t screen)
{
  unsigned short *bmpbuf = NULL;
  if ((bmpbuf = reader_load_bmp(reader_path_theme_pic)) != NULL) {
    fill_screen_bitmap(screen, bmpbuf);
    trigger_screen_update(screen);
  }
}

static void on_paint(void)
{
  switch (user_state) {
  case READ_TEXT:
    reader_save_page(reader_get_current_file(), reader_get_current_page());
    reader_mmi_update_screen(TOP, reader_text, reader_get_current_page());
    reader_mmi_update_screen(BOTTOM, reader_text,
                             reader_get_current_page() + 1);
    break;
  case SELECT_TEXT:
    if (!already_draw_left) {
      show_reader_theme_pic(TOP);
      already_draw_left = 1;
    }
    reader_file_draw_menu(BOTTOM);
    break;
  case SELECT_COLOR:
    if (!already_draw_left) {
      show_reader_theme_pic(TOP);
      already_draw_left = 1;
    }
    reader_color_show_window(BOTTOM);
    break;
  case ABOUT_INFO:
    reader_about_info(BOTTOM);
    break;
  case SHOW_TIME:
    if (!already_draw_left) {
      show_reader_theme_pic(TOP);
      already_draw_left = 1;
    }
    reader_show_time_info(BOTTOM);
    break;
  case CONFIG_OPTION: {
    unsigned short pic_name[READER_FILENAME_MAX];
    unsigned short *bmpbuf;
    if (reader_option_bg_pic_enabled()) {
      reader_full_pic_name(pic_name, reader_option_bg_pic());
      if ((bmpbuf = reader_load_bmp(pic_name)) != NULL) {
        fill_screen_bitmap(TOP, bmpbuf);
        trigger_screen_update(TOP);
      } else {
        show_reader_theme_pic(TOP);
      }
    } else {
      show_reader_theme_pic(TOP);
    }
    reader_option_draw_menu(BOTTOM);
                      }
    break;
  case NO_TEXT:
    break;
  }
}

static int on_timer(void)
{
  static reader_time_t old_tm;
  reader_time_t tm = { 0 };
  unsigned int e;
  int show_time = (user_state == SHOW_TIME);
  int ret = 0;

  if ((e = reader_time_elapse()) > (unsigned int) (show_time ? 100 : 500)) {
    //use a shorter interval to make it look precise,
    //however, a rough timer is OK if no one looks at it
    reader_time_reset();
    if (user_state == READ_TEXT && reader_option_timer_enabled()) {
      if (reader_option_turn_timer_check(e)) {
        reader_option_turn_timer_reset();
        if (0 == reader_turn_next_page()) {
          ret = 1;
        }
      }
    } else if (user_state == SELECT_TEXT && reader_file_rolling_name()) {
      reader_file_move_name(1);
      ret = 1;
    }
    //refresh time stat.
    reader_time_get(&tm);
    if (memcmp(&old_tm, &tm, sizeof(tm)) != 0) {
      old_tm = tm;
      reader_time_inc(1);
      if (show_time) {
        //time changed, refresh time info
        reader_set_time_info(&tm);
        ret = 1;
      }
    }
  }

  return ret;
}

static unsigned short *get_reader_text(unsigned short *dirname, unsigned short *filename)
{
  int ret = reader_init(dirname, filename);
  unsigned short *p;
  static unsigned short buf[32];
  int n;

  switch (ret) {
  case 0:
    return reader_get_current_content(NULL);
  case ERR_OPEN_FILE:
    p = READER_STR(err_msg_file_cant_open);
    n = reader_wcslen(p);
    reader_wcsncpy(buf, p, n);
    buf[n] = '\n';
    reader_itoa((errno<0 ? -errno : errno), &buf[n+1]);
    reader_init_content(buf, reader_wcslen(buf));
    return buf;
  case ERR_FILE_FORMAT:
    p = READER_STR(err_msg_file_format);
    reader_init_content(p, reader_wcslen(p));
    return p;
  }

  //should not reach here
  assert(0);
  return NULL;
}

