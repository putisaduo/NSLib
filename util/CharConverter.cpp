#include <ctype.h>
#include <locale.h>
#include <stdlib.h>

#ifdef __GNUC__
#include <strings.h>
#elif _WIN32
#define strcasecmp _stricmp
#include <windows.h>
#endif

#include "CharConverter.h"

namespace NSLib { namespace util {
  wchar16_t CharConverter::combine(wchar16_t high, wchar16_t low)
  {
    return ((high & 0xff) << 8) + (low & 0xff);
  }

  char CharConverter::high(wchar16_t ch)    
  {
    return (char)(ch >> 8);
  }

  char CharConverter::low(wchar16_t ch)
  {
    return (char)ch;
  }

  int char16strtombs(char* mbcs, const char16_t* pt, int len)
  {
/*
    mbstate_t mbs;
    mbrlen (NULL,0,&mbs);   // initialize mbs

    while (*pt) {
      size_t length = c16rtomb(mbcs, *pt, &mbs);
      if (length==0 || (int)length>len)
        break;
      for (int i=0; i<(int)length; ++i) 
        putchar(mbcs[i]);
      ++pt;
    }
*/
    char16_t* s = (char16_t*)pt;
    while (*s != 0) {
      char ch = *((char*)s),
           ch1 = *(((char*)s)+1);
      //cerr << ch << "." << ch1 << ".";
      //cerr << (int)ch << "=" << (int)ch1 << ".";
      if (ch1 != 0)
        *(mbcs++) = ch1;
      *(mbcs++) = ch;
      s++;
    }

    return 0;
  }

  int mbstochar16str(char16_t* pc16, const char* mbcs, int len)
  {
    mbstate_t mbs;
    mbrlen (NULL,0,&mbs);   /* initialize mbs */

    while (len>0) {
      size_t length = mbrtoc16( pc16, mbcs, len, &mbs);
      if (length==0 || (int)length>len)
        break;
      wprintf (L"[%lc]", pc16);
      mbcs += length; ++pc16;
      len -= length;
    }

    return 0;
  }

  char* CharConverter::wideToChar(const wchar16_t* dbcs, const char* enc)
  {
#ifdef _WIN32
    size_t len = wcslen(dbcs);
    char* mbcs = new char[4*len+2];
    setlocale(LC_CTYPE, getenc(enc));
    int n = WideCharToMultiByte( 54936, 0, dbcs, -1, mbcs, 4*len+1, NULL, NULL);
    setlocale(LC_CTYPE, "C");
#else
    size_t len = strlen16(dbcs);
    char* mbcs = new char[4*len+2];

    char* old_ctype = setlocale(LC_CTYPE, "");
    setlocale(LC_CTYPE, getenc(enc));
    //int n = (int)wcstombs(mbcs, dbcs, 2*len+1);
    int n = (int)char16strtombs(mbcs, dbcs, 2*len+1);
    setlocale(LC_CTYPE, old_ctype);
#endif
    if (n==0)
      n=-1;

    if ( n < 0 ) {
      delete[] mbcs;
      return NULL;
    }

    mbcs[n] = 0;
    mbcs[n+1] = 0;
    return mbcs;
  }

  wchar16_t* CharConverter::charToWide(const char* mbcs, const char* enc){
    size_t len = strlen(mbcs);
    wchar16_t* dbcs = new wchar16_t[len+1];
#ifdef _WIN32
    setlocale(LC_CTYPE, getenc(enc));
    int n = MultiByteToWideChar( 54936, 0, mbcs, len, dbcs, len+1);
    setlocale(LC_CTYPE, "C");
#else
    char* old_ctype = setlocale(LC_CTYPE, "");
    setlocale(LC_CTYPE, getenc(enc));
    int n = (int)mbstochar16str(dbcs, mbcs, len);
    setlocale(LC_CTYPE, old_ctype);
#endif
    if (n==0)
      n=-1;
    
    if ( n < 0 ) {
      delete[] dbcs;
      return NULL;
    }

    dbcs[n] = 0;
    return dbcs;
  }
  
  const char* CharConverter::getenc(const char* name) {
    if ( strcasecmp(name, "big5") == 0 ) return LANG_CHT;
    else if (strcasecmp(name, "gb2312") == 0) return LANG_CHS;
    else if (strcasecmp(name, "8859-1") == 0) return LANG_ENG;
    return name;
  }

}}
