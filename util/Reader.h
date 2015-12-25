#include "StdHeader.h"
#ifndef _NSLib_util_Reader_
#define _NSLib_util_Reader_

#include "store/FSDirectory.h"

namespace NSLib{namespace util{
  class BasicReader{
  public:
    virtual void close()=0;
    virtual int read(char_t* b, const int start, const int length)=0;
    virtual int read ( char_t buf[] ) = 0;
    virtual int available () = 0;
    virtual char_t readChar() = 0;
    virtual char_t peek() = 0;
    virtual int position() = 0;
    virtual void seek(int position) = 0;
    virtual ~BasicReader(){
    }
  };

  class StringReader:public BasicReader{
  private:
    const char_t* data;
    int pt;
    int len;
    bool delVal;
  public:
    StringReader(const char_t* value);
    StringReader(const char_t* value, const int length, const bool deletevalue=false);
    ~StringReader();
    
    int read(char_t* buf );
    int read(char_t* buf, const int start, const int length );
    int available();
    void close();
    char_t readChar();
    char_t peek();
    int position();
    void seek(int position);
  };

  class FileReader:public BasicReader{
  private:
    NSLib::store::FSInputStream* stream;
  public:
    
    FileReader ( const fchar_t* fname );
    ~FileReader();
    
    int read ( char_t* buf );
    int read ( char_t* buf, const int start, const int length );
    int available ();
    char_t readChar();
    char_t peek();
    void close();
    int position();
    void seek(int position);
  };
}}
#endif
