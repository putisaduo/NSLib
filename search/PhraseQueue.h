#include "StdHeader.h"
#ifndef _NSLib_search_PriorityQueue_
#define _NSLib_search_PriorityQueue_

#include "util/PriorityQueue.h"
#include "PhrasePositions.h"

namespace NSLib{ namespace search{
	class PhraseQueue: public NSLib::util::PriorityQueue<PhrasePositions*> {
	public:
		PhraseQueue(const int size) {
			initialize(size,false);
		}

	protected:
		bool lessThan(PhrasePositions* pp1, PhrasePositions* pp2) {
			if (pp1->doc == pp2->doc) 
				return pp1->position < pp2->position;
			else
				return pp1->doc < pp2->doc;
		}
	};
}}
#endif
