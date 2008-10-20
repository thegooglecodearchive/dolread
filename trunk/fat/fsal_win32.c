#include <reader_core.h>
#include <direct.h>

typedef struct {
  long handle;
  unsigned short path[READER_PATH_MAX];
} DIR_ITER;

static char root_dir[READER_PATH_MAX];
static unsigned short root_dir_uni[READER_PATH_MAX];

static char *replace_chr(char *str, int c1, int c2) {
  while (*str) {
    if (*str == c1) {
      *str = c2;
    }
    str ++;
  }
  return str;
}

static unsigned short *replace_wchr(unsigned short *str, int c1, int c2) {
  while (*str) {
    if (*str == c1) {
      *str = c2;
    }
    str ++;
  }
  return str;
}

static int translate_path(unsigned short *path, unsigned short *real_path) {
  unsigned short cwd[READER_PATH_MAX];
  int len;

  if (*path == '/') {
    reader_wcsncpy(real_path, root_dir_uni, READER_PATH_MAX);
    path++;
  } else {
    _wgetcwd(cwd, READER_PATH_MAX);
    reader_wcsncpy(real_path, cwd, READER_PATH_MAX);
  }
  len = reader_wcslen(real_path);
  if (real_path[len-1] != '\\') {
    reader_wcscat(real_path, L"\\");
  }
  reader_wcscat(real_path, path);
  replace_wchr(real_path, '/', '\\');
  return 0;
}

int fsal_chdir(unsigned short *path) {
  unsigned short real_path[READER_PATH_MAX] = {0};

  translate_path(path, real_path);
  return _wchdir(real_path);
}

fsal_dir_handle_t fsal_diropen(unsigned short *path) {
  long handle;
  struct _wfinddata_t filestruct;
  unsigned short real_path[READER_PATH_MAX] = {0};
  int i = 0;
  DIR_ITER *iter = NULL;

  iter = (DIR_ITER *)malloc(sizeof(*iter));
  if (iter == NULL) {
    return 0;
  }
  translate_path(path, real_path);
  reader_wcscpy(iter->path, real_path);
  reader_wcscat(real_path, L"\\*");
  handle = _wfindfirst(real_path, &filestruct);
  if (handle < 0) {
    READER_TRACEWM((L"real_path=%s\n", real_path));
    READER_TRACEM(("error in _wfindfirst():%d\n", errno));
    return 0;
  }
  iter->handle = handle;
  return (fsal_dir_handle_t)iter;
}

int fsal_dirnext(fsal_dir_handle_t hdir, unsigned short *filename, struct stat *st) {
  DIR_ITER *dir = (DIR_ITER *)hdir;
  int ret;
  struct _wfinddata_t filestruct;
  unsigned short fullname[READER_PATH_MAX];

  ret = _wfindnext(dir->handle,&filestruct);
  if (ret != 0) {
    return ret;
  }
  //do not return '.' and '..' entries for simulated root dir
  if ((wcscmp(dir->path, root_dir_uni) == 0) //is root dir
   && ((wcscmp(filestruct.name, reader_path_this) == 0)
   || ((wcscmp(filestruct.name, reader_path_upper) == 0)))
   ) {
    return fsal_dirnext((fsal_dir_handle_t)dir, filename, st);
  }
  reader_wcscpy(filename, filestruct.name);
  reader_wcscpy(fullname, dir->path);
  reader_wcscat(fullname, L"\\");
  reader_wcscat(fullname, filename);
  //"struct stat" and "struct _stat" are identical
  return _wstat(fullname, (struct _stat *)st);
}

int fsal_dirclose(fsal_dir_handle_t hdir) {
  DIR_ITER *dir = (DIR_ITER *)hdir;
  _findclose(dir->handle);
  free(dir);
  return 0;
}

int fsal_init(void) {
  //set the root dir for the simulated FAT
  _getcwd(root_dir, READER_PATH_MAX);
  printf("root=%s\n", root_dir);
  _wgetcwd(root_dir_uni, READER_PATH_MAX);
  _chdir(root_dir);
  return 0;
}

size_t fsal_read(void *buffer, size_t size, size_t count, fsal_file_handle_t stream) {
  return fread(buffer, size, count, (FILE *)stream);
}

fsal_file_handle_t fsal_open(unsigned short *name, char *mode) {
  unsigned short *wmode = NULL;
  unsigned short real_name[READER_PATH_MAX];

  if (strcmp(mode, "rb") == 0) {
    wmode = L"rb";
  } else if (strcmp(mode, "wb") == 0) {
    wmode = L"wb";
  } else if (strcmp(mode, "rb+") == 0) {
    wmode = L"rb+";
  } else if (strcmp(mode, "a+") == 0) {
    wmode = L"a+";
  }
  translate_path(name, real_name);
  return (fsal_file_handle_t)_wfopen(real_name, wmode);
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
