#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include "dfconv.h"

char *get_file_content(char *fn, int *psz)
{
  int n_read, size;
  FILE *fd;
  char *content;

  if ((fd = fopen(fn, "rb")) == NULL) {
    printf("can't open test file.\n");
    exit(-1);
  }
  fseek(fd, 0, SEEK_END);
  size = ftell(fd);
  rewind(fd);
  content = (char *)malloc((size + 1) * sizeof(char));
  if (content == NULL) {
    fclose(fd);
    return NULL;
  }
  n_read = fread(content, 1, size, fd);
  fclose(fd);
  if (n_read < size) {
    free(content);
    return NULL;
  }

  *psz = n_read;
  return content;
}

int main(int argc, char *argv[])
{
  char *content;
  int len;

  if (argc < 2) {
    printf("usage: html2txt <html_file>\n");
    return 2;
  }

  //suppress the '\n' -> "\r\n" translation
  _setmode( _fileno( stdout ), _O_BINARY );

  content = get_file_content(argv[1], &len);
  if (content == NULL) {
    return 1;
  }
  len = dfConvert(content, len);
  content[len] = '\0';
  printf("%s", content);
  free(content);

  return 0;
}
