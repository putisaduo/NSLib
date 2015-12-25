#include "StdHeader.h"
#ifndef _NSLib_index_TermInfosReader_
#define _NSLib_index_TermInfosReader_

#include "Terms.h"
#include "SegmentTermEnum.h"
#include "store/Directory.h"
#include "FieldInfos.h"
#include "TermInfo.h"
#include "TermInfosWriter.h"

namespace NSLib{ namespace index {
  class TermInfosReader{
  private:
    NSLib::store::Directory& directory;
    const fchar_t* segment;
    FieldInfos& fieldInfos;

    SegmentTermEnum* _enum;
    int size;

    Term** indexTerms;
    TermInfo** indexInfos;
    long_t* indexPointers;
    int indexTermsLength;

    const fchar_t* segmentname( const fchar_t* ext );
  public:
    TermInfosReader(Directory& dir, const fchar_t* seg, FieldInfos& fis);
    ~TermInfosReader();
    void close();
    
    // Returns the number of term/value pairs in the set. 
    int Size();

        DEFINE_MUTEX(getInt_LOCK);
    // Returns the nth term in the set. 
    // synchronized
    Term* get(const int position);

        DEFINE_MUTEX(getTerm_LOCK);
    // Returns the TermInfo for a Term in the set, or NULL. 
    // synchronized
    TermInfo* get(const Term& term);
    

        DEFINE_MUTEX(getPosition_LOCK);
    // Returns the position of a Term in the set or -1. 
    // synchronized 
    int getPosition(const Term& term);

        DEFINE_MUTEX(getTerms_LOCK);
    // Returns an enumeration of all the Terms and TermInfos in the set. 
    // synchronized 
    SegmentTermEnum& getTerms();

        DEFINE_MUTEX(getTermsTerm_LOCK);
    // Returns an enumeration of terms starting at or after the named term. 
    // synchronized 
    SegmentTermEnum& getTerms(const Term& term);
  private:    
    void readIndex();

    

    // Returns the offset of the greatest index entry which is less than term.
    int getIndexOffset(const Term& term);

    void seekEnum(const int indexOffset);  

    // Scans within block for matching term. 
    TermInfo* scanEnum(const Term& term);

    Term* scanEnum(const int position);
  };
}}
#endif
