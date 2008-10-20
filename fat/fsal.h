/* File System Abstract Layer */

#ifndef __FSAL_H__
#define __FSAL_H__ 1

#ifdef WIN32
#include <io.h>
#include <sys/stat.h>
#else
#include <fat.h>
//directory operations and attributes
#include <sys/dir.h>
#endif

typedef unsigned int fsal_dir_handle_t;
typedef unsigned int fsal_file_handle_t;

int fsal_init(void);

//path name in utf-16
int fsal_chdir(unsigned short *path);

fsal_dir_handle_t fsal_diropen(unsigned short *path);

int fsal_dirclose(fsal_dir_handle_t dir);

//file name in utf-16
int fsal_dirnext(fsal_dir_handle_t dir, unsigned short *filename, struct stat *st);

//file name in utf-16
fsal_file_handle_t fsal_open(unsigned short *name, char *mode);

int fsal_close(fsal_file_handle_t fd);

size_t fsal_read(void *buffer, size_t size, size_t count, fsal_file_handle_t stream);

size_t fsal_write(void *buffer, size_t size, size_t count, fsal_file_handle_t stream);

int fsal_seek(fsal_file_handle_t stream, long offset, int origin);

int fsal_tell(fsal_file_handle_t stream);

void fsal_rewind(fsal_file_handle_t stream);

#endif
