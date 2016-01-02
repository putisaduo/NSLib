#include "StdHeader.h"
#ifndef _NSLib_index_SegmentHeader_
#define _NSLib_index_SegmentHeader_

#include "SegmentInfo.h"
#include "util/BitVector.h"
#include "util/VoidMap.h"
#include "Term.h"
#include "FieldInfos.h"
#include "FieldsReader.h"
#include "IndexReader.h"
#include "TermInfosReader.h"

namespace NSLib{ namespace index{

class SegmentTermDocs: public TermDocs 
{
protected:
  // SegmentReader parent
  const void* parent; 

private:
  InputStream* freqStream;
  int freqCount;
  NSLib::util::BitVector* deletedDocs;
public:
  int doc;
  int freq;

  //Parent is a segment reader
  SegmentTermDocs( void* Parent);
  virtual ~SegmentTermDocs();
  
  void seek(Term* term);
  void seek(const TermInfo* ti);
  void close();
  int Doc()const;
  int Freq()const;

  bool next();

  // Optimized implementation. 
  int read(int docs[], int freqs[]);

  // As yet unoptimized implementation. 
  bool skipTo(const int target);

protected:
  void skippingDoc();
};



class SegmentTermPositions: public SegmentTermDocs,
                            public virtual TermPositions
{
private:
  InputStream* proxStream;
  int proxCount;
  int position;

public:
  //parent is a segment reader
  SegmentTermPositions(void* Parent);
  ~SegmentTermPositions();
  
  void seek(TermInfo* ti);

  void close();
  int nextPosition();
  bool next();
  int read(const int docs[], const int freqs[]);

  // The virtual members required in TermPositions are defined in the subclass 
  //    SegmentTermDocs, but why is there a compiler error because of this.
  void seek(Term* term);
  int Doc() const;
  int Freq() const;
  int read(int docs[], int freqs[]);
  bool skipTo(const int target);

protected:
  void skippingDoc();
};





class Norm {
public:
  InputStream& in;
  l_byte_t* bytes;

  Norm(InputStream& instrm);
  ~Norm();
};




class SegmentReader: public IndexReader{
private:
  bool closeDirectory;
  const fchar_t* segment;

  bool deletedDocsDirty;
  
  
  DEFINE_MUTEX(norms_mutex);
  NSLib::util::VoidMap<const char_t*,Norm*> norms;
public:

  FieldsReader* fieldsReader;
  NSLib::util::BitVector* deletedDocs;
  InputStream* freqStream;
  TermInfosReader* tis;
  InputStream* proxStream;
  FieldInfos* fieldInfos;

  SegmentReader(SegmentInfo& si, const bool closeDir=false);
  virtual ~SegmentReader();

  DEFINE_MUTEX(doClose_LOCK);
  //syncronized
  void doClose();

  static bool hasDeletions(const SegmentInfo& si);
  
  DEFINE_MUTEX(doDelete_LOCK);
  //syncronized
  void doDelete(const int docNum);

  fStringArrayConst& files();
  TermEnum& getTerms() const;
  TermEnum& getTerms(const Term* t) const;
  
  DEFINE_MUTEX(document_LOCK);
  //syncronized
  //todo: make const func
  NSLib::document::Document& document(const int n);
  
  DEFINE_MUTEX(isDeleted_LOCK);
  //syncronized
  //todo: make const func
  bool isDeleted(const int n);
  
  TermDocs& termDocs() const;
  TermPositions& termPositions() const;

  int docFreq(const Term& t) const;
  int NumDocs();
  int MaxDoc() const;
  l_byte_t* getNorms(const char_t* field);
  void getNorms(const char_t* field, l_byte_t* bytes, const int offset);
  InputStream* normStream(const char_t* field);

  static void segmentname(fchar_t* buffer, const fchar_t* segment, const fchar_t* ext, const int x=-1);
  void segmentname(fchar_t* buffer, const fchar_t* ext, const int x=-1 );
  //essentially samee as above, but copies return
  fchar_t* segmentname(const fchar_t* ext, const int x=-1);
  
private:
  void openNorms();
  void closeNorms();
};

class SegmentReaderLockWith:public NSLockWith{
public:
  SegmentReader* reader;
  void* doBody();

  SegmentReaderLockWith(NSLock* lock, SegmentReader* rdr){
    this->lock = lock;
    this->reader = rdr;
  }
};

}}

#endif
