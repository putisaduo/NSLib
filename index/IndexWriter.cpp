#include "StdHeader.h"
#include "IndexWriter.h"

#include "util/VoidList.h"
#include "store/Directory.h"
#include "store/RAMDirectory.h"
#include "store/Lock.h"
#include "document/Document.h"
#include "DocumentWriter.h"
#include "SegmentInfos.h"
#include "SegmentMerger.h"
# include "util/CharConverter.h"
#ifdef USE_INFO_STREAM
# include <ostream>
#endif

using namespace std;
using namespace NSLib::store;
using namespace NSLib::util;
namespace NSLib{ namespace index {

  /*IndexWriter::IndexWriter():
    segmentInfos (*new SegmentInfos),
    ramDirectory (*new store::RAMDirectory)
  {
  }*/

  void IndexWriter::_finalize(){    
    if ( writeLock != NULL ){
      writeLock->release();                          // release write lock
      _DELETE( writeLock );
    }
    delete &ramDirectory;
    delete &segmentInfos;
  }
  IndexWriter::~IndexWriter() {
    _finalize();
  }

    void IndexWriter::_IndexWriter(const bool create){
    NSLock* newLock = directory.makeLock(CONST_STRING("write.lock"));
    if (!newLock->obtain()){                      // obtain write lock
      delete newLock;
      _finalize();
      _THROWC( "Index locked for write or no write access." );
    }
    writeLock = newLock;                   // save it

    NSLock* lock = directory.makeLock(CONST_STRING("commit.lock"));
    IndexWriterLockWith with ( lock,this,create );
      
    LOCK_MUTEX(DIRECTORIES_MUTEX); // in- & inter-process sync
    with.run();
    UNLOCK_MUTEX(DIRECTORIES_MUTEX);
    
    delete lock;
    
    //TODO: check this
    //segmentInfos.dv=true; //delete segments on clear;
  }

  IndexWriter::IndexWriter(const fchar_t* path, analysis::Analyzer& a, const bool create):
    directory( FSDirectory::getDirectory(path, create) ),
    analyzer(a),
    segmentInfos (*new SegmentInfos),
    ramDirectory (*new store::RAMDirectory),
    infoStream(NULL),
    writeLock(NULL),
    ownDir(true)
  {
    _IndexWriter ( create );
  }

  IndexWriter::IndexWriter(store::Directory& d, analysis::Analyzer& a, const bool create):
    directory(d),
    analyzer(a),
    segmentInfos (*new SegmentInfos),
    ramDirectory (*new store::RAMDirectory),
    infoStream(NULL),
    writeLock(NULL),
    ownDir(false)
  {
    _IndexWriter ( create );
  }
  
  void* IndexWriterLockWith::doBody() {
    if (create)
      writer->segmentInfos.write(writer->directory);
    else
      writer->segmentInfos.read(writer->directory);
    return NULL;
  }
  void* IndexWriterLockWith2::doBody() {
    writer->segmentInfos.write(writer->directory);    // commit before deleting
    writer->deleteSegments(*segmentsToDelete);    // delete now-unused segments
    return NULL;
  }

  void IndexWriter::close( const bool closeDir ) {
    LOCK_MUTEX(close_LOCK);
        
    flushRamSegments();
    ramDirectory.close();

    if ( closeDir || ownDir )
      directory.close();

    if ( writeLock != NULL )
      writeLock->release();                          // release write lock
    _DELETE( writeLock );

    UNLOCK_MUTEX(close_LOCK);
  }

  int IndexWriter::docCount() {
    LOCK_MUTEX(docCount_LOCK);
      
    int count = 0;
    for (uint i = 0; i < segmentInfos.size(); i++) {
      SegmentInfo& si = segmentInfos.info(i);
      count += si.docCount;
    }
    
    UNLOCK_MUTEX(docCount_LOCK);
    return count;
  }

  void IndexWriter::addDocument(document::Document& doc) {
    DocumentWriter* dw = new DocumentWriter(ramDirectory, analyzer, maxFieldLength);
    fchar_t* segmentName = newSegmentName();
    dw->addDocument(segmentName, doc);
    
    LOCK_MUTEX(THIS_LOCK);
      segmentInfos.push_back(new SegmentInfo(segmentName, 1, ramDirectory));
      maybeMergeSegments();
    UNLOCK_MUTEX(THIS_LOCK);

    // delete segmentName;  newSegmentName() doesn't call new or even malloc.
    // --RGR
    delete[] segmentName; //the util::Misc::join uses new[] --BVK
    delete dw;
  }


  void IndexWriter::optimize() {
    LOCK_MUTEX(optimize_LOCK);
    flushRamSegments();
    while (segmentInfos.size() > 1 ||
        (segmentInfos.size() == 1 &&
          (SegmentReader::hasDeletions(segmentInfos.info(0)) ||
            &segmentInfos.info(0).dir !=  &directory)))
    {
      int minSegment = segmentInfos.size() - mergeFactor;
      mergeSegments(minSegment < 0 ? 0 : minSegment);
    }
    UNLOCK_MUTEX(optimize_LOCK);
  }
    
  void IndexWriter::addIndexes(Directory** dirs, const int dirsLength) {
      LOCK_MUTEX(addIndexes_LOCK);
    optimize();            // start with zero or 1 seg
    for (int i = 0; i < dirsLength; i++) {
      SegmentInfos sis; // = *new SegmentInfos();    // read infos from dir
      sis.read( *dirs[i]);
      for (uint j = 0; j < sis.size(); j++) {
        segmentInfos.push_back(&sis.info(j));    // add each info
      }
    }
    optimize();            // cleanup
    UNLOCK_MUTEX(addIndexes_LOCK);
  }


  void IndexWriter::flushRamSegments() {
    int minSegment = segmentInfos.size()-1; //don't make this unsigned...
    int docCount = 0;
    while (minSegment >= 0 &&
      &segmentInfos.info(minSegment).dir == &ramDirectory) {
      docCount += segmentInfos.info(minSegment).docCount;
      minSegment--;
    }
    if (minSegment < 0 ||        // add one FS segment?
        (docCount + segmentInfos.info(minSegment).docCount) > mergeFactor ||
        !(&segmentInfos.info(segmentInfos.size()-1).dir == &ramDirectory))
      minSegment++;

    if (minSegment >= (int)segmentInfos.size())
      return;            // none to merge
    mergeSegments(minSegment);
  }

  void IndexWriter::maybeMergeSegments() {
    long_t targetMergeDocs = mergeFactor;
    while (targetMergeDocs <= maxMergeDocs) {
      // find segments smaller than current target size
      int minSegment = segmentInfos.size();
      int mergeDocs = 0;
      while (--minSegment >= 0) {
        SegmentInfo& si = segmentInfos.info(minSegment);
        if (si.docCount >= targetMergeDocs)
          break;
        mergeDocs += si.docCount;
      }

      if (mergeDocs >= targetMergeDocs)      // found a merge to do
        mergeSegments(minSegment+1);
      else
        break;
          
      targetMergeDocs *= mergeFactor;      // increase target size
    }
  }

  void IndexWriter::mergeSegments(const uint minSegment) {
    const fchar_t* mergedName = newSegmentName();
    int mergedDocCount = 0;
#ifdef USE_INFO_STREAM
    if (infoStream != NULL) 
      *infoStream<< "merging segments" << "\n";
#endif
    SegmentMerger merger(directory, mergedName);
    util::VoidList<SegmentReader*> segmentsToDelete;
    for (uint i = minSegment; i < segmentInfos.size(); i++) {
      SegmentInfo& si = segmentInfos.info(i);
#ifdef USE_INFO_STREAM
      if ( infoStream != NULL)
        *infoStream << " " << si.name << " (" << si.docCount << " docs)";
#endif      
      SegmentReader* reader = new SegmentReader(si,false);
      merger.add(*reader);
      if ((&reader->directory == &this->directory) || // if we own the directory
        (&reader->directory == &this->ramDirectory))
        segmentsToDelete.push_back(reader);    // queue segment for deletion
      
      mergedDocCount += si.docCount;
    }
#ifdef USE_INFO_STREAM
    if (infoStream != NULL) {
      *infoStream<<"\n into "<<mergedName<<" ("<<mergedDocCount<<" docs)";
    }
#endif
    merger.merge();

    //TODO: check this code. Should delete on pop?
    //segmentInfos.setSize(minSegment);      // pop old infos & add new
    while ( segmentInfos.size() > minSegment )
      segmentInfos.pop_back();
    
    segmentInfos.push_back( new SegmentInfo(mergedName, mergedDocCount, directory));
    


    NSLock* lock = directory.makeLock(CONST_STRING("commit.lock"));
    IndexWriterLockWith2 with ( lock,this,&segmentsToDelete );
      
    LOCK_MUTEX(DIRECTORIES_MUTEX); // in- & inter-process sync
    with.run();
    UNLOCK_MUTEX(DIRECTORIES_MUTEX);

    delete lock;
    delete[] mergedName; //ADD:
  }

  void IndexWriter::deleteSegments(util::VoidList<SegmentReader*> &segments) {
    fStringArrayConst deletable;
    deletable.setDoDelete(util::DELETE_TYPE_DELETE_ARRAY);

    fStringArrayConst &deleteArray = readDeleteableFiles();
    deleteFiles(deleteArray, deletable); // try to delete deleteable
    delete &deleteArray;
    
    for (uint i = 0; i < segments.size(); i++) {
      SegmentReader* reader = segments.at(i);
      fStringArrayConst& files = reader->files();
      if (&reader->directory == &this->directory)
        deleteFiles(files, deletable);    // try to delete our files
      else
        deleteFiles(files, reader->directory); // delete, eg, RAM files

      delete &files;
    }

    writeDeleteableFiles(deletable);      // note files we can't delete
  }

  void IndexWriter::deleteFiles(const fStringArrayConst& files, store::Directory& directory) {
    for (uint i = 0; i < files.size(); i++)
      directory.deleteFile( files[i] );
  }

  void IndexWriter::deleteFiles(const fStringArrayConst& files, fStringArrayConst& deletable) {
    for (uint i = 0; i < files.size(); i++) {
      const fchar_t* file = files[i];
      try {
        directory.deleteFile(file);      // try to delete each file
      } catch (exception e) {        // if delete fails
        if (directory.fileExists(file)) {
#ifdef USE_INFO_STREAM
          if (infoStream != NULL)
            *infoStream << e.what( ) << "; Will re-try later.\n";
#endif
          deletable.push_back(fstringDuplicate(file));      // add to deletable
        }
      }
    }
  }

  fStringArrayConst& IndexWriter::readDeleteableFiles() {
    fStringArrayConst& result = *new fStringArrayConst(true,util::DELETE_TYPE_DELETE_ARRAY);

    if (!directory.fileExists(CONST_STRING("deletable")))
      return result;

    InputStream& input = directory.openFile(CONST_STRING("deletable"));
    _TRY {
      for (int i = input.readInt(); i > 0; i--)    // read file names
        result.push_back(util::CharConverter::wideToChar(input.readString(), "8859-1"));
    } _FINALLY(
        input.close();
        delete &input;
    );
    
    
    return result;
  }

  void IndexWriter::writeDeleteableFiles(fStringArrayConst& files) {
    OutputStream& output = directory.createFile(CONST_STRING("deleteable.new"));
    _TRY {
      output.writeInt(files.size());
      for (uint i = 0; i < files.size(); i++)
        output.writeString( CharConverter::charToWide(files.at(i), LANG_ENG));
    } _FINALLY(
        output.close();
        delete &output;
    );
    
    directory.renameFile(CONST_STRING("deleteable.new"), CONST_STRING("deletable"));
  }

  fchar_t* IndexWriter::newSegmentName() {
      LOCK_MUTEX(newSegmentName_LOCK);
    char_t buf[9];
    integerToString(segmentInfos.counter++,buf,CHAR_RADIX); //36 is RADIX of 10 digits and 26 numbers
    
    UNLOCK_MUTEX(newSegmentName_LOCK);
    return util::Misc::fjoin( CONST_STRING("_"),util::CharConverter::wideToChar(buf, LANG_ENG));
  }
}}
