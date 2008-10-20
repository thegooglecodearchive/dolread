#include <stdlib.h>
#include <string.h>
#include <reader_core.h>

enum {
  ITEMS_PER_PAGE = 12,
  MAX_ITEMS = 256, //max entries in a single directory
  MAX_NAME_LEN = 32, //32 in unicode
  FILENAME_BORDER = 128,
  FILENAME_TAB_SIZE = 8*1024
};

static unsigned short reader_current_file[READER_FILENAME_MAX];
unsigned short reader_current_dir[READER_PATH_MAX];
int reader_file_menu_cursor;

//file names table in unicode for sort, display and handle
static unsigned short uni_names_tab[FILENAME_TAB_SIZE];

#if TRACE_FILENAME || ENABLE_CONSOLE
//buffer for trace out
static unsigned short trace_buf[4*1024];
#endif
#if TRACE_FILENAME
static int trace_ip;
#define TRACE_FN_LEN 128
#endif

/*The file list is limited by 3 elements:
1. <MAX_ITEMS>, only <MAX_ITEMS> entries will be listed in a single directory
2. sizeof(uni_names_tab), if the size of lfn of files reached this size,
   no more entries will be listed
*/

reader_file_item_t file_menu_items[MAX_ITEMS + 1];
static int n_item, n_pages;

static int roll_name_display;
static int name_cursor;

static int compare_name(const void *f1, const void *f2);

//calculate how many chars can be displayed within 'lim' pixels
static int calc_chars(unsigned short *p, int lim);
static int calc_width(unsigned short *p, int n);

unsigned short *reader_file_get_dir(void) {
  return reader_current_dir;
}

static int is_root_dir(unsigned short *path) {
  return (0 == reader_wcscmp(path, reader_path_root));
}

static int is_upper_dir(unsigned short *path) {
  if (0 == reader_wcscmp(path, reader_path_upper)) {
    return 1;
  }
  return 0;
}

//0 means the file will not be listed
//1 means a file ends with the wanted extension name
//2 means will be listed because is a directory which is not '.'
static int is_file_to_be_listed(unsigned short *filename, struct stat *st) {
  unsigned short *p;
  unsigned short ext1[4] = {'.', 'd', 'r', 0};
  unsigned short ext2[4] = {'.', 'D', 'R', 0};
  unsigned short ext3[5] = {'.', 'd', 'r', 'b', 0};
  unsigned short ext4[5] = {'.', 'D', 'R', 'B', 0};

  //do not list 'this' dir
  if (filename[0] == '.' && filename[1] == 0) {
    return 0;
  }
  //only list files ends with ".dr" or ".DR"
  if (! (st->st_mode & S_IFDIR)) {
    p = reader_wcsrchr(filename, '.');
    if ((p == NULL)) {
      return 0;
    }
    if ((memcmp(p, ext1, 8) != 0)
      && (memcmp(p, ext2, 8) != 0)
      && (memcmp(p, ext3, 10) != 0)
      && (memcmp(p, ext4, 10) != 0)) {
      return 0;
    } else {
      return 1;
    }
  }
  return 2;
}

//This function looks a little weird
//because of the weird activity of chdir() in libfat
//take a look at test_dir() in libfat unit test
//you can find it in test/fat/src/main.c
static int mychdir(unsigned short *path) {
  int ret = 0;
  unsigned short root[] = {'/', '.', 0};

#if ENABLE_CONSOLE
  char *p = "path in mydir:";
  char *p2;
  char *buf = (char *)trace_buf;
  int len = reader_wcslen(path), i;
  strcpy(buf, p);
  p2 = buf + strlen(buf);
  sprintf(p2, "%d", path[0]);
  for (i = 1; i < len; i++) {
    p2 = buf + strlen(buf);
    sprintf(p2, ",%d", path[i]);
  }
  console_log(buf);
#endif

  if (*path == '/') {
    ret = fsal_chdir(root);
    path ++;
  }
  if (*path) {
    ret = fsal_chdir(path);
    if (ret != 0) {
      //restore the working path
      if (reader_current_dir[0] != '\0') { //has been intialized
        mychdir(reader_current_dir);
      }
    }
  }
  return ret;
}

int reader_file_set_dir(unsigned short *path) {
  int i, len, attr;
  struct stat st;
  unsigned short filename[READER_PATH_MAX];
  int uni_tab_cursor = 0;
  unsigned short *puni = NULL;
  fsal_dir_handle_t dir_handle;

  if (0 != mychdir(path)) {
    return ERR_CHDIR;
  }

  if (path != reader_current_dir) {
    reader_wcsncpy(reader_current_dir, path, READER_PATH_MAX);
  }

  dir_handle = fsal_diropen(reader_path_this);

#if TRACE_FILENAME
  memset(trace_buf, 0, sizeof(trace_buf));
  trace_ip = 0;
#endif

  //calc menu item number and page number
  i = 0;
  while ((i < MAX_ITEMS)
    && (fsal_dirnext(dir_handle, filename, &st) == 0)) {
    if (*filename == 0) {
      //no lfn is found
      continue;
    }

#if TRACE_FILENAME
    if (trace_ip < sizeof(trace_buf)/sizeof(unsigned short) - TRACE_FN_LEN) {
      reader_wcsncpy(&trace_buf[trace_ip], filename, TRACE_FN_LEN);
      trace_ip += (reader_wcslen(filename)+1);
    }
#endif

    attr = is_file_to_be_listed(filename, &st);
    if (attr == 0) {
      continue;
    }
    len = reader_wcslen(filename);
    if (uni_tab_cursor+len+1 > FILENAME_TAB_SIZE) {
      break;
    }
    puni = uni_names_tab+uni_tab_cursor;
    reader_wcsncpy(puni, filename, len);
    puni[len] = 0;
    uni_tab_cursor += len+1; //move over last '\0\0'

    READER_TRACEWM((L"uniname=%s\n", puni));
    file_menu_items[i].lfn = puni;
    file_menu_items[i].is_folder = (st.st_mode & S_IFDIR);
    file_menu_items[i].size = st.st_size;
    i ++;
  }
  fsal_dirclose(dir_handle);
  n_item = i;

  n_pages = n_item / ITEMS_PER_PAGE + 1;
  if (n_item % ITEMS_PER_PAGE == 0) {
    n_pages --;
  }

  //sort the file items
  qsort(file_menu_items, n_item, sizeof(reader_file_item_t), compare_name);

#if TRACE_FILENAME
  {
    FILE *fd = NULL;
    fd = fopen("console.txt", "a+");
    fprintf(fd, "trace_ip=%d\n", trace_ip);
    fclose(fd);

    fd = fopen("fnames", "wb");
    fwrite(trace_buf, 2, trace_ip, fd);
    fclose(fd);
  }
#endif

  return 0;
}

//return if number is found in the given string
//if found any number, output the number to num arg and the substring index that the number begins to prefix arg and return 1
//else return 0
static int look_for_numbers(unsigned short *str, int *prefix, int *num) {
  unsigned short *p, *num_pos;
  int i;
  unsigned short num_buf[MAX_NAME_LEN + 1] = {0};

  p = str;
  i = 0;
  num_pos = NULL;
  while (*p) {
    if (*p >= '0' && *p <= '9') {
      if (num_pos == NULL) {
        num_pos = p;
      }
      if (i < MAX_NAME_LEN) {
        num_buf[i ++] = *p;
      }
    } else if (num_pos != NULL) { //cut the number
      break;
    }
    p ++;
  }
  if (num_pos == NULL) {
    return 0;
  }

  *prefix = num_pos - str;
  *num = reader_atoi(num_buf);

  return 1;
}

static int is_parent_folder(const reader_file_item_t *f) {
  if (! (f->is_folder)) {
    return 0;
  }

  if (0 == reader_wcscmp(f->lfn, reader_path_upper)) {
    return 1;
  }

  return 0;
}

static int compare_name(const void *f1, const void *f2) {
  const reader_file_item_t *p1 = (const reader_file_item_t *)f1;
  const reader_file_item_t *p2 = (const reader_file_item_t *)f2;
  unsigned short *n1, *n2;
  int prefix1 = 0, prefix2 = 0;
  int ret, num1, num2;

  //folders are listed prior to the files
  if (p1->is_folder != p2->is_folder) {
    if (p1->is_folder) {
      return -1;
    }
    return 1;
  }

  //parent folder is listed at the beginning
  if (is_parent_folder(p1)) {
    return -1;
  }
  if (is_parent_folder(p2)) {
    return 1;
  }

  assert (p1->is_folder == p2->is_folder);
  n1 = p1->lfn;
  n2 = p2->lfn;

  //look for numbers in names
  if (0 == look_for_numbers(n1, &prefix1, &num1)) {
    return reader_wcscmp(n1, n2);
  }

  if (0 == look_for_numbers(n2, &prefix2, &num2)) {
    return reader_wcscmp(n1, n2);
  }

  ret = reader_wcscmp_n(n1, prefix1, n2, prefix2);
  if (0 == ret) {
    //same prefix, compare numbers
    if (num1 == num2) {
      //same number, the shorter the bigger because of the prefix 0s
      return reader_wcslen(n2) - reader_wcslen(n1);
    }
    return num1 - num2;
  }

  return ret;
}

int reader_file_into_dir(unsigned short *path) {
  reader_file_item_t *pitem = NULL;
  unsigned short abspath[READER_PATH_MAX];
  int len;

  if (path != NULL) {
    if (is_upper_dir(path)) {
      return reader_file_upper_dir();
    }
    pitem = &file_menu_items[reader_file_menu_cursor];
    reader_wcsncpy(abspath, reader_current_dir, READER_PATH_MAX);
    len = reader_wcslen(abspath);
    //append sub-dir name
    if (abspath[len-1] != '/') {
      //add '/' at the end
      abspath[len++] = '/';
    }
    reader_wcscpy(&abspath[len], pitem->lfn);
    reader_file_set_dir(abspath);
  } else {
    reader_file_set_dir(reader_current_dir);
  }
  reader_file_menu_cursor = 0;
  return 0;
}

void reader_file_init(void) {
  reader_file_set_dir(reader_path_root);
  reader_file_menu_cursor = 0;
}

//return 0 means cursor changed, 1 means remained unchange.
int reader_file_menu_up(void) {
  if (reader_file_menu_cursor > 0) {
    reader_file_menu_cursor --;
    return 0;
  }
  return 1;
}

int reader_file_menu_down(void) {
  if (reader_file_menu_cursor + 1 < n_item) {
    reader_file_menu_cursor ++;
    return 0;
  }
  return 1;
}

int reader_file_menu_prev_page(void) {
  int t = reader_file_menu_cursor;

  reader_file_menu_cursor -= ITEMS_PER_PAGE;
  if (reader_file_menu_cursor < 0) {
    reader_file_menu_cursor = 0;
  }

  return (t == reader_file_menu_cursor);
}

int reader_file_menu_next_page(void) {
  int t = reader_file_menu_cursor;

  reader_file_menu_cursor += ITEMS_PER_PAGE;
  if (reader_file_menu_cursor >= n_item) {
    reader_file_menu_cursor = n_item - 1;
  }

  return (t == reader_file_menu_cursor);
}

int reader_file_current_item(unsigned short *filename) {
  reader_file_item_t *pitem = &file_menu_items[reader_file_menu_cursor];
  int ret = pitem->is_folder;

  if (filename == NULL) {
    return ret;
  }

  reader_wcsncpy(filename, pitem->lfn, READER_FILENAME_MAX);
  return ret;
}

int reader_file_menu_get_cursor_pos(void) {
  return reader_file_menu_cursor % ITEMS_PER_PAGE;
}

int reader_file_menu_get_cursor(void) {
  return reader_file_menu_cursor;
}

int reader_file_menu_set_cursor(int cursor) {
  assert(cursor >= 0);
  if (cursor >= 0 && cursor < n_item) {
    reader_file_menu_cursor = cursor;
    return 0;
  }
  return 1;
}

int reader_file_menu_map_cursor(int y) {
  int y_begin = DEFAULT_FONT_SIZE + LINE_INTERVAL;
  int y_end = READER_SCREEN_HEIGHT - (DEFAULT_FONT_SIZE + LINE_INTERVAL);
  int ret;

  if (y < y_begin || y > y_end) {
    return -1;
  }

  ret = (y - y_begin) / (DEFAULT_FONT_SIZE + LINE_INTERVAL);
  if (ret >= ITEMS_PER_PAGE) {
    ret = ITEMS_PER_PAGE - 1;
  }

  return (reader_file_menu_cursor/ITEMS_PER_PAGE)*ITEMS_PER_PAGE + ret;
}

int reader_file_menu_set_cursor_pos(int pos) {
  int current_page, current_item, new_cursor;

  assert(pos >= 0 && pos < ITEMS_PER_PAGE);
  current_page = reader_file_menu_cursor / ITEMS_PER_PAGE;
  current_item = reader_file_menu_cursor % ITEMS_PER_PAGE;
  if (current_item == pos) {
    return 1;
  }
  new_cursor = current_page * ITEMS_PER_PAGE + pos;
  if (new_cursor < n_item) {
    reader_file_menu_cursor = new_cursor;
    return 0;
  }

  return 1;
}

void reader_file_draw_menu(screen_t screen) {
  int i, j, ibegin, iend, y, nc, n;
  unsigned short tmp_str[16];
  int current_page, current_item;
  unsigned short *p;

  reader_mmi_fill_screen(screen, READER_COLOR_BLACK);

  reader_caption(screen, READER_STR(file_menu_prompt));

  assert(n_item > 0);

  //page number
  current_page = reader_file_menu_cursor / ITEMS_PER_PAGE;
  current_item = reader_file_menu_cursor % ITEMS_PER_PAGE;
  i = reader_itoa(current_page + 1, tmp_str);
  tmp_str[i ++] = '/';
  j = reader_itoa(n_pages, tmp_str + i);
  reader_textout_ex(screen, READER_SCREEN_WIDTH - 1, DEFAULT_HEADER_BLANK, tmp_str, i + j, DEFAULT_FONT_SIZE, READER_COLOR_YELLOW, TEXT_OUT_ALIGN_RIGHT);

  //show cursor
  y = (current_item + 1) * (DEFAULT_FONT_SIZE + LINE_INTERVAL) + DEFAULT_HEADER_BLANK;
  reader_rectangle(screen, 0, y, READER_SCREEN_WIDTH - 1,
                   y + DEFAULT_FONT_SIZE, READER_COLOR_WHITE, 1);

  //show menu items
  ibegin = current_page * ITEMS_PER_PAGE;
  if (current_page == n_pages - 1) { //last page
    iend = n_item;
  } else {
    iend = ibegin + ITEMS_PER_PAGE;
  }

#if TRACE_FILENAME
  memset(trace_buf, 0, sizeof(trace_buf));
  trace_ip = 0;
#endif

  roll_name_display = 0;
  for (i = ibegin, j = 0; i < iend; i ++, j ++) {
    color_t text_color = (j == current_item) ? READER_COLOR_BLACK : READER_COLOR_WHITE;
    int name_begin = (j == current_item) ? name_cursor : 0;
    y = (j + 1) * (DEFAULT_FONT_SIZE + LINE_INTERVAL) + DEFAULT_HEADER_BLANK;
    p = file_menu_items[i].lfn;
#if TRACE_FILENAME
    if (trace_ip < sizeof(trace_buf)-TRACE_FN_LEN) {
      reader_wcscpy(&trace_buf[trace_ip], p);
      trace_ip += (reader_wcslen(p)+1);
    }
#endif
    //show name
    p += name_begin;
    nc = calc_chars(p, FILENAME_BORDER);
    reader_textout_ex(screen, 0, y, p, nc, DEFAULT_FONT_SIZE, text_color, 0);
    if (nc < (int)reader_wcslen(p)) {
      unsigned short append_mark[3];
      append_mark[0] = '.';
      append_mark[1] = '.';
      append_mark[2] = 0;
      if (j == current_item) {
        roll_name_display = 1;
      }
      //append ".."
      reader_textout_ex(screen, calc_width(p, nc), y, append_mark, 2, DEFAULT_FONT_SIZE, text_color, 0);
    }

    //show additional info, file size or <dir>
    if (file_menu_items[i].is_folder) {
      reader_textout_ex(screen, READER_SCREEN_WIDTH - 1, y, READER_STR(dir_mark), reader_wcslen(READER_STR(dir_mark)), DEFAULT_FONT_SIZE, text_color, TEXT_OUT_ALIGN_RIGHT);
    } else {
      int k;
      int len, len_kb;

      //only show length as 0K when the file is empty, or else at least 1K
      len = file_menu_items[i].size;
      if (len == 0) {
        len_kb = 0;
      } else {
        len_kb = len / 1024;
        if (len_kb == 0) {
          len_kb = 1;
        }
      }
      k = reader_itoa(len_kb, tmp_str);
      tmp_str[k ++] = 'K';
      reader_textout_ex(screen, READER_SCREEN_WIDTH - 1, y, tmp_str, k, DEFAULT_FONT_SIZE, text_color, TEXT_OUT_ALIGN_RIGHT);
    }
  }

  //show current path
  //draw path status bg
  y = (ITEMS_PER_PAGE + 1) * (DEFAULT_FONT_SIZE + LINE_INTERVAL) + DEFAULT_HEADER_BLANK - 1;
  reader_rectangle(screen, 0, y, READER_SCREEN_WIDTH - 1,
                   y + DEFAULT_FONT_SIZE + 1, READER_COLOR_BLUE, 1);
  //show dir name
  n = calc_chars(reader_current_dir, READER_SCREEN_WIDTH-DEFAULT_FONT_SIZE);
  reader_textout_ex(screen, 0, y + 1, reader_current_dir, n, DEFAULT_FONT_SIZE, READER_COLOR_YELLOW, 0);

  reader_mmi_trigger_screen_update(screen);
  READER_TRACEM(("roll=%d\n", roll_name_display));

#if TRACE_FILENAME
  {
    FILE *fd = NULL;
    fd = fopen("dispname", "wb");
    fwrite(trace_buf, 2, trace_ip, fd);
    fclose(fd);
  }
#endif
}

void reader_file_reset_name_cursor(void) {
  name_cursor = 0;
}

int reader_file_rolling_name(void) {
  return roll_name_display;
}

void reader_file_move_name(int movement) {
  name_cursor += movement;
}

static int calc_chars(unsigned short *p, int lim) {
  int i = 0, len = 0;
  unsigned short c;

  while ((c = p[i]) != 0 && (len < lim)) {
    len += get_chr_width(c, DEFAULT_FONT_SIZE);
    i ++;
  }

  return i;
}

static int calc_width(unsigned short *p, int n) {
  int i;
  int width = 0;

  for (i = 0; i < n; i++) {
    width += get_chr_width(p[i], DEFAULT_FONT_SIZE);
  }

  return width;
}

unsigned short *reader_get_current_file() {
  return reader_current_file;
}

int reader_set_current_file(unsigned short *filename) {
  reader_wcsncpy(reader_current_file, filename, READER_FILENAME_MAX);
  return 0;
}

int reader_file_set_cursor_by_name(unsigned short *filename) {
  int i = 0;

  while (file_menu_items[i].is_folder) {
    i ++;
  }

  while (i < n_item) {
    if (0 == reader_wcscmp(file_menu_items[i].lfn, filename)) {
      reader_file_menu_cursor = i;
      return i;
    }
    i++;
  }

  return -1;
}

int reader_dir_set_cursor_by_name(unsigned short *dirname) {
  int i;

  for (i = 0; (i < n_item) && file_menu_items[i].is_folder; i++) {
    if (0 == reader_wcscmp(file_menu_items[i].lfn, dirname)) {
      reader_file_menu_cursor = i;
      return i;
    }
  }

  return -1;
}

int reader_file_upper_dir(void) {
  unsigned short abspath[READER_PATH_MAX];
  unsigned short prevpath[READER_PATH_MAX];
  int i, len, ret;
  unsigned short *p;

  if (is_root_dir(reader_current_dir)) {
    return 1;
  }

  reader_wcsncpy(abspath, reader_current_dir, READER_PATH_MAX);
  len = reader_wcslen(abspath);

  //remove the pervious directory string
  if (abspath[len-1] == '/') {
    abspath[len-1] = '\0'; 
  }
  p = reader_wcsrchr(abspath, '/');
  assert(p != NULL);
  reader_wcsncpy(prevpath, &p[1], READER_PATH_MAX);
  if (p == abspath) {
    //keep '/' for root dir
    p[1] = '\0';
  } else {
    *p = '\0';
  }
  if (0 != (ret = reader_file_set_dir(abspath))) {
    console_log("upper dir failed.");
    return ret;
  }
  //back to upper dir, locate cursor to the previous dir
  i = reader_dir_set_cursor_by_name(prevpath);
  if (i >= 0) {
    reader_file_menu_cursor = i;
  } else {
    reader_file_menu_cursor = 0;
  }

  return 0;
}
