#include "StdHeader.h"
#include "PhrasePositions.h"

#include "index/Terms.h"

using namespace NSLib::index;
namespace NSLib{ namespace search{

	PhrasePositions::PhrasePositions(TermPositions& t, const int o):
		tp(t),
		offset(o),
		position(0),
		count(0)
	{
		Next();
	}

	void PhrasePositions::Next()  {	  // increments to next doc
		if (!tp.next()) {
			tp.close();				  // close stream
			doc = INT_MAX;			  // sentinel value
			return;
		}
		doc = tp.Doc();
		position = 0;
	}

	void PhrasePositions::firstPosition() {
		count = tp.Freq();				  // read first pos
		nextPosition();
	}

	bool PhrasePositions::nextPosition() {
		if (count-- > 0) {				  // read subsequent pos's
			position = tp.nextPosition() - offset;
			return true;
		} else
			return false;
	}
}}
