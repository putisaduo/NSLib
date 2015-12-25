#include "StdHeader.h"
#include "TermQuery.h"

#include "SearchHeader.h"
#include "Scorer.h"
#include "index/Term.h"
#include "TermScorer.h"
#include "index/IndexReader.h"
#include "util/StringBuffer.h"
#include "index/Terms.h"

using namespace NSLib::index;
namespace NSLib{ namespace search {


	/** Constructs a query for the term <code>t</code>. */
	TermQuery::TermQuery(Term& t):
		term( t.pointer() ),
		idf ( 0.0f ),
		weight ( 0.0f )
	{
	}
	TermQuery::~TermQuery(){
	    term->finalize();
	}

	const char_t* TermQuery::getQueryName() const{
		return _T("TermQuery");
	}
    
	float TermQuery::sumOfSquaredWeights(Searcher& searcher){
		idf = Similarity::idf(*term, searcher);
		weight = idf * boost;
		return weight * weight;			  // square term weights
	}
    
	void TermQuery::normalize(const float norm) {
		weight *= norm;				  // normalize for query
		weight *= idf;				  // factor from document
	}
      
	//added by search highlighter
	Term* TermQuery::getTerm()
	{
		return term->pointer();
	}
    
	Scorer* TermQuery::scorer(IndexReader& reader){
		TermDocs* termDocs = &reader.termDocs(term);
		if (termDocs == NULL)
			return NULL;
        
		//Termscorer will delete termDocs when finished
		return new TermScorer(*termDocs, reader.getNorms(term->Field()), weight);
	}
    
	/** Prints a user-readable version of this query. */
	const char_t* TermQuery::toString(const char_t* field) {
		NSLib::util::StringBuffer buffer;
		if ( stringCompare(term->Field(),field)!= 0 ) {
			buffer.append(term->Field());
			buffer.append(_T(":"));
		}
		buffer.append(term->Text());
		if (boost != 1.0f) {
			buffer.append(_T("^"));
			buffer.append( boost,10 );
		}
		return buffer.ToString();
	}
}}

