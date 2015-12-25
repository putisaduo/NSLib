#include "StdHeader.h"
#ifndef _NSLib_search_FuzzyQuery_
#define _NSLib_search_FuzzyQuery_

#include "index/IndexReader.h"
#include "index/Term.h"
#include "MultiTermQuery.h"

namespace NSLib{ namespace search{
	static const double FUZZY_THRESHOLD = 0.5;
	static const double SCALE_FACTOR = 1.0f / (1.0f - FUZZY_THRESHOLD);

	/** Subclass of FilteredTermEnum for enumerating all terms that are similiar to the specified filter term.

	<p>Term enumerations are always ordered by Term.compareTo().  Each term in
	the enumeration is greater than all that precede it.  */
	class FuzzyTermEnum: public FilteredTermEnum {
	private:
		double distance;
		bool fieldMatch;
		bool endEnum;

		Term* searchTerm;
		const char_t* field;
		char_t* text;
		int textlen;

	public:
		FuzzyTermEnum(IndexReader& reader, Term* term);
		~FuzzyTermEnum();

		bool EndEnum();

		void close();

	protected:
		/**
		The termCompare method in FuzzyTermEnum uses Levenshtein distance to 
		calculate the distance between the given term and the comparing term. 
		*/
		bool termCompare(Term* term) ;

		float difference();



		/******************************
		* Compute Levenshtein distance
		******************************/
	    
	private:
		/**
		Finds and returns the smallest of three integers 
		*/
		static int Min(const int a, const int b, const int c);
	    
		/**
		* This static array saves us from the time required to create a new array
		* everytime editDistance is called.
		*/
		int* e;
		int eWidth;
		int eHeight;
	    
		/**
		Levenshtein distance also known as edit distance is a measure of similiarity
		between two strings where the distance is measured as the number of character 
		deletions, insertions or substitutions required to transform one string to 
		the other string. 
		<p>This method takes in four parameters; two strings and their respective 
		lengths to compute the Levenshtein distance between the two strings.
		The result is returned as an integer.
		*/ 
		int editDistance(char_t* s, char_t* t, int n, int m) ;

	};

    /** Implements the fuzzy search query */
    class FuzzyQuery: public MultiTermQuery {
        private:
        Term* fuzzyTerm;
        
        public:
		FuzzyQuery(Term* term);
		~FuzzyQuery();
        
        void prepare(IndexReader& reader);
        
        const char_t* toString(const char_t* field);

		const char_t* getQueryName() const;
    };
}}
#endif
