#include "StdHeader.h"

#include "DateField.h"
using namespace NSLib::util;
using namespace std;
namespace NSLib{ namespace document{ 

	/** Converts a millisecond time to a string suitable for indexing. */
	char_t* DateField::timeToString(const long_t time) {
		if (time < 0)
			_THROWC ("time too early");

		char_t* buf = new char_t[DATE_LEN+1];
		integerToString(time,buf,CHAR_RADIX);
		int bufLen = stringLength(buf);
		if ( bufLen > DATE_LEN)
			_THROWC ( "time too late");
		
		if ( bufLen < DATE_LEN ){
			for ( int i=DATE_LEN-1;i>=DATE_LEN-bufLen;i-- )
				buf[i] = buf[i-(DATE_LEN-bufLen)];
			for ( int i=0;i<DATE_LEN-bufLen;i++ )
				buf[i] = '0';
			buf[DATE_LEN] = 0;
		}

		return buf;
	}

	/** Converts a string-encoded date into a millisecond time. */
	long_t DateField::stringToTime(const char_t* time) {
		char_t* s = stringDuplicate(time);
		long_t ret = 0;
		try{
			char_t* end = s+stringLength(s)-1;
			ret = stringToIntegerBase(s,&end,CHAR_RADIX);
		}_FINALLY(delete[] s;);
		
		return ret;
	}
}}
