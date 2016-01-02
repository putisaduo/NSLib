#include "StdHeader.h"
#include "TermScorer.h"

#include "index/Terms.h"
#include "Scorer.h"
#include "search/Similarity.h"

#include <iostream>

using namespace NSLib::index;
namespace NSLib{ namespace search {

//TermScorer takes TermDocs and delets it when TermScorer is cleaned up
TermScorer::TermScorer(TermDocs& td, l_byte_t* _norms, const float w):
  termDocs(td),
  norms(_norms),
  weight(w),
  pointer(0)
{
  for (int i = 0; i < NSLIB_SCORE_CACHE_SIZE; i++)
    scoreCache[i] = Similarity::tf(i) * weight;
    
  pointerMax = termDocs.read(docs, freqs);    // fill buffers
    
  if (pointerMax != 0)
    doc = docs[0];
  else {
    termDocs.close();     // close stream
    doc = INT_MAX;        // set to sentinel value
  }
}

TermScorer::~TermScorer(){
  delete &termDocs;
}

void TermScorer::score(HitCollector& c, const int end) {
  int d = doc;          // cache doc in local
  //cerr << "TermScorer::score " << doc << " / " << end << endl;
  while (d < end) {          // for docs in window
    const int f = freqs[pointer];
    float score =          // compute tf(f)*weight
      f < NSLIB_SCORE_CACHE_SIZE        // check cache
      ? scoreCache[f]        // cache hit
      : Similarity::tf(f)*weight;      // cache miss
        
    score *= Similarity::normf(norms[d]);    // normalize for field
      
    //cerr << "\tscore[" << d << "]= " << score << endl;
    c.collect(d, score);        // collect score
      
    if (++pointer == pointerMax) {
      pointerMax = termDocs.read(docs, freqs);  // refill buffers
      if (pointerMax != 0) {
        pointer = 0;
      } else {
        termDocs.close();        // close stream
        doc = INT_MAX;      // set to sentinel value
        return;
      }
    } 
    d = docs[pointer];
  }
  doc = d;            // flush cache
}
  
}}
