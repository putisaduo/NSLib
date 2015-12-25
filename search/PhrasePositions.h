#include "StdHeader.h"
#ifndef _NSLib_search_PhrasePositions_
#define _NSLib_search_PhrasePositions_

#include "index/Terms.h"

using namespace NSLib::index;
namespace NSLib{ namespace search{

	class PhrasePositions {
	public:
		int doc;					  // current doc
		int position;					  // position in doc
		int count;					  // remaining pos in this doc
		int offset;					  // position in phrase
		TermPositions& tp;				  // stream of positions
		PhrasePositions* next;				  // used to make lists

		PhrasePositions(TermPositions& t, const int o);

		void Next();

		void firstPosition();

		bool nextPosition();
	};
}}
#endif
