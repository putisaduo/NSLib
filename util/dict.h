#ifndef _NSLib_util_dict_
#define _NSLib_util_dict_

#include <wchar.h>
#include <uchar.h>

#ifdef _WIN32
#  define wchar16_t wchar_t
#else
#  define wchar16_t char16_t
#endif

namespace NSLib {namespace util {
	extern int DICT_INDEX[][2];
	extern wchar16_t* DICT_STRING;
}}

#endif

