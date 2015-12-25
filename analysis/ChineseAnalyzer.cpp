#include "ChineseAnalyzer.h"
#include "ChineseTokenizer.h"

namespace NSLib{ namespace analysis {
  ChineseAnalyzer::ChineseAnalyzer() {
    StopFilter::fillStopTable( stopTable, const_cast<char_t**>(STOP_WORDS),STOP_WORDS_LENGTH );
  }

  TokenStream& ChineseAnalyzer::tokenStream(const char_t* fieldName, BasicReader* reader) 
  {
    TokenStream* ret = new ChineseTokenizer(reader);
    ret = new StandardFilter(ret, true);
    ret = new LowerCaseFilter(ret, true);
    ret = new StopFilter(ret, true, stopTable);
    return *ret;
  }
}}
