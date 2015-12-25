#include "StdHeader.h"
#ifndef _NSLib_search_DateFilter_
#define _NSLib_search_DateFilter_

#include "document/DateField.h"
#include "index/Term.h"
#include "index/Terms.h"
#include "index/IndexReader.h"
#include "util/BitSet.h"
#include "Filter.h"

using namespace NSLib::index;
using namespace NSLib::util;
using namespace NSLib::document;
namespace NSLib{ namespace search{
	// A Filter that restricts search results to a range of time.
    //For this to work, documents must have been indexed with a {@link
    //DateField}. 
	class DateFilter: public Filter {
	private:
		const char_t* field;

		char_t* start;
		char_t* end;
	public:
		~DateFilter();

		/** Constructs a filter for field <code>f</code> matching times between
			<code>from</code> and <code>to</code>. */
		DateFilter(const char_t* f, long_t from, long_t to);
		
		/** Constructs a filter for field <code>f</code> matching times before
			<code>time</code>. */
		static DateFilter* Before(const char_t* field, long_t time) ;

		/** Constructs a filter for field <code>f</code> matching times after
			<code>time</code>. */
		static DateFilter* After(const char_t* field, long_t time) ;

		/** Returns a BitSet with true for documents which should be permitted in
			search results, and false for those that should not. */
		BitSet* bits(IndexReader& reader) const ;
	};
}}
#endif
