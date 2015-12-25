#include "StdHeader.h"
#include "Misc.h"

#include <sys/types.h>
#include <time.h>

#include <sys/timeb.h>

#include <stddef.h> //file constants
#include <sys/stat.h>
#include <sys/timeb.h>
#include "StringBuffer.h"
#include "VoidList.h"

using namespace std;
namespace NSLib{ namespace util{
    
//keep wide conversions
  //static 
  char* Misc::wideToChar(const wchar16_t* s){
    int len = strlen16(s);
    char* msg=new char[len+1];
    wideToChar( s,msg,len+1 );
    return msg;
  }

  wchar16_t* Misc::charToWide(const char* s){
    int len = strlen(s);
    wchar16_t* msg = new wchar16_t[len+1];
    charToWide(s,msg,len+1);
    return msg;
  }
  
  void Misc::wideToChar(const wchar16_t* s, char* d, size_t len){
    for ( int i=0; i<(int)len && i<strlen16(s)+1;i++ )
      d[i] = ((unsigned int)s[i]>0x80?'?':(char)s[i]);
  }
  void Misc::charToWide(const char* s, wchar16_t* d, size_t len){
    for (int i=0; i<(int)len && i<(int)strlen(s)+1; i++ )
      d[i] = (char_t)s[i];
  }

  //static
  bool Misc::isLetter(int c){
    int ranges[8][2] = {
      {65,90},
      {97,122},
      {170,0},
      {181,0},
      {186,0},
      {192,214},
      {216,246},
      {248,255}
    };

    for ( int i=0;i<8;i++ ){
      if ( ranges[i][1]== 0 && ranges[i][0] == c )
        return true;
      else if ( c >= ranges[i][0] && c <= ranges[i][1] )
        return true;
    }
    return false;
  }


  //static 
  long_t Misc::currentTimeMillis(){
    struct Struct_timeb tstruct;
    getTime( &tstruct );

    ulong_t tm = tstruct.time;
    return (tm * 1000) + tstruct.millitm;
  }

/*
  //static 
  const char_t* Misc::replace_all( const char_t* val, const char_t* srch, const char_t* repl )
  {
    int cnt = 0;
    int repLen = stringLength(repl);
    int srchLen = stringLength(srch);
    int srcLen = stringLength(val);

    const char_t* pos = val;
    while( (pos = stringFind(pos+1, srch)) != NULL ) {
      ++cnt;
    }

    int lenNew = (srcLen - (srchLen * cnt)) + (repLen * cnt);
    char_t* ret = new char_t[lenNew+1];
    ret[lenNew] = 0;
    if ( cnt == 0 ){
      stringCopy(ret,val);
      return ret;
    }

    char_t* cur = ret; //position of return buffer
    const char_t* lst = val; //position of value buffer
    pos = val; //searched position of value buffer
    while( (pos = stringFind(pos+1,srch)) != NULL ) {
      stringNCopy(cur,lst,pos-lst); //copy till current
      cur += (pos-lst);
      lst = pos; //move val position

      stringCopy( cur,repl); //copy replace
      cur += repLen; //move return buffer position
      lst += srchLen; //move last value buffer position
    }
    stringCopy(cur, lst ); //copy rest of buffer

    return ret;
  }
  */

  //static 
  bool Misc::dir_Exists(const fchar_t* path){
    struct Struct_Stat buf;
    int ret = Cmd_Stat(path,&buf);
    return ( ret == 0);
  }

    //static
  char_t* Misc::join ( const char_t* a, const char_t* b, const char_t* c,
        const char_t* d,const char_t* e,const char_t* f, const int maxLen)
  {
    char_t *buf = new char_t[maxLen];
    stringPrintF(buf, _T("%s%s%s%s%s%s"),
                 a==NULL?_T(""):a, b==NULL?_T(""):b,
                 c==NULL?_T(""):c, d==NULL?_T(""):d,
                 e==NULL?_T(""):e, f==NULL?_T(""):f );
    return buf;
  }
    //static
  fchar_t* Misc::fjoin ( const fchar_t* a, const fchar_t* b, const fchar_t* c,
       const fchar_t* d,const fchar_t* e,const fchar_t* f, const int maxLen)
  {
    fchar_t *buf = new fchar_t[maxLen];
    fstringPrintF(buf, CONST_STRING("%s%s%s%s%s%s"), 
                  a==NULL?CONST_STRING(""):a, b==NULL?CONST_STRING(""):b, 
                  c==NULL?CONST_STRING(""):c, d==NULL?CONST_STRING(""):d, 
                  e==NULL?CONST_STRING(""):e, f==NULL?CONST_STRING(""):f);
    return buf;
  }
  
  //static
  bool Misc::priv_isDotDir( const char_t* name )
  {
    if( name[0] == '\0' ) {
      return (false);
    }
    if( name[0] == '.' && name[1] == '\0' ) {
      return (true);
    }
    if( name[1] == '\0' ) {
      return (false);
    }
    if( name[0] == '.' && name[1] == '.' && name[2] == '\0' ) {
      return (true);
    }

    return (false);
  }

}}
