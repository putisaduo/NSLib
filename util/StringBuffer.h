#include "StdHeader.h"
#ifndef _NSLib_util_StringBuffer_
#define _NSLib_util_StringBuffer_


namespace NSLib{ namespace util {
  class StringBuffer{
  private:
    char_t* buffer;
    int bufferLength;

    void growBuffer(int minLength);
  public:
    int len;
    StringBuffer();
    StringBuffer(const char_t* value);
    StringBuffer(int initSize);
    ~StringBuffer();

    void append(const char_t chr);
    void append( const char_t* value );
    void append( const int value );
    void append ( const double value, const int digits );

    void prepend( const char_t* value );

    int length();
    char_t* ToString();
    char_t* getBuffer();
    void clear();
  };
}}
#endif
