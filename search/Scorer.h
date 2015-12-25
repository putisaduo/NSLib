#include "StdHeader.h"
#ifndef _NSLib_search_Scorer_
#define _NSLib_search_Scorer_

#include "HitCollector.h"
namespace NSLib{ namespace search {
    class Scorer {
	public:
		virtual ~Scorer(){
		}
		virtual void score(HitCollector& hc, const int maxDoc) = 0;
    };
}}
#endif
