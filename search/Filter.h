#ifndef _NSLib_search_Filter_
#define _NSLib_search_Filter_

#include "index/IndexReader.h"
#include "util/BitSet.h"

namespace NSLib{ namespace search {
	// Abstract base class providing a mechanism to restrict searches to a subset
	// of an index. 
	class Filter {
	public:
		// Returns a BitSet with true for documents which should be permitted in
		//search results, and false for those that should not. 
		virtual NSLib::util::BitSet* bits(NSLib::index::IndexReader& reader) const=0;
	};
}}
#endif
