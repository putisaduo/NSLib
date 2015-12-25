#include "StdHeader.h"
#include "StandardAnalyzer.h"

#include "util/Reader.h"
#include "AnalysisHeader.h"
#include "Analyzers.h"
#include "StandardFilter.h"
#include "StandardTokenizer.h"

using namespace NSLib::util;
using namespace NSLib::analysis;

namespace NSLib{ namespace analysis {

  // <summary> Builds an analyzer. </summary>
  StandardAnalyzer::StandardAnalyzer()
  {
    StopFilter::fillStopTable( stopTable,const_cast<char_t**>(STOP_WORDS),STOP_WORDS_LENGTH );
  }

  //<summary> Builds an analyzer with the given stop words. </summary>
  StandardAnalyzer::StandardAnalyzer( char_t* stopWords[], int stopWordsLength)
  {
    StopFilter::fillStopTable( stopTable,stopWords,stopWordsLength );
  }

  StandardAnalyzer::~StandardAnalyzer(){
  }


  // <summary>
  // Constructs a StandardTokenizer filtered by a 
  // StandardFilter, a LowerCaseFilter and a StopFilter.
  // </summary>
  TokenStream& StandardAnalyzer::tokenStream(const char_t* fieldName, BasicReader* reader) 
  {
    TokenStream* ret = new StandardTokenizer(*reader);
    ret = new StandardFilter(ret,true);
    ret = new LowerCaseFilter(ret,true);
    ret = new StopFilter(ret,true, stopTable);
    return *ret;
  }

}}
