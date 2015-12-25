#include "StdHeader.h"
#include "FastCharStream.h"

#include "util/Arrays.h"
#include "util/Reader.h"

namespace NSLib{ namespace util {

  //Initializes a new instance of the FastCharStream class NSLIB_EXPORT.
  FastCharStream::FastCharStream(BasicReader& reader):
    input(reader),
    col(1),
    line(1)
  {
  }
  
  //Returns the next char_t from the stream.
  char_t FastCharStream::GetNext()  {
    if (Eos()) 
    {
      throw "warning : FileReader.GetNext : Read char_t over eos.";
      return '\0';
    }
    char_t ch = input.readChar();
    col = input.position()+1;
    
    if(ch == '\n') 
    {
      line++;
      col = 1;
    }
    
    return ch;
  }

  void FastCharStream::UnGet(){
    if ( input.position() == 0 )
      _THROWC("error : FileReader.UnGet : ungetted first char_t");

    input.seek(input.position()-1);
  }
  
  //Returns the current top char_t from the input stream without removing it.
  char_t FastCharStream::Peek(){
    try{
      return input.peek();
    }catch(...){}
    return 0;
  }
  
  
  //Returns <b>True</b> if the end of stream was reached.
  bool FastCharStream::Eos()  {
    return input.available()==0;
  }

  // Gets the current column.
  int FastCharStream::Column() {
    return col;
  }

  // Gets the current line.
  int FastCharStream::Line() {
    return line;
  }

}}
