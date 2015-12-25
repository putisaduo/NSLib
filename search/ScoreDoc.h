#ifndef _NSLib_search_ScoreDoc_
#define _NSLib_search_ScoreDoc_

namespace NSLib{ namespace search {
	struct ScoreDoc {
	public:
		float score;
		int doc;

		ScoreDoc(const int d, const float s):
			score(s),
			doc(d)
		{
	    }
	};
}}
#endif
