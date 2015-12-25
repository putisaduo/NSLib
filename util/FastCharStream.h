#include "StdHeader.h"
#ifndef _NSLib_util_FastCharStream_
#define _NSLib_util_FastCharStream_

#include "util/Arrays.h"
#include "util/Reader.h"

namespace NSLib{ namespace util {

  //Ported implementation of the FastCharStream class NSLIB_EXPORT.
  class FastCharStream
  {
    int col;
    int line;
  
  public:
    BasicReader& input;

    //Initializes a new instance of the FastCharStream class NSLIB_EXPORT.
    FastCharStream(BasicReader& reader);
    
    //Returns the next char_t from the stream.
    char_t GetNext();

    void UnGet();
    
    //Returns the current top char_t from the input stream without removing it.
    char_t Peek();
    
    
    //Returns <b>True</b> if the end of stream was reached.
    bool Eos()  ;

    // Gets the current column.
    int Column();

    // Gets the current line.
    int Line();
  };
}}
#endif
