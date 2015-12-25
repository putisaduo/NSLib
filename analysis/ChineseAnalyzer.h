#ifndef _NSLib_analysis_standard_ChineseAnalyzer
#define _NSLib_analysis_standard_ChineseAnalyzer

#include "analysis/StandardAnalyzer.h"
#include "util/WordSegmenter.h"

using namespace NSLib::util;
using namespace NSLib::analysis;

namespace NSLib{ namespace analysis { 
  class ChineseAnalyzer : public Analyzer 
  {
  private:
    VoidMap< char_t*, char_t*> stopTable;
  public:
    ChineseAnalyzer();
    TokenStream& tokenStream(const char_t* fieldName, BasicReader* reader);
  };
}}

#endif
