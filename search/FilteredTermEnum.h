#include "StdHeader.h"
#ifndef _NSLib_search_FilteredTermEnum_
#define _NSLib_search_FilteredTermEnum_

#include "index/IndexReader.h"
#include "index/Term.h"
#include "index/Terms.h"

using namespace NSLib::index;
namespace NSLib{ namespace search {
    /** Abstract class for enumerating a subset of all terms. 
    
      <p>Term enumerations are always ordered by term->compareTo().  Each term in
      the enumeration is greater than all that precede it.  */
    class FilteredTermEnum: public TermEnum {
    private:
        Term* currentTerm;
        TermEnum* actualEnum;
        
    protected:
        /** Equality compare on the term */
        virtual bool termCompare(Term* term) = 0;
        
        /** Indiciates the end of the enumeration has been reached */
        virtual bool EndEnum() = 0;
        
        void setEnum(TermEnum* actualEnum) ;
    
    public:
        FilteredTermEnum(IndexReader& reader, Term* term);
        ~FilteredTermEnum();
        
		/** Equality measure on the term */
        virtual float difference() = 0;

        /** 
         * Returns the docFreq of the current Term in the enumeration.
         * Initially invalid, valid after next() called for the first time. 
         */
        int DocFreq() const ;
        
        /** Increments the enumeration to the next element.  True if one exists. */
        bool next() ;
        
        /** Returns the current Term in the enumeration.
         * Initially invalid, valid after next() called for the first time. */
        Term* getTerm(const bool pointer=true) ;
        
        /** Closes the enumeration to further activity, freeing resources.  */
        void close();
    };
}}
#endif
