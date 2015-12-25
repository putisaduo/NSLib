#include "StdHeader.h"
#ifndef _NSLib_analysis_StandardTokenizerConstants
#define _NSLib_analysis_StandardTokenizerConstants

namespace NSLib{ namespace analysis { 
  enum TokenTypes
  {
    ALPHANUM,
    APOSTROPHE,
    ACRONYM,
    COMPANY,
    EMAIL,
    HOST,
    NUM,
    _EOF
  };
  
  const static char_t *tokenImage[] = {
                  _T("<ALPHANUM>"),
                  _T("<APOSTROPHE>"),
                  _T("<ACRONYM>"),
                  _T("<COMPANY>"),
                  _T("<EMAIL>"),
                  _T("<HOST>"),
                  _T("<NUM>"),
                  _T("<EOF>"),
                  _T("<P>"),
                  _T("<HAS_DIGIT>"),
                  _T("<ALPHA>"),
                  _T("<LETTER>"),
                  _T("<DIGIT>"),
                  _T("<NOISE>")
  };

}}
#endif
