#ifndef __READER_UTIL_H__
#define __READER_UTIL_H__

//convert an integer into its unicode string form, such as input 123 will output L"123"
//return the number of wide chars in the converted string
int reader_itoa(int i, unsigned short *str);

int reader_atoi(const unsigned short *nptr);

int reader_wcscmp_n(unsigned short *wcs1, int n1, unsigned short *wcs2, int n2);

void reader_dbg_print_number(int x, int y, int i);

#if ENABLE_CONSOLE
void console_log(char *buf);
#else
#define console_log(exp)     ((void)0)
#endif

/*reader_str2uni() convert a GBK string to UNICODE
 *return the length of converted string in UNICODE bytes(unsigned short)
 *The converted string(uni_str) must be terminated by '\0\0'
 *If the uni_str buffer is not enough to hold all the str content in UNICODE,
 *uni_str[0:len2-2] is for string storage and uni_str[len2-1] = '\0\0'
 *This should be true: (uni_str[ret] == '0' || uni_str[len2-1] == '0')
 */
int reader_str2uni(unsigned char *str, int len, unsigned short *uni_str, int len2);

//get full path name from a selected file name of the picture
void reader_full_pic_name(unsigned short *out_name, unsigned short *in_name);

#ifdef WIN32
#include <string.h>
#define reader_wcslen wcslen
#define reader_wcscmp wcscmp
#define reader_wcscpy wcscpy
#define reader_wcsncpy wcsncpy
#define reader_wcsrchr wcsrchr
#define reader_wcscat wcscat

#else
int reader_wcslen(unsigned short *wcs);
int reader_wcscmp(unsigned short *src, unsigned short *dst);
unsigned short *reader_wcscpy(unsigned short *dest, unsigned short *source);
unsigned short *reader_wcsncpy (unsigned short *dest, unsigned short *source, int count);
unsigned short *reader_wcsrchr(unsigned short *string, unsigned short ch);
unsigned short *reader_wcscat(unsigned short *dst, unsigned short *src);

#endif

#endif
