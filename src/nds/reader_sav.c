#include <reader_core.h>

typedef union _tag_reader_file_saver {
  int all;
  struct {
    int layout: 1;
    int page_number: 31;
  } parts;
} reader_file_saver_t;

enum {
  RESERVED_SPACE = 4024,  //reserved 4K bytes in the beginning for color info and future use...
  SCENARIO_ADDR = 128, //user scenario data start from here...
  TIME_ADDR = 512,
  OPTION_ADDR = 528,
  SAVER_MAGIC = 0xceec
};

void reader_save_page(unsigned int file_index, int page_number) {
  reader_file_saver_t saver;
  saver.all = PA_Load32bit(RESERVED_SPACE + file_index * sizeof(file_index));
  saver.parts.page_number = page_number;
  PA_Save32bit(RESERVED_SPACE + file_index * sizeof(file_index), saver.all);
}

//return page number saved by reader_save_page()
int reader_load_page(unsigned int file_index) {
  reader_file_saver_t saver;
  saver.all = PA_Load32bit(RESERVED_SPACE + file_index * sizeof(file_index));
  return saver.parts.page_number;
}

//--color
void reader_save_color(void) {
  PA_Save32bit(0, SAVER_MAGIC);
  PA_SaveData(4, &reader_color_context, sizeof(reader_color_context));
}

void reader_load_color(void) {
  if (PA_Load32bit(0) != SAVER_MAGIC) {
    memcpy(&reader_color_context, &reader_color_context_def, sizeof(reader_color_context));
    return;
  }

  PA_LoadData(4, &reader_color_context, sizeof(reader_color_context));  
}

//--option
void reader_save_option() {
  PA_Save32bit(OPTION_ADDR, SAVER_MAGIC);
  PA_SaveData(OPTION_ADDR + 4, &reader_option, sizeof(reader_option));
}

void reader_load_option() {
  if (PA_Load32bit(OPTION_ADDR) != SAVER_MAGIC) {
    memcpy(&reader_option, &reader_option_def, sizeof(reader_option));
    return;
  }
  PA_LoadData(OPTION_ADDR + 4, &reader_option, sizeof(reader_option));

  if (reader_option_timer_enabled()) {
    reader_option_turn_timer_reset();
  }
}

//--layout
int reader_load_layout(unsigned int file_index) {
  reader_file_saver_t saver;
  saver.all = PA_Load32bit(RESERVED_SPACE + file_index * sizeof(file_index));
  return saver.parts.layout;
}

void reader_save_layout(unsigned int file_index, int layout) {
  reader_file_saver_t saver;
  saver.all = PA_Load32bit(RESERVED_SPACE + file_index * sizeof(file_index));
  saver.parts.layout = layout;
  PA_Save32bit(RESERVED_SPACE + file_index * sizeof(file_index), saver.all);
}

//--scenario
void reader_save_scenario(reader_scenario_saver_t *data) {
  PA_Save32bit(SCENARIO_ADDR, SAVER_MAGIC);
  PA_SaveData(SCENARIO_ADDR + 4, data, sizeof(*data));
}

//return 0 means load successfully, -1 means no record saved yet
int reader_load_scenario(reader_scenario_saver_t *data) {
  if (PA_Load32bit(SCENARIO_ADDR) != SAVER_MAGIC) {
    return -1;
  }
  PA_LoadData(SCENARIO_ADDR + 4, data, sizeof(*data));
  return 0;
}

unsigned int reader_load_time(void) {
  if (PA_Load32bit(TIME_ADDR) != SAVER_MAGIC) {
    return 0;
  }
  return PA_Load32bit(TIME_ADDR + 4);
}

void reader_save_time(unsigned int t) {
  PA_Save32bit(TIME_ADDR, SAVER_MAGIC);
  PA_Save32bit(TIME_ADDR + 4, t);
}

