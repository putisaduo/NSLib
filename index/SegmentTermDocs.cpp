#include "StdHeader.h"
#include "SegmentHeader.h"

#include "util/BitVector.h"
#include "Term.h"

#include <iostream>

namespace NSLib{ namespace index {

SegmentTermDocs::SegmentTermDocs(void* Parent):
  parent(Parent),
  deletedDocs( ((SegmentReader*)parent)->deletedDocs),
  doc(0),
  freq(0),
  freqCount(0),
  freqStream ( &((SegmentReader*)parent)->freqStream->clone())
  //freqStream ( ((SegmentReader*)parent)->freqStream)
{
  //freqStream->seek(0); 
}

SegmentTermDocs::~SegmentTermDocs(){
}
  

void SegmentTermDocs::seek( Term* term) {
  TermInfo* ti = ((SegmentReader*)parent)->tis->get(*term);
  seek(ti);
  delete ti;
}
  
void SegmentTermDocs::seek(const TermInfo* ti) {
  //cerr << "SegmentTermDocs::seek: " << ti << endl;
  if (ti == NULL) {
    freqCount = 0;
  } else {
    freqCount = ti->docFreq;
    doc = 0;
    freqStream->seek(ti->freqPointer);
  }
}
  
void SegmentTermDocs::close() {
  freqStream->close();
  //_DELETE( freqStream );
}

int SegmentTermDocs::Doc()const { return doc; }
int SegmentTermDocs::Freq()const { return freq; }

bool SegmentTermDocs::next() {
  while (true) {
    if (freqCount == 0)
      return false;

    int sdocCode = freqStream->readVInt();
    uint docCode = sdocCode;// shift off low bit TODO: check this... was >>>
    doc += docCode >> 1;
    if ((docCode & 1) != 0)        // if low bit is set
      freq = 1;          // freq is one
    else
      freq = freqStream->readVInt();      // else read freq
   
    freqCount--;
      
    if ( (deletedDocs == NULL) || (deletedDocs->get(doc) == false ) )
      break;
    skippingDoc();
  }
  return true;
}

/** Optimized implementation. */
int SegmentTermDocs::read(int docs[], int freqs[]) {
  int end = sizeof(docs)/sizeof(int);
  int i = 0;
  //cerr << "SegmentTermDocs::read: " << end << " " << freqCount << endl;
  while (i < end && freqCount > 0) {
    // manually inlined call to next() for speed
    int sdocCode = freqStream->readVInt();
    uint docCode = sdocCode;
    doc += docCode >> 1;        // shift off low bit TODO: check this... was >>>
    if ((docCode & 1) != 0)        // if low bit is set
      freq = 1;          // freq is one
    else
      freq = freqStream->readVInt();      // else read freq
    freqCount--;
     
    if (deletedDocs == NULL || !deletedDocs->get(doc)) {
      docs[i] = doc;
      freqs[i] = freq;
      ++i;
    }
    //cerr << "\tSegmentTermDocs::read: " << sdocCode << " " << freqCount << " " 
    //     <<deletedDocs << " " << doc << " " << i << " " << freqs[i] << endl;
  }
  return i;
}

/** As yet unoptimized implementation. */
bool SegmentTermDocs::skipTo(const int target){
  do {
    if (!next())
      return false;
  } while (target > doc);
  return true;
}

void SegmentTermDocs::skippingDoc() {
}

}}
