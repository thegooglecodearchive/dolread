#include <reader_core.h>
#include <reader_mmi.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

page_t reader_pages[MAX_PAGE];
int reader_total_pages;

static unsigned short reader_current_content[256*1024];
static int reader_current_content_len;

static int compact_layout;
static int font_size;

static int space_sizes[] = {8, 7};
static int tab_sizes[] = {16, 14};
static int font_sizes[] = {16, 14};
static font_t *asc_fonts[] = {asc_font_16, asc_font_14};

void draw_chr(screen_t s, int x, int y, unsigned short *matrix, int width, int fs, color_t color) {
  int dy, dx;
  unsigned short c;

  for (dy = 0; dy < fs; dy++) {
    c = matrix[dy];
    for (dx = 0; dx < width; dx++) {
      if (c & (1 << dx)) {
        reader_setpixel(s, x + dx, y + dy, color);
      }
    }
  }
}

//binary search wide char c in unicode table tab
//if found c in tab return the index of c
//else return -1
static int binary_search(unsigned short c, unsigned short *tab, int n) {
  int low = 0;
  int high = n - 1;
  int mid;

  while (low <= high) {
    mid = (low + high) / 2;
    if (c < tab[mid]) {
      high = mid - 1;
    } else if (c > tab[mid]) {
      low = mid + 1;
    } else {
      return mid;
    }
  }

  return -1;
}

int get_chr_width(unsigned short c, int fs) {
  int b = ((fs == DEFAULT_FONT_SIZE) ? 0 : 1);

  if (c <= 0xff) { //ascii, variable width
    switch (c) {
    case 0x20:
      return space_sizes[b];
    case 0x09:
      return tab_sizes[b];
    case 0x0a: //ignore '\n'
    case 0x0d: //ignore '\r'
      return 0;
    default:
      return asc_fonts[b][c].width;
    }
  }

  return font_sizes[b];
}

int reader_textout_ex(screen_t s, int x, int y, unsigned short *str, int n, int fs, color_t color, unsigned int flag) {
  unsigned short c;
  int index, width;
  int cnt = 0;
  int b = ((fs == DEFAULT_FONT_SIZE) ? 0 : 1);
  font_t *asc_font = asc_fonts[b];

#ifdef WIN32
  unsigned short buf[1024];

  assert(n >= 0 && n < 1024);
  memcpy(buf, str, 2*n);
  buf[n] = 0;
  READER_TRACEW((L"%s\n",buf));
#endif

  //skip unicode flag
  if (*str == 0xfeff) {
    str ++;
    n --;
  }

  if (n == 0) {
    return 0;
  }

  if (flag & TEXT_OUT_ALIGN_RIGHT) {
    width = 0;
    cnt = 0;
    while (((c = str[cnt]) != 0) && (cnt < n)) {
      width += get_chr_width(c, fs);
      cnt ++;
    }
    x -= width;
  }

  cnt = 0;
  while (((c = *str) != 0) && (cnt < n)) {
    width = get_chr_width(c, fs);
    if (width > 0) {
      if (c <= 0xff && ! isspace(c)) { //ascii, variable width
        draw_chr(s, x, y, asc_font[c].matrix, width, fs, color);
      } else { //gb2312, fixed-width
        index = binary_search(c, uni_table, FONT_TOTAL);
        if (index >= 0) {
          if (fs == DEFAULT_FONT_SIZE) {
            draw_chr(s, x, y, chn_font_matrix_16[index], width, fs, color);
          } else {
            draw_chr(s, x, y, chn_font_matrix_14[index], width, fs, color);
          }          
        }
        //we just leave it blank for unknown chars now, maybe we can draw a question mark or square instead...
      }
      x += width;
    }
    str ++;
    cnt ++;
  }

  return cnt;
}

enum {
  WHOLE_SPACE = 0x3000, /*È«½Ç¿Õ¸ñ*/
  WHOLE_PERIOD = 0x3002, /*¡£*/
  WHOLE_QUESTION = 0xff1f, /*£¿*/
  WHOLE_EXCALMATORY = 0xff01, /*£¡*/
  WHOLE_REVERSE_QUOTATION = 0x201d, /*¡±*/
  WHOLE_COLON = 0xff1a /*£º*/
};
#define reader_isspace(c) ((c) == ' ' || (c) == '\r' || (c) == '\n' || (c) == '\t' || (c) == WHOLE_SPACE)
#define reader_isletter(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))
#define reader_ismark(c) ((c) == ',' || (c) == '.' || (c) == '!' || (c) == '?' || (c) == '"' || (c) == ':' || (c) == ';' || (c) == '\'')
#define reader_is_sentence_end(c) ((c) == '\n' || (c) == WHOLE_PERIOD || (c) == WHOLE_QUESTION || (c) == WHOLE_EXCALMATORY || (c) == WHOLE_REVERSE_QUOTATION || (c) == WHOLE_COLON || (c) == '.' || (c) == '!' || (c) == '?' || (c) == '"')

int reader_last_page_lines;
int reader_last_line_chars;
int reader_format_pages(unsigned short *content, int n) {
  int np = 0, nl = 1;
  int i;
  unsigned short c, lookahead, lookbehind;
  int width;
  int last_space = -1; //for word indicator in English layout
  int need_nl = 0, skip = 0;
  int fs = reader_get_font_size();
  int max_line = reader_get_max_line(fs);
  int x_start = reader_get_start_x(fs);
  int x_off = x_start;

  if (n == 0) {
    return np + 1;
  }

  reader_pages[0].page_offset = 0;
  reader_pages[0].line_offest[0] = 0;
  for (i = 0; i < n; i++) {
    c = content[i];
    width = get_chr_width(c, font_size);

    if (c == ' ') {
      last_space = i;
    } else if (c > 0x7f) {
      //not keep a whole word when processing non-English
      last_space = -1;
    }

    if (c == '\n') { //switch to next line
      if (i < n - 1) {
        //new line when 
        //1. not compact layout
        //or 2. multiple new lines
        //or 3. a space followed
        //or 4. a end mark of a sentence preceded
        if ((! compact_layout)
          || ((i > 0) && ((lookbehind = content[i - 1]) != 0) && reader_is_sentence_end(lookbehind))
          || (((lookahead = content[i + 1]) != 0) && reader_isspace(lookahead))) {
          READER_TRACE(("\n---p%d l%d %d, switch at enter.---\n", np, nl, i));
          x_off = x_start;
          need_nl = 1;
          skip = 1;  //skip '\n'
          i ++;
        }
      }
    } else {
      x_off += width;
      READER_TRACE(("%c off=%d\n", c, x_off));
      
      if (x_off > READER_SCREEN_WIDTH) { //switch to next line
        need_nl = 1;
        if (last_space < 0) { //no space encountered
          READER_TRACE(("\n---p%d l%d %d(%c), switch at offset %d.---\n", np, nl, i, content[i], x_off));
          x_off = width + x_start; //put the overbound char to next line
        } else {
          if (reader_isletter(c) || reader_ismark(c)) { //do not split the word or word and the following mark
            x_off = x_start;
            i = last_space;
            skip = 1; //skip space
            i ++;
          } else {
            x_off = width + x_start; //put the overbound char to next line
          }
        }
      }
    }
    if (need_nl) {
      need_nl = 0;
      last_space = -1;
      if (nl == max_line) { //switch to next page
        if (np == MAX_PAGE - 1) {
          reader_last_page_lines = nl;
          reader_last_line_chars = i - (reader_pages[np].page_offset + reader_pages[np].line_offest[nl - 1]);
          return np + 1;
        }
        np ++;
        reader_pages[np].page_offset = i;
        reader_pages[np].line_offest[0] = 0;
        nl = 1;
      } else {
        reader_pages[np].line_offest[nl ++] = i - reader_pages[np].page_offset; //skip '\n'
      }
      if (skip) { //already skipped, 
        skip = 0;
        i --;
      }
    }
  }

  reader_last_page_lines = nl;
  reader_last_line_chars = i - (reader_pages[np].page_offset + reader_pages[np].line_offest[nl - 1]);

  READER_TRACE(("last lines=%d last chars=%d\n", reader_last_page_lines, reader_last_line_chars));
  return np + 1;
}

int reader_init(unsigned short *dirname, unsigned short *filename) {
  int pg;
  int n, sz;
  fsal_file_handle_t fd;
  reader_individual_saver_t saver;
  int ret;

  if (dirname != NULL) {
    if ((ret = reader_file_set_dir(dirname)) != 0) {
      return ret;
    }
  }
  if (filename != NULL) {
    reader_set_current_file(filename);
    if ((ret = reader_file_set_cursor_by_name(filename)) < 0) {
      return ERR_SET_CURSOR_BY_NAME;
    }
  } else {
    filename = reader_get_current_file();
  }

  //construct a sliding window for file content
#if TRACE_FILENAME
  {
  FILE *dbgfd;
  dbgfd = fopen("initname", "wb");
  fwrite(filename, 2, reader_wcslen(filename), dbgfd);
  fclose(dbgfd);
  }
#endif
  fd = fsal_open(filename, "rb");
  if (fd == 0) {
    return ERR_OPEN_FILE;
  }
  n = fsal_read(&saver, sizeof(saver), 1, fd);
  if ((n != 1) || (strcmp(saver.magic, "DRB") != 0)) {
    return ERR_FILE_FORMAT;
  }

  sz = sizeof(reader_current_content)/sizeof(unsigned short);
  reader_current_content_len = fsal_read(reader_current_content, sizeof(unsigned short), sz, fd);
  fsal_close(fd);
  reader_set_layout(saver.file_saver.parts.layout, 0);
  //format when all options are set
  reader_total_pages = reader_format_pages(reader_current_content, reader_current_content_len);
  READER_TRACEM(("reader_total_pages=%d\n", reader_total_pages));
  pg = saver.file_saver.parts.page_number;
  if (pg > 0 && pg < reader_total_pages - 1) {
    //restore the last time position
    //if come to the last page, read from the beginning
    reader_set_current_page(pg);
  } else {
    //read from the beginning
    reader_set_current_page(0);
  }
  return 0;
}

void reader_init_content(unsigned short *text, int size) {
  reader_set_layout(0, 0);
  reader_total_pages = reader_format_pages(text, size);
  reader_set_current_page(0);
}

int reader_get_layout(void) {
  return compact_layout;
}

static int locate_page(unsigned int offset, int n) {
  int i;

  for (i = 0; i < n - 2; i += 2) {
    if (offset >= reader_pages[i].page_offset && offset < reader_pages[i + 2].page_offset) {
      return i;
    }
  }

  return i;
}

static void reformat_text(void) {
  unsigned int pg_off;
  pg_off = reader_pages[reader_get_current_page()].page_offset;
  reader_total_pages = reader_format_pages(reader_current_content, reader_current_content_len);
  reader_set_current_page(locate_page(pg_off, reader_total_pages));
}

void reader_set_layout(int compact, int reformat) {
  int ns = (compact != 0);

  if (ns != compact_layout) {
    compact_layout = ns;
    if (reformat) {
      reformat_text();
    }
  }
}

int reader_get_font_size(void) {
  return font_size;
}

void reader_set_font_size(int fs, int reformat) {
  //adjust unsaved font size value
  if (fs != DEFAULT_FONT_SIZE &&  fs != 14) {
    fs = DEFAULT_FONT_SIZE;
  }

  if (fs != font_size) {
    font_size = fs;
    if (reformat) {
      reformat_text();
    }
  }
}

int reader_switch_font_size(void) {
  int new_size;

  if (font_size == DEFAULT_FONT_SIZE) {
    new_size = 14;
  } else {
    new_size = DEFAULT_FONT_SIZE;
  }
  //invoked by user, must reformat text
  reader_set_font_size(new_size, 1);
  return new_size;
}

int reader_get_max_line(int fs) {
  switch (fs) {
  case DEFAULT_FONT_SIZE:
    return 13;
  case 14:
    return MAX_LINE; //smallest font uses MAX_LINE
  default:
    assert(0);
  }
  //never reach here
  return -1;
}

unsigned short *reader_get_current_content(int *size) {
  if (size) {
    *size = reader_current_content_len;
  }
  return reader_current_content;
}
