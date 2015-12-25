#include "StdHeader.h"
#ifndef _NSLib_search_SloppyPhraseScorer_
#define _NSLib_search_SloppyPhraseScorer_

#include "PhraseScorer.h"
#include "index/Terms.h"

using namespace NSLib::index;

namespace NSLib{ namespace search{
	class SloppyPhraseScorer: public PhraseScorer {
	private:
		int slop;

	public:
		SloppyPhraseScorer(TermPositions** tps, int tpsLength, int s, l_byte_t* n, float w);

	protected:
		float phraseFreq();
	};
}}
#endif

