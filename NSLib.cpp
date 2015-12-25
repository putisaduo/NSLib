#include <locale.h>
#include <string>
#include <iostream>

#include "NSLib.h"

#include "index/IndexReader.h"
#include "index/IndexWriter.h"
#include "search/IndexSearcher.h"
#include "search/DateFilter.h"
#include "document/Document.h"
#include "document/Field.h"
#include "document/DateField.h"
#include "store/Directory.h"
#include "store/FSDirectory.h"
#include "queryParser/QueryParser.h"
#include "analysis/StandardAnalyzer.h"

#include "util/enc.h"
#include "queryParser/Lexer.h"
#include "queryParser/TokenList.h"
#include "util/Reader.h"
#include "analysis/ChineseAnalyzer.h"
#include "analysis/ChineseTokenizer.h"
#include "util/CharConverter.h"
#include "util/HTMLHighLighter.h"

using namespace std;
using namespace NSLib::index;
using namespace NSLib::store;
using namespace NSLib::document;
using namespace NSLib::search;
using namespace NSLib::queryParser;
using namespace NSLib::analysis;
using namespace NSLib::util;

char errstr[errlen];
void resetError(){
  //errstr[0]=0;
}

#ifdef  __cplusplus
extern "C" {
#endif
NSLIBDLL_API void NSL_Cleanup(){
  //NSL_ClearSearch();

  //delete doc;
  //doc = NULL;
}

NSLIBDLL_API void NSL_Init(){
  //setlocale
  //errstr[0]=0;
}

void reportError(const char* error){
  //unsigned int i;
  //if ( error == NULL )
  //  return;

  //for ( i=0;i<errlen-1&&i<strlen(error);i++ )
  //  errstr[i]=error[i];
  //errstr[i+1]=0;
}

NSLIBDLL_API int NSL_WOpen(const fchar_t* wdir, int create){
  if ( wdir == NULL ) return -1;

  resetError();
  int ret = -1;
  try{
    Directory* d = NULL;

    if ( IndexReader::indexExists(wdir) ){
      d = &FSDirectory::getDirectory(wdir, false );
      delete d;
    } else if ( create ) {
      ChineseAnalyzer a;
      IndexWriter w(wdir,a,true);
      w.close();  
    }

    ret = 0;
  }catch(exception e){
    reportError( e.what());
  }catch(...){
    reportError( "Unknown error");
  }

  return ret;
}

NSLIBDLL_API int NSL_WClose(const fchar_t* wdir){
  if ( wdir == NULL ) return -1;

  resetError();
  int ret = -1;
  try{
    Directory* d = &FSDirectory::getDirectory(wdir,false);
    d->close();
    ret = 0;
  }catch(exception e){
    reportError( e.what());
  }catch(...){
    reportError( "Unknown error");
  }
  return ret;
}

NSLIBDLL_API int NSL_WOptimize(const fchar_t* wdir){
  if ( wdir == NULL ) return -1;

  resetError();
  int ret = -1;
  try{
    ChineseAnalyzer a;
    IndexWriter w(wdir,a,false);
    w.optimize();
    w.close();
    ret = 0;
  }catch(exception e){
    reportError( e.what());
  }catch(...){
    reportError( "Unknown error");
  }

  return ret;
}


//adds a field to the current document
NSLIBDLL_API int NSL_WAdd_Field(void* doc,
                const char_t* wfield, 
                const char_t* wvalue, 
                const int store, 
                const int index, 
                const int token){
  if ( doc == NULL || wfield == NULL || wvalue == NULL ) return -1;

  resetError();
  int ret = -1;

  StringReader& reader = *new StringReader(stringDuplicate(wvalue), stringLength(wvalue), true);

  try{
    NSLib::document::Field* fld = new NSLib::document::Field(wfield,&reader,store!=0,index!=0,token!=0);
    ((Document*)doc)->add(*fld);
    ret = 0;
  }catch(exception e){
    reportError( e.what());
  }catch(...){
    reportError( "Unknown error");
  }

  return ret;
}


//adds a date field to current document
NSLIBDLL_API int NSL_WAdd_Date(void* doc,
              const char_t* wfield, 
              type_long time, 
              int store, 
              int index, 
              int token){
  if ( doc == NULL || wfield == NULL ) return -1;

  resetError();
  int ret = -1;

  char_t* wvalue = NSLib::document::DateField::timeToString(time);
  try{
    NSLib::document::Field* fld = new NSLib::document::Field(wfield,wvalue,store!=0,index!=0,token!=0);
    ((Document*)doc)->add(*fld);
    ret = 0;
  }catch(exception e){
    reportError( e.what());
  }catch(...){
    reportError( "Unknown error");
  }
  delete[] wvalue;
  return ret;
}

//adds the current document
NSLIBDLL_API int NSL_WInsert_Document(const fchar_t* wdir, void* doc){
  if ( wdir == NULL || doc == NULL ) return -1;

  resetError();
  int ret = -1;

  IndexWriter* w = NULL;
  try{
    ChineseAnalyzer a;
    w = new IndexWriter(wdir,a,false);
    w->addDocument(*((Document*)doc));
    ret = 0;
  }catch(exception e){
    reportError( e.what());
  }catch(...){
    reportError( "Unknown error");
  }
  if ( w != NULL )
    w->close();
  delete w;

  return ret;
}

NSLIBDLL_API char* NSL_wideToChar(const char16_t* wstr)
{
  string str = ws2str(wstr);
  char* ret = new char[str.size()+2];
  strcpy(ret, str.c_str());
  return ret;
  //return NSLib::util::CharConverter::wideToChar(wstr, "8859-1");
}

NSLIBDLL_API void* NSL_WSearch(const fchar_t* wdir, const char_t* wquery, 
                               const char_t* wfield, const char_t* wgroupby)
{
  if ( wdir == NULL || wquery == NULL ) return NULL;

  Query* q = NULL;
  ChineseAnalyzer analyzer;

  try{
    std::cerr << endl << " $$$$$$$$$ WSearch: " << wdir << endl;
    //std::cerr << ws2str(wquery) << endl;
    QueryParser qp(wfield,analyzer);
    q = &qp.Parse(wquery);
  }catch(exception e){
    cerr << endl << "################ parsing error." << e.what() << endl;
    reportError( e.what());
  }catch(...){
    reportError( "Unknown error");
  }

  Hits* hits = NULL;
  if ( q != NULL ){
    try{
      std::cerr << endl << " $$$$$$$$$ WSearch starts ... " << endl;
      //NSL_ClearSearch();
      IndexSearcher* searcher = new IndexSearcher(wdir);
      std::cerr << endl << " $$$$$ WSearchng  ... " << endl;
      hits = &searcher->search(*q, const_cast<char_t*>(wgroupby));
      std::cerr << endl << " $$$$$$$$$ WSearch done. " << endl;

      //ret = 1;
    }catch(exception e){
      reportError( e.what());
    }catch(...){
      reportError( "Unknown error");
    }
  }

  //delete q;
  return hits;
}

NSLIBDLL_API type_long NSL_WGet_DateField(const void* doc, const char_t* wfield){
  if ( doc == NULL || wfield == NULL ) return -1;

  type_long ret = 0;
  
  try{
    if ( doc != NULL ){
      const char_t* val = ((Document*)doc)->get(wfield);
      if ( val != NULL ){
        try{
          ret = DateField::stringToTime(val);
        }catch(...){
          reportError("Field is not a date");
        }
      }else{
        reportError("Field does not exist");
      }
    }
  }catch(exception e){
    reportError( e.what());
  }catch(...){
    reportError( "Unknown error");
  }

  return ret;
}

NSLIBDLL_API void NSL_WFree_Keywords(char_t* wkeywords) {
  if ( wkeywords == NULL ) return;
  delete[] wkeywords;
}

NSLIBDLL_API char_t* NSL_WGet_Keywords(const char_t* wquery) {
  if ( wquery == NULL ) return NULL;

  Lexer lexer(wquery);
  TokenList* tokens = NULL;

  try {
    tokens = &lexer.Lex();
  }catch (...) {
    //parse error, can't continue
    return NULL;
  }
#ifdef _UNICODE
  NSLibString wprev = _T("");
  NSLibString wkeywords = _T("");
#else
  string wprev = "";
  string wkeywords = "";
#endif
  while ( tokens->Peek().Type != NSLib::queryParser::EOF_ ) {
    QueryToken token = tokens->Extract();
    if ( token.Type == NSLib::queryParser::NOT || 
      token.Type == NSLib::queryParser::MINUS ) {
      tokens->Extract();
      continue;
    }

    if ( token.Type == NSLib::queryParser::COLON ) {
#ifdef _UNICODE
      wprev = _T("");
#else
      wprev = "";
#endif
      continue;
    }else if ( wprev.size() != 0 ) {
      wkeywords += wprev;
#ifdef _UNICODE
      wkeywords += _T(" ");
#else
      wkeywords += " ";
#endif
    }

    if (token.Type != NSLib::queryParser::TERM &&
      token.Type != NSLib::queryParser::NUMBER &&
      token.Type != NSLib::queryParser::WILDTERM &&
      token.Type != NSLib::queryParser::PREFIXTERM) continue;
    
    wprev = token.Value;
  }

  if ( wprev.size() != 0 ) {
    //wkeywords += wprev;
    //wkeywords += L" ";
    StringReader* reader = new StringReader(wprev.c_str());
    HytungReader* lreader = new HytungReader(reader, false);
    WordSegmenter seg(lreader, false);
    char_t buf[255];
    while (seg.next(buf, sizeof(buf))) {
      wkeywords += buf; 
#ifdef _UNICODE
      wkeywords += _T(" ");
#else
      wkeywords += " ";
#endif
    }
    delete lreader;
    delete reader;

  }

  if ( wkeywords.size() == 0 ) return NULL;
  return stringDuplicate(wkeywords.c_str());
}

NSLIBDLL_API char_t* NSL_WGet_Field_HTMLHiLite(const void* doc, 
                          const char_t* wfield, 
                          const char_t* wkeywords,
                          int detail){
  
    if ( doc == NULL || wfield == NULL || wkeywords == NULL ) 
    return NULL;

  char_t* wvalue = NULL;
  try{
    //Document& doc = hits->doc(hitPos);
    if ( doc != NULL ){
      if ( ((Document*)doc)->get(wfield) == NULL ){
        reportError("Field does not exist");
      }else{
        wvalue = HTMLHighLighter::highlight(((Document*)doc)->get(wfield), wkeywords, detail);
      }
    }
  }catch(exception e){
    reportError( e.what());
  }catch(...){
    reportError( "Unknown error");
  }

  return wvalue;
}

NSLIBDLL_API char_t* NSL_WGet_Field(const void* doc, const char_t* wfield){
  if ( doc == NULL || wfield == NULL ) return NULL;
  const char_t* wvalue = NULL;
  try{
    //Document& doc = hits->doc(hitPos);
    if ( doc != NULL ){
      wvalue = ((Document*)doc)->get(wfield);

      if ( wvalue == NULL ){
        reportError("Field does not exist");
      }
    }
  }catch(exception e){
    reportError( e.what());
  }catch(...){
    reportError( "Unknown error");
  }

  if ( wvalue ) return stringDuplicate(wvalue);
  return NULL;
}

NSLIBDLL_API void NSL_WFree_Field(char_t* wvalue) {
  delete[] wvalue;
}

//deletes documents returned from specified query.
//returns number of documents deleted.
//returns -1 if an error occurs
NSLIBDLL_API int NSL_WDelete(const fchar_t* wdir, 
               const char_t* wquery, 
               const char_t* wfield){
    if ( wdir == NULL || wquery == NULL || wfield == NULL ) return -1;

  int ret = -1;

  Query* q = NULL;
  IndexReader *r = NULL;
  IndexSearcher *s = NULL;
  Hits* h = NULL;

  try{
    ChineseAnalyzer analyzer;
    q = &QueryParser::Parse(wquery,wfield,analyzer);

    r = &IndexReader::open(wdir);
    s = new IndexSearcher(*r);

    h = &s->search(*q, const_cast<char_t*>(_T("")));
    ret = 0;
    for ( int i=0;i<h->Length();i++ ){
      r->Delete(h->id(i));
      ret++;
    }
  }catch(exception e){
    reportError( e.what());
    ret = -1;
  }catch(...){
    reportError( "Unknown error");
    ret = -1;
  }
  if ( r != NULL )
    r->close();
  if ( s != NULL )
    s->close();

  delete r;
  //delete s;
  delete h;
  //delete q;

  return ret;
}

NSLIBDLL_API int NSL_Open(const char* dir, int create){
  if ( dir == NULL ) return -1;
#ifdef _F_UNICODE
  char_t* wdir = CharConverter::charToWide(dir, LANG_ENG);
  int ret = NSL_WOpen(wdir, create);
  delete[] wdir;
#else
  int ret = NSL_WOpen(dir, create);
#endif
  return ret;
}

NSLIBDLL_API int NSL_Close(const char* dir){
  if ( dir == NULL ) return -1;
#ifdef _F_UNICODE
  char_t* wdir = CharConverter::charToWide(dir, LANG_ENG);
  int ret = NSL_WClose(wdir);
  delete[] wdir;
#else
  int ret = NSL_WClose(dir);
#endif
  return ret;
}

NSLIBDLL_API int NSL_Optimize(const char* dir){
  if ( dir == NULL ) return -1;
#ifdef _F_UNICODE
  char_t* wdir = CharConverter::charToWide(dir, LANG_ENG);
  int ret = NSL_WOptimize(wdir);
  delete[] wdir;
#else
  int ret = NSL_WOptimize(dir);
#endif
  return ret;
}

//NSLIBDLL_API void NSL_Document_Info(char* info, const int len){
//  if ( doc != NULL ){
//    const char_t* inf = doc->toString();
//    CharConverter::wideToChar(inf, wcslen(inf), info, len, "8859-1");
//    delete inf;
//  }else{
//    info[0]=0;
//    reportError("No document available");
//  }
//}

//resets the current document
NSLIBDLL_API void* NSL_New_Document(){
  return new Document();
}

NSLIBDLL_API void NSL_Free_Document(void* doc){
  if ( doc == NULL ) return;
  delete ((Document*)doc);
}

//adds a field to the current document
NSLIBDLL_API int NSL_Add_Field(void* doc,
               const char* field, 
               const char* value, 
               const char* venc,
               const int store, 
               const int index, 
               const int token){
  if ( doc == NULL || field == NULL || value == NULL || venc == NULL ) return -1;

#ifdef _UNICODE
  char_t* wfield = CharConverter::charToWide(field, LANG_ENG);
  char_t* wvalue = CharConverter::charToWide(value, venc);
  int ret = NSL_WAdd_Field(doc, wfield, wvalue, store, index, token);
  delete[] wfield;
  delete[] wvalue;
#else
  int ret = NSL_WAdd_Field(doc, field, value, store, index, token);
#endif
  return ret;
}
//adds a date field to current document
NSLIBDLL_API int NSL_Add_Date(void* doc,
              const char* field, 
              type_long time, 
              int store, 
              int index, 
              int token){
  if ( doc == NULL || field == NULL ) return -1;
#ifdef _UNICODE
  char_t* wfield =  CharConverter::charToWide(field, LANG_ENG);
  int ret = NSL_WAdd_Date(doc, wfield, time, store, index, token);
  delete[] wfield;
#else
  int ret = NSL_WAdd_Date(doc, field, time,  store, index, token);
#endif
  return ret;
}

//adds the current document
NSLIBDLL_API int NSL_Insert_Document(const char* dir, void* doc){
  if ( dir == NULL || doc == NULL ) return -1;
#ifdef _F_UNICODE
  char_t* wdir = CharConverter::charToWide(dir, LANG_ENG);
  int ret = NSL_WInsert_Document(wdir, doc);
  delete[] wdir;
#else
  int ret = NSL_WInsert_Document(dir, doc);
#endif
  return ret;
}

NSLIBDLL_API void* NSL_Search(const char* dir, const char* query, 
                const char* qenc, const char* field, const char* groupby)
{
  if ( dir == NULL || query == NULL || qenc == NULL ) return NULL;
#ifdef _F_UNICODE
  fchar_t* wdir = CharConverter::charToWide(dir, LANG_ENG);
#else
  size_t len = strlen(dir);
  fchar_t* wdir = new fchar_t[len+1];
  fstringCopy(wdir, dir);
#endif
  char_t* wquery = CharConverter::charToWide(query, qenc);
  char_t* wfield = CharConverter::charToWide(field, LANG_ENG);
  char_t* wgroupby = CharConverter::charToWide(groupby, LANG_ENG);
  void* hits = NSL_WSearch(wdir, wquery, wfield, wgroupby);
  delete[] wgroupby;
  delete[] wquery;
  delete[] wfield;

#ifdef _F_UNICODE
  delete[] wdir; 
#endif
  return hits;
}

NSLIBDLL_API void* NSL_SearchWQ(const char* dir, const char_t* wquery, 
                const char* field, const char* groupby)
{
  if ( dir == NULL || wquery == NULL) return NULL;
#ifdef _F_UNICODE
  fchar_t* wdir = CharConverter::charToWide(dir, LANG_ENG);
#else
  size_t len = strlen(dir);
  fchar_t* wdir = new fchar_t[len+1];
  fstringCopy(wdir, dir);
#endif
  char_t* wfield = CharConverter::charToWide(field, LANG_ENG);
  char_t* wgroupby = CharConverter::charToWide(groupby, LANG_ENG);
  void* hits = NSL_WSearch(wdir, wquery, wfield, wgroupby);
  delete[] wgroupby;
  delete[] wfield;

#ifdef _F_UNICODE
  delete[] wdir; 
#endif
  return hits;
}

NSLIBDLL_API void NSL_ClearSearch(void* hits){
  if ( hits == NULL ) return;

  ((Hits*)hits)->searcher.close();
  delete ((Hits*)hits); //need to modify Hits' destructor to close searcher.
}

NSLIBDLL_API int NSL_HitCount(const void* hits){
  if ( hits == NULL ){
    reportError("No hits available");
    return -1;
  }else
    return ((Hits*)hits)->Length();
}

NSLIBDLL_API const char* NSL_HitGroupby(const void* hits)
{
  if ( hits == NULL ){
    reportError("No hits available");
    return "";
  }else
    return ((Hits*)hits)->GroupbyResult();
}

NSLIBDLL_API void* NSL_Hit(const void* hits, int hitPos){
  if ( hits != NULL && hitPos < ((Hits*)hits)->Length() ) 
    return &((Hits*)hits)->doc(hitPos);
  return NULL;
}

NSLIBDLL_API type_long NSL_Get_DateField(const void* doc, const char* field){
  if ( doc == NULL || field == NULL ) return -1;
#ifdef _UNICODE
  char_t* wfield = CharConverter::charToWide(field, LANG_ENG);
  type_long ret = NSL_WGet_DateField(doc, wfield);
  delete[] wfield;
#else
  type_long ret = NSL_WGet_DateField(doc, field);
#endif

  return ret;
}

NSLIBDLL_API void NSL_Free_Keywords(char* keywords) {
  if ( keywords == NULL ) return;
  delete[] keywords;
}

//parse query to get tokens for future highlight
NSLIBDLL_API char* NSL_Get_Keywords(const char* query, const char* enc) {
  if ( query == NULL || enc == NULL ) return NULL;
#ifdef _UNICODE
  char_t* wquery = CharConverter::charToWide(query, enc);
  char_t* wkeywords = NSL_WGet_Keywords(wquery);
  delete[] wquery;
  if ( wkeywords ) {
    char* keywords = CharConverter::wideToChar(wkeywords, enc);
      delete[] wkeywords;
    return keywords;
  }
#else
  char_t* keywords = NSL_WGet_Keywords(query);
  if ( keywords ) {
    return keywords;
  }
#endif
  return NULL;
}

NSLIBDLL_API char* NSL_Get_Field_HTMLHiLite(const void* doc, 
                      const char* field, 
                      const char* keywords,
                            const char* enc,
                      int detail){
  
    if ( doc == NULL || field == NULL || keywords == NULL || enc == NULL ) 
    return NULL;

#ifdef _UNICODE
  char_t* wfield = CharConverter::charToWide(field, LANG_ENG);
  char_t* wkeywords = CharConverter::charToWide(keywords, enc);
  char_t* wvalue = NSL_WGet_Field_HTMLHiLite(doc, wfield, wkeywords, detail);
  delete[] wfield;
  delete[] wkeywords;
  if ( wvalue ) {
    char* value = CharConverter::wideToChar(wvalue, enc);
    delete [] wvalue;
    return value;
  }
#else
  char_t* value = NSL_WGet_Field_HTMLHiLite(doc, field, keywords, detail);
  if ( value ) {
    return value;
  }
#endif
  return NULL;
}

NSLIBDLL_API char* NSL_Get_Field(const void* doc, const char* field, const char* venc){
  if ( doc == NULL || field == NULL || venc == NULL ) return NULL;
#ifdef _UNICODE
  char_t* wfield = CharConverter::charToWide(field, LANG_ENG);
  char_t* wvalue = NSL_WGet_Field(doc, wfield);
  delete[] wfield;
  if ( wvalue ) {
    char* value = CharConverter::wideToChar(wvalue, venc);
    delete[] wvalue;
    return value;
  }
#else
  char_t* value = NSL_WGet_Field(doc, field);
  if ( value ) {
    return value;
  }
#endif

  return NULL;
}

NSLIBDLL_API void NSL_Free_Field(char* value) {
  delete[] value;
}

NSLIBDLL_API int NSL_Delete(const char* dir, 
              const char* query, 
              const char* qenc, 
              const char* field){
    if ( dir == NULL || query == NULL || qenc == NULL || field == NULL ) return -1;
#ifdef _F_UNICODE
  fchar_t* wdir = CharConverter::charToWide(dir, LANG_ENG);
#else
  size_t len = strlen(dir);
  fchar_t* wdir = new fchar_t[len+1];
  fstringCopy(wdir, dir);
#endif
#ifdef _UNICODE
  char_t* wfield = CharConverter::charToWide(field, LANG_ENG);
  char_t* wquery = CharConverter::charToWide(query, qenc);
  int ret = NSL_WDelete(wdir, wquery, wfield);
  delete[] wfield;
  delete[] wquery;
#else
  int ret = NSL_WDelete(dir, query, field);
#endif
#ifdef _F_UNICODE
  delete[] wdir;
#endif
  return ret;
}

//NSLIBDLL_API void NSL_ErrStr(char* pl, int len){
//  for ( int i=0;i<len&&i<errlen&&i<strlen(errstr);i++ )
//    pl[i]=errstr[i];
//  pl[i]=0;
//}

#ifdef  __cplusplus
}//extern "C" 
#endif
