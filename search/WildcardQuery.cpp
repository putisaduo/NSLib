#include "StdHeader.h"
#include "WildcardQuery.h"

using namespace NSLib::index;
namespace NSLib{ namespace search {

    WildcardQuery::WildcardQuery(Term* term):
        MultiTermQuery( term )
    {
        wildcardTerm = term->pointer();
    }
    WildcardQuery::~WildcardQuery(){
        wildcardTerm->finalize();
    }

	const char_t* WildcardQuery::getQueryName() const{
		return _T("WildcardQuery");
	}

    void WildcardQuery::prepare(IndexReader& reader) {
        try {
			FilteredTermEnum* w = new WildcardTermEnum(reader, wildcardTerm);
			MultiTermQuery::setEnum( w );
        } catch (...) {
			printf( "Prepare error\n" );
		}
    }

	const char_t* WildcardQuery::toString(const char_t* field){
		NSLib::util::StringBuffer buffer;
		if ( stringCompare(wildcardTerm->Field(),field)!= 0 ) {
			buffer.append(wildcardTerm->Field());
			buffer.append(_T(":"));
		}
		buffer.append(wildcardTerm->Text());
		if (boost != 1.0f) {
			buffer.append(_T("^"));
			buffer.append( boost,10 );
		}
		return buffer.ToString();
	}
}}
