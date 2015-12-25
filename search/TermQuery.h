#include "StdHeader.h"
#ifndef _NSLib_search_TermQuery_
#define _NSLib_search_TermQuery_

#include "SearchHeader.h"
#include "Scorer.h"
#include "index/Term.h"
#include "TermScorer.h"
#include "index/IndexReader.h"
#include "util/StringBuffer.h"
#include "index/Terms.h"

using namespace NSLib::index;
namespace NSLib{ namespace search {

    // A Query that matches documents containing a term.
    //  this may be combined with other terms with a {@link BooleanQuery}.
    class TermQuery: public Query {
    private:
		Term* term;
		float idf;
		float weight;
	public:
		// Constructs a query for the term <code>t</code>. 
		TermQuery(Term& t);
		~TermQuery();

		const char_t* getQueryName() const;
	    
		float sumOfSquaredWeights(Searcher& searcher);
	    
		void normalize(const float norm);
	      
		//added by search highlighter
		Term* getTerm()
		;
	    
		Scorer* scorer(IndexReader& reader);
	    
		// Prints a user-readable version of this query. 
		const char_t* toString(const char_t* field);

		void prepare(IndexReader& reader){}
    };
}}
#endif

