#ifndef _NSLib_util_Misc_
#define _NSLib_util_Misc_
#include "StdHeader.h"

#include <sys/stat.h>

namespace NSLib{ namespace util{
  static int DELETE_TYPE_NONE = 0;
  static int DELETE_TYPE_DELETE = 1;
  static int DELETE_TYPE_DELETE_ARRAY = 2;


  class Misc{
  public:
    static long_t currentTimeMillis();
    //static const char_t* replace_all( const char_t* val, const char_t* srch, const char_t* repl);
    static bool dir_Exists(const fchar_t* path);
    static char_t* join(const char_t* a, const char_t* b, const char_t* c=NULL,
                        const char_t* d=NULL, const char_t* e=NULL, const char_t* f=NULL, const int maxLen=256 );
    static fchar_t* fjoin(const fchar_t* a, const fchar_t* b, const fchar_t* c=NULL,
                        const fchar_t* d=NULL, const fchar_t* e=NULL, const fchar_t* f=NULL, const int maxLen=256);
    static bool priv_isDotDir( const char_t* name );
    static bool isLetter(int c); //replacement for isAlpha. Does the same as java.
    
    static char* wideToChar(const wchar16_t* s);
    static wchar16_t* charToWide(const char* s);
    
    static void charToWide(const char* s, wchar16_t* d, size_t len);
    static void wideToChar(const wchar16_t* s, char* d, size_t len);
  };
  
  template <class parent>class IGCollectable{
  protected:
    bool _canDelete;
  public:
    int __refcount;
    IGCollectable(){
      _canDelete=true;
      __refcount = 1;
    }
    virtual ~IGCollectable(){
#ifdef _DEBUG
      if ( __refcount != 0 ){
        _THROWC(  "ERROR: Object collected with non-zero refcount" );
        return;
      }
#endif
    }
    void __addref(){
      __refcount ++;
    }
    //if canDelete is false then don't delete the object - might be a static object
    void finalize(){
      if ( this == NULL )
        return;
      __refcount --;
  #ifdef _DEBUG
      if ( __refcount < 0 ){
        _THROWC( "ERROR: Refcount Sync");
        return;
      }
  #endif
      if ( __refcount == 0 && _canDelete ){
        delete this;
      }
    }
    parent* pointer(){
      __addref();
      return (parent*)this;
    }
  };

}}
#endif
