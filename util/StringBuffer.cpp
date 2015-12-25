#include "StdHeader.h"
#include "StringBuffer.h"

#include <cstdlib>

#ifdef HAVE_GCVT
# define _gcvt gcvt
#endif

namespace NSLib{ namespace util {

  void StringBuffer::growBuffer(int minLength){
    bufferLength += NSLIB_STREAM_BUFFER_SIZE;
    if ( bufferLength<minLength )
      bufferLength = minLength;
          
    char_t* tmp = new char_t[bufferLength];
    stringNCopy(tmp,buffer,len+1);
    delete[] buffer;
    buffer = tmp;
  }

  StringBuffer::StringBuffer(const char_t* value):
    len(stringLength(value))
  {
    bufferLength = len + 1;
    //minimum half buffer size
    if ( bufferLength < (NSLIB_STREAM_BUFFER_SIZE/2) )
      bufferLength = NSLIB_STREAM_BUFFER_SIZE/2;

    buffer = new char_t[bufferLength];
    stringNCopy(buffer,value,len+1);
  }

  StringBuffer::StringBuffer(int initSize):
    buffer (new char_t[initSize]),
    bufferLength (initSize+1),
    len(0)
  {
  }

  StringBuffer::StringBuffer():
    //buffer (new char_t[NSLIB_STREAM_BUFFER_SIZE]),
    bufferLength (NSLIB_STREAM_BUFFER_SIZE),
    len(0)
  {
    buffer = new char_t[NSLIB_STREAM_BUFFER_SIZE];
  }

  StringBuffer::~StringBuffer(){
    delete[] buffer;
  }

  void StringBuffer::append(const char_t character){
    if ( len + 1 > bufferLength )
      growBuffer(len + 1);
    buffer[len] = character;
    len++;
  }

  void StringBuffer::append( const char_t* value ){
    int sl = stringLength(value);
    if ( len+sl+1 > bufferLength )
      growBuffer ( len+sl+1 );

    for ( int i=0;i<sl;i++ )
      buffer[len+i] = value[i];
    
    len += sl;
  }

  void StringBuffer::append(const int value ){
    char_t buf[30];
    integerToString(value,buf,10);
    append(buf);
  }

  void StringBuffer::prepend( const char_t* value ){
    int sl = stringLength(value);
    if ( len+sl+1 > bufferLength )
      growBuffer ( len+sl+1 );
    
    //move buffer up
    for ( int i=len+1;i>=0;i-- )
      buffer[i+sl] = buffer[i];
    len += sl;

    for ( int i=0;i<sl;i++ )
      buffer[i] = value[i];
  }

  void StringBuffer::append ( const double value, const int digits) {
    //todo2: what's a wide char double to string equiv
    char buf[30];
#ifdef _WIN32
    _gcvt(value,digits,buf);
#else
    gcvt(value,digits,buf);
#endif 
    
#ifdef _UNICODE
    //convert char to wide char
    char_t* wbuf = Misc::charToWide( const_cast<char*>(buf));
    append(wbuf);
    delete wbuf;

    if ( buf[strlen(buf)-1] == '.' )
      append(_T("0"));
#else
    append( buf );
    if ( buf[stringLength(buf)-1] == '.' )
      append("0");
#endif
  }

  int StringBuffer::length(){
    return len;
  }

  char_t* StringBuffer::ToString(){
    char_t* ret = new char_t[len+1];
    stringNCopy(ret,buffer,len);
    ret[len]=0;
    return ret;
  }

  char_t* StringBuffer::getBuffer(){
    if ( len==bufferLength )
      growBuffer( len+1 ); //space for NULL terminator
    buffer[len] = 0;
    return buffer;
  }

  void StringBuffer::clear(){
    len = 0;
    delete[] buffer;
    bufferLength = NSLIB_STREAM_BUFFER_SIZE;
    buffer = new char_t[bufferLength];
  }

}}

