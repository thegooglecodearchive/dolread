/* libfat Unit Test
 * by Chris Liu
*/

// Includes
#ifdef NDS
#include <PA9.h>                // Include for PA_Lib
#include <fat.h>
#include <sys/dir.h>
#include <errno.h>
#include <ConvertUTF.h>
#else
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

void PA_OutputSimpleText(int s, int x, int y, char *str);
void PA_OutputText(int s, int x, int y, char *fmt, ...);
void PA_InitRand(void);
int PA_Rand(void);
#endif

#define K (1024)
#define M (K*K)

#define DUMP_FN "fndump"
#define TEST_FN "FATTest.txt"
#define TEST_DAT_FN "FATTest.dat"
#define TEST_RES(r) if (!(r)) goto err;

#define _dim(a) (sizeof(a)/sizeof(a[0]))

//most cards can not pass readom seek reading
//only R4 and SC can pass this case now
#define TEST_RANDOM_READ 1

//EZ4 can pass random seek reading
//if this is set to 1
#define ALIGNED_4BYTE 1

static char large_buf[256*1024];

int dump_filenames(char *dirname, char *filename);

unsigned int ref_time(void);
typedef int tester_t();
typedef struct {
  tester_t *tester;
  char *description;
} test_desc_t;

int test_simple_rw()
{
  volatile char stack_test[4*1024] = {0};
  volatile char *p = stack_test;
  char *test_str = "Just writing text to a file :)";
  int len = strlen(test_str);
  FILE *test_write = fopen(TEST_FN, "wb");       //wb = create/truncate & write 
  fwrite(test_str, len, 1, test_write);
  fclose(test_write);

  FILE *testRead = fopen(TEST_FN, "rb");        //rb = read
  char filetext[32];
  fread(filetext, len, 1, testRead);
  fclose(testRead);
  p[2] = 1;
  return memcmp(filetext, test_str, len);
}

int test_huge_rw()
{
  char *test_str = "1234";
  FILE *test_write = fopen(TEST_DAT_FN, "wb");       //wb = create/truncate & write 
  int times = 1024*1024, i, nw = -1, nr = -1;
  for (i = 0; i < times; i++) {
    nw = fwrite(test_str, 5, 1, test_write); //write a 5MB file
    TEST_RES(1 == nw);
  }
  fclose(test_write);

  char buf[5] = {0};
  FILE *test_read = fopen(TEST_DAT_FN, "rb");
  for (i = 0; i < times; i++) {
    nr = fread(buf, 5, 1, test_read);
    TEST_RES(1 == nr);
    TEST_RES(0 == memcmp(buf, test_str, 5));
  }
  fclose(test_read);
  return 0;

err:
  PA_OutputText(0, 1, 1, "i=%d nw=%d nr=%d buf=%s", i, nw, nr, buf);
  return -1;
}

int verify(char *p, int sz) {
  char *cmp_str = "1234";
  int i;
  for (i = 0; i < sz; i++) {
    if (p[i] != cmp_str[i%5]) {
      return 0;
    }
  }
  return 1;
}

int test_block_read() {
  FILE *fd = fopen(TEST_DAT_FN, "rb");
  int err_code = -1;
  int i = 0;
  int sizes[] = {1, 2, 3, 4, 5, 16, 17, 54, 50230};
  int ret, sz = 0;

  TEST_RES(fd != NULL);
  err_code --;
  while (1 == fread(large_buf, sizeof(large_buf), 1, fd)) i++;
  TEST_RES(20 == i); // 5MB/256K=20
  err_code --;
  rewind(fd);
  fread(large_buf, sizeof(large_buf), 1, fd);
  fclose(fd);
  fd = NULL;
  for (i = 0; i < _dim(sizes); i++) {
    err_code = -10;
    fd = fopen("fat_tmp", "wb");
    TEST_RES(fd != NULL);
    err_code --;
    sz = sizes[i];
    ret = fwrite(large_buf, sz, 1, fd);
    TEST_RES(ret == 1);
    err_code --;
    fclose(fd);
    fd = fopen("fat_tmp", "rb");
    TEST_RES(fd != NULL);
    err_code --;
    ret = fread(large_buf, sz, 1, fd);
    TEST_RES(ret == 1);
    err_code --;
    fclose(fd);
    TEST_RES(verify(large_buf, sz));
    err_code --;
  }
  return 0;

err:
  PA_OutputText(0, 1, 1, "i=%d err=%d sz=%d", i, err_code, sz);
  if (fd != NULL) {
    fclose(fd);
  }
  return -1;
}

#if TEST_RANDOM_READ
static char *get_expected_string(int i) {
  switch (i) {
  case 0:
    return "1234";
  case 1:
    return "2341";
  case 2:
    return "3412";
  case 3:
    return "4123";
  default:
    //should not reach here
    return NULL;
  }
}

static int test_random_read(FILE *fd, int pos) {
  int nr;
  char buf[5] = {0};
  char *p = "NULL";

  fseek(fd, pos, SEEK_SET);
  TEST_RES(pos == ftell(fd));
  nr = fread(buf, 4, 1, fd);
  TEST_RES(1 == nr);
  p = get_expected_string(pos%4);
  TEST_RES(0 == memcmp(buf, p, 4));
  return 0;
err:
  PA_OutputText(0, 1, 4, "nr=%d buf=%s p=%s", nr, buf, p);
  return -1;
}
#endif

int test_seek()
{
  char *test_str = "1234";
  FILE *fd = NULL;
  int times = 1024*1024, i = -1, pos = -1;
  int nr = -1, nw = -1;
  int err_code = -1;

  fd = fopen(TEST_DAT_FN, "wb");       //wb = create/truncate & write
  TEST_RES(fd != NULL);
  for (i = 0; i < times; i++) {
    nw = fwrite(test_str, 4, 1, fd); //write a 4MB file
    TEST_RES(1 == nw);
  }
  nw = -2;
  fclose(fd);
  fd = NULL;

  fd = fopen(TEST_DAT_FN, "rb");
#if TEST_RANDOM_READ
  //test seek for reading
  //test random seek
  for (i = 0; i < 1024; i++) {
    pos = PA_Rand() % (4*1024*1023);
#if ALIGNED_4BYTE
    //make it 4-byte aligned
    pos &= ~3;
#endif
    //printf("pos=%d\n", pos);
    TEST_RES(0 == test_random_read(fd, pos));
  }
#endif  
  //test rewind
  rewind(fd);
  TEST_RES(0 == ftell(fd));
  char buf[16] = {0};
  nr = fread(buf, 4, 1, fd);
  TEST_RES(1 == nr);
  TEST_RES(0 == memcmp(buf, "1234", 4));
  fclose(fd);
  fd = NULL;

  //test seek for writing
  fd = fopen(TEST_DAT_FN, "rb+");       //wb = create/truncate & write 
  fseek(fd, 4, SEEK_SET);
  nw = fwrite("5678", 4, 1, fd);
  TEST_RES(1 == nw);
  err_code --;
  //fflush() doesn't work, close and re-open to flush
  fclose(fd);
  fd = fopen(TEST_DAT_FN, "rb+");
  nr = fread(buf, 8, 1, fd);
  TEST_RES(1 == nr);
  TEST_RES(0 == memcmp(buf, "12345678", 8));
  err_code --;
  fseek(fd, 3, SEEK_SET);
  nw = fwrite("58", 2, 1, fd);
  TEST_RES(1 == nw);
  fclose(fd);
  fd = fopen(TEST_DAT_FN, "rb");
  nr = fread(buf, 8, 1, fd);
  TEST_RES(1 == nr);
  TEST_RES(0 == memcmp(buf, "12358678", 8));
  
  fclose(fd);
  return 0;

err:
  PA_OutputText(0, 1, 1, "i=%d pos=%d", i, pos);
  PA_OutputText(0, 1, 2, "nw=%d nr=%d err=%d", nw, nr, err_code);
  if (fd != NULL) {
    fclose(fd);
  }
  return -1;
}

//The following test cases only work on NDS
#ifdef NDS
int test_dir()
{
  //open, iterate and close
	struct stat st;
	char filename[256] = {0};
  int status = 0;
  int n_items = 0;
  int err_code = -1;

  DIR_ITER* dir = diropen("/");
  TEST_RES(dir != NULL);
  while (dirnext(dir, filename, &st) == 0) {
    n_items ++;
  }
  TEST_RES(n_items > 0); //at least have this rom or a dir containing this rom
  dirreset(dir);
  int n_items2 = 0;
  while (dirnext(dir, filename, &st) == 0) {
    n_items2 ++;
  }
  TEST_RES(n_items2 == n_items);
  dirclose(dir);

  //make a dir, then write files
  if (0 != chdir("fat_test")) {
    TEST_RES(0 == mkdir("fat_test", 0));
    TEST_RES(0 == chdir("fat_test"));
  }
  FILE *fd = fopen("1", "wb");
  int n = fwrite("1", 1, 1, fd);
  fclose(fd);
  TEST_RES(1 == n);
  dir = diropen("../fat_test"); 
  TEST_RES(dir != NULL);
  while (dirnext(dir, filename, &st) == 0) {
    if (0 == strcmp(filename, ".")) {
      status ++;
      TEST_RES(st.st_mode & S_IFDIR);
    } else if (0 == strcmp(filename, "..")) {
      status ++;
      TEST_RES(st.st_mode & S_IFDIR);
    } else if (0 == strcmp(filename, "1")) {
      status ++;
      TEST_RES(0 == (st.st_mode & S_IFDIR));
      TEST_RES(1 == st.st_size);
    } else {
      status = -1;
      break;
    }
  }
  //3 itmes: ".", ".." and "1"
  TEST_RES(3 == status);
  dirclose(dir);

  //go to parent dir and access the file in subdir 
  err_code --;
  TEST_RES(0 == chdir(".."));
  fd = fopen("fat_test/1", "rb");
  char c;
  n = fread(&c, 1, 1, fd);
  fclose(fd);
  TEST_RES((1 == n) && ('1' == c));

  //test absolute path
  err_code --;
  status = 0;
  TEST_RES(0 == chdir("/fat_test"));
  dir = diropen("."); 
  TEST_RES(dir != NULL);
  while (dirnext(dir, filename, &st) == 0) {
    if (0 == strcmp(filename, ".")) {
      status ++;
      TEST_RES(st.st_mode & S_IFDIR);
    } else if (0 == strcmp(filename, "..")) {
      status ++;
      TEST_RES(st.st_mode & S_IFDIR);
    } else if (0 == strcmp(filename, "1")) {
      status ++;
      TEST_RES(0 == (st.st_mode & S_IFDIR));
      TEST_RES(1 == st.st_size);
    } else {
      status = -1;
      break;
    }
  }
  //3 itmes: ".", ".." and "1"
  TEST_RES(3 == status);
  dirclose(dir);

  //test upper path
  err_code --;
  if (0 != chdir("fat_test1")) {
    TEST_RES(0 == mkdir("fat_test1", 0));
    TEST_RES(0 == chdir("fat_test1"));
  }
  status = 0;
  //The following code is the only working way to change to a absolute path
  //both chdir("/fat_test") and chdir("/./fat_test") failed
  TEST_RES(0 == chdir("/."));
  TEST_RES(0 == chdir("fat_test"));
  //do we really go to "/fat_test"??
  dir = diropen("."); 
  TEST_RES(dir != NULL);
  while (dirnext(dir, filename, &st) == 0) {
    if (0 == strcmp(filename, ".")) {
      status ++;
      TEST_RES(st.st_mode & S_IFDIR);
    } else if (0 == strcmp(filename, "..")) {
      status ++;
      TEST_RES(st.st_mode & S_IFDIR);
    } else if (0 == strcmp(filename, "1")) {
      status ++;
      TEST_RES(0 == (st.st_mode & S_IFDIR));
      TEST_RES(1 == st.st_size);
    } else if (0 == strcmp(filename, "fat_test1")) {
      status ++;
      TEST_RES(st.st_mode & S_IFDIR);
    } else {
      status = -1;
      break;
    }
  }
  //4 itmes: ".", "..", "1" and "fat_test1"
  TEST_RES(4 == status);
  dirclose(dir);

  return 0;

err:
  PA_OutputText(0, 1, 1, "err=%d n_items=%d status=%d filename=%s", err_code, n_items, status, filename);
  return -1;
}
#endif

int test_copy() {
  int err_code = 0;
  int nr = -1, nw = -1;
  char *fn1 = "FATTest1.dat";
  char *fn2 = "FATTest2.dat";
  char *str = "Dolphin Reader\n";
  int len = strlen(str);
  int times = 1024, i;
  char buf[256];

  //back to root dir
#ifdef NDS
  TEST_RES(0 == chdir("/."));
#endif

  FILE *fd = fopen(fn1, "wb");
  for (i = 0; i < times; i++) {
    fwrite(str, len, 1, fd);
  }
  fclose(fd);

  FILE *fd1 = fopen(fn1, "rb");
  err_code = 1;
  TEST_RES(NULL != fd1);
  FILE *fd2 = fopen(fn2, "wb");
  err_code = 2;
  TEST_RES(NULL != fd2);
  err_code = 3;
  while ((nr = fread(buf, 1, 256, fd1)) > 0) {
    nw = fwrite(buf, 1, nr, fd2);
    TEST_RES(nw == nr);
  }
  fclose(fd1);
  fclose(fd2);
  return 0;

err:
  PA_OutputText(0, 1, 1, "err_code=%d, nr=%d, nw=%d", err_code, nr, nw);
  return -1;
}

int test_fprintf() {
  FILE *fd = NULL;
  int err_code = 0, ret = -1;
  char *p = "test number is 123";
  char buf[32] = "not_init";

  fd = fopen(TEST_FN, "wb");
  err_code --;
  TEST_RES(NULL != fd);
  ret = fprintf(fd, "test %s is %d", "number", 123);
  err_code --;
  TEST_RES(ret > 0);  
  fclose(fd);

  fd = fopen(TEST_FN, "rb");
  err_code --;
  TEST_RES(NULL != fd);
  ret = fread(buf, 1, 32, fd);
  err_code --;
  TEST_RES(ret == strlen(p) && (0 == strcmp(p, buf)));
  fclose(fd);
  return 0;

err:
  if (fd) fclose(fd);
  PA_OutputText(0, 1, 1, "err_code=%d, ret=%d, buf=%s", err_code, ret, buf);
  return -1;
}


#ifdef NDS

size_t wstrlen(const unsigned short *string) {
  size_t len = 0;
  while (*string ++) len ++;
  return len;
}

int chdir_utf16(unsigned short *dirname) {
  UTF16 *utf16Start = dirname;
  UTF16 *utf16End = dirname + wstrlen(dirname) + 1;
  UTF8 utf8_buf[256];
  UTF8 *utf8Start = utf8_buf;
	ConversionResult result;

  //convert utf-16 to utf-8
	result = ConvertUTF16toUTF8((const UTF16 **)&utf16Start, utf16End, &utf8Start, (UTF8 *)&(utf8_buf[256]), strictConversion);
  if (result != conversionOK) {
    return 1;
  }
  //pass utf-8 to chdir()
  return chdir(utf8_buf);
}

FILE *fopen_utf16(unsigned short *fn, char *mode) {
  UTF16 *utf16Start = fn;
  UTF16 *utf16End = fn + wstrlen(fn) + 1;
  UTF8 utf8_buf[256];
  UTF8 *utf8Start = utf8_buf;
	ConversionResult result;

  //convert utf-16 to utf-8
	result = ConvertUTF16toUTF8((const UTF16 **)&utf16Start, utf16End, &utf8Start, (UTF8 *)&(utf8_buf[256]), strictConversion);
  if (result != conversionOK) {
    return NULL;
  }
  //pass utf-8 to fopen()
  return fopen((char *)utf8_buf, mode);
}

//libfat dones't generate long file name in unicode
//unzip test_files.zip and copy files to the root path of SD/TF card
//before enable this test
int test_lfn() {
  int ret = -1, err_code = 0, n = -1;
  unsigned char uni_dir[12] = {
    0x2d, 0x4e, 0x87, 0x65, 0xee, 0x76, 0x55, 0x5f, 
    0x0d, 0x54, };
  char *fn = "123";
  FILE *fd = NULL;
  char buf[4] = {0};
  unsigned char uni_fn[16] =
  {
    0x00, 0x4e, 0x8c, 0x4e, 0x09, 0x4e, 0x2e, 0x00, 
    0x74, 0x00, 0x78, 0x00, 0x74, 0x00, };

  ret = chdir("/.");
  err_code --;
  TEST_RES(ret == 0);
  ret = chdir("test_unicode");
  err_code --;
  TEST_RES(ret == 0);
  ret = chdir_utf16((unsigned short *)uni_dir);
  err_code --;
  TEST_RES(ret == 0);
  fd = fopen(fn, "rb");
  err_code --;
  TEST_RES(fd != NULL);
  n = fread(buf, 1, 4, fd);
  err_code --;
  TEST_RES((n == 3) && (0 == strcmp(buf, fn)));
  fd = fopen(fn, "rb");
  err_code --;
  TEST_RES(fd != NULL);
  n = fread(buf, 1, 4, fd);
  err_code --;
  TEST_RES((n == 3) && (0 == strcmp(buf, fn)));
  fclose(fd);
  fd = fopen_utf16((unsigned short *)uni_fn, "rb");
  err_code --;
  TEST_RES(fd != NULL);
  n = fread(buf, 1, 4, fd);
  err_code --;
  TEST_RES((n == 3) && (0 == strcmp(buf, fn)));
  fclose(fd);
  //back to root path
  ret = chdir("/.");
  err_code --;
  TEST_RES(ret == 0);

  return 0;

err:
  PA_OutputText(0, 1, 1, "err_code=%d, ret=%d, errno=%d", err_code, ret, errno);
  if (0 == dump_filenames(".", DUMP_FN)) {
    PA_OutputText(0, 1, 2, "file names dumped to %s", DUMP_FN);
  }
  if (fd != NULL) {
    fclose(fd);
  }
  return -1;
}

int dump_filenames(char *dirname, char *fn) {
  static char buf[32*1024];
  static char filename[260];
  char *p;
  int cursor = 0, len, n;
  FILE *fd = NULL;
	struct stat st;
  DIR_ITER *dir = NULL;

  dir = diropen(dirname);
  while (dirnext(dir, (char *)filename, &st) == 0) {
    p = buf + cursor;
    len = strlen(filename);
    strcpy(p, filename);
    cursor += (len+1);
  }

  fd = fopen(fn, "wb");
  if (fd == NULL) {
    return -1;
  }
  n = fwrite(buf, cursor*sizeof(*buf), 1, fd);
  if (n != 1) {
    return -1;
  }
  fclose(fd);
  return 0;
}
#endif

int test_read_specific() {
#ifdef NDS
  char *files[] = {
    "/test_sp/1.bmp", 
    "/test_sp/fat_tmp.bmp"
    };
#else
  char *files[] = {"test_sp\\fat_tmp.bmp", "test_sp\\1.bmp"};
#endif
  int i, ret;

  for (i = 0; i < _dim(files); i ++) {
    PA_OutputText(0, 1, 3, "reading file %s", files[i]);
    ret = read_specific(files[i]);
    if (ret < 0) {
      return ret;
    }
  }
  return 0;
}

int read_specific(char *name) {
  FILE *fd = NULL;
  char *p = NULL;
  int err_code = 0;
  int len, pos, nr;

  fd = fopen(name, "rb");
  err_code --;
  TEST_RES(fd != NULL);
  fseek(fd, 0, SEEK_END);
  len = ftell(fd);
  err_code --;
  TEST_RES(len == 50230);
  p = malloc(len);
  rewind(fd);
  pos = ftell(fd);
  err_code --;
  TEST_RES(pos == 0);
  p[0] = 0; 
  nr = fread(p, len, 1, fd);
  err_code --;
  TEST_RES(nr == 1);
  err_code --;
  TEST_RES(p[0] == 'B');
  free(p);
  fclose(fd);
  return 0;

err:
  PA_OutputText(0, 1, 1, "err_code=%d, errno=%d", err_code, errno);
  if (fd != NULL) {
    fclose(fd);
  }
  if (p != NULL) {
    free(p);
  }
  return -1;
}

int test_malloc() {
  void *p = NULL;
  int i;

  for (i = 0; i < 3; i ++) {
    p = malloc(1024);
    PA_OutputText(0, 1, i+1, "%d malloc: %d", i+1, p);
  }
  free(p);
  p = malloc(1024);
  PA_OutputText(0, 1, i+1, "%d malloc: %d", i+1, p);

  return 0;
}

// Function: main()
int main(int argc, char **argv)
{
#ifdef NDS
  PA_Init();                    // Initializes PA_Lib
  PA_InitVBL();                 // Initializes a standard VBL

  PA_InitText(1, 0);            // Initialise the text system on the top screen
  PA_InitText(0, 0);            // Initialise the text system on the bottom screen

  PA_WaitForVBL();
  PA_WaitForVBL();
  PA_WaitForVBL();              // wait a few VBLs

  fatInitDefault();             //Initialise fat library
#endif

  PA_InitRand();

  unsigned int t1;
  int i, j, y;
  test_desc_t tests[] = {
#if 1
    {test_simple_rw, "Small(<1KB) File RW Test"},
    {test_huge_rw, "Huge(5MB) File RW Test"},
    {test_block_read, "Large Block Read Test"}, //this case requires test_huge_rw()
#ifdef NDS
    {test_dir, "Directory Functions Test"},
#endif
    {test_copy, "Copy Files Test"},
    {test_seek, "Seek File Test"},
    {test_fprintf, "fprintf() Test"},
#endif    
#ifdef NDS
    {test_lfn, "Unicode Filename Test"},
#endif
    {test_read_specific, "Read Specific File Test"},
    {test_malloc, "Malloc Test"},
  };

  PA_OutputSimpleText(1, 1, 1, "FAT Test - Do not power off");
  for (i=0; i<_dim(tests); i++) {
    j = i+1;
    if (j > 8) {
      y = 2*(j%8)+1;
    } else {
      y = 2*j+1;
    }
    PA_OutputText(1, 1, y, "%d.%s", j, tests[i].description);
    t1 = ref_time();
    TEST_RES(0 == tests[i].tester());
    PA_OutputText(1, 1, y+1, "%d seconds", ref_time() - t1);
  }

  PA_OutputSimpleText(1, 1, 20, "Test Passed.");
  PA_OutputSimpleText(1, 1, 21, "You can power off now :)");
  return 0;

err:
  PA_OutputSimpleText(1, 1, 20, "Test Failed.");
  PA_OutputSimpleText(1, 1, 21, "You can power off now :(");

#ifdef NDS
  //nothing to do but wait....
  while (1) {

    PA_WaitForVBL();
  }
#endif

  return -1;
}                               // End of main()


#ifdef WIN32

void PA_OutputSimpleText(int s, int x, int y, char *str)
{
  printf("%s\n", str);
}

void PA_OutputText(int s, int x, int y, char *fmt, ...)
{
  va_list varg;
  va_start(varg, fmt);
  vprintf(fmt, varg);
  printf("\n");
  va_end(varg);
}

void PA_InitRand(void) {
  srand(time(NULL));
}

int PA_Rand(void) {
  return rand();
}

unsigned int ref_time(void) {
  return time(NULL);
}
#else
unsigned int ref_time(void) {
  return PA_RTC.Hour*3600 + PA_RTC.Minutes*60 + PA_RTC.Seconds;
}
#endif

