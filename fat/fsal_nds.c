#include <reader_core.h>
#include <ConvertUTF.h>

extern int chdir(const char *path);

static UTF8 utf8_buf[256];

int fsal_chdir(unsigned short *dirname) {
  UTF16 *utf16Start = dirname;
  UTF16 *utf16End = dirname + reader_wcslen(dirname) + 1;
  UTF8 *utf8Start = utf8_buf;
  ConversionResult result;

  //convert utf-16 to utf-8
  result = ConvertUTF16toUTF8((const UTF16 **)&utf16Start, utf16End, &utf8Start, 
			      (UTF8 *)&(utf8_buf[256]), strictConversion);
  if (result != conversionOK) {
    return 1;
  }
  //pass utf-8 to chdir()
  return chdir((char *)utf8_buf);
}

fsal_dir_handle_t fsal_diropen(unsigned short *path) {
  UTF16 *utf16Start = path;
  UTF16 *utf16End = path + reader_wcslen(path) + 1;
  UTF8 *utf8Start = utf8_buf;
  ConversionResult result;

  //convert utf-16 to utf-8
  result = ConvertUTF16toUTF8((const UTF16 **)&utf16Start, utf16End, &utf8Start, 
			      (UTF8 *)&(utf8_buf[256]), strictConversion);
  if (result != conversionOK) {
    return 0;
  }
  //pass utf-8 to diropen()
  return (fsal_dir_handle_t)diropen((char *)utf8_buf);
}

//fetch lfn, if no lfn is found, return an empty filename
int fsal_dirnext(fsal_dir_handle_t hdir, unsigned short *filename, struct stat *st) {
  UTF16 *utf16Start = filename;
  UTF8 *utf8Start = utf8_buf;
  UTF8 *utf8End;
  int ret;
  ConversionResult result;

  ret = dirnext((DIR_ITER *)hdir, (char *)utf8_buf, st);
  utf8End = utf8_buf + strlen((char *)utf8_buf) + 1;
  result = ConvertUTF8toUTF16((const UTF8 **)&utf8Start, utf8End, &utf16Start, 
			      &filename[256], strictConversion);
  if (result != conversionOK) {
    filename[0] = 0;
  }
  return ret;
}

int fsal_dirclose(fsal_dir_handle_t hdir) {
  return dirclose((DIR_ITER *)hdir);
}

int fsal_init(void) {
  return fatInitDefault();
}

size_t fsal_read(void *buffer, size_t size, size_t count, fsal_file_handle_t stream) {
  return fread(buffer, size, count, (FILE *)stream);
}

fsal_file_handle_t fsal_open(unsigned short *fn, char *mode) {
  UTF16 *utf16Start = fn;
  UTF16 *utf16End = fn + reader_wcslen(fn) + 1;
  UTF8 *utf8Start = utf8_buf;
  ConversionResult result;

  //convert utf-16 to utf-8
  result = ConvertUTF16toUTF8((const UTF16 **)&utf16Start, utf16End, &utf8Start, 
			      (UTF8 *)&(utf8_buf[256]), strictConversion);
  if (result != conversionOK) {
    return 0;
  }
  //pass utf-8 to fopen()
  return (fsal_file_handle_t)fopen((char *)utf8_buf, mode);
}

int fsal_close(fsal_file_handle_t fd) {
  return fclose((FILE *)fd);
}

int fsal_seek(fsal_file_handle_t stream, long offset, int origin) {
  return fseek((FILE *)stream, offset, origin);
}

size_t fsal_write(void *buffer, size_t size, size_t count, fsal_file_handle_t stream) {
  return fwrite(buffer, size, count, (FILE *)stream);
}

int fsal_tell(fsal_file_handle_t stream) {
  return ftell((FILE *)stream);
}

void fsal_rewind(fsal_file_handle_t stream) {
  rewind((FILE *)stream);
}

