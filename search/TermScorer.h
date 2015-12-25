#include "StdHeader.h"
#ifndef _NSLib_search_TermScorer_
#define _NSLib_search_TermScorer_

#include "index/Terms.h"
#include "Scorer.h"
#include "search/Similarity.h"

using namespace NSLib::index;
namespace NSLib{ namespace search {
    
  class TermScorer: public Scorer {
  private:
    TermDocs& termDocs;
    l_byte_t* norms;
    const float weight;
    int doc;

    int docs[128];    // buffered doc numbers
    int freqs[128];    // buffered term freqs
    int pointer;
    int pointerMax;

    float scoreCache[NSLIB_SCORE_CACHE_SIZE];
  public:

    //TermScorer takes TermDocs and delets it when TermScorer is cleaned up
    TermScorer(TermDocs& td, l_byte_t* _norms, const float w);

    ~TermScorer();

    void score(HitCollector& c, const int end);
    };
}}
#endif
