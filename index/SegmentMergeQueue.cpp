#include "StdHeader.h"
#include "SegmentMergeQueue.h"

#include "SegmentMergeInfo.h"
namespace NSLib{ namespace index {


	SegmentMergeQueue::SegmentMergeQueue(const int size) {
 		initialize(size,false);
	}

	SegmentMergeQueue::~SegmentMergeQueue(){
	}

	void SegmentMergeQueue::close() {
		while (top() != NULL)
			pop()->close();
	}

	bool SegmentMergeQueue::lessThan(SegmentMergeInfo* stiA, SegmentMergeInfo* stiB) {
		//SegmentMergeInfo* stiA = (SegmentMergeInfo*)a;
		//SegmentMergeInfo* stiB = (SegmentMergeInfo*)b;

		int comparison = stiA->term->compareTo(*stiB->term);
		if (comparison == 0)
			return stiA->base < stiB->base; 
		else
			return comparison < 0;
	}

}}
