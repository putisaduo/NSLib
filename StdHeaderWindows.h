#pragma once

#include <ctype.h>
#include <map>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <vector>
#include <stdexcept>
#include <fcntl.h> //file constants, also will include io.
#include <stdio.h> 

extern wchar_t* _wstrlwr(wchar_t* str);
extern wchar_t* _wstrupr(wchar_t* str);

# define REQUIRE_DIRENT
# ifdef COMPILER_MINGW32
#  define THROW_TYPE runtime_error
# else
#  define THROW_TYPE exception
# endif
# define processSleep _sleep
# define ulong_t unsigned __int64
# define long_t __int64 //8 bytes signed
# define Struct_Stat _stat
# define getTime _ftime
# define Struct_timeb _timeb

# ifdef _EXPORT_SYMBOLS
#  define EXPORT_NSLIB __declspec(dllexport) 
# else
#  define EXPORT_NSLIB 
# endif

# ifdef _UNICODE
#  define stringToIntegerBase _wcstoi64
#  define stringUpper _wstrupr
#  define stringLower _wstrlwr
# else
#  ifdef COMPILER_MINGW32
#   define stringToIntegerBase strtol
#  else
#   define stringToIntegerBase _strtoi64
#  endif
#  define stringICompare _stricmp 
#  define stringUpper _strupr
#  define stringLower _strlwr
# endif

#ifdef _F_UNICODE

# define fchar_t wchar_t
# define openFile _wopen
# define makeDirectory _wmkdir
# define Cmd_Stat _wstat
# define CmdFindFirst _wfindfirst
# define CmdFindNext _wfindnext
# define CmdFindClose _findclose
# define fileRename _wrename
# define fileFullName _wfullpath
# define printFormatted wprintf
# define fstringCat wcscat
# define fstringLength wcslen
# define fstringCompare wcscmp
# define fstringNCopy wcsncpy
# define fstringCopy wcscpy
# define Struct_Find _wfinddata_t
# define unlinkFile _wunlink
# define fstringDuplicate NSLibwcsdup
# define fstringPrintF _swprintf
# define PATH_DELIMITER _T("\\")
# define CONST_STRING(y) _T(y)
# define F_TO_CHAR_T(x) fstringDuplicate(x)

#else

# define fchar_t char
# define openFile _open
# define makeDirectory _mkdir
# define Cmd_Stat _stat
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
# define unlinkFile _unlink

  char * NSLibstrdup(const char *v);
# define fstringDuplicate NSLibstrdup
# define fstringPrintF sprintf
# define PATH_DELIMITER "\\"
# define CONST_STRING(y) y
# define F_TO_CHAR_T(x) util::Misc::charToWide(x)

#endif

# ifdef COMPILER_MINGW32
  typedef int intptr_t; //todo2: cygwin already has this defined
# endif

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
# define NSLibString wstring
# define stringCompare wcscmp
# define stringNCopy wcsncpy
# define stringCopy wcscpy
# define stringICompare _wcsicmp
     int _swprintf(char16_t* wcs, const char16_t* fmt, ...);
# define stringPrintF _swprintf
# define stringCSpn wcscspn
# define stringLength wcslen
# define stringToFloat wcstod
# define stringToInteger _wtoi
# define stringFind wcsstr
# define stringFindChar wcschr
# define stringAdvance _wcsninc
# define stringCat wcscat
# define integerToString _i64tow
# define stringToken wcstok
# ifndef _T
#  define _T(x)       L ## x
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

