#include <reader_core.h>
#include <stdio.h>
#include <stdlib.h>

enum {
  //individual data
  PAGE_ADDR = 4,

  //8KB save file
  SAVER_SIZE = 8 *1024,

  //global data
  TAG_ADDR = 0, //8 bytes
  COLOR_ADDR = 8, //16 bytes
  TIME_ADDR = 32, //8 bytes
  OPTION_ADDR = 40, //1024 bytes

  SCENARIO_ADDR_V00 = 64,
  SCENARIO_ADDR = 1064 //2048 bytes
};

#define SAVER_MAGIC 0xceeccffc


#define SAV_FILE (reader_path_save)
static unsigned short save_filename_v00[] = {'/','D','o','l','R','e','a','d','.','s','a','v',0};

//tag for DR saver file
#define SAVER_TAG "DRSAV01"

//--page
void reader_save_page(unsigned short *filename, int page_number) {
  reader_file_saver_t saver;
  fsal_file_handle_t fd = fsal_open(filename, "rb+");
  if (fd == 0) {
    return;
  }
  fsal_seek(fd, PAGE_ADDR, SEEK_SET);
  fsal_read(&saver, sizeof(saver), 1, fd);
  saver.parts.page_number = page_number;
  fsal_seek(fd, PAGE_ADDR, SEEK_SET);
  fsal_write(&saver, sizeof(saver), 1, fd);
  fsal_close(fd);
}

//return page number saved by reader_save_page()
int reader_load_page(unsigned short *filename) {
  reader_file_saver_t saver;
  fsal_file_handle_t fd = fsal_open(filename, "rb");
  if (fd == 0) {
    return 0;
  }
  fsal_seek(fd, PAGE_ADDR, SEEK_SET);
  fsal_read(&saver, sizeof(saver), 1, fd);
  fsal_close(fd);
  return saver.parts.page_number;
}

//--layout
int reader_load_layout(unsigned short *filename) {
  reader_file_saver_t saver;
  fsal_file_handle_t fd = fsal_open(filename, "rb");
  if (fd == 0) {
    return 0;
  }
  fsal_seek(fd, PAGE_ADDR, SEEK_SET);
  fsal_read(&saver, sizeof(saver), 1, fd);
  fsal_close(fd);
  return saver.parts.layout;
}

void reader_save_layout(unsigned short *filename, int layout) {
  reader_file_saver_t saver;
  fsal_file_handle_t fd = fsal_open(filename, "rb+");
  if (fd == 0) {
    return;
  }
  fsal_seek(fd, PAGE_ADDR, SEEK_SET);
  fsal_read(&saver, sizeof(saver), 1, fd);
  saver.parts.layout = layout;
  fsal_seek(fd, PAGE_ADDR, SEEK_SET);
  fsal_write(&saver, sizeof(saver), 1, fd);
  fsal_close(fd);
}

//--color
void reader_save_color(void) {
  unsigned int i = SAVER_MAGIC;
  fsal_file_handle_t fd = fsal_open(SAV_FILE, "rb+");
  if (fd == 0) {
    return;
  }
  fsal_seek(fd, COLOR_ADDR, SEEK_SET);
  fsal_write(&i, 4, 1, fd);
  fsal_write(reader_color_context, sizeof(reader_color_context), 1, fd);
  fsal_close(fd);
}

void reader_load_color(void) {
  unsigned int i;
  fsal_file_handle_t fd = fsal_open(SAV_FILE, "rb");
  if (fd == 0) {
    memcpy(reader_color_context, reader_color_context_def, sizeof(reader_color_context));
    return;
  }
  fsal_seek(fd, COLOR_ADDR, SEEK_SET);
  fsal_read(&i, 4, 1, fd);
  if (i != SAVER_MAGIC) {
    fsal_close(fd);
    memcpy(reader_color_context, reader_color_context_def, sizeof(reader_color_context));
    return;
  }
  fsal_read(reader_color_context, sizeof(reader_color_context), 1, fd);
  fsal_close(fd);
}

//--option
void reader_save_option() {
  unsigned int i = SAVER_MAGIC;
  fsal_file_handle_t fd = fsal_open(SAV_FILE, "rb+");
  if (fd == 0) {
    return;
  }
  fsal_seek(fd, OPTION_ADDR, SEEK_SET);
  fsal_write(&i, 4, 1, fd);
  fsal_write(&reader_option, sizeof(reader_option), 1, fd);
  fsal_close(fd);
}

void reader_load_option() {
  unsigned int i;
  fsal_file_handle_t fd = fsal_open(SAV_FILE, "rb");
  if (fd == 0) {
    memcpy(&reader_option, &reader_option_def, sizeof(reader_option));
    return;
  }
  fsal_seek(fd, OPTION_ADDR, SEEK_SET);
  fsal_read(&i, 4, 1, fd);
  if (i != SAVER_MAGIC) {
    fsal_close(fd);
    memcpy(&reader_option, &reader_option_def, sizeof(reader_option));
    return;
  }
  fsal_read(&reader_option, sizeof(reader_option), 1, fd);
  fsal_close(fd);

  if (reader_option_timer_enabled()) {
    reader_option_turn_timer_reset();
  }
  
  //verify brightness range
  if (reader_option.brightness > 3) {
    reader_option.brightness = 1;
  }
  reader_dev_dsl_set_brightness(reader_option.brightness);
}

//--scenario
void reader_save_scenario(reader_scenario_saver_t *data) {
  unsigned int i = SAVER_MAGIC;
  fsal_file_handle_t fd = fsal_open(SAV_FILE, "rb+");
  if (fd == 0) {
    return;
  }
  fsal_seek(fd, SCENARIO_ADDR, SEEK_SET);
  fsal_write(&i, 4, 1, fd);
  fsal_write(data, sizeof(*data), 1, fd);
  fsal_close(fd);
}

//return 0 means load successfully, -1 means no record saved yet
int reader_load_scenario(reader_scenario_saver_t *data) {
  unsigned int i;
  fsal_file_handle_t fd = fsal_open(SAV_FILE, "rb");
  if (fd == 0) {
    return -1;
  }
  fsal_seek(fd, SCENARIO_ADDR, SEEK_SET);
  fsal_read(&i, 4, 1, fd);
  if (i != SAVER_MAGIC) {
    fsal_close(fd);
    return -1;
  }
  fsal_read(data, sizeof(*data), 1, fd);
  fsal_close(fd);
  return 0;
}

//--time
void reader_save_time(unsigned int t) {
  unsigned int i = SAVER_MAGIC;
  fsal_file_handle_t fd = fsal_open(SAV_FILE, "rb+");
  if (fd == 0) {
    return;
  }
  fsal_seek(fd, TIME_ADDR, SEEK_SET);
  fsal_write(&i, 4, 1, fd);
  fsal_write(&t, 4, 1, fd);
  fsal_close(fd);
}

unsigned int reader_load_time(void) {
  unsigned int i;
  fsal_file_handle_t fd = fsal_open(SAV_FILE, "rb");
  if (fd == 0) {
    return -1;
  }
  fsal_seek(fd, TIME_ADDR, SEEK_SET);
  fsal_read(&i, 4, 1, fd);
  if (i != SAVER_MAGIC) {
    fsal_close(fd);
    return 0;
  }
  fsal_read(&i, 4, 1, fd);
  fsal_close(fd);
  return i;
}

static int move_data(fsal_file_handle_t fd_old, fsal_file_handle_t fd_new, 
                     int pos_old, int pos_new, 
                     char *buf, int size) {
  fsal_seek(fd_old, pos_old, SEEK_SET);
  fsal_read(buf, size, 1, fd_old);
  fsal_seek(fd_new, pos_new, SEEK_SET);
  fsal_write(buf, size, 1, fd_new);
  return 0;
}

int reader_save_init(void) {
  fsal_file_handle_t fd = fsal_open(SAV_FILE, "rb"), fd_v00 = 0;
  char buf[SAVER_SIZE] = SAVER_TAG;
  int n;

  if (fd == 0) {
    fd = fsal_open(SAV_FILE, "wb");
    if (fd == 0) {
      return -1;
    }
    n = fsal_write(buf, sizeof(buf), 1, fd);
    if (n != 1) {
      fsal_close(fd);
      return -1;
    }
    //try converting the existing saver
    fd_v00 = fsal_open(save_filename_v00, "rb");
    if (fd_v00 != 0) {
      //time
      move_data(fd_v00, fd, TIME_ADDR, TIME_ADDR, buf, 8);
      //scenario
      move_data(fd_v00, fd, SCENARIO_ADDR_V00, SCENARIO_ADDR, buf, 2048);
      fsal_close(fd_v00);
    }
    fsal_close(fd);
    return 0;
  }
  fsal_close(fd);
  return 0;
}
