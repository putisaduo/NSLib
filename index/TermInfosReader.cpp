#include "StdHeader.h"
#include "TermInfosReader.h"

#include "Term.h"
#include "Terms.h"
#include "SegmentTermEnum.h"
#include "store/Directory.h"
#include "FieldInfos.h"
#include "TermInfo.h"
#include "TermInfosWriter.h"
#include "util/Arrays.h"
#include <iostream>

namespace NSLib{ namespace index {

const fchar_t* TermInfosReader::segmentname( const fchar_t* ext ){
  fchar_t* buf = new fchar_t[MAX_PATH];
  fstringPrintF(buf, CONST_STRING("%s%s"), segment,ext );
  return buf;
}

TermInfosReader::TermInfosReader(Directory& dir, const fchar_t* seg, FieldInfos& fis):
  directory (dir),
  segment (seg),
  fieldInfos (fis),
  indexTerms(NULL)
{

  const fchar_t* n = segmentname(CONST_STRING(".tis"));
  _enum = new SegmentTermEnum( directory.openFile( n ), fieldInfos, false);
  delete[] n;

  size = ( _enum->size );
  readIndex();
}
  
TermInfosReader::~TermInfosReader(){
}
  
void TermInfosReader::close() {
  if (_enum != NULL){
      _enum->close();
      _enum->input.close();
      delete &_enum->input;
        _DELETE(_enum);
    }

  for ( int i=0;i<indexTermsLength;i++ ){
    indexTerms[i]->finalize();
    delete indexInfos[i];
  }
  delete[] indexTerms;
  delete[] indexPointers;
  delete[] indexInfos;
    
  indexTerms = NULL;
  indexPointers = NULL;
  indexInfos = NULL;
}
int TermInfosReader::Size() {
  return size;
}

Term* TermInfosReader::get(const int position) {
  if (size == 0) return NULL;

      DEFINE_MUTEX(getInt_LOCK);
  if ( _enum != NULL && _enum->getTerm() != NULL && position >= _enum->position &&
      position < (_enum->position + NSLIB_WRITER_INDEX_INTERVAL )){
      Term* ret = scanEnum(position);
        
      UNLOCK_MUTEX(getInt_LOCK);
    return ret;      // can avoid seek
    }

  seekEnum(position / NSLIB_WRITER_INDEX_INTERVAL); // must seek
  Term* ret = scanEnum(position);
    
  UNLOCK_MUTEX(getInt_LOCK);
  return ret;
}


TermInfo* TermInfosReader::get(const Term& term){
  //cerr << "  TermInfosReader::get " << size << endl;
  if (size == 0) return NULL;
  
  LOCK_MUTEX(getTerm_LOCK);
  
  // optimize sequential access: first try scanning cached _enum w/o seeking
  if ( _enum->getTerm(false) != NULL        // term is at or past current
      && ((_enum->prev != NULL && term.compareTo(*_enum->prev) > 0)
          || term.compareTo(*_enum->getTerm(false)) >= 0)) 
  { 
    int _enumOffset = (_enum->position/NSLIB_WRITER_INDEX_INTERVAL)+1;
    if (indexTermsLength == _enumOffset    // but before end of block
        || term.compareTo(*indexTerms[_enumOffset]) < 0)
    {
      TermInfo* ret = scanEnum(term);        // no need to seek
      UNLOCK_MUTEX(getTerm_LOCK);
      //cerr << "  TermInfosReader::get " << _enumOffset << " " 
      //     << indexTermsLength << " " << ret->docFreq << endl;
      return ret;
    }
  }
      
  // random-access: must seek
  seekEnum(getIndexOffset(term));
  TermInfo* ret = scanEnum(term);
    
  UNLOCK_MUTEX(getTerm_LOCK);
  return ret;
}
  

int TermInfosReader::getPosition(const Term& term) {
  if (size == 0) return -1;
    
  LOCK_MUTEX(getPosition_LOCK);

  int indexOffset = getIndexOffset(term);
  seekEnum(indexOffset);

  while(term.compareTo(*_enum->getTerm(false)) > 0 && _enum->next()) {}

      UNLOCK_MUTEX(getPosition_LOCK);
  if (term.compareTo(*_enum->getTerm(false)) == 0)
    return _enum->position;
  else
    return -1;
}

SegmentTermEnum& TermInfosReader::getTerms(){
    LOCK_MUTEX(getTerms_LOCK);
  if (_enum->position != -1)        // if not at start
    seekEnum(0);          // reset to start
    
  //TODO2: check this code
  SegmentTermEnum* cln = _enum->clone();
    
  UNLOCK_MUTEX(getTerms_LOCK);
  return *cln;
}

SegmentTermEnum& TermInfosReader::getTerms(const Term& term) {
    LOCK_MUTEX(getTermsTerm_LOCK);
  get(term);            // seek _enum to term

  SegmentTermEnum* cln = _enum->clone();
    
  UNLOCK_MUTEX(getTermsTerm_LOCK);
  return *cln;
}

void TermInfosReader::readIndex() {
  const fchar_t* buf = segmentname(CONST_STRING(".tii"));
  InputStream& is = directory.openFile( buf );
  delete[] buf;
  SegmentTermEnum& indexEnum = *new SegmentTermEnum( is, fieldInfos, true);

  _TRY {
    indexTermsLength = indexEnum.size;

    indexTerms = new Term*[indexTermsLength];
    indexInfos = new TermInfo*[indexTermsLength];
    indexPointers = new long_t[indexTermsLength];

    //cerr << "   TermInfosReader::readIndex tii " << indexTermsLength << endl;
    for (int i = 0; indexEnum.next(); i++) {
      indexTerms[i] = indexEnum.getTerm();
      indexInfos[i] = indexEnum.getTermInfo();
      indexPointers[i] = indexEnum.indexPointer;
    }
  } _FINALLY( indexEnum.close(); delete &indexEnum; is.close();  delete &is; );
}

  
int TermInfosReader::getIndexOffset(const Term& term)  {
  int lo = 0;            // binary search indexTerms[]
  int hi = indexTermsLength - 1;

  while (hi >= lo) {
    int mid = (lo + hi) >> 1;
    int delta = term.compareTo(*indexTerms[mid]);
    if (delta < 0)
      hi = mid - 1;
    else if (delta > 0)
      lo = mid + 1;
    else
      return mid;
  }
  return hi;
}

void TermInfosReader::seekEnum(const int indexOffset) {
  _enum->seek( indexPointers[indexOffset],
    (indexOffset * NSLIB_WRITER_INDEX_INTERVAL) - 1,
    *indexTerms[indexOffset], *indexInfos[indexOffset]);
}  


TermInfo* TermInfosReader::scanEnum(const Term& term) {
  //std::cerr << " TermInfosReader::scanEnum: " << ws2str(term.Field()) << endl;
  while ( term.compareTo(*_enum->getTerm(false) ) > 0 && _enum->next()) {}

  if (_enum->getTerm(false) != NULL && term.compareTo(*_enum->getTerm(false)) == 0)
    return _enum->getTermInfo();
  else
    return NULL;
}


Term* TermInfosReader::scanEnum(const int position) {
  while(_enum->position < position)
    if (!_enum->next())
      return NULL;

  return _enum->getTerm();
}

}}
