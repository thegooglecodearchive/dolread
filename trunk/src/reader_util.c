#include <ctype.h>
#include <reader_core.h>

int reader_itoa(int val, unsigned short *buf)
{
  unsigned short *p;
  unsigned short *firstdig;
  unsigned short temp;
  unsigned digval;
  int digits = 0;
  
  p = buf;
  
  firstdig = p;
  
  do {
    digval = (unsigned) (val % 10);
    val /= 10;
    *p++ = (unsigned short) (digval + '0');
    digits++;
  } while (val > 0);
  
  //reverse string
  p--;
  do {
    temp = *p;
    *p = *firstdig;
    *firstdig = temp;
    --p;
    ++firstdig;
  } while (firstdig < p);
  return digits;
}

int reader_atoi(const unsigned short *nptr)
{
  int c;              /* current char */
  int total;         /* current total */
  int sign;           /* if '-', then negative, otherwise positive */
  
  c = (int)*nptr++;
  sign = c;           /* save sign indication */
  if (c == '-' || c == '+')
    c = (int)(unsigned char)*nptr++;    /* skip sign */
  
  total = 0;
  
  while (isdigit(c)) {
    total = 10 * total + (c - '0');     /* accumulate digit */
    c = (int)*nptr++;    /* get next char */
  }
  
  if (sign == '-')
    return -total;
  else
    return total;   /* return result, negated if necessary */
}

//wcs1[:n1] < wcs2[:n2], return -1
//wcs1[:n1] > wcs2[:n2], return 1
//wcs1[:n1] == wcs2[:n2], return 0
int reader_wcscmp_n(unsigned short *wcs1, int n1, unsigned short *wcs2, int n2) {
  int ret = 0;
  int lim = (n1 < n2) ? n1 : n2;
  int i = 0;
  
  while ((i < lim) && (! (ret = (int)(*wcs1 - *wcs2))) && *wcs2) {
    ++wcs1; ++ wcs2; ++ i;
  }
  
  if ( ret < 0 )
    ret = -1 ;
  else if ( ret > 0 )
    ret = 1 ;
  else {
    return n1 - n2;
  }
  
  return ret;
}

void reader_dbg_print_number(int x, int y, int i)
{
#ifdef NDS
  unsigned short dbgbuf[16] = {0};
  int n = reader_itoa(i, dbgbuf);
  reader_textout(0, x, y, dbgbuf, n, 16);
  trigger_screen_update(0);
#else
  printf("(%d, %d) shows %d\n", x, y, i);
#endif
}

#if ENABLE_CONSOLE
void console_log(char *buf) {
  unsigned short log_fn[] = {'/', 'c', 'o', 'n', 's', 'o', 'l', 'e', '.', 't', 'x', 't', 0};
  fsal_file_handle_t fd;
  fd = fsal_open(log_fn, "a+");
  fsal_write(buf, strlen(buf), 1, fd);
  fsal_write("\n", 1, 1, fd);
  fsal_close(fd);
}
#endif


void reader_full_pic_name(unsigned short *out_name, unsigned short *in_name) {
  reader_wcscpy(out_name, reader_path_bg);
  reader_wcscat(out_name, reader_path_separator);
  reader_wcscat(out_name, in_name);
}

//only nds device require those util functions, we can use libc on Win32 sim
#ifdef NDS
int reader_wcslen(unsigned short *wcs) {
  int len = 0;
  while (*wcs ++) len ++;
  return len;
}

int reader_wcscmp(unsigned short *src, unsigned short *dst) {
  int ret = 0 ;
  
  while( ! (ret = (int)(*src - *dst)) && *dst)
    ++src, ++dst;
  
  if ( ret < 0 )
    ret = -1 ;
  else if ( ret > 0 )
    ret = 1 ;
  
  return( ret );
}

unsigned short *reader_wcsncpy(unsigned short *dest, unsigned short *source, int count)
{
  unsigned short *start = dest;
  
  while (count && (*dest++ = *source++))    /* copy string */
    count--;
  
  if (count)                              /* pad out with zeroes */
    while (--count)
      *dest++ = L'\0';
    
  return(start);
}

unsigned short *reader_wcscpy(unsigned short *dest, unsigned short *source)
{
  unsigned short *start = dest;
  
  while ((*dest++ = *source++));    /* copy string */

  return(start);
}

unsigned short *reader_wcsrchr(unsigned short *string, unsigned short ch)
{
  unsigned short *start = (unsigned short *)string;
  while (*string++)                       /* find end of string */
    ;
  /* search towards front */
  while (--string != start && *string != ch)
    ;
  
  if (*string == ch)
    return string;
  
  return(NULL);
}

unsigned short *reader_wcscat(unsigned short *dst, unsigned short *src)
{
  unsigned short *cp = dst;
  
  while( *cp )
    cp++;                   /* find end of dst */
  
  while( (*cp++ = *src++) != 0) ;       /* Copy src to end of dst */
  
  return( dst );                  /* return dst */  
}
#endif
