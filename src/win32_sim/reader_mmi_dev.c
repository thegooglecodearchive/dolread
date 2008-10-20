#include <reader_core.h>
#include <windows.h>
#include <assert.h>

color_t reader_text_color;
color_t reader_bg_color;

#ifdef USE_DOUBLE_FRAME_BUFFER
color_t vram_double[2][READER_SCREEN_HEIGHT][READER_SCREEN_HEIGHT]; //use the big square to fit for both looking
#endif

void reader_setpixel(screen_t screen, int x, int y, color_t color) {
  assert(x >= 0 && x < READER_SCREEN_WIDTH && y >= 0 && y < READER_SCREEN_HEIGHT);

#ifdef USE_DOUBLE_FRAME_BUFFER
#ifdef NDS_LOOK_FEEL
  {
  int y1 = READER_SCREEN_HEIGHT - 1 - y;
  assert(x >= 0 && x < READER_SCREEN_WIDTH && y1 >= 0 && y1 < READER_SCREEN_HEIGHT);
  vram_double[screen][x][y1] = color;
  }
#else

  vram_double[screen][y][x] = color;
#endif

#else

  sim_setpixel(screen, x, y, color);
#endif
}

void fill_screen(screen_t screen, color_t color) {
#ifdef USE_DOUBLE_FRAME_BUFFER
  int i;
  for (i = 0; i < READER_SCREEN_HEIGHT*READER_SCREEN_HEIGHT; i++) {
    *((color_t *)vram_double[screen]+i) = color;
  }
#else

  int x, y;

  for (x = 0; x < READER_SCREEN_WIDTH; x ++) {
    for (y = 0; y < READER_SCREEN_HEIGHT; y ++) {
      reader_setpixel(screen, x, y, color);
    }
  }
#endif
}

void fill_screen_bitmap(screen_t screen, color_t *bmpbuf) {
  int x, y;
  int i = 0;
#ifdef USE_DOUBLE_FRAME_BUFFER
#ifndef NDS_LOOK_FEEL
  for (x = 0; x < READER_SCREEN_WIDTH; x ++) {
    for (y = READER_SCREEN_HEIGHT-1; y > -1; y --) {    
      vram_double[screen][y][x] = bmpbuf[i++];
    }
  }
#endif
#endif
}

#ifdef USE_DOUBLE_FRAME_BUFFER
void trigger_screen_update(screen_t screen) {
  int x, y;

#ifdef NDS_LOOK_FEEL

  for (y = 0; y < READER_SCREEN_WIDTH; y ++) {
    for (x = 0; x < READER_SCREEN_HEIGHT; x += 2) {
      sim_setpixel2(screen, x, y, vram_double[screen][y][x], vram_double[screen][y][x+1]);
    }
  }
#else
  for (y = 0; y < READER_SCREEN_HEIGHT; y ++) {
    for (x = 0; x < READER_SCREEN_WIDTH; x += 2) {
      sim_setpixel2(screen, x, y, vram_double[screen][y][x], vram_double[screen][y][x+1]);
    }
  }
#endif
}
#endif
