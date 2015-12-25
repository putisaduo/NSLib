#include "ChineseTokenizer.h"

namespace NSLib{ namespace analysis {
  HytungReader::HytungReader(util::BasicReader *in, bool del)
    :m_in(in), m_del(del)
  {
  }

  HytungReader::~HytungReader() {
    if (m_del) delete m_in;
  }

  bool HytungReader::eof() {
    return m_in->available() == 0;
  }

  char_t HytungReader::next() {
    return m_in->readChar();
  }

  char_t HytungReader::peek() {
    return m_in->peek();
  }

  void HytungReader::seek(size_t pos) {
    m_in->seek(pos);
  }

  size_t HytungReader::position() {
    return m_in->position();
  }

  ChineseTokenizer::ChineseTokenizer(util::BasicReader* in)
  {
    m_ws = new WordSegmenter(new HytungReader(in, true), true);
  }

  ChineseTokenizer::~ChineseTokenizer(){
    delete m_ws;
  }

  Token* ChineseTokenizer::next() {
    if ( m_ws->next(m_buf, sizeof(m_buf)/sizeof(char_t)) == NULL ) return NULL;
    return new Token(m_buf, 0, stringLength(m_buf));
  }

  void ChineseTokenizer::close() {
  }
}}
