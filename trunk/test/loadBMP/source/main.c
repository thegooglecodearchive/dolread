/*Display a bitmap on disk*/

// Includes
#include <PA9.h>       // Include for PA_Lib

unsigned short bmp_buf[256*192];

#define MAX_RAM_FILE_SIZE (2 * 1024 * 1024)
PA_FSRam(MAX_RAM_FILE_SIZE);

static int fs_init()
{
  int file_num = 0;

  PAFSStart = (char *) PA_FileSystem;   // Tells to use the ram...
  file_num = PA_FSRamInit();

  return file_num;
}

// Function: main()
int main(int argc, char ** argv)
{
	PA_Init();    // Initializes PA_Lib
	PA_InitVBL(); // Initializes a standard VBL	

	// This will initialise a 16 bit background on each screen. This must be loaded before any other background.
	// If you need to load this after a backgrounds, you'll have to use PA_ResetBgSys, PA_Init8bit, then reload
	// your backgrounds...
	PA_Init16bitBg(1, 3);
	PA_InitText(0, 0);

  int n = fs_init();
  if (n == 0) {
    PA_OutputText(0, 1, 0, "%s", "no file found");
    while (1)
    {
      PA_WaitForVBL();
    }
  } else {
    PA_OutputText(0, 1, 0, "%d files found", n);
  }
  int i = 0, j;
  PA_LoadBmpToBuffer(bmp_buf, 0, 0, PA_PAFSFile(i), 256);
  DMA_Copy(bmp_buf, (void*)(PA_DrawBg[1]), 256*192, DMA_16NOW);

	// Infinite loop to keep the program running
	while (1)
	{
    if (Pad.Newpress.A) {
      if (++i == n) {
        i = 0;
      }
      for(j = 1; j < 10; j++) {
        PA_OutputText(0, 1, j, "%s", "                                  ");
      }      
      PA_LoadBmpToBuffer(bmp_buf, 0, 0, PA_PAFSFile(i), 256);
      DMA_Copy(bmp_buf, (void*)(PA_DrawBg[1]), 256*192, DMA_16NOW);
      PA_OutputText(0, 1, 1, "%s", PA_PAFSFileName(i));
    }
		PA_WaitForVBL();
	}
	
	return 0;
} // End of main()

