#include "StdHeader.h"
#include "DateFilter.h"

using namespace NSLib::index;
using namespace NSLib::util;
using namespace NSLib::document;
namespace NSLib{ namespace search{

	DateFilter::~DateFilter(){
		delete[] field;
		delete[] start;
		delete[] end;
	}

	/** Constructs a filter for field <code>f</code> matching times between
		<code>from</code> and <code>to</code>. */
	DateFilter::DateFilter(const char_t* f, long_t from, long_t to):
		field( stringDuplicate(f) ),
		start ( DateField::timeToString(from) ),
		end ( DateField::timeToString(to) )
	{
	}
	
	/** Constructs a filter for field <code>f</code> matching times before
		<code>time</code>. */
	DateFilter* DateFilter::Before(const char_t* field, long_t time) {
		return new DateFilter(field, 0,time);
	}

	/** Constructs a filter for field <code>f</code> matching times after
		<code>time</code>. */
	DateFilter* DateFilter::After(const char_t* field, long_t time) {
		return new DateFilter(field,time, DateField::MAX_TIME );
	}

	/** Returns a BitSet with true for documents which should be permitted in
		search results, and false for those that should not. */
	BitSet* DateFilter::bits(IndexReader& reader) const {
		BitSet& bts = *new BitSet(reader.MaxDoc());
		
		Term* t = new Term(field, start);
		TermEnum& _enum = reader.getTerms(t);
		t->finalize();

		TermDocs& termDocs = reader.termDocs();
		if (_enum.getTerm(false) == NULL){
			delete &_enum;
			return &bts;
		}

		_TRY {
			Term* stop = new Term(field, end);
			while (_enum.getTerm(false)->compareTo(*stop) <= 0) {
				termDocs.seek(_enum.getTerm(false));
				_TRY {
					while (termDocs.next())
						bts.set(termDocs.Doc());
				} _FINALLY ( termDocs.close() );

				if (!_enum.next()) {
					break;
				}
			}
			stop->finalize();
		} _FINALLY (
    		_enum.close();
    		delete &_enum;
		);
		return &bts;
	}
}}
