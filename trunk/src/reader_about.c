#include <reader_core.h>

static int show_magic;

void reader_about_info(screen_t screen) {
  int text_pos_y[] = {20, 40, 60, 80, 100, 120, 140, 160, 200, 230};
  unsigned char *strs[] = {program_name, version_string, build_number, copyright_str, dev_str, by_str, program_author, art_author, url_str, return_prompt};
  int i;
  unsigned short *p;
  
  reader_mmi_fill_screen(screen, READER_COLOR_BLACK);

  reader_caption(screen, READER_STR(about_info));

  for (i = 0; i < _dim(text_pos_y); i++) {
    p = READER_STR(strs[i]);
    if ((unsigned int)p == (unsigned int)program_name
      ||(unsigned int)p == (unsigned int)program_author
      ||(unsigned int)p == (unsigned int)art_author
      ||(unsigned int)p == (unsigned int)url_str
      ) {
      reader_textout_ex(screen, 0, text_pos_y[i], p, reader_wcslen(p), DEFAULT_FONT_SIZE, READER_COLOR_THIN_BLUE, 0);
    } else {
      reader_textout_ex(screen, 0, text_pos_y[i], p, reader_wcslen(p), DEFAULT_FONT_SIZE, READER_COLOR_WHITE, 0);
    }    
  }

  if (show_magic) {
    p = READER_STR(magic_str);
    reader_textout_ex(screen, 80, 100, p, reader_wcslen(p), DEFAULT_FONT_SIZE, READER_COLOR_THIN_BLUE, 0);
  }

  reader_mmi_trigger_screen_update(screen);
}

void reader_about_show_magic(void) {
  show_magic = 1;
}

void reader_about_hide_magic(void) {
  show_magic = 0;
}
