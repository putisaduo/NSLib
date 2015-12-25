#include "StdHeader.h"
#ifndef _NSLib_document_DateField_
#define _NSLib_document_DateField_

namespace NSLib{ namespace document{ 

	const static char_t* MAX_DATE_STRING = _T("zzzzzzzzz");
	const static char_t* MIN_DATE_STRING = _T("000000000");

	/** Provides support for converting dates to strings and vice-versa.  The
	* strings are structured so that lexicographic sorting orders by date.  This
	* makes them suitable for use as field values and search terms.  */
	class DateField {
	private:
		static void initDS();
	public:
		const static int DATE_LEN = 9;
		const static long_t MAX_TIME = 101559956668415;
		
		/** Converts a millisecond time to a string suitable for indexing. */
		static char_t* timeToString(const long_t time);

		/** Converts a string-encoded date into a millisecond time. */
		static long_t stringToTime(const char_t* s);
	};
}}

#endif
