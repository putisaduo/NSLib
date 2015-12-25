#ifndef _NSLib_index_IndexReader_
#define _NSLib_index_IndexReader_
#include "StdHeader.h"

#include "store/Directory.h"
#include "store/FSDirectory.h"
#include "store/Lock.h"
#include "document/Document.h"
#include "SegmentInfos.h"
#include <sys/stat.h>
#include "Terms.h"

namespace NSLib{ namespace index {

//IndexReader is an abstract class, providing an interface for accessing an
//index.  Search of an index is done entirely through this abstract interface,
//so that any subclass which implements it is searchable.
//
//<p> Concrete subclasses of IndexReader are usually constructed with a call to
//the static method {@link #open}.
//
//<p> For efficiency, in this API documents are often referred to via
//<i>document numbers</i>, non-negative integers which each name a unique
//document in the index.  These document numbers are ephemeral--they may change
//as documents are added to and deleted from an index.  Clients should thus not
//rely on a given document having the same number between sessions. 
  class IndexReader {
  private:
    store::NSLock* writeLock;
  protected:
    IndexReader(store::Directory& dir);
    
    virtual void doDelete(const int docNum) = 0;

    // Implements close. 
    virtual void doClose() = 0;

  public:
    store::Directory& directory;

    // Release the write lock, if needed. 
    virtual ~IndexReader();

    // Returns an IndexReader reading the index in an FSDirectory in the named path. 
    static IndexReader& open(const fchar_t* path, const bool closeDir=true);

    // Returns an IndexReader reading the index in the given Directory. 
    static IndexReader& open( store::Directory& directory, const bool closeDir=true);

    // Returns the time the index in the named directory was last modified. 
    static long_t lastModified(const fchar_t* directory);

    // Returns the time the index in the named directory was last modified. 
    //static long_t lastModified(File directory);

    // Returns the time the index in this directory was last modified. 
    static long_t lastModified(store::Directory& directory);

    // Returns <code>true</code> if an index exists at the specified directory.
    // If the directory does not exist or if there is no index in it.
    // <code>false</code> is returned.
    // @param  directory the directory to check for an index
    // @return <code>true</code> if an index exists; <code>false</code> otherwise
    //
    static bool indexExists(const fchar_t* directory);

    // Returns <code>true</code> if an index exists at the specified directory.
    // If the directory does not exist or if there is no index in it.
    // @param  directory the directory to check for an index
    // @return <code>true</code> if an index exists; <code>false</code> otherwise
    //static boolean indexExists(File directory);

    // Returns <code>true</code> if an index exists at the specified directory.
    // If the directory does not exist or if there is no index in it.
    // @param  directory the directory to check for an index
    // @return <code>true</code> if an index exists; <code>false</code> otherwise
    // @throws IOException if there is a problem with accessing the index
    static bool indexExists(store::Directory& directory);

    // Returns the number of documents in this index. 
    virtual int NumDocs() = 0;

    // Returns one greater than the largest possible document number.
    //  This may be used to, e.g., determine how big to allocate an array which
    //  will have an element for every document number in an index.
    virtual int MaxDoc() const = 0;

    // Returns the stored fields of the <code>n</code><sup>th</sup>
    //  <code>Document</code> in this index. 
    virtual document::Document& document(const int n) =0;

    // Returns true if document <i>n</i> has been deleted 
    virtual bool isDeleted(const int n) = 0;

    // Returns the byte-encoded normalization factor for the named field of
    //  every document.  This is used by the search code to score documents.
    //  @see org.apache.NSLib.search.Similarity#norm
    virtual l_byte_t* getNorms(const char_t* field) = 0;//array

    // Returns an enumeration of all the terms in the index.
    //  The enumeration is ordered by Term.compareTo().  Each term
    //  is greater than all that precede it in the enumeration.
    virtual TermEnum& getTerms() const =0;

    // Returns an enumeration of all terms after a given term.
    //  The enumeration is ordered by Term.compareTo().  Each term
    //  is greater than all that precede it in the enumeration.
    virtual TermEnum& getTerms(const Term* t) const = 0;

    // Returns the number of documents containing the term <code>t</code>. 
    virtual int docFreq(const Term& t) const = 0;

    // Returns an enumeration of all the documents which contain
    //  <code>term</code>. For each document, the document number, the frequency of
    //  the term in that document is also provided, for use in search scoring.
    //  Thus, this method implements the mapping:
    //  <p><ul>
    //  Term &nbsp;&nbsp; =&gt; &nbsp;&nbsp; &lt;docNum, freq&gt;<sup>*</sup>
    //  </ul>
    //  <p>The enumeration is ordered by document number.  Each document number
    //  is greater than all that precede it in the enumeration. 
    TermDocs& termDocs(Term* term) const;

    // Returns an enumeration of all the documents which contain
    //  <code>term</code>.  For each document, in addition to the document number
    //  and frequency of the term in that document, a list of all of the ordinal
    //  positions of the term in the document is available.  Thus, this method
    //  implements the mapping:
        //
    //  <p><ul>
    //  Term &nbsp;&nbsp; =&gt; &nbsp;&nbsp; &lt;docNum, freq,
    //    &lt;pos<sub>1</sub>, pos<sub>2</sub>, ...
    //  pos<sub>freq-1</sub>&gt;
    //  &gt;<sup>*</sup>
    //  </ul>
    //  <p> This positional information faciliates phrase and proximity searching.
    //  <p>The enumeration is ordered by document number.  Each document number is
    //  greater than all that precede it in the enumeration. 
    TermPositions& termPositions(Term* term);

    // Returns an unpositioned {@link TermPositions} enumerator. 
    virtual TermPositions& termPositions() const = 0;

    // Returns an unpositioned {@link TermDocs} enumerator. 
    virtual TermDocs& termDocs() const = 0;

    DEFINE_MUTEX(Delete_LOCK);
    // Deletes the document numbered <code>docNum</code>.  Once a document is
    //  deleted it will not appear in TermDocs or TermPostitions enumerations.
    //  Attempts to read its field with the {@link #document}
    //  method will result in an error.  The presence of this document may still be
    //  reflected in the {@link #docFreq} statistic, though
    //  this will be corrected eventually as the index is further modified.  
    // synchronized
    void Delete(const int docNum);

    // Deletes all documents containing <code>term</code>.
    //  This is useful if one uses a document field to hold a unique ID string for
    //  the document.  Then to delete such a document, one merely constructs a
    //  term with the appropriate field and the unique ID string as its text and
    //  passes it to this method.  Returns the number of documents deleted. 
    int Delete(Term* term);

    DEFINE_MUTEX(close_LOCK);
    // Closes files associated with this index.
    // Also saves any new deletions to disk.
    // No other methods should be called after this has been called.
    // synchronized
    void close();

    // Returns <code>true</code> iff the index in the named directory is
    // currently locked.
    // @param directory the directory to check for a lock
    // @throws IOException if there is a problem with accessing the index
    static bool isLocked(store::Directory& directory);

    // Returns <code>true</code> iff the index in the named directory is
    // currently locked.
    // @param directory the directory to check for a lock
    // @throws IOException if there is a problem with accessing the index
    static bool isLocked(const fchar_t* directory);


    // Forcibly unlocks the index in the named directory.
    // <P>
    // Caution: this should only be used by failure recovery code,
    // when it is known that no other process nor thread is in fact
    // currently accessing this index.
    static void unlock(store::Directory& directory);
  };

  class IndexReaderLockWith:public NSLockWith{
  public:
    store::Directory* directory;
    bool closeDir;
    void* doBody();
    IndexReaderLockWith(store::NSLock* lock, store::Directory* dir, bool cd){
      this->lock = lock;
      this->directory = dir;
      this->closeDir = cd;
    }
  };
}}

#endif


