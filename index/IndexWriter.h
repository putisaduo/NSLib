#include "StdHeader.h"
#ifndef _NSLib_index_IndexWriter_
#define _NSLib_index_IndexWriter_

#include "util/VoidList.h"
#include "store/Directory.h"
#include "store/Lock.h"
#include "document/Document.h"
#include "analysis/AnalysisHeader.h"
#include "SegmentHeader.h"

using namespace std;
using namespace NSLib::store;
namespace NSLib{ namespace index {
    //  An IndexWriter creates and maintains an index.
    //
    //  The third argument to the <a href="#IndexWriter"><b>constructor</b></a>
    //  determines whether a new index is created, or whether an existing index is
    //  opened for the addition of new documents.
    //
    //  In either case, documents are added with the <a
    //  href="#addDocument"><b>addDocument</b></a> method.  When finished adding
    //  documents, <a href="#close"><b>close</b></a> should be called.
    //
    //  If an index will not have more documents added for a while and optimal search
    //  performance is desired, then the <a href="#optimize"><b>optimize</b></a>
    //  method should be called before the index is closed.
  class IndexWriter {
  private:
    // how to analyze text
    analysis::Analyzer& analyzer;

    Directory& ramDirectory; // for temp segs

    NSLock* writeLock;

    DEFINE_MUTEX(THIS_LOCK);
    
    void _IndexWriter(const bool create);

    void _finalize();

    bool ownDir; //if directory belongs to this object
  public:
    // the segments
    SegmentInfos& segmentInfos;

    // where this index resides
    Directory& directory;        

    // Release the write lock, if needed. 
    ~IndexWriter();

    // If non-NULL, information about merges will be printed to this. 
    ostream* infoStream;

        //The maximum number of terms that will be indexed for a single field in a
        //document.  This limits the amount of memory required for indexing, so that
        //collections with very large files will not crash the indexing process by
        //running out of memory.
        //
        //<p>By default, no more than 10,000 terms will be indexed for a field. 
    const static int maxFieldLength = 10000;
    
    // Determines how often segment indexes are merged by addDocument().  With
    // smaller values, less RAM is used while indexing, and searches on
    // unoptimized indexes are faster, but indexing speed is slower.  With larger
    // values more RAM is used while indexing and searches on unoptimized indexes
    // are slower, but indexing is faster.  Thus larger values (> 10) are best
    // for batched index creation, and smaller values (< 10) for indexes that are
    // interactively maintained.
    //
    // <p>This must never be less than 2.  The default value is 10.
    const static int mergeFactor = 10;

    // Determines the largest number of documents ever merged by addDocument().
    // Small values (e.g., less than 10,000) are best for interactive indexing,
    // as this limits the length of pauses while indexing to a few seconds.
    // Larger values are best for batched indexing and speedier searches.
    //
    // <p>The default value is {@link Integer#MAX_VALUE}. 
    const static int maxMergeDocs = INT_MAX;

        // Constructs an IndexWriter for the index in <code>path</code>.  Text will
    //  be analyzed with <code>a</code>.  If <code>create</code> is true, then a
    //  new, empty index will be created in <code>path</code>, replacing the index
    //  already there, if any. 
    IndexWriter(const fchar_t* path, analysis::Analyzer& a, const bool create);

    // Constructs an IndexWriter for the index in <code>d</code>.  Text will be
    //  analyzed with <code>a</code>.  If <code>create</code> is true, then a new,
    //  empty index will be created in <code>d</code>, replacing the index already
    //  there, if any. 
    IndexWriter(Directory& d, analysis::Analyzer& a, const bool create);

        DEFINE_MUTEX(close_LOCK);
    // Flushes all changes to an index, closes all associated files, and closes
    //  the directory that the index is stored in. 
    // synchronized
    void close(const bool closeDir=true);

        DEFINE_MUTEX(docCount_LOCK);
    // Returns the number of documents currently in this index. 
    // synchronized
    int docCount();


    // Adds a document to this index.
    void addDocument(document::Document& doc);

        DEFINE_MUTEX(optimize_LOCK);
    // Merges all segments together into a single segment, optimizing an index
    //  for search. 
    // synchronized
    void optimize();
      
      
        DEFINE_MUTEX(addIndexes_LOCK);
    // Merges all segments from an array of indexes into this index.
    //
    // <p>This may be used to parallelize batch indexing.  A large document
    // collection can be broken into sub-collections.  Each sub-collection can be
    // indexed in parallel, on a different thread, process or machine.  The
    // complete index can then be created by merging sub-collection indexes
    // with this method.
    //
    // <p>After this completes, the index is optimized. 
    // synchronized
    void addIndexes(Directory** dirs, const int dirsLength);


    

    //Some operating systems (e.g. Windows) don't permit a file to be deleted
    //  while it is opened for read (e.g. by another process or thread).  So we
    //  assume that when a delete fails it is because the file is open in another
    //  process, and queue the file for subsequent deletion. 
    void deleteSegments(util::VoidList<SegmentReader*> &segments);
    void deleteFiles(const fStringArrayConst& files, Directory& directory);

    void deleteFiles(const fStringArrayConst& files, fStringArrayConst& deletable);
  private:
    // Merges all RAM-resident segments. 
    void flushRamSegments();

    // Incremental segment merger.  
    void maybeMergeSegments();

    // Pops segments off of segmentInfos stack down to minSegment, merges them,
    //  and pushes the merged index onto the top of the segmentInfos stack. 
    void mergeSegments(const uint minSegment);

    fStringArrayConst& readDeleteableFiles();
    void writeDeleteableFiles(fStringArrayConst& files);
    
    
    DEFINE_MUTEX(newSegmentName_LOCK);
    // synchronized
    fchar_t* newSegmentName();
  };

  class IndexWriterLockWith:public NSLockWith{
  public:
    IndexWriter* writer;
    bool create;
    void* doBody();
    IndexWriterLockWith(NSLock* lock, IndexWriter* wr, bool cr){
      this->lock = lock;
      this->writer = wr;
      this->create = cr;
    }
  };

  class IndexWriterLockWith2:public NSLockWith{
  public:
    util::VoidList<SegmentReader*>* segmentsToDelete;
    IndexWriter* writer;
    void* doBody();
    IndexWriterLockWith2(NSLock* lock, IndexWriter* wr, util::VoidList<SegmentReader*>* std){
      this->lock = lock;
      this->writer = wr;
      this->segmentsToDelete = std;
    }
  };

}}
#endif
