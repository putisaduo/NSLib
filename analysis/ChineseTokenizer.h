#include "StdHeader.h"

#ifndef _NSLib_analysis_chinesetokenizer_
#define _NSLib_analysis_chinesetokenizer_

#include "analysis/Analyzers.h"
#include "util/WordSegmenter.h"

using namespace NSLib::analysis;
using namespace NSLib::util;

namespace NSLib{ namespace analysis {

  class HytungReader: public util::ChineseReader {
  private:
    util::BasicReader * m_in;
    bool m_del;

  public:
    HytungReader(util::BasicReader* in, bool del);
    ~HytungReader();

    bool eof();
    char_t next();
    char_t peek();
    void seek(size_t pos);
    size_t position();
  };

  // Normalizes tokens extracted with {@link StandardTokenizer}. 
  class ChineseTokenizer: public Tokenizer{
  private:
    WordSegmenter* m_ws;
#ifdef _UNICODE
    char_t m_buf[100];
#else
    char    m_buf[200];
#endif
  public:
    // Construct filtering <i>in</i>. 
    ChineseTokenizer(util::BasicReader* in);

    ~ChineseTokenizer();

    // Returns the next token in the stream, or NULL at EOS.
    // <p>Removes <tt>'s</tt> from the end of words.
    // <p>Removes dots from acronyms.
    Token* next();
    void close();
  };
}}
#endif
