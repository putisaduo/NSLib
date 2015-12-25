#include "StdHeader.h"
#ifndef _NSLib_search_PhraseScorer_
#define _NSLib_search_PhraseScorer_

#include "PhraseQueue.h"
#include "PhrasePositions.h"
#include "HitCollector.h"
#include "Scorer.h"
#include "Similarity.h"

namespace NSLib{ namespace search{
	class PhraseScorer: public Scorer {
	protected:
		l_byte_t* norms;
		float weight;

		PhraseQueue& pq;
		PhrasePositions* first;
		PhrasePositions* last;

	public:
		PhraseScorer(TermPositions** tps, const int tpsLength, l_byte_t* n, float w);

		void score(HitCollector& results, const int end);

	protected:
		virtual float phraseFreq() =0;

		void pqToList();

		void firstToLast();
	};
}}
#endif
