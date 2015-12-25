#include "StdHeader.h"
#include "IndexReader.h"

#include "store/Directory.h"
#include "store/FSDirectory.h"
#include "store/Lock.h"
#include "document/Document.h"
#include "SegmentInfos.h"
#include "SegmentsReader.h"
#include <sys/stat.h>
#include <iostream>
#include "Terms.h"

using namespace NSLib::util;
namespace NSLib{ namespace index {

IndexReader::IndexReader(store::Directory& dir):directory(dir) {
  writeLock = NULL;
}
  
IndexReader::~IndexReader() {
    if (writeLock != NULL) {
    writeLock->release();  // release write lock
    writeLock = NULL;
  }
}

//static 
IndexReader& IndexReader::open(const fchar_t* path, const bool closeDir){
  std::cerr << "IndexReader opening " << path << std::endl;
  return open(FSDirectory::getDirectory(path,false), closeDir);
}

//static 
IndexReader& IndexReader::open( store::Directory& directory, const bool closeDir){
  std::cerr << "making lock commit.lock" << std::endl;
  store::NSLock* lock = directory.makeLock(CONST_STRING("commit.lock"));
  IndexReaderLockWith with ( lock,&directory,closeDir );
    
  LOCK_MUTEX(DIRECTORIES_MUTEX); // in- & inter-process sync
  void* ret =NULL;
  try{
    ret = with.run();
  }catch(exception e){
    UNLOCK_MUTEX(DIRECTORIES_MUTEX);
    delete lock;
    throw;
  }
  UNLOCK_MUTEX(DIRECTORIES_MUTEX);

  delete lock;

  return *(IndexReader*)ret;
}
  
void* IndexReaderLockWith::doBody() {
  SegmentInfos infos;
  cerr << " IndexReaderLockWith::doBody " << endl;
  infos.read(*directory);
  cerr << " IndexReaderLockWith::doBody " << infos.size() << endl;
  if (infos.size() == 1){      // index is optimized
    IndexReader* ret = new SegmentReader(infos.info(0), closeDir);
    return ret;
  }

  SegmentReader** readers = new SegmentReader*[infos.size()];
  for (uint i = 0; i < infos.size(); i++)
    readers[i] = new SegmentReader(infos.info(i), i==infos.size()-1);
  return new SegmentsReader(*directory, readers, infos.size());
}


  //static 
long_t IndexReader::lastModified(const fchar_t* directory) {
  struct Struct_Stat buf;
  Cmd_Stat(directory, &buf);
  return buf.st_mtime;
}

/* Returns the time the index in the named directory was last modified. 
//static 
long_t IndexReader::lastModified(File directory) {
  return FSDirectory.fileModified(directory, "segments");
}*/

  //static 
long_t IndexReader::lastModified(store::Directory& directory) {
  return directory.fileModified(CONST_STRING("segments"));
}

  //static 
bool IndexReader::indexExists(const fchar_t* directory) {
  fchar_t f[MAX_PATH];
  fstringCopy(f,directory);
  fstringCat(f, CONST_STRING("/segments"));
  return NSLib::util::Misc::dir_Exists(f);
}

/**
* Returns <code>true</code> if an index exists at the specified directory.
* If the directory does not exist or if there is no index in it.
* @param  directory the directory to check for an index
* @return <code>true</code> if an index exists; <code>false</code> otherwise
//static 
boolean IndexReader::indexExists(File directory) {
  return (new File(directory, "segments")).exists();
}*/

  //static
bool IndexReader::indexExists(store::Directory& directory) {
  return directory.fileExists(CONST_STRING("segments"));
}

TermDocs& IndexReader::termDocs(Term* term) const {
  TermDocs& _termDocs = termDocs();
  _termDocs.seek(term);
  return _termDocs;
}

TermPositions& IndexReader::termPositions( Term* term) {
  TermPositions& _termPositions = termPositions();
  _termPositions.seek(term);
  return _termPositions;
}

void IndexReader::Delete(const int docNum) {
  LOCK_MUTEX(Delete_LOCK);
  if (writeLock == NULL) {
    store::NSLock* wl = directory.makeLock(CONST_STRING("write.lock"));
    if (!wl->obtain()){        // obtain write lock
        delete wl;
      _THROWC("Index locked for write or no write access.");
    }
    writeLock = wl;
  }
  doDelete(docNum);
  UNLOCK_MUTEX(Delete_LOCK);
}


int IndexReader::Delete(Term* term) {
  TermDocs* docs = &termDocs(term);
  if ( docs == NULL ) 
    return 0;
  int n = 0;
  _TRY {
    while (docs->next()) {
      Delete(docs->Doc());
      n++;
    }
  } _FINALLY ( docs->close(); );

  return n;
}

void IndexReader::close() {
  LOCK_MUTEX(close_LOCK);
  doClose();
  if (writeLock != NULL) {
    writeLock->release();  // release write lock
    writeLock = NULL;
  }
  UNLOCK_MUTEX(close_LOCK);
}

//static 
bool IndexReader::isLocked(store::Directory& directory) {
  return directory.fileExists(CONST_STRING("write.lock"));
}

  //static
bool IndexReader::isLocked(const fchar_t* directory) {
  fchar_t f[MAX_PATH];
  fstringCopy( f, directory );
  fstringCat ( f,CONST_STRING("/write.lock") );
  return NSLib::util::Misc::dir_Exists(f);
}

  //static 
void IndexReader::unlock(store::Directory& directory){
  try{
    directory.deleteFile(CONST_STRING("write.lock"),false);
  }catch(...){}
  try{
    directory.deleteFile(CONST_STRING("commit.lock"),false);
  }catch(...){}
}

}}
