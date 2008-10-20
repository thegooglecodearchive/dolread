#include <stdio.h>
#include <windows.h>
#include <direct.h>
#include <io.h>
#include "dirIterate.h"

int directoryIterate(char *inDir, IterateOP *op) {
  long handle;
  struct _finddata_t filestruct;
  char path_search[_MAX_PATH];

  if (_chdir(inDir)) {
    return -1;
  }

  handle = _findfirst("*", &filestruct);
  if((handle == -1)) {
    return 0; //empty dir
  }

  if( filestruct.attrib & FILE_ATTRIBUTE_DIRECTORY ) {
    if( filestruct.name[0] != '.' ) {
      directoryIterate(filestruct.name, op);
      _chdir("..");
    }
  } else {
    _getcwd(path_search,_MAX_PATH);
    strcat(path_search,"\\");
    strcat(path_search,filestruct.name);
    (void)op(path_search);
  }

  while(_findnext(handle,&filestruct) == 0) {
    if( filestruct.attrib & FILE_ATTRIBUTE_DIRECTORY ) {
      if( filestruct.name[0] != '.' ) {
        directoryIterate(filestruct.name, op);
        _chdir("..");
      }
    } else {
      _getcwd(path_search,_MAX_PATH);
      strcat(path_search,"\\");
      strcat(path_search,filestruct.name);
      (void)op(path_search);
    }
  }
  _findclose(handle);

  return 0;
}
