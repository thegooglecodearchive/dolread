#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <direct.h>
#include "dirIterate.h"
#include "dfconv.h"
#include <windows.h>
#include <reader_core.h>

static char *inDir, *outDir;
static int dirLen;

char cwd[_MAX_PATH];

#define EXT_NAME ".dr"

//return the size of processed buffer
static int preprocess_content(unsigned short *src, unsigned short *dst, int size) {
  int i, j;

  //convert "\r\n" to "\n"
  for (i = 0, j = 0; i < size - 1; i++) {
    if (src[i] != '\r') {
      dst[j ++] = src[i];
    } else {
      if (src[i + 1] == '\n') {
        dst[j ++] = '\n';
        i ++;
      }
    }
  }

  //final char
  dst[j ++] = src[size - 1];

  return j;
}

char *get_file_content(char *fn, int *psz)
{
  int n_read, size;
  FILE *fd;
  char *content;

  if ((fd = fopen(fn, "rb")) == NULL) {
    fprintf(stderr, "can't open file %s.\n", fn);
    return NULL;
  }
  fseek(fd, 0, SEEK_END);
  size = ftell(fd);
  rewind(fd);
  content = (char *)malloc(size * sizeof(char));
  if (content == NULL) {
    fprintf(stderr, "memory exhausted.\n");
    fclose(fd);
    return NULL;
  }
  n_read = fread(content, 1, size, fd);
  fclose(fd);
  if (n_read < size) {
    fprintf(stderr, "read file %s error.\n", fn);
    free(content);
    return NULL;
  }

  *psz = n_read;
  return content;
}

//text buffer in NDS device is 512KB
#define BUF_SIZE (255*1025) //in unicode size

static int iter_op(char *fileName) {
  char outName[_MAX_PATH], infn[_MAX_PATH];
  char buf[_MAX_PATH];
  int len, n, nw;
  char *p;
  char cmdbuf[512];
  int is_html = 0;
  char *content;
  unsigned short *content_uni, *final_content_uni, *cursor;
  FILE *fd;
  reader_individual_saver_t saver = {"DRB", {0}};
  int findex; //file index if large file is divided into many parts

  fprintf(stderr, "converting %s...", fileName);

  len = strlen(fileName);
  assert(len < _MAX_PATH);
  if (len < 5) {
	  printf("type not match.\n");
    return -1;
  }

  if (_stricmp(&fileName[len - 4], ".txt") != 0 
    && _stricmp(&fileName[len - 4], ".htm") != 0 
    && _stricmp(&fileName[len - 5], ".html") != 0
    && (len <= 6 || _stricmp(&fileName[len - 6], ".shtml") != 0)) {
	  printf("type not match.\n");
    return -1;
  }
  
  if (_stricmp(&fileName[len - 4], ".htm") == 0 
    || _stricmp(&fileName[len - 5], ".html") == 0
    || (len > 6 && _stricmp(&fileName[len - 6], ".shtml") == 0)) {
    is_html = 1;
  }

  strcpy(infn, fileName);

  if (_stricmp(&fileName[len - 4], ".txt") == 0) {
    strcpy(&infn[len - 4], EXT_NAME);
  }

  if (is_html) {
    if (_stricmp(&infn[len - 4], ".htm") == 0) {
      strcpy(&infn[len - 4], EXT_NAME);
    } else if (_stricmp(&infn[len - 5], ".html") == 0) {
      strcpy(&infn[len - 5], EXT_NAME);
    } else if (_stricmp(&infn[len - 6], ".shtml") == 0) {
      strcpy(&infn[len - 6], EXT_NAME);
    }
  }

  sprintf(outName, "%s\\%s", outDir, infn + dirLen + 1);
  p = strrchr(outName, '\\');
  assert(p != NULL);
  strcpy(buf, outName);  
  buf[p - outName] = 0;
  _getcwd(cwd,_MAX_PATH);
  if (_chdir(buf)) {
    //dst dir doesn't exist, make it
    sprintf(cmdbuf, "mkdir \"%s\"", buf);
    if (system(cmdbuf) < 0) {
      fprintf(stderr, "mkdir %s failed.\n", buf);
    }
  }
  _chdir(cwd);

  content = get_file_content(fileName, &len);
  if (content == NULL) {
    fprintf(stderr, "failed.\n");
    return -1;
  }

  if (is_html) {
    len = dfConvert(content, len);
  }

  if ((unsigned char)content[0] == (unsigned char)0xff && (unsigned char)content[1] == (unsigned char)0xfe) { //unicode endian flag
    n = (len - 2) / 2;
    content_uni = (unsigned short *)malloc(len - 2);
    if (content_uni == NULL) {
      free(content);
      return -1;
    }
    memcpy(content_uni, content + 2, len - 2);
  } else {
    //get unicode buffer required length
    n = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, content, len,
                            NULL, 0);
    content_uni = (unsigned short *)malloc(n * sizeof(unsigned short));
    if (content_uni == NULL) {
      free(content);
      return -1;
    }
    n = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, content, len,
                            content_uni, n);
  }
  free(content);
  final_content_uni = (unsigned short *)malloc(n * sizeof(unsigned short));
  if (final_content_uni == NULL) {
    fprintf(stderr, "memory exhausted.\n");
    free(content_uni);
    return -1;
  }
  n = preprocess_content(content_uni, final_content_uni, n);
  free(content_uni);

  cursor = final_content_uni;
  findex = 0;
  while (n > 0) {
    int to_write = n;
    if (findex == 0) {
      fd = fopen(outName, "wb");
    } else {
      //get clips file name
      char clipName[_MAX_PATH];
      char clipIndex[16];
      char *ptmp;
      int ncp;
      ptmp = strrchr(outName, '.');
      assert(ptmp != NULL);
      ncp = ptmp - outName;
      strncpy(clipName, outName, ncp);
      sprintf(clipIndex, "-P%d.dr", findex);
      strcpy(&clipName[ncp], clipIndex);
      fd = fopen(clipName, "wb");
    }
    if (fd == NULL) {
      fprintf(stderr, "failed.\n");
      return -1;
    }
    //write file individual saver at the beginning
    fwrite(&saver, sizeof(saver), 1, fd);
    if (n > BUF_SIZE) {
      to_write = BUF_SIZE;
    }
    nw = fwrite(cursor, 2, to_write, fd);
    fclose(fd);
    n -= nw;
    cursor += nw;
    findex ++;
  }
  free(final_content_uni);
  fprintf(stderr, "converted to %s.\n", outName);
  return 0;
}

int initDirectories(char *dirInput, char *dirOutput)
{
  _getcwd(cwd,_MAX_PATH);

  //test input dir exists
  if (_chdir(dirInput)) {
    return -2;
  }

  //go back to working dir
  _chdir(cwd);

  //test output dir exists
  if (_chdir(dirOutput)) {
    if (_mkdir(dirOutput)) {
      return -1;
    }
  }

  //go back to working dir
  _chdir(cwd);

  return 0;
}

int main(int argc, char *argv[])
{
  int ret;
  char dirname[_MAX_PATH];
  char dirname_in[_MAX_PATH];
  char dirname_out[_MAX_PATH];

  if (argc < 3) {
    printf("usage: rec_conv <input directory> <output directory>\n");
    return -1;
  }

  _getcwd(dirname, _MAX_PATH);
  strcat(dirname, "\\");
  strcpy(dirname_in, dirname);
  strcat(dirname_in, argv[1]);
  strcpy(dirname_out, dirname);
  strcat(dirname_out, argv[2]);

  inDir = dirname_in;
  outDir = dirname_out;
  dirLen = strlen(dirname_in);
  ret = initDirectories(inDir, outDir);
  if (ret < 0) {
    printf("directory not found.\n");
    return -2;
  }

  directoryIterate(inDir, iter_op);

  return 0;
}
