#include "StdHeader.h"
#include "ExactPhraseScorer.h"

#include "PhraseScorer.h"
#include "index/Terms.h"

using namespace NSLib::index;
namespace NSLib{ namespace search{

ExactPhraseScorer::ExactPhraseScorer(TermPositions** tps, int tpsLength, 
                                     l_byte_t* n, float w):
    PhraseScorer(tps, tpsLength, n, w)
{
}


float ExactPhraseScorer::phraseFreq(){
  // sort list with pq
  for (PhrasePositions* pp = first; pp != NULL; pp = pp->next) {
    pp->firstPosition();
    pq.put(pp);				  // build pq from list
  }
  pqToList();					  // rebuild list from pq
    
  int freq = 0;
  do {					  // find position w/ all terms
    while (first->position < last->position) {	  // scan forward in first
      do {
        if (!first->nextPosition())
          return (float)freq;
      } while (first->position < last->position);
      firstToLast();
    }
    freq++;					  // all equal: a match
  } while (last->nextPosition());
      
  return (float)freq;
}

}}
