#include "StdHeader.h"
#include "SegmentHeader.h"

#include "SegmentInfo.h"
#include "FieldInfos.h"
#include "FieldsReader.h"
#include "IndexReader.h"
#include "TermInfosReader.h"
#include "util/BitVector.h"
#include "Terms.h"
#include "util/VoidMap.h"

#include <iostream>

using namespace NSLib::util;
namespace NSLib{ namespace index{


Norm::Norm(InputStream& instrm):in(instrm) , bytes(NULL)
{ 
    //CONSTRUCTOR
}
Norm::~Norm(){
}




//static
void SegmentReader::segmentname(fchar_t* buffer, const fchar_t* segment, 
                                const fchar_t* ext, const int x)
{
  if ( x== -1 )
    fstringPrintF(buffer, CONST_STRING("%s%s"), segment,ext );
  else
    fstringPrintF(buffer, CONST_STRING("%s%s%d"), segment,ext,x );
}

fchar_t* SegmentReader::segmentname(const fchar_t* ext, const int x){
      fchar_t* buf = new fchar_t[MAX_PATH];
  segmentname(buf,ext,x);
      return buf ;
}

void SegmentReader::segmentname(fchar_t* buffer, const fchar_t* ext, const int x ){
  SegmentReader::segmentname(buffer, segment,ext,x);
}

SegmentReader::~SegmentReader(){
  delete fieldInfos;
  delete fieldsReader;
  delete tis;
  delete freqStream;
  delete proxStream;
  delete[] segment;
  delete deletedDocs;
}

SegmentReader::SegmentReader(SegmentInfo& si, const bool closeDir) : 
  IndexReader(si.dir), //init super
  closeDirectory(closeDir), 
  deletedDocs(NULL),
  deletedDocsDirty(false),
  fieldInfos(NULL),
  // make sure that all index files have been read or are kept open
  // so that if an index update removes them we'll still have them
  freqStream( NULL ),
  proxStream( NULL )
{
  segment = fstringDuplicate(si.name);
    
  fchar_t buf[MAX_PATH];
  segmentname(buf, CONST_STRING(".fnm"));
  fieldInfos = new FieldInfos(directory, buf );

  segmentname(buf, CONST_STRING(".frq"));
  freqStream = &directory.openFile( buf );
    
  segmentname(buf, CONST_STRING(".prx"));
  proxStream=  &directory.openFile( buf );
    
  //TODO: better to do in init, but fieldInfos wasn't being inited before calling fieldsReader????
  fieldsReader = new FieldsReader(directory, segment, *fieldInfos);
  tis = new TermInfosReader(directory, segment, *fieldInfos);

  if (hasDeletions(si)){
    segmentname(buf, CONST_STRING(".del"));
    deletedDocs = new NSLib::util::BitVector(directory, buf );
  }
  openNorms();
}

void SegmentReader::doClose() {
    LOCK_MUTEX(doClose_LOCK);
  if (deletedDocsDirty) {
    NSLock* lock = directory.makeLock(CONST_STRING("commit.lock"));
    SegmentReaderLockWith with ( lock, this );
      
    LOCK_MUTEX(DIRECTORIES_MUTEX); // in- & inter-process sync
    with.run();
    UNLOCK_MUTEX(DIRECTORIES_MUTEX);

    delete lock;
    deletedDocsDirty = false;
  }

  fieldsReader->close();
  tis->close();

  if (freqStream != NULL)
    freqStream->close();
  if (proxStream != NULL)
    proxStream->close();

  closeNorms();

  if (closeDirectory)
    directory.close();
    
  UNLOCK_MUTEX(doClose_LOCK);
}

//static
bool SegmentReader::hasDeletions(const SegmentInfo& si) {
  fchar_t f[MAX_PATH];
  SegmentReader::segmentname(f, si.name,CONST_STRING(".del"),-1 );
  bool ret = si.dir.fileExists( f );
  return ret;
}

void SegmentReader::doDelete(const int docNum)  {
    LOCK_MUTEX(doDelete_LOCK);
  if (deletedDocs == NULL)
    deletedDocs = new NSLib::util::BitVector(MaxDoc());
  deletedDocsDirty = true;
  deletedDocs->set(docNum);
  UNLOCK_MUTEX(doDelete_LOCK);
}

fStringArrayConst& SegmentReader::files() {
  fStringArrayConst& files = *new fStringArrayConst(true, util::DELETE_TYPE_DELETE_ARRAY);
  files.push_back( segmentname(CONST_STRING(".fnm") ));
  files.push_back( segmentname(CONST_STRING(".fdx") ));
  files.push_back( segmentname(CONST_STRING(".fdt") ));
  files.push_back( segmentname(CONST_STRING(".tii") ));
  files.push_back( segmentname(CONST_STRING(".tis") ));
  files.push_back( segmentname(CONST_STRING(".frq") ));
  files.push_back( segmentname(CONST_STRING(".prx") ));

  const fchar_t* tmp = segmentname(CONST_STRING(".del"));
  if (directory.fileExists( tmp ) )
    files.push_back( tmp );
  else
    delete[] tmp;

  for (int i = 0; i < fieldInfos->size(); i++) {
    FieldInfo& fi = fieldInfos->fieldInfo(i);
    if (fi.isIndexed)
      files.push_back( segmentname(CONST_STRING(".f"), i) );
  }
  return files;
}

TermEnum& SegmentReader::getTerms() const {
  return tis->getTerms();
}

TermEnum& SegmentReader::getTerms(const Term* t) const {
  return tis->getTerms(*t);
}

NSLib::document::Document& SegmentReader::document(const int n) {
  LOCK_MUTEX(document_LOCK);
  if (isDeleted(n)){
      UNLOCK_MUTEX(document_LOCK);
    _THROWC(  "attempt to access a deleted document" );
  }
  NSLib::document::Document& ret = fieldsReader->doc(n);
    
  UNLOCK_MUTEX(document_LOCK);
  return ret;
}

bool SegmentReader::isDeleted(const int n) {
  static DEFINE_MUTEX(asdf);
  LOCK_MUTEX(isDeleted_LOCK);
  bool ret = (deletedDocs != NULL && deletedDocs->get(n));
    
  UNLOCK_MUTEX(isDeleted_LOCK);
  return ret;
}

TermDocs& SegmentReader::termDocs() const {
  return *new SegmentTermDocs((void*)this);
}

TermPositions& SegmentReader::termPositions() const {
  return *new SegmentTermPositions((void*)this);
}

int SegmentReader::docFreq(const Term& t) const {
  TermInfo* ti = tis->get(t);
  if (ti != NULL){
    int ret = ti->docFreq;
    delete ti;
    return ret;
  }
  return 0;
}

int SegmentReader::NumDocs() {
  int n = MaxDoc();
  if (deletedDocs != NULL)
    n -= deletedDocs->Count();
  return n;
}

int SegmentReader::MaxDoc() const {
  return fieldsReader->Size();
}

l_byte_t* SegmentReader::getNorms(const char_t* field) {
  Norm* norm = (Norm*)norms.get(field);
  if (norm == NULL)
    return NULL;
  if (norm->bytes == NULL) {
    l_byte_t* bytes = new l_byte_t[MaxDoc()]; //array
    getNorms(field, bytes, 0);
    norm->bytes = bytes;
  }
  return norm->bytes;
}

void SegmentReader::getNorms(const char_t* field, l_byte_t* bytes, const int offset) {
  InputStream* _normStream = normStream(field);
  if (_normStream == NULL)
    return;            // use zeros in array
  _TRY {
    _normStream->readBytes(bytes, offset, MaxDoc());
  } _FINALLY (
    _normStream->close();
    delete _normStream;
  );
}

InputStream* SegmentReader::normStream(const char_t* field) {
  Norm* norm = norms.get(field);
  if (norm == NULL)
    return NULL;

  //InputStream* result = &norm->in.clone();
  InputStream* result = &norm->in;
  result->seek(0);
  return result;
}

void SegmentReader::openNorms() {
  for (int i = 0; i < fieldInfos->size(); i++) {
    FieldInfo& fi = fieldInfos->fieldInfo(i);
    if (fi.isIndexed) {
      fchar_t f[MAX_PATH];
      segmentname(f, CONST_STRING(".f"), fi.number);
      //TODO, should fi.name be copied?
      //cerr << "  SegmentReader::openNorms " << fi.name << " vs. " << f << endl;
      norms.put(fi.name, new Norm( directory.openFile( f ) ));
    }
  }
}
void SegmentReader::closeNorms() {
  LOCK_MUTEX(norms_mutex);
  map<const char_t*,Norm*, util::charCompare>::iterator itr = norms.begin();
    while (itr != norms.end()) {
      itr->second->in.close();
      delete &itr->second->in;
      delete itr->second->bytes;
      delete itr->second;

      itr ++;
    }
  UNLOCK_MUTEX(norms_mutex);
}

void* SegmentReaderLockWith::doBody() {
  fchar_t tmpName[MAX_PATH];
  fchar_t delName[MAX_PATH];
  reader->segmentname(tmpName, CONST_STRING(".tmp"));
  reader->segmentname(delName, CONST_STRING(".del"));
    
  reader->deletedDocs->write(reader->directory,  tmpName );
  reader->directory.renameFile( tmpName, delName );
  return NULL;
}

}}
