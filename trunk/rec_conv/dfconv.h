#ifndef __DF_CONV_H__
#define __DF_CONV_H__ 1

int dfConvert( char * Buffer, int iLen);  // convert string
int dfCheckChar( int iLen,
                 char *Buffer,
                 int iPointer,
                 char *Check,
                 int iCheckLen );

#endif
