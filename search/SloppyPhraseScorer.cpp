#include "StdHeader.h"
#include "SloppyPhraseScorer.h"

#include "PhraseScorer.h"
#include "index/Terms.h"

using namespace NSLib::index;
namespace NSLib{ namespace search{

	SloppyPhraseScorer::SloppyPhraseScorer(TermPositions** tps, int tpsLength, int s, l_byte_t* n, float w):
		PhraseScorer(tps,tpsLength,n,w),
		slop(s)
	{
	}

	float SloppyPhraseScorer::phraseFreq() {
		pq.clear();
		int end = 0;
		for (PhrasePositions* pp = first; pp != NULL; pp = pp->next) {
			pp->firstPosition();
			if (pp->position > end)
				end = pp->position;
			pq.put(pp);				  // build pq from list
		}

		float freq = 0.0f;
		bool done = false;
		do {
			PhrasePositions* pp = pq.pop();
			int start = pp->position;
			int next = pq.top()->position;
			for (int pos = start; pos <= next; pos = pp->position) {
				start = pos;				  // advance pp to min window
				if (!pp->nextPosition()) {
					done = true;				  // ran out of a term -- done
					break;
				}
			}

			int matchLength = end - start;
			if (matchLength <= slop)
				freq += 1.0 / (matchLength + 1);	  // penalize longer matches

			if (pp->position > end)
				end = pp->position;
			pq.put(pp);				  // restore pq
		} while (!done);

		return freq;
	}
}}

