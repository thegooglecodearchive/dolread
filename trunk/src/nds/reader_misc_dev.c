#include <PA9.h>

void reader_dev_dsl_set_brightness(unsigned char brightness) {
  PA_SetDSLBrightness(brightness);
}

