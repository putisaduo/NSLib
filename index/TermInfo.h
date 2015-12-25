#ifndef _NSLib_index_TermInfo
#define _NSLib_index_TermInfo
#include "StdHeader.h"

namespace NSLib{ namespace index {

	// A TermInfo is the record of information stored for a term.
	class TermInfo {
	public:
		// The number of documents which contain the term. 
		long_t freqPointer;
		long_t proxPointer;
		int docFreq;

		TermInfo();
		~TermInfo();

		TermInfo(const int df, const long_t fp, const long_t pp);

		TermInfo(const TermInfo& ti);

		void set(const int df, const long_t fp, const long_t pp);

		void set(const TermInfo& ti);
	};
}}
#endif
