#include "StdHeader.h"
#include "PhraseScorer.h"

#include "PhraseQueue.h"
#include "PhrasePositions.h"
#include "HitCollector.h"
#include "Scorer.h"
#include "Similarity.h"

namespace NSLib{ namespace search{


PhraseScorer::PhraseScorer(TermPositions** tps, const int tpsLength, l_byte_t* n, float w):
	norms(n),
	weight(w),
	pq( *new PhraseQueue(tpsLength) )
{
	// use PQ to build a sorted list of PhrasePositions
	for (int i = 0; i < tpsLength; i++)
		pq.put( new PhrasePositions(*tps[i], i) );
	pqToList();
}

void PhraseScorer::score(HitCollector& results, const int end){
	while (last->doc < end) {			  // find doc w/ all the terms
		while (first->doc < last->doc) {		  // scan forward in first
			do {
				first->Next();
			} while (first->doc < last->doc);
			firstToLast();
			if (last->doc >= end)
				return;
		}

		// found doc with all terms
		float freq = phraseFreq();		  // check for phrase

		if (freq > 0.0) {
			float score = Similarity::tf(freq)*weight; // compute score
			score *= Similarity::normf(norms[first->doc]); // normalize
			results.collect(first->doc, score);	  // add to results
		}
		last->Next();				  // resume scanning
	}
}


void PhraseScorer::pqToList() {
	last = first = NULL;
	while (pq.top() != NULL) {
		PhrasePositions* pp = pq.pop();
		if (last != NULL) {			  // add next to end of list
			last->next = pp;
		} else
			first = pp;
		last = pp;
		pp->next = NULL;
	}
}

void PhraseScorer::firstToLast() {
	last->next = first;			  // move first to end of list
	last = first;
	first = first->next;
	last->next = NULL;
}

}}
