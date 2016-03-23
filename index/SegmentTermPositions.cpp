#include "StdHeader.h"
#include "SegmentHeader.h"

#include "Terms.h"

using namespace NSLib::util;
namespace NSLib{ namespace index{
SegmentTermPositions::SegmentTermPositions(void* Parent):
  SegmentTermDocs(Parent),
  proxStream( &((SegmentReader*)parent)->proxStream->clone() ),
  //proxStream( ((SegmentReader*)parent)->proxStream),
  position(0),
  proxCount(0)
{
  //proxStream->seek(0);
}

SegmentTermPositions::~SegmentTermPositions(){
}

void SegmentTermPositions::seek(TermInfo* ti) {
  SegmentTermDocs::seek(ti);
  if (ti != NULL)
    proxStream->seek(ti->proxPointer);
  else
    proxCount = 0;
}

void SegmentTermPositions::close() {
  SegmentTermDocs::close();
  proxStream->close();
  //_DELETE( proxStream );
}

int SegmentTermPositions::nextPosition() {
  proxCount--;
  return position += proxStream->readVInt();
}

bool SegmentTermPositions::next() {
  for (int f = proxCount; f > 0; f--)      // skip unread positions
    proxStream->readVInt();

  if (SegmentTermDocs::next()) {          // run super
    proxCount = freq;          // note frequency
    position = 0;          // reset position
    return true;
  }
  return false;
}

int SegmentTermPositions::read(const int docs[], const int freqs[]) {
  _THROWC( "Runtime WException");
}

void SegmentTermPositions::seek(Term* term){
  //todo: hack!!! 
  //supposed to call base class which calls seek(terminfo) of this class
  //but base class is not calling this class (calls it's own implementation) - grrr!!!
  //SegmentTermDocs::seek(term); (used to be just this line)

  TermInfo* ti = ((SegmentReader*)parent)->tis->get(*term);
  seek(ti);
  delete ti;
};
int SegmentTermPositions::Doc() const{
  return SegmentTermDocs::Doc();
};
int SegmentTermPositions::Freq() const{
  return SegmentTermDocs::Freq();
};

int SegmentTermPositions::read(int docs[], int freqs[]){
  return SegmentTermDocs::read(docs,freqs);
};
bool SegmentTermPositions::skipTo(const int target){
  return SegmentTermDocs::skipTo(target);
};

void SegmentTermPositions::skippingDoc() {
  for (int f = freq; f > 0; f--)      // skip all positions
    proxStream->readVInt();
}
}}
