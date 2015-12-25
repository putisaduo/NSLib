#pragma once

#include <ctype.h>
#include <map>
#include <string>
#include <limits.h>
#include <math.h>
#include <vector>
#include <stdexcept>
#include <fcntl.h> //file constants, also will include io.
#include <stdio.h> 

//# include "config.h"
#include <uchar.h>

#define wchar16_t char16_t

extern char16_t* _wstrlwr(char16_t* str);
extern char16_t* _wstrupr(char16_t* str);

extern int strlen16(const char16_t* strarg);

typedef unsigned long long ulong_t;
typedef long long long_t;

typedef int INT;
typedef char CHAR;
typedef CHAR *LPSTR;
typedef char16_t WCHAR;
typedef char16_t* PWCHAR;
typedef WCHAR* LPWSTR;
typedef long long  LONGLONG;
typedef unsigned long long ULONGLONG;

# define THROW_TYPE runtime_error
# define processSleep sleep
//# define PATH_DELIMITER _T("/")
# define getTime ftime
# define Struct_timeb timeb
# define _S_IREAD  0444
# define _S_IWRITE 0333  // write and execute permissions

# ifdef _UNICODE
#  include <wctype.h> //@XP@
#  include <wchar.h> //@XP@
#  define stringToIntegerBase _wstrtoll //@XP@
#  define stringUpper _wstrupr
#  define stringLower _wstrlwr
# else
#  define stringToIntegerBase strtoll
#  define stringICompare _stricmp 
#  define stringUpper _strupr
#  define stringLower _strlwr
# endif

#ifdef _F_UNICODE
#  define Cmd_Stat _wstat 
#  define Struct_Stat stat 
#  define opendir _wopendir //@XP@
#  define unlinkFile _wunlink
#  define openFile _wopen
#  define makeDirectory _wmkdir
# else

# define fchar_t char
# define CmdFindFirst _findfirst
# define CmdFindNext _findnext
# define CmdFindClose _findclose
# define fileRename rename
# define fileFullName _fullpath
# define printFormatted printf
# define fstringCat strcat
# define fstringLength strlen
# define fstringCompare strcmp
# define fstringNCopy strncpy
# define fstringCopy strcpy
# define Struct_Find _finddata_t
#  define Cmd_Stat stat
#  define Struct_Stat stat
#  define unlinkFile unlink
#  define openFile open
#  define makeDirectory(x) mkdir(x, 0777) // must do this or file will be created Read only
  char * NSLibstrdup(const char *v);
# define fstringDuplicate NSLibstrdup
# define fstringPrintF sprintf
# define PATH_DELIMITER "/"
# define CONST_STRING(y) y
# define F_TO_CHAR_T(x) util::Misc::charToWide(x)

# endif

void sleep( clock_t wait );
LPSTR strlwr( LPSTR str );
LPSTR strupr( LPSTR str );
char * _i64toa(
         LONGLONG value, /* [I] Value to be converted */
         char *str,      /* [O] Destination for the converted value */
         int radix);      /* [I] Number base for conversion */
LPWSTR _i64tow(
         LONGLONG value, /* [I] Value to be converted */
         LPWSTR str,     /* [O] Destination for the converted value */
         INT radix);      /* [I] Number base for conversion */
        
long_t _filelength(int filehandle);

INT      _wtoi(const char16_t* str);
INT      _wcsicmp(const char16_t* str1, const char16_t* str2);
INT      _wcscmp(const char16_t* str1, const char16_t* str2);
LONGLONG _wstrtoll(const char16_t* str1, char16_t** str2, INT radix);
INT      _wstat(const char16_t* path, struct stat* buf);
INT      _wunlink(const char16_t* path);
INT      _wrename(const char16_t* oldname, const char16_t* newname);
INT      _wopen(const char16_t* path, INT oflag, INT pmode);
INT      _wmkdir(const char16_t* path);

INT strcspn16(const char16_t* str1, const char16_t * str2);
INT strlen16(const char16_t* strarg);
INT strncpy16(char16_t* dest, const char16_t* src, int n);
INT strcpy16(char16_t* dest, const char16_t* src);
float _wstrtof(const char16_t* str);
INT _wcsstr16(const char16_t* str1, const char16_t* str2);
char16_t* _wcschr(const char16_t* str1, const char16_t ch);

///////////////////////////////////////////////////////////////////
//
// Non platform specific Unicode/NonUncode definitions
//
#ifdef _UNICODE
# define isSpace iswspace
# define isDigit iswdigit
# define isAlNum iswalnum
# define toLower towlower

# define TO_CHAR_T(x) NSLib::util::Misc::charToWide(x)
# define _cout wcout
# define _cin wcin
# define _THROWX(y) throw THROW_TYPE(Misc::wideToChar(y))
# define char_t char16_t
# define NSLibString u16string
# define stringCompare _wcscmp
# define stringNCopy strncpy16
# define stringCopy strcpy16
# define stringICompare _wcsicmp
     int _swprintf(char16_t* wcs, const char16_t* fmt, ...);
# define stringPrintF _swprintf
# define stringCSpn strcspn16
# define stringLength strlen16
# define stringToFloat _wstrtof
# define stringToInteger _wtoi
# define stringFind _wcsstr16
# define stringFindChar _wcschr
# define stringAdvance _wcsninc
# define stringCat wcscat
# define integerToString _i64tow
# define stringToken wcstok
# ifndef _T
#  define _T(x)       u ## x
# endif

  char16_t *NSLibwcsdup(const char16_t *v);
# define stringDuplicate NSLibwcsdup

#else
//todo: does linux need to be checked whether it is ascii or not?
# define isSpace(x) (((unsigned)x) < 0x100?isspace(x):0)
# define isDigit(x) (((unsigned)x) < 0x100?isdigit(x):0)
# define isAlNum(x) (((unsigned)x) < 0x100?isalnum(x):0)
# define toLower(x) (((unsigned)x) < 0x80?tolower(x):x)

  char * NSLibstrdup(const char *v);
# define stringDuplicate NSLibstrdup

# define TO_CHAR_T(x) stringDuplicate(x)
# define _cout cout
# define _cin cin
# define Struct_Find _finddata_t
# define _THROWX(y) throw THROW_TYPE(y)
# define char_t char
# define NSLibString string
# define stringCompare strcmp
# define stringNCopy strncpy
# define stringCopy strcpy

# ifndef _T
#  define _T(x)       x
# endif
   
# define stringPrintF sprintf
# define stringCSpn strcspn
# define stringLength strlen
# define stringToFloat strtod
# define stringToInteger atoi
# define stringFind strstr
# define stringFindChar strchr
# define stringAdvance strinc
# define stringCat strcat
# define integerToString _i64toa
# define stringToken strtok

#endif

