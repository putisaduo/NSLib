#include "StdHeader.h"
#ifndef _NSLib_index_SegmentMerger_
#define _NSLib_index_SegmentMerger_

#include "store/Directory.h"
#include "document/Document.h"
#include "util/BitVector.h"
#include "util/VoidList.h"
#include "SegmentMergeInfo.h"
#include "SegmentMergeQueue.h"
#include "FieldInfos.h"
#include "FieldsWriter.h"
#include "TermInfosWriter.h"

namespace NSLib{ namespace index {
	class SegmentMerger {
	private:
		Directory& directory;
		const fchar_t* segment;
		NSLib::util::VoidList<SegmentReader*> readers;
		FieldInfos* fieldInfos;

		SegmentMergeQueue* queue;
		NSLib::store::OutputStream* freqOutput;
		NSLib::store::OutputStream* proxOutput;
		TermInfosWriter* termInfosWriter;
		TermInfo termInfo; //(new) minimize consing
	public:		  
		SegmentMerger(Directory& dir, const fchar_t* name);
		~SegmentMerger();

		void add(SegmentReader& reader);

		SegmentReader& segmentReader(const int i);

		void merge();

	private:
		const fchar_t* segmentname(const fchar_t* ext, const int x=-1);

		void mergeFields();

		void mergeTerms();

		void mergeTermInfos();

		void mergeTermInfo( SegmentMergeInfo** smis, const int n);
		       
		int appendPostings(SegmentMergeInfo** smis, const int n);

		void mergeNorms();
	};
}}
#endif
