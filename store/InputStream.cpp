#include "StdHeader.h"

#include "util/Arrays.h"

#include <iostream>

using namespace std;
using namespace NSLib::util;
namespace NSLib{ namespace store{

InputStream::InputStream():
  length(0),
  bufferStart(0),
  bufferLength(0),
  bufferPosition(0),
  //buffer(NULL),
  chars(NULL),
  chars_length(0)
{
}

InputStream::InputStream(InputStream& clone):
  length(clone.length),
  bufferStart(clone.bufferStart),
  bufferLength(clone.bufferLength),
  bufferPosition(clone.bufferPosition),
  chars(NULL),
  chars_length(0),
  filePath(clone.filePath)
{
  if ( clone.buffer != NULL && clone.bufferLength>0) {
    cerr << "Clone BufferLength=" << bufferLength << endl;
    //buffer = new l_byte_t[bufferLength];
    memcpy(buffer, clone.buffer, sizeof(l_byte_t)*bufferLength);
  }
}

l_byte_t InputStream::readByte() {
  if (bufferPosition >= bufferLength)
    refill();

  return buffer[bufferPosition++];
}

void InputStream::readBytes(l_byte_t* b, const int offset, const int len){
  if (len < NSLIB_STREAM_BUFFER_SIZE) {
    for (int i = 0; i < len; i++)      // read byte-by-byte
      b[i + offset] = (l_byte_t)readByte();
  } else {            // read all-at-once
    long_t start = getFilePointer();
    readInternal(b, offset, len);

    bufferStart = start + len;      // adjust stream variables
    bufferPosition = 0;
    bufferLength = 0;          // trigger refill() on read
  }
}

int InputStream::readInt() {
  return ((readByte() & 0xFF) << 24) | ((readByte() & 0xFF) << 16)
    | ((readByte() & 0xFF) <<  8) |  (readByte() & 0xFF);
}

int InputStream::readVInt() {
  l_byte_t b = readByte();
  int i = b & 0x7F;
  for (int shift = 7; (b & 0x80) != 0; shift += 7) {
    b = readByte();
    i |= (b & 0x7F) << shift;
  }
  return i;
}

long_t InputStream::readLong() {
  return (((long_t)readInt()) << 32) | (readInt() & 0xFFFFFFFFL);
}

long_t InputStream::readVLong() {
  l_byte_t b = readByte();
  long_t i = b & 0x7F;
  for (int shift = 7; (b & 0x80) != 0; shift += 7) {
    b = readByte();
    i |= (b & 0x7FL) << shift;
  }
  return i;
}

char* InputStream::readByteString()
{
  int len = readVInt();
  if ( len == 0 )
    return "";
  char* str = new char[len+1];
  for (int i=0; i<len; i++)
     str[i] = readByte();
  str[len] = 0;

  return str;
}

char_t* InputStream::readString(const bool unique){
  int len = readVInt();
  if (chars == NULL || len > chars_length){
    if ( len == 0 ){
      if ( unique )
        return stringDuplicate(_T(""));
      else
        return _T("");
    }

    if ( chars != NULL )
      delete[] chars;
    chars = new char_t[len+1];
    chars_length = len;
  }
  readChars(chars, 0, len);
  chars[len] = 0;
  //cerr << " InputStream::readString: " << ws2str(chars) << endl;

  if ( unique )
    return stringDuplicate(chars);
  else
    return chars;
}

void InputStream::readChars( char_t* buffer, const int start, const int len) {
  const int end = start + len;
  for (int i = start; i < end; i++) {
    l_byte_t b = readByte();
    if ((b & 0x80) == 0)
      buffer[i] = (char_t)(b & 0x7F);
    else if ((b & 0xE0) != 0xE0) {
      buffer[i] = (char_t)(((b & 0x1F) << 6)
        | (readByte() & 0x3F));
    } else 
      buffer[i] = (char_t)(((b & 0x0F) << 12)
        | ((readByte() & 0x3F) << 6)
          |  (readByte() & 0x3F));
    }
}

long_t InputStream::getFilePointer() {
  return bufferStart + bufferPosition;
}

void InputStream::seek(const long_t pos) {
  if ( pos < 0 )
    _THROWC( "IO Argument Error. Value must be a positive value.");
  if (pos >= bufferStart && pos < (bufferStart + bufferLength))
    bufferPosition = (int)(pos - bufferStart);  // seek within buffer
  else {
    bufferStart = pos;
    bufferPosition = 0;
    bufferLength = 0;          // trigger refill() on read()
    seekInternal(0);
  }
}
long_t InputStream::Length() {
  return length;
}

void InputStream::close(){
  if( chars != NULL ) 
    delete[] chars ; chars = NULL;
  
  bufferLength = 0;
  bufferPosition = 0;
  bufferStart = 0;
}

InputStream::~InputStream(){
  close();
}

void InputStream::refill() {
  long_t start = bufferStart + bufferPosition;
  long_t end = start + NSLIB_STREAM_BUFFER_SIZE;
  if (end > length)          // don't read past EOF
    end = length;
  bufferLength = (int)(end - start);
  if (bufferLength == 0)
    _THROWC( "InputStream read past EOF");
  //if ( buffer == NULL) 
  //  buffer = new l_byte_t[bufferLength];
  readInternal(buffer, 0, bufferLength);

  bufferStart = start;
  bufferPosition = 0;
}

}}

