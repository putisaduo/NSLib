#include "StdHeader.h"
#include "Reader.h"

using namespace NSLib::store;

namespace NSLib{namespace util{

  FileReader::FileReader(const fchar_t* fname):
    stream( new FSInputStream(fname) )
  {
  }
  FileReader::~FileReader(){
    close();
  }
  void FileReader::close(){
    if ( stream != NULL){
      stream->close();
      delete stream;
      stream = NULL;
    }
  }
  int FileReader::read(char_t* buf, const int start, const int length){
    int av = available();
    if ( av > 0 ){
      for ( int i=0;i<length;i++ )
        buf[i+start] = stream->readByte();
      //FSInputStream::readBytes((char_t*)buf,start,length);
      
      if ( av < length )
        return av;
      else
        return length;
    }else
      return 0;
  }
  int FileReader::read(char_t* buf){
    return read(buf,0,stringLength(buf));
  }
  int FileReader::available(){
    return stream->Length() - stream->getFilePointer();
  }
  char_t FileReader::readChar(){
    return (char_t)stream->readByte();
  }
  char_t FileReader::peek(){
    if ( available() == 0 )
      return 0;
    char_t ret = readChar();
    stream->seek(stream->getFilePointer()-1);
    return ret;
  }
  int FileReader::position(){
    return stream->getFilePointer();
  }
  void FileReader::seek(int position){
    stream->seek(position);
  }




  StringReader::StringReader ( const char_t* value ):
    data(value),
    delVal(false)
  {
    pt = 0;
    len = stringLength(value);
    
  }
  StringReader::StringReader ( const char_t* value, const int length, const bool deletevalue ):
    data(value),
    len(length),
    delVal(deletevalue)
  {
    pt = 0;
  }
  StringReader::~StringReader(){
    close();
  }

  int StringReader::available(){
    return len-pt;
  }
  int StringReader::read ( char_t* buf ){
    return read(buf,0,stringLength(buf));
  }
  int StringReader::read ( char_t* buf, const int start, const int length ){
    if ( pt >= len )
      return -1;
    int rd = 0;
    while ( pt < len && rd < length ){
      buf[start+rd] = data[pt];
      rd ++;
      pt ++;
    }
    return rd;
  }
  char_t StringReader::readChar(){
    if ( pt>=len )
      throw "String reader EOF";

    char_t ret = data[pt];
    pt++;
    return ret;
  }
  char_t StringReader::peek(){
    if ( pt>=len ) //todo: check this, used to be pt+1>...
      throw "String reader EOF";
    return data[pt]; //todo: check this, used to be data[pt+1]
  }
  void StringReader::close(){
    if ( delVal )
      delete[] data;
  }
  int StringReader::position(){
    return pt;
  }
  void StringReader::seek(int position){
    pt=position;
  }

}}

