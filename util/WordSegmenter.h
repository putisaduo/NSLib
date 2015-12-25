#include "StdHeader.h"

#ifndef _NSLib_util_wordseg_
#define _NSLib_util_wordseg_

#include <wchar.h>

#ifndef _WIN32
#include <wctype.h>
#endif

#include "util/dict.h"
#include "util/enc.h"

namespace NSLib { namespace util {
  class ChineseReader {
  public:
    virtual bool eof() = 0;
    virtual char_t next() = 0;
    virtual char_t peek() = 0;
    virtual void seek(size_t pos) = 0;
    virtual size_t position() = 0;
  };

  /*
  class StringReader : ChineseReader{
  private:
    const char_t* m_str;
    const char_t* m_ptr;
    size_t         m_slen;
    bool           m_del;

  public:
    StringReader(const char_t str, bool del);
    ~StringReader();
  };
  */

  class WordSegmenter {
  private:
    bool    m_del;
    ChineseReader* m_reader;

  public:
    WordSegmenter(ChineseReader* reader, bool del);
    ~WordSegmenter();

    char_t* next(char_t* buf, size_t len);

  private:
    bool isWord(char_t* str);
    bool ischnalnum(char_t ch);
    bool ischnspace(char_t ch);
    bool iswestenalnum(char_t ch);
  };
}}

#endif
