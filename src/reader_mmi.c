#include <reader_core.h>

void reader_mmi_trigger_screen_update(screen_t screen) {
#ifdef USE_DOUBLE_FRAME_BUFFER
  trigger_screen_update(screen);
#endif
}

void reader_mmi_fill_screen(screen_t screen, color_t color) {
  fill_screen(screen, color);
}

void reader_mmi_fill_screen_trigger(screen_t screen, color_t color) {
  fill_screen(screen, color);
  trigger_screen_update(screen);
}

static int info_visible;

int reader_info_visible(void) {
  return info_visible;
}

void reader_set_info_visible(int v) {
  info_visible = (v != 0);
}

int get_header_blank(int fs) {
  switch (fs) {
  case DEFAULT_FONT_SIZE:
    return 3;
  case 14:
    return 2;
  default:
    assert(0);
  }
  //never reach here
  return -1;
}

int reader_get_start_x(int fs) {
  switch (fs) {
  case DEFAULT_FONT_SIZE:
    return 0;
  case 14:
    return 4;
  default:
    assert(0);
  }
  //never reach here
  return -1;
}

unsigned short *reader_load_bmp(unsigned short *bmpfile) {
  static unsigned short bmp_buf[READER_SCREEN_WIDTH*READER_SCREEN_HEIGHT];
  static unsigned short last_access_file[READER_FILENAME_MAX];
  fsal_file_handle_t fd = 0;
  int len, nr;
  void *p = NULL;

  //try the last access cache
  if (reader_wcscmp(bmpfile, last_access_file) == 0) {
    return bmp_buf;
  }
  reader_wcscpy(last_access_file, bmpfile);
  fd = fsal_open(bmpfile, "rb");
  if (fd == 0) {
    goto err;
  }
  fsal_seek(fd, 0, SEEK_END);
  len = fsal_tell(fd);
  fsal_rewind(fd);
  p = malloc(len);
  if (p == NULL) {
    goto err;
  }
  nr = fsal_read(p, len, 1, fd);
  if (nr != 1) {
    goto err;
  }
  fsal_close(fd);
  PA_LoadBmpToBuffer(bmp_buf, 0, 0, p, READER_SCREEN_HEIGHT);
  free(p);
  return bmp_buf;
err:
  if (fd != 0) {
    fsal_close(fd);
  }
  if (p != NULL) {
    free(p);
  }
  return NULL;
}

void reader_mmi_update_screen(screen_t screen, unsigned short *content, int page_number) {
  page_t *page = reader_pages + page_number;
  int i, j;
  int lines;
  int last_line_chars;
  unsigned short page_number_str[16];
  int fs = reader_get_font_size();
  int header = get_header_blank(fs);
  int max_line = reader_get_max_line(fs);
  int start_x = reader_get_start_x(fs);
  unsigned short *bmpbuf;
  unsigned short pic_name[READER_FILENAME_MAX];

  if (reader_option_bg_pic_enabled()) {
    reader_full_pic_name(pic_name, reader_option_bg_pic());
    if ((bmpbuf = reader_load_bmp(pic_name)) != NULL) {
      fill_screen_bitmap(screen, bmpbuf);
    } else {
      //can not load bg pic
      reader_option_disable_bg_pic();
      fill_screen(screen, reader_bg_color);
    }
  } else {
    fill_screen(screen, reader_bg_color);
  }

  if (page_number >= reader_total_pages) {
    //erase screen anyway
#ifdef USE_DOUBLE_FRAME_BUFFER
    trigger_screen_update(screen);
#endif
    return;
  }

  content += page->page_offset;

  if (page_number == reader_total_pages - 1) { //last page
    lines = reader_last_page_lines;
  } else {
    lines = max_line;
  }

  if (! info_visible) {
    header += 6;
  }

  for (i = 0; i < lines - 1; i++) {
    reader_textout(screen, start_x, i * (fs + LINE_INTERVAL) + header, content + page->line_offest[i], page->line_offest[i + 1] - page->line_offest[i], fs);
  }
  if (page_number == reader_total_pages - 1) {
    last_line_chars = reader_last_line_chars;
  } else {
    last_line_chars = reader_pages[page_number + 1].page_offset - (reader_pages[page_number].page_offset + page->line_offest[i]);
  }
  reader_textout(screen, start_x, i * (fs + LINE_INTERVAL) + header, content + page->line_offest[i], last_line_chars, fs);

  if (info_visible) {
    //show page number at the bottom line
    page_number ++; //start from index 0, show from 1
    if (page_number & 1) {
      //show page number in left screen
      if (page_number > reader_total_pages) {
        page_number = reader_total_pages;
      }
      i = reader_itoa(page_number, page_number_str);
      page_number_str[i ++] = '/';
      j = reader_itoa(reader_total_pages, page_number_str + i);
      assert(i + j < 12);
      if (reader_get_layout()) {
        //append compact layout flag
        reader_wcsncpy(&page_number_str[i+j], READER_STR(compact_layout_flag), 3);
        page_number_str[i + j + 3] = 0;
      } else {
        page_number_str[i + j] = 0;
      }
      reader_textout_ex(screen, 0, max_line * (fs + LINE_INTERVAL) + header, page_number_str, 16, fs, READER_COLOR_BLUE, 0);
      i = reader_itoa(page_number * 100 / reader_total_pages, page_number_str);
      page_number_str[i ++] = '%';
      reader_textout_ex(screen, READER_SCREEN_WIDTH - 1, max_line * (fs + LINE_INTERVAL) + header, page_number_str, i, fs, READER_COLOR_BLUE, TEXT_OUT_ALIGN_RIGHT);
    } else {
      //show progress bar in right screen
      int y1 = max_line * (fs + LINE_INTERVAL) + header + 6;
      int x_cur = page_number * (READER_SCREEN_WIDTH - 5) / reader_total_pages;
      reader_rectangle(screen, 0, y1, READER_SCREEN_WIDTH - 1, y1 + 3, READER_COLOR_BLUE, 1);
      reader_rectangle(screen, x_cur, y1, x_cur + 3, y1 + 3, READER_COLOR_YELLOW, 1);
    }
  }

#ifdef USE_DOUBLE_FRAME_BUFFER
  trigger_screen_update(screen);
#endif
}

int reader_current_page;

int reader_turn_next_page() {
  if (reader_current_page + 2 < reader_total_pages) {
    reader_current_page += 2;
    return 0;
  }
  return 1;
}

int reader_turn_prev_page() {
  if (reader_current_page >= 2) {
    reader_current_page -= 2;
    return 0;
  }
  return 1;
}

int reader_turn_next_page10() {
  int pg = reader_current_page;
  pg += 10;
  if (pg > reader_total_pages - 1) {
    pg = (reader_total_pages - 1) / 2 * 2;
  }
  if (pg != reader_current_page) {
    reader_current_page = pg;
    return 0;
  }
  return 1;
}

int reader_turn_prev_page10() {
  int pg = reader_current_page;
  pg -= 10;
  if (pg < 0) {
    pg = 0;
  }
  if (pg != reader_current_page) {
    reader_current_page = pg;
    return 0;
  }
  return 1;
}

int reader_get_current_page() {
  return reader_current_page;
}

int reader_set_current_page(int pg) {
  unsigned int old = reader_current_page;
  //page number must be even for dual screen
  if (pg & 1) {
    pg --;
  }
  reader_current_page = pg;
  return old;
}
