#include "StdHeader.h"
#ifndef _NSLib_index_SegmentMergeInfo_
#define _NSLib_index_SegmentMergeInfo_

#include "util/BitVector.h"
#include "SegmentTermEnum.h"
#include "SegmentHeader.h"

namespace NSLib{ namespace index {
	class SegmentMergeInfo {
	public:
		SegmentTermEnum& termEnum;
		Term* term;
		int base;
		int* docMap;				  // maps around deleted docs
		SegmentReader& reader;
		SegmentTermPositions& postings;

		SegmentMergeInfo(const int b, SegmentTermEnum& te, SegmentReader& r);
		~SegmentMergeInfo();

		bool next();

		void close();
 
	};
}}
#endif

