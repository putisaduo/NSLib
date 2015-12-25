#include "StdHeader.h"
#ifndef _NSLib_search_PhraseQuery_
#define _NSLib_search_PhraseQuery_

#include "SearchHeader.h"
#include "Scorer.h"
#include "BooleanQuery.h"
#include "TermQuery.h"

#include "index/Term.h"
#include "index/Terms.h"
#include "index/IndexReader.h"

#include "util/StringBuffer.h"
#include "util/VoidList.h"

#include "ExactPhraseScorer.h"
#include "SloppyPhraseScorer.h"

using namespace NSLib::util;
namespace NSLib{ namespace search{
	// A Query that matches documents containing a particular sequence of terms.
	// This may be combined with other terms with a {@link BooleanQuery}.
	class PhraseQuery: public Query {
	private:
		const char_t* field;
		VoidList<Term*> terms;
		float idf;
		float weight;

		int slop;

	public:
        // Constructs an empty phrase query. 
        PhraseQuery();
		~PhraseQuery();

        const char_t* getQueryName() const;
        
        //Sets the number of other words permitted between words in query phrase.
        //If zero, then this is an exact phrase search.  For larger values this works
        //like a <code>WITHIN</code> or <code>NEAR</code> operator.
        //
        //<p>The slop is in fact an edit-distance, where the units correspond to
        //moves of terms in the query phrase out of position.  For example, to switch
        //the order of two words requires two moves (the first move places the words
        //atop one another), so to permit re-orderings of phrases, the slop must be
        //at least two.
        //
        //<p>More exact matches are scored higher than sloppier matches, thus search
        //results are sorted by exactness.
        //
        //<p>The slop is zero by default, requiring exact matches.
        void setSlop(const int s) { slop = s; }
        
        // Returns the slop.  See setSlop(). 
        int getSlop() { return slop; }
        
        // Adds a term to the end of the query phrase. 
        void add(Term* term);
        
        float sumOfSquaredWeights(Searcher& searcher);
        
        void normalize(const float norm);
        
        Scorer* scorer(IndexReader& reader);
        
        //added by search highlighter
        void getTerms(Term**& ret, int& size);
        
        // Prints a user-readable version of this query. 
        const char_t* toString(const char_t* f);

		void prepare(IndexReader& reader){}
	};
}}
#endif
