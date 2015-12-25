#include "StdHeader.h"

#include "FilteredTermEnum.h"

using namespace NSLib::index;
namespace NSLib{ namespace search {

    void FilteredTermEnum::setEnum(TermEnum* actualEnum) {
        this->actualEnum = actualEnum;
        
        // Find the first term that matches
        Term* term = actualEnum->getTerm();
        if ( termCompare(term) ){ 
            currentTerm->finalize();
            currentTerm = term->pointer();
        }
        else next();
    }

    FilteredTermEnum::FilteredTermEnum(IndexReader& reader, Term* term){
        currentTerm = NULL;
        actualEnum = NULL;
    }
    FilteredTermEnum::~FilteredTermEnum(){
    }

    /** 
     * Returns the docFreq of the current Term in the enumeration.
     * Initially invalid, valid after next() called for the first time. 
     */
    int FilteredTermEnum::DocFreq() const {
        if (actualEnum == NULL) return -1;
        
        return actualEnum->DocFreq();
    }
    
    /** Increments the enumeration to the next element.  True if one exists. */
    bool FilteredTermEnum::next() {
        if (actualEnum == NULL) return false; // the actual enumerator is not initialized!
        
        currentTerm->finalize();
        currentTerm = NULL;
        
        while (currentTerm == NULL) {
            if (EndEnum()) return false;
            if (actualEnum->next()) {
                Term* term = actualEnum->getTerm();
                if (termCompare(term)) {
                    currentTerm->finalize();
                    currentTerm = term->pointer();
                    return true;
                }
            }
            else return false;
        }
        currentTerm->finalize();
        currentTerm = NULL;
        
        return false;
    }
    
    /** Returns the current Term in the enumeration.
     * Initially invalid, valid after next() called for the first time. */
    Term* FilteredTermEnum::getTerm(const bool pointer) {
		if ( pointer && currentTerm!=NULL )
			return currentTerm->pointer();
		else
			return currentTerm;
    }
    
    /** Closes the enumeration to further activity, freeing resources.  */
    void FilteredTermEnum::close(){
        actualEnum->close();
        currentTerm->finalize();
        
        currentTerm = NULL;
        actualEnum = NULL;
    }
}}
