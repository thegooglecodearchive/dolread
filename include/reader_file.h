#ifndef __READER_FILE_H__
#define __READER_FILE_H__ 1

void reader_file_init(void);

unsigned short *reader_get_current_file();
int reader_set_current_file(unsigned short *filename);

int reader_file_set_dir(unsigned short *path);
unsigned short *reader_file_get_dir(void);

unsigned short *reader_file_get_dir_uni(void);
int reader_file_set_dir_uni(unsigned short *uni_dir);

int reader_file_upper_dir(void);

void reader_file_draw_menu(screen_t screen);

//return 0 means cursor changed, 1 means remained unchange.
int reader_file_menu_up(void);
int reader_file_menu_down(void);
int reader_file_menu_prev_page(void);
int reader_file_menu_next_page(void);
//pos arg is a relative pos in the current page
int reader_file_menu_set_cursor_pos(int pos);
int reader_file_menu_set_cursor(int cursor);
int reader_file_menu_get_cursor(void);

//input y coordinate, output the cursor position
int reader_file_menu_map_cursor(int y);

int reader_file_into_dir(unsigned short *path);

//return 0 means file, 1 means dir
//set current filename to the output string 'filename'
int reader_file_current_item(unsigned short *filename);

int reader_file_rolling_name(void);
void reader_file_reset_name_cursor(void);

// > 0 means move name to right
// < 0 means move name to left
void reader_file_move_name(int movement);

//return the menu cursor index if filename is found in dir list
//return -1 if filename is not found
int reader_file_set_cursor_by_name(unsigned short *filename);
int reader_dir_set_cursor_by_name(unsigned short *dirname);

typedef struct _tag_file_item {
  unsigned short *lfn;
  int is_folder; //directory attrib read from FAT
  int size;
}
reader_file_item_t;
extern reader_file_item_t file_menu_items[];

#endif
