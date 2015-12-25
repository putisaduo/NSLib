#include "StdHeader.h"
#ifndef _NSLib_search_Similarity_
#define _NSLib_search_Similarity_

#include "index/Term.h"
#include "SearchHeader.h"

namespace NSLib{ namespace search {
	
	// Internal class used for scoring.
	// <p>Public only so that the indexing code can compute and store the
	// normalization byte for each document. 
	class Similarity {
	private:
		
		static float* SIMILARITY_NORM_TABLE();

	public:
		// Computes the normalization byte for a document given the total number of
		// terms contained in the document.  These values are stored in an index and
		// used by the search code. 
		static l_byte_t normb(const int numTerms);

		static float normf(const l_byte_t normByte);

		static float tf(const int freq);

		static float tf(const float freq);
		    
		static float idf(const NSLib::index::Term& term, const Searcher& searcher);

		static float idf(const int docFreq, const int numDocs);
		    
		static float coord(const int overlap, const int maxOverlap);
	};
}}
#endif
