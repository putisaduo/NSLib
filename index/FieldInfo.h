#include "StdHeader.h"
#ifndef _NSLib_index_FieldInfo_
#define _NSLib_index_FieldInfo_

namespace NSLib{ namespace index {
	class FieldInfo {
	public:
		char_t* name;
		bool isIndexed;
		const int number;

		FieldInfo(char_t* na, const bool tk, const int nu):
			name( stringDuplicate(na) ),
			isIndexed(tk),
			number (nu)
		{
	    }
	    
	    ~FieldInfo(){
			delete [] name;
	    }
	};
}}
#endif
