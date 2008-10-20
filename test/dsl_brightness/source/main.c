/* PAlib DSL Brightness control demo
 * by Chris Liu
 */

// PAlib include
#include <PA9.h>

void print_brightness(int x, int y, u8 c) {
  PA_OutputText(0, x, y,"brightness=%d", c);
  PA_OutputText(1, x, y,"brightness=%d", c);
}

void print_usage() {
  PA_OutputSimpleText(0, 1, 2,"A:increase B:decrease");
  PA_OutputSimpleText(1, 1, 2,"A:increase B:decrease");
}

int main(void)	{
    u8 brightness = 0;
    int updated = 0;

    // PAlib Inits
    PA_Init();
    PA_InitVBL();

    // Text Init
    PA_InitText(1, // Top screen...
            2);
    PA_InitText(0, // Bottom screen...
            2);

    print_brightness(1, 1, brightness);
    print_usage();

    while(1)    { // Inifinite loop
        updated = 0;
        if (Pad.Newpress.B) {
          if (brightness > 0) {
            brightness --;
            PA_SetDSLBrightness(brightness);
            updated = 1;
          }
        } else if (Pad.Newpress.A) {
          if (brightness < 3) {
            brightness ++;
            PA_SetDSLBrightness(brightness);
            updated = 1;
          }
        }
        if (updated) {
          print_brightness(1, 1, brightness);
        }
        PA_WaitForVBL();
    }

    return 0;
}

