#include <reader_core.h>

enum {
  FONT_R,
  FONT_G,
  FONT_B,
  BG_R,
  BG_G,
  BG_B
};

enum {
  DEF_FONT_R = 3,
  DEF_FONT_G = 3,
  DEF_FONT_B = 3,
  DEF_BG_R = 25,
  DEF_BG_G = 25,
  DEF_BG_B = 25,
  N_COLOR_OPTIONS = 6,
  MAX_COLOR_VAL = 31
};

reader_color_context_t reader_color_context, reader_color_context_bak;

reader_color_context_t reader_color_context_def = {DEF_FONT_R, DEF_FONT_G, DEF_FONT_B, DEF_BG_R, DEF_BG_G, DEF_BG_B};

static int cursor;

static color_t demo_bg_color, demo_text_color;

void reader_color_show_window(screen_t screen) {
  int text_pos_y[] = {20, 140, 230};
  int bar_pos_y[] = {50, 70, 90, 170, 190, 210};
  color_t colors[] = {READER_COLOR_RED, READER_COLOR_GREEN, READER_COLOR_BLUE};
  unsigned char *strs[] = {font_color_str, bg_color_str, user_action_prompt};
  int i, j;
  color_t bc, cc;
  unsigned char cv;
  unsigned short buf[2];

  reader_mmi_fill_screen(screen, READER_COLOR_BLACK);

  reader_caption(screen, READER_STR(color_conf_prompt));

  //draw static text
  for (i = 0; i < 3; i++) {
    unsigned short *p = READER_STR(strs[i]);
    reader_textout_ex(screen, 0, text_pos_y[i], p, reader_wcslen(p), DEFAULT_FONT_SIZE, READER_COLOR_WHITE, 0);
  }

  //draw colorful test text
  reader_rectangle(screen, 64, 110, 128, 126, demo_bg_color, 1);
  reader_textout_ex(screen, 64, 110, READER_STR(test_str), reader_wcslen(READER_STR(test_str)), DEFAULT_FONT_SIZE, demo_text_color, 0);

  //draw color bars
  for (i = 0; i < N_COLOR_OPTIONS; i++) {
    cv = reader_color_context[i];
    if (i != cursor) {
      bc = colors[i % 3];
      cc = READER_COLOR_WHITE;
    } else {
      cc = colors[i % 3];
      bc = READER_COLOR_WHITE;
    }
    reader_rectangle(screen, 10, bar_pos_y[i], 138, bar_pos_y[i] + 4, bc, 1);
    reader_rectangle(screen, 10 + cv * 4, bar_pos_y[i], 14 + cv * 4, bar_pos_y[i] + 4, cc, 1);
    j = reader_itoa(cv, buf);
    reader_textout_ex(screen, 148, bar_pos_y[i] - 6, buf, j, DEFAULT_FONT_SIZE, colors[i % 3], 0);
  }

  reader_mmi_trigger_screen_update(screen);
}

void reader_color_set() {
  reader_text_color = READER_RGB(reader_color_context[FONT_R], reader_color_context[FONT_G], reader_color_context[FONT_B]);
  reader_bg_color = READER_RGB(reader_color_context[BG_R], reader_color_context[BG_G], reader_color_context[BG_B]);
}

void reader_color_select_ready() {
  cursor = 0;
  demo_text_color = reader_text_color;
  demo_bg_color = reader_bg_color;
  memcpy(reader_color_context_bak, reader_color_context, sizeof(reader_color_context));
}

void reader_color_select_cancel() {
  memcpy(reader_color_context, reader_color_context_bak, sizeof(reader_color_context));
}

int reader_color_cursor_up(void) {
  if (cursor == 0) {
    return 1;
  }
  cursor --;
  return 0;
}

int reader_color_cursor_down(void) {
  if (cursor == N_COLOR_OPTIONS - 1) {
    return 1;
  }
  cursor ++;
  return 0;
}

int reader_color_inc(void) {
  unsigned char v = reader_color_context[cursor];
  if (v < MAX_COLOR_VAL) {
    v ++;
    reader_color_context[cursor] = v;
    if (cursor >= 0 && cursor < 3) {
      demo_text_color = READER_RGB(reader_color_context[FONT_R], reader_color_context[FONT_G], reader_color_context[FONT_B]);
    } else {
      demo_bg_color = READER_RGB(reader_color_context[BG_R], reader_color_context[BG_G], reader_color_context[BG_B]);
    }
    return 0;
  }
  return 1;
}

int reader_color_dec(void) {
  unsigned char v = reader_color_context[cursor];
  if (v > 0) {
    v --;
    reader_color_context[cursor] = v;
    if (cursor >= 0 && cursor < 3) {
      demo_text_color = READER_RGB(reader_color_context[FONT_R], reader_color_context[FONT_G], reader_color_context[FONT_B]);
    } else {
      demo_bg_color = READER_RGB(reader_color_context[BG_R], reader_color_context[BG_G], reader_color_context[BG_B]);
    }
    return 0;
  }
  return 1;
}

int reader_color_set_default(void) {
  if (0 == memcmp(reader_color_context_def, reader_color_context, sizeof(reader_color_context))) {
    return 1;
  }

  memcpy(reader_color_context, reader_color_context_def, sizeof(reader_color_context));
  demo_text_color = READER_RGB(reader_color_context[FONT_R], reader_color_context[FONT_G], reader_color_context[FONT_B]);
  demo_bg_color = READER_RGB(reader_color_context[BG_R], reader_color_context[BG_G], reader_color_context[BG_B]);
  return 0;
}
