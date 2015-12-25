#include "StdHeader.h"
#ifndef _NSLib_index_SegmentInfo_
#define _NSLib_index_SegmentInfo_

#include "store/Directory.h"

namespace NSLib{ namespace index{
	class SegmentInfo {
	public:
		const fchar_t* name;				  // unique name in dir
		const int docCount;				  // number of docs in seg
		NSLib::store::Directory& dir;				  // where segment resides

		SegmentInfo(const fchar_t* Name, const int DocCount, NSLib::store::Directory& Dir):
			name ( fstringDuplicate(Name) ),
			docCount(DocCount),
			dir(Dir)
		{
	    }
		~SegmentInfo(){
			delete[] name;
		}
	};
}}
#endif
