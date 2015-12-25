#include "StdHeader.h"
#ifndef _NSLib_util_charconverter_
#define _NSLib_util_charconverter_

#include <wchar.h>
#include <string.h>
#include "util/enc.h"

#ifdef _WIN32
  #define wchar16_t wchar_t
#else
  #define wchar16_t char16_t
#endif

namespace NSLib{ namespace util{

class CharConverter{
public:
  static wchar16_t combine(wchar16_t high, wchar16_t low);
  static char high(wchar16_t ch);
  static char low(wchar16_t ch);
  static char* wideToChar(const wchar16_t* dbcs, const char* enc);
  static wchar16_t* charToWide(const char* mbcs, const char* enc);
private:
  static const char* getenc(const char* name);
};

}}

#endif
