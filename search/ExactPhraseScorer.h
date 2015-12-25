#include "StdHeader.h"
#ifndef _NSLib_search_ExactPhraseScorer_
#define _NSLib_search_ExactPhraseScorer_

#include "PhraseScorer.h"
#include "index/Terms.h"

using namespace NSLib::index;
namespace NSLib{ namespace search{
    class ExactPhraseScorer: public PhraseScorer {
    public:
    ExactPhraseScorer(TermPositions** tps, int tpsLength, l_byte_t* n, float w);

    protected:
        float phraseFreq();
    };
}}
#endif
