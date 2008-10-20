#ifndef __DIR_ITERATE_H__
#define __DIR_ITERATE_H__ 1

//---type
typedef int IterateOP(char *fileName);

//return 0 means operation succeeded
//return -1 means the specified dir name does not exist
int directoryIterate(char *dirname, IterateOP *op);

#endif
