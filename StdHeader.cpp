#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <iostream>

#include "StdHeader.h"
#include "util/CharConverter.h"

#ifdef __GNUC__
# include <unistd.h>
#endif

#include <locale>

using namespace std;

#ifdef _WIN32

wchar_t* _wstrlwr(wchar_t* str) {
#ifdef SUPPORT_ALL_WESTEN
  locale loc1 ( "Latvian");
  for ( wchar_t* ptr = str; *ptr != 0; ptr++ ) {
    *ptr = use_facet<ctype<wchar_t> >(loc1).tolower(*ptr);
#else
  for ( wchar_t* ptr = str; *ptr != 0; ptr++ ) {
    *ptr = towlower(*ptr);
#endif
  }
  return str;
}

wchar_t* _wstrupr(wchar_t* str) {
#ifdef SUPPORT_ALL_WESTEN
  locale loc1 ( "Latvian");
  for ( wchar_t* ptr = str; *ptr != 0; ptr++ ) {
    *ptr = use_facet<ctype<wchar_t> >(loc1).toupper(*ptr);
#else
  for ( wchar_t* ptr = str; *ptr != 0; ptr++ ) {
    *ptr = towupper(*ptr);
#endif
  }
  return str;
}

int _swprintf(wchar_t* wcs, const wchar_t* format, ...) {
  va_list ap;
  va_start(ap, format);
  int len = vswprintf(wcs, format, ap);
  va_end(ap);
  return len;
}

int _wstat(const wchar_t* path, struct stat* buffer) {
  char* byte = NSLib::util::CharConverter::wideToChar(path, "8859-1");
  int result = stat(byte, buffer);
  delete byte;
  return result;
}
wchar_t *NSLibwcsdup(const wchar_t *v){
    int len = wcslen(v);
    wchar_t* ret = new wchar_t[len+1];
    wcsncpy(ret,v,len+1);
    return ret;
}

#elif defined(__GNUC__)

char16_t* _wstrlwr(char16_t* str) {
#ifdef SUPPORT_ALL_WESTEN
  locale loc1 ( "Latvian");
  for ( char16_t* ptr = str; *ptr != 0; ptr++ ) {
    *ptr = use_facet<ctype<char16_t> >(loc1).tolower(*ptr);
#else
  for ( char16_t* ptr = str; *ptr != 0; ptr++ ) {
    *ptr = towlower(*ptr);
#endif
  }
  return str;
}

char16_t* _wstrupr(char16_t* str) {
#ifdef SUPPORT_ALL_WESTEN
  locale loc1 ( "Latvian");
  for ( char16_t* ptr = str; *ptr != 0; ptr++ ) {
    *ptr = use_facet<ctype<char16_t> >(loc1).toupper(*ptr);
#else
  for ( char16_t* ptr = str; *ptr != 0; ptr++ ) {
    *ptr = towupper(*ptr);
#endif
  }
  return str;
}

INT strcspn16(const char16_t* str1, const char16_t * str2)
{
  std::u16string s1 = str1, s2 = str2;
  return s1.find_first_of(str2);
}

char16_t* _wcschr(const char16_t* str1, const char16_t ch)
{
  std::u16string s1 = str1;
  std::u16string::size_type found = s1.find_first_of(ch);
  if (found==std::u16string::npos)
    return NULL;
  return const_cast<char16_t*>(str1 + found);
}

int strlen16(const char16_t* strarg)
{
  if(!strarg)
    return -1; //strarg is NULL pointer
  char16_t* str = const_cast<char16_t*>(strarg);
  for(;*str;++str)
    ; // empty body
  return str-strarg;
}

int strncpy16(char16_t* dest, const char16_t* src, int n)
{
  if(!src)
    return -1; //src is NULL pointer
  char16_t* str = const_cast<char16_t*>(src);
  int i = 0;
  for(; *str && i<n;++str, ++i)
    dest[i] = *str;
  return i;
}

int strcpy16(char16_t* dest, const char16_t* src)
{
  if(!src)
    return -1; //src is NULL pointer
  char16_t* str = const_cast<char16_t*>(src);
  int i = 0;
  for(; *str; ++str,++i)
    dest[i] = *str;
  return i;
}

INT _wcsstr16(const char16_t* str1, const char16_t* str2)
{
  std::u16string s1 = str1, s2 = str2;
  return s1.find(str2);
}

char16_t *NSLibwcsdup(const char16_t *v){
  int len = strlen16(v);
  char16_t* ret = new char16_t[len+1];
  strncpy16(ret,v,len);
  ret[len] = 0;
  //cerr << " NSLibwcsdup: " << ws2str(v) << "->" << ws2str(ret) << ":" << len << endl;
  return ret;
}

int _swprintf(char16_t* wcs, const char16_t* format, ...) {
  va_list ap;
  va_start(ap, format);
  int len = 0;
  for ( const char16_t* p = format; *p; p++ ) {
    if ( *p != '%' ) {
      *(wcs + len) = *p;
      len++;
      continue;
    }
    switch(*++p) {
    case 'd':
      {
      int ival = va_arg(ap, int);
      char buf[100];
      sprintf(buf, "%d", ival);
      for ( char* pbuf = buf; *pbuf; pbuf++, len++) 
        *(wcs + len) = *pbuf;
      break;
      }
    case 's':
      for ( const char16_t *wsval = va_arg(ap, char16_t*); *wsval; wsval++, len++)
        *(wcs + len) = *wsval;
      break;
    default:
      *(wcs + len) = *p;
      len ++;
      break;
    }
  }
  wcs[len] = 0;
  va_end(ap);
  return len;
}

int _wstat(const char16_t* path, struct stat* buffer) {
  char* byte = NSLib::util::CharConverter::wideToChar(path, "8859-1");
  int result = stat(byte, buffer);
  delete byte;
  return result;
}

/* Pauses for a specified number of milliseconds. */
void sleep( clock_t wait )
{
  clock_t goal;
  goal = wait + clock();
  while( goal > clock() )
    ;
}

long_t _filelength(int filehandle) 
{
  struct Struct_Stat info;
  int err = fstat(filehandle, &info);
  return info.st_size;
}


/*********************************************************************
 *                  strlwr   (NTDLL.@)
 *
 * convert a string in place to lowercase
 */
LPSTR strlwr( LPSTR str )
{
  LPSTR ret = str;
#ifdef SUPPORT_ALL_WESTEN
  locale loc1 ( "Latvian");
  for ( ; *str; str++) {
    use_facet<ctype<char_t> >(loc1).tolower(&str[0], &str[stringLength(&str[0])-1]);
#else
  for ( ; *str; str++) {
    *str = tolower(*str);
#endif
  }
  return ret;
}


/*********************************************************************
 *                  strupr   (NTDLL.@)
 */
LPSTR strupr( LPSTR str )
{
  LPSTR ret = str;
#ifdef SUPPORT_ALL_WESTEN
  locale loc1 ( "Latvian");
  for ( ; *str; str++) {
    use_facet<ctype<char_t> >(loc1).toupper(&str[0], &str[stringLength(&str[0])-1]);
#else
  for ( ; *str; str++) {
    *str = toupper(*str);
#endif
  }
  return ret;
}


/*********************************************************************
 *      _i64toa   (NTDLL.@)
 *
 * Converts a large integer to a string.
 *
 * RETURNS
 *  Always returns str.
 *
 * NOTES
 *  Converts value to a '\0' terminated string which is copied to str.
 *  The maximum length of the copied str is 65 bytes. If radix
 *  is 10 and value is negative, the value is converted with sign.
 *  Does not check if radix is in the range of 2 to 36.
 *  If str is NULL it crashes, as the native function does.
 *
 * DIFFERENCES
 * - The native DLL converts negative values (for base 10) wrong:
 *                     -1 is converted to -18446744073709551615
 *                     -2 is converted to -18446744073709551614
 *   -9223372036854775807 is converted to  -9223372036854775809
 *   -9223372036854775808 is converted to  -9223372036854775808
 *   The native msvcrt _i64toa function and our ntdll _i64toa function
 *   do not have this bug.
 */
char * _i64toa(
    LONGLONG value, /* [I] Value to be converted */
    char *str,      /* [O] Destination for the converted value */
    int radix)      /* [I] Number base for conversion */
{
  ULONGLONG val;
  int negative;
  char buffer[65];
  char *pos;
  int digit;

  if (value < 0 && radix == 10) {
    negative = 1;
    val = -value;
  } else {
    negative = 0;
    val = value;
  } /* if */

  pos = &buffer[64];
  *pos = '\0';

  do {
    digit = val % radix;
    val = val / radix;
    if (digit < 10) 
      *--pos = '0' + digit;
    else 
      *--pos = 'a' + digit - 10;
  } while (val != 0L);

  if (negative) 
    *--pos = '-';

  memcpy(str, pos, &buffer[64] - pos + 1);
  return str;
}

/*********************************************************************
 *      _i64tow   (NTDLL.@)
 *
 * Converts a large integer to an unicode string.
 *
 * RETURNS
 *  Always returns str.
 *
 * NOTES
 *  Converts value to a '\0' terminated wstring which is copied to str.
 *  The maximum length of the copied str is 33 bytes. If radix
 *  is 10 and value is negative, the value is converted with sign.
 *  Does not check if radix is in the range of 2 to 36.
 *  If str is NULL it just returns NULL.
 *
 * DIFFERENCES
 * - The native DLL converts negative values (for base 10) wrong:
 *                     -1 is converted to -18446744073709551615
 *                     -2 is converted to -18446744073709551614
 *   -9223372036854775807 is converted to  -9223372036854775809
 *   -9223372036854775808 is converted to  -9223372036854775808
 *   The native msvcrt _i64tow function and our ntdll function do
 *   not have this bug.
 */
LPWSTR _i64tow(
    LONGLONG value, /* [I] Value to be converted */
    LPWSTR str,     /* [O] Destination for the converted value */
    INT radix)      /* [I] Number base for conversion */
{
  ULONGLONG val;
  int negative;
  WCHAR buffer[65];
  PWCHAR pos;
  WCHAR digit;

  if (value < 0 && radix == 10) {
    negative = 1;
    val = -value;
  } else {
    negative = 0;
    val = value;
  } /* if */

  pos = &buffer[64];
  *pos = '\0';

  do {
    digit = val % radix;
    val = val / radix;
    if (digit < 10)
      *--pos = '0' + digit;
    else
      *--pos = 'a' + digit - 10;
  } while (val != 0L);

  if (negative) 
    *--pos = '-';

  if (str != NULL) 
    memcpy(str, pos, (&buffer[64] - pos + 1) * sizeof(WCHAR));
  return str;
}

INT _wtoi(const char16_t* str) {
  char* byte = NSLib::util::CharConverter::wideToChar(str, "8859-1");
  int result = atoi(byte);
  delete byte;
  return result;
}

INT _wunlink(const char16_t* path) {
  char* byte = NSLib::util::CharConverter::wideToChar(path, "8859-1");
  int result = unlink(byte);
  delete byte;
  return result;
}

INT _wrename(const char16_t* oldname, const char16_t* newname) {
  char* obyte = NSLib::util::CharConverter::wideToChar(oldname, "8859-1");
  char* nbyte = NSLib::util::CharConverter::wideToChar(newname, "8859-1");
  int result = rename(obyte, nbyte);
  delete obyte;
  delete nbyte;
  return result;
}

INT _wopen(const char16_t* path, INT oflag, INT pmode) {
  char* byte = NSLib::util::CharConverter::wideToChar(path, "8859-1");
  int result = open(byte, oflag, pmode);
  delete byte;
  return result;
}

INT _wmkdir(const char16_t* path) {
  char* byte = NSLib::util::CharConverter::wideToChar(path, "8859-1");
  int result = mkdir(byte, 0777);
  delete byte;
  return result;
}

INT _wcsicmp(const char16_t* str1, const char16_t* str2) {
  const char16_t* ptr1 = str1;
  const char16_t* ptr2 = str2;
  while (*ptr1 != 0 && *ptr2 != 0) {
    if ( towlower(*ptr1) == towlower(*ptr2) ) {
      ptr1++;
      ptr2++;
      continue;
    } else if ( *ptr1 < *ptr2 ) 
      return -1;
    else 
      return 1;
  }

  if ( *ptr1 == 0 && *ptr2 == 0 ) return 0;
  else if ( *ptr2 != 0 ) return -1;
  return 1;
}

INT _wcscmp(const char16_t* str1, const char16_t* str2) {
  const char16_t* ptr1 = str1;
  const char16_t* ptr2 = str2;
  while (*ptr1 != 0 && *ptr2 != 0) {
    if ( *ptr1 == *ptr2 ) {
      ptr1++;
      ptr2++;
      continue;
    } else if ( *ptr1 < *ptr2 ) 
      return -1;
    else 
      return 1;
  }

  if ( *ptr1 == 0 && *ptr2 == 0 ) return 0;
  else if ( *ptr2 != 0 ) return -1;
  return 1;
}

LONGLONG _wstrtoll(const char16_t* str1, char16_t** str2, INT radix) {
  char* byte = NSLib::util::CharConverter::wideToChar(str1, "8859-1");
  LONGLONG result = atoll(byte);
  delete byte;
  return result;
}

float _wstrtof(const char16_t* str) {
  char* byte = NSLib::util::CharConverter::wideToChar(str, "8859-1");
  float result = atof(byte);
  delete byte;
  return result;
}
#endif

char * NSLibstrdup(const char *v){
  if (!v)
    return NULL;
  int len = strlen(v);
  char* ret = new char[len+1];
  strncpy(ret,v,len+1);
  return ret;
}

string ws2str(const char16_t* src)
{
  if (src==NULL)
    return "";
  string ret = "";
  //cerr << " ws2str: ";
  char16_t* s = (char16_t*)src;
  while (*s != 0) {
    char ch = *((char*)s),
         ch1 = *(((char*)s)+1);
    if (ch1 != 0) {
      ret += ch1;
    }
    ret += ch;
    //std::cerr << std::hex << (int)(unsigned char)ch << " "
    //                      << (int)(unsigned char)ch1 << " ";
    s++;
  }
  //cerr << endl << ret << endl;
  return ret;
}
