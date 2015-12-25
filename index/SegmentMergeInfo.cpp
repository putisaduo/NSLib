#include "StdHeader.h"
#include "SegmentMergeInfo.h"

#include "util/BitVector.h"
#include "SegmentTermEnum.h"
#include "SegmentHeader.h"

namespace NSLib{ namespace index {


	SegmentMergeInfo::~SegmentMergeInfo(){
	}
	SegmentMergeInfo::SegmentMergeInfo(const int b, SegmentTermEnum& te, SegmentReader& r):
		docMap(NULL),
		base(b),
		reader(r),
		termEnum(te),
		postings ( *new SegmentTermPositions(&r) )
	{
		term = te.getTerm();

		if (reader.deletedDocs != NULL) {
			// build array which maps document numbers around deletions 
			NSLib::util::BitVector& deletedDocs = *reader.deletedDocs;
			int maxDoc = reader.MaxDoc();
			docMap = new int[maxDoc];
			int j = 0;
			for (int i = 0; i < maxDoc; i++) {
				if (deletedDocs.get(i))
					docMap[i] = -1;
				else
					docMap[i] = j++;
			}
		}
	}

	bool SegmentMergeInfo::next() {
		if (termEnum.next()) {
			term->finalize();
			term = termEnum.getTerm();
			return true;
		} else {
			term->finalize(); //TODO: test HighFreqTerms errors with this
			term = NULL;
			return false;
		}
	}

	void SegmentMergeInfo::close() {
		termEnum.close();
		postings.close();
		
		delete &postings;
		term->finalize();
		if ( docMap != NULL )
			_DELETE( docMap );
	}

}}
