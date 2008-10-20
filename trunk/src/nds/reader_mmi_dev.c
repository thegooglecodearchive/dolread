#include <PA9.h>  // PAlib include
#include <reader_core.h>
#include <string.h>

color_t reader_text_color;
color_t reader_bg_color;

#ifdef USE_DOUBLE_FRAME_BUFFER
color_t vram_double[2][READER_SCREEN_WIDTH][READER_SCREEN_HEIGHT];
#endif

//input (x, y) is in logical coordinate
//logical (x, y) -> device (255-y, x)
void reader_setpixel(screen_t screen, int x, int y, color_t color) {
#ifdef USE_DOUBLE_FRAME_BUFFER
  {
  int y1 = READER_SCREEN_HEIGHT - 1 - y;
  //save in [y][x] order for device to use the locality
  vram_double[screen][x][y1] = color;
  }
#else
  PA_Put16bitPixel(screen, READER_SCREEN_HEIGHT - 1 - y, x, color);
#endif
}

void fill_screen(screen_t screen, color_t color) {
#ifdef USE_DOUBLE_FRAME_BUFFER
  int i;
  for (i = 0; i < READER_SCREEN_WIDTH*READER_SCREEN_HEIGHT; i++) {
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

#ifdef USE_DOUBLE_FRAME_BUFFER
void trigger_screen_update(screen_t screen) {
  DMA_Copy(vram_double[screen], (void*)(PA_DrawBg[screen]), READER_SCREEN_WIDTH*READER_SCREEN_HEIGHT, DMA_16NOW);
}
#endif

void fill_screen_bitmap(screen_t screen, color_t *bmpbuf) {
#ifdef USE_DOUBLE_FRAME_BUFFER
  DMA_Copy(bmpbuf, (void *)vram_double[screen], READER_SCREEN_WIDTH*READER_SCREEN_HEIGHT, DMA_16NOW);
#endif
}

