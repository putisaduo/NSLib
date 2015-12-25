#include "StdHeader.h"
#ifndef _NSLib_index_SegmentMergeQueue_
#define _NSLib_index_SegmentMergeQueue_

#include "util/PriorityQueue.h"
#include "SegmentMergeInfo.h"

namespace NSLib{ namespace index {
	class SegmentMergeQueue:public NSLib::util::PriorityQueue<SegmentMergeInfo*> {
	public:
		SegmentMergeQueue(const int size);
		~SegmentMergeQueue();

		void close();
	protected:
		bool lessThan(SegmentMergeInfo* stiA, SegmentMergeInfo* stiB);

	};
}}
#endif
