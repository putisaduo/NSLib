#include "StdHeader.h"
#ifndef _NSLib_search_WildcardQuery_
#define _NSLib_search_WildcardQuery_

#include "index/IndexReader.h"
#include "index/Term.h"
#include "MultiTermQuery.h"
#include "WildcardTermEnum.h"

using namespace NSLib::index;
namespace NSLib{ namespace search {
    /** Implements the wildcard search query */
    class WildcardQuery: public MultiTermQuery {
        private: Term* wildcardTerm;
    
        public:
        WildcardQuery(Term* term);
        ~WildcardQuery();

		const char_t* getQueryName() const;
   
        void prepare(IndexReader& reader) ;

		const char_t* toString(const char_t* field);
    };
}}
#endif
