#ifndef __READER_MMI_DEV_H__
#define __READER_MMI_DEV_H__ 1

//API provided here are only expected to be called by reader_mmi module
void reader_setpixel(screen_t screen, int x, int y, color_t color);
void fill_screen(screen_t screen, color_t color);
void trigger_screen_update(screen_t screen);
//for bitmap bg support
void fill_screen_bitmap(screen_t screen, color_t *bmpbuf);

#endif
