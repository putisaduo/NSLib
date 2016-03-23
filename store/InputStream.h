#ifndef _NSLib_store_InputStream_
#define _NSLib_store_InputStream_
#include "StdHeader.h"
#include <string>

namespace NSLib{ namespace store{

class InputStream{
private:
  l_byte_t buffer[NSLIB_STREAM_BUFFER_SIZE]; //array of bytes
  //l_byte_t* buffer; //array of bytes
  char_t* chars; //array of chars
  int chars_length;

protected:
  long_t length;          // set by subclasses
  long_t bufferStart;     // position in file of buffer
  int bufferLength;       // end of valid l_byte_ts
  int bufferPosition;     // next l_byte_t to read
  std::string filePath;

  InputStream(InputStream& clone);

public:
  InputStream();
  virtual ~InputStream();

  virtual InputStream& clone()=0;

  l_byte_t readByte();
  void readBytes(l_byte_t* b, const int offset, const int len);
  int readInt();
  int readVInt();
  long_t readLong();
  long_t readVLong();
  char* readByteString();
  char_t* readString(const bool unique = true);
  void readChars( char_t* buffer, const int start, const int len);

  virtual void close();
  
  // RandomAccessFile-like methods
  long_t getFilePointer();

  void seek(const long_t pos);
  long_t Length();

protected:
  void refill();

  virtual void readInternal(l_byte_t* b, const int offset, const int len) = 0;
  virtual void seekInternal(const long_t pos) = 0;
};

}}

#endif
