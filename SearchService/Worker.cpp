#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <pion/http/response_writer.hpp>

#include <exception>

#include <Encrypt.h>
#include <NSLib.h>
#include "analysis/ChineseAnalyzer.h"

#include "Worker.h"

namespace {
namespace Cfg {
  using std::string;
  
  static const string modelListKey (".models");
}; // namespace Cfg
}; // namespace


namespace UBCService {

void SearchThread(Worker* worker)
{ 
  while (true) {
    if (worker->jobQueue.empty()) 
      std::this_thread::sleep_for (std::chrono::milliseconds(50));
    else
      worker->search();
  }
} 

Worker::Worker(std::string indexPath)
{
  m_indexBase = "C:\\index\\";
  if (indexPath.size() > 0)
    m_indexBase = indexPath;
  m_index = "SearchDB";
  searcher = new IndexSearcher((m_indexBase + "100").c_str()); 
  cout << "Searcher initialized." << endl;
  searchThread = new thread(SearchThread, this);
}

void Worker::search(Headers header, request_ptr request_ptr,
                            tcp::connection_ptr tcp_conn)
{   
  Job job= { header, request_ptr, tcp_conn };

  queueMutex.lock();
  jobQueue.push(job);
  queueMutex.unlock();
}   

void Worker::search()
{
  Job& job = jobQueue.front(); 
  Headers& header = job.header;
  
  high_resolution_clock::time_point _start = high_resolution_clock::now();

  vector<string> databases;
  char* pch = strtok((char*)(header.database.c_str()),";");
  while (pch != NULL) {
    string db = pch;
    databases.push_back(db);
    pch = strtok(NULL, ";");
    cerr << "db: " << db << endl;
  }

  std::string database = header.database.empty()? m_index : header.database;
  std::u16string wquery = header.searchQuery,
                 wfield = header.field,
                 wgroupby = header.groupby;
  int numResults = header.numResults;
  WStringVec showFields;
  header.getShowFields(showFields);
  string result = prv_search(database, wquery, wfield, wgroupby, showFields, numResults);
  
  cerr << getTimeString() << "writing results" << endl;
  
  // Set Content-type to "text/plain" (plain ascii text)
  response_writer_ptr writer(response_writer::create(job.tcp_conn,
                                          *job.http_request_ptr,
                                          boost::bind(&tcp::connection::finish,
                                          job.tcp_conn)));
  writer->get_response().set_content_type(http::types::CONTENT_TYPE_TEXT);
  writer << result << http::types::STRING_CRLF;
  writer->send();

  nanoseconds ns = duration_cast<nanoseconds>(high_resolution_clock::now() - _start);
  cerr << getTimeString() << "Search succeeded in " << (ns.count() / 1000000) << " ms" << endl;

  queueMutex.lock();
  jobQueue.pop();
  queueMutex.unlock();
} 

std::string Worker::prv_search(std::string database, std::u16string wquery, 
                           std::u16string wfield, std::u16string wgroupby,
                           WStringVec& showFields, int numResults)
{
  std::string result = "{ \"stat\":[{\"database\":{\"" + database + "\":\"" ;
  /*
  char16_t* searchQuery0 = L"(CodePath:(E0001)) AND (FullText:(((\"𪛖\"))))";
  char16_t* searchQuery1 = L"FullText:\"字彙\"";
  char16_t* searchQuery = L"Title:\"𪕹\"";
  */  
  //char16_t* searchQuery0 = L"(CodePath: (CA02DB000B OR CA02DB0001 OR CA02DB0002 OR CA02DB0003)) AND (FullText: (\"饰\" OR \"飾\") OR (\"飾\") OR (\"餙\"))";
  //char16_t* searchQuery0 = L"(CodePath: (CA02DB0001 OR CA02DB0002 OR CA02DB0003)) AND (FullText: (\"饰\" OR \"飾\") OR (\"飾\") OR (\"餙\"))";
  //char16_t* searchQuery0 = L"(CodePath: (CA02DB0002 OR CA02DB0003)) AND (FullText: (\"饰\" OR \"飾\") OR (\"飾\") OR (\"餙\"))";
  //char16_t* searchQuery0 = L"(CodePath: (CA02DB0003)) AND (FullText: (\"饰\" OR \"飾\") OR (\"飾\") OR (\"餙\"))";
  //char16_t* searchQuery0 = L"(CodePath: (CA02DB0003)) AND (FullText: (\"饰\" OR \"飾\") OR (\"飾\") OR (\"餙\"))";
  //wquery = searchQuery0;
  std::cerr << std::endl << "@@@@@@@@@ Worker::search: "<< database << " "<<std::endl;
  int qlen = wquery.length() * sizeof(char16_t);
  char* wq = (char*) wquery.c_str();
  for (int i = 0; i<qlen; i++)
    std::cerr << std::hex << (int)(unsigned char)wq[i] << " ";
  std::cerr << std::endl;
  const char16_t* groupby = wgroupby.size()>0 ? wgroupby.c_str():NULL;
  void* hits = WSearch((m_indexBase + database).c_str(), wquery.c_str(), 
                          wfield.c_str(), groupby);
  if ( hits == NULL )
    result += " 0\"}}]}";
  else {
    const char* groupby_str = NSL_HitGroupby(hits);
    //"stat"：["database":{"index110":"1500","index100":"36000"},
    //     "group":{"1928": "39", "1929": "56"}],
    //"result": 
    //  [ { "RecordId": "26",
    int count = NSL_HitCount(hits);
    //printf( "     keywords: %s\n", keywords );
    printf( "     query: %hs >>>> found %d results, groupby: %s\n", 
           wquery.c_str(), count, groupby_str);
    result += boost::str(boost::format("%d\"}}, {\"group\":{%s}}],\"result\":[") 
                         % count % groupby_str);
    
    //char* val = NULL;
    for ( int i = 0; i < (std::min)(numResults, count); i++ ) {
      result += (i==0)? "{" : ",{";
      void* doc = NSL_Hit(hits, i);
      //printf( "     No: %d\n", i );

      bool firstField = true;
      for(WStringVec::iterator it=showFields.begin(); it!=showFields.end(); it++)
      {
        char* showField = NSL_wideToChar(it->c_str());
      //printf( "%s: %ld\n", showField.c_str(), doc);
        result += firstField? "" : ",";
        char16_t* val = NSL_WGet_Field(doc, it->c_str());
        if (val != NULL) {
          std::u16string valStr = val;
          int len = valStr.length() * sizeof(char16_t);
          size_t base64_output_length;
          char* base64_output = Base64::encode((const unsigned char*)val, len, 
                                                base64_output_length);
          std::string tmp = boost::str(boost::format(" \"%s\":\"%s\"") 
                                    % showField % base64_output);
          result += tmp;
          NSL_WFree_Field(val);
          delete[] base64_output;
        } else {
          result += boost::str(boost::format(" \"%s\":\"\"") % showField);
        }
        firstField = false;
        delete[] showField;
      }
      result += "}";
    }
    result += "]}";
  }
  NSL_ClearSearch(hits);

  return result;
  /*
  {
    "stat"：["database":{"index110":"1500","index100":"36000"},
         "group":{"1928": "39", "1929": "56"}],
    "result": 
      [ { "RecordId": "26",
            "CodePath": "F0001,CA13C,CA13D,CA13DB0005",
            "Path": "", "PathInnerXml": "史部集成·傳記譜系·明·郭勳·三家世典（國朝典故本）·正文",
            "Title": "", "TitleInnerXml": "",
            "ImagePath": "6", "ImageOrderId": "6", "Extender": "",
            "FullText": "天下已大定，而梁王把匝剌瓦爾密實元之遺孽，獨恃雲南險遠，執我信使，納我逋逃。"},
         {  "RecordId": "36",
            "CodePath": "F0001,CA13C,CA13D,CA13DB0005", "Path": ""
        }
    ]
  }
  */
}

void* Worker::WSearch(const char* wdir, const char_t* wquery, 
                      const char_t* wfield, const char_t* wgroupby)
{
  if ( wdir == NULL || wquery == NULL ) return NULL;

  Query* q = NULL;
  ChineseAnalyzer analyzer;

  try{
    cerr << endl << " $$$$$$$$$ WSearch: " << wdir << endl;
    //std::cerr << ws2str(wquery) << endl;
    QueryParser qp(wfield,analyzer);
    q = &qp.Parse(wquery);
  }catch(std::exception e){
    cerr << endl << "################ parsing error." << e.what() << endl;
    cerr << "exception caught: " << e.what() << endl;
  }catch(...){
    cerr << "exception caught: " << "Unknown error" << endl;
  }

  Hits* hits = NULL;
  if ( q != NULL ){
    try{
      std::cerr << endl << " $$$$$$$$$ WSearch starts ... " << endl;
      //IndexSearcher* searcher = new IndexSearcher(wdir);
      std::cerr << endl << " $$$$$ WSearchng  ... " << endl;
      hits = &searcher->search(*q, const_cast<char_t*>(wgroupby));
      std::cerr << endl << " $$$$$$$$$ WSearch done. " << endl;
    }catch(std::exception e){
      cerr << "exception caught: " << e.what() << endl;
    }catch(...){
      cerr << "exception caught: " << "Unknown error" << endl;
    }
  }

  //delete q;
  return hits;
}


void Worker::prv_addEscapeChar(char* src, char* des)
{
  char* ps=src, *ps_last=ps, *pd=des;
  bool gb18030Found = false;
  for (int i = 0; i<(int)strlen(src); i++, ps++) {
    if (gb18030Found) {
      gb18030Found = false;
      continue;
    }
    if (*(unsigned char*)ps>=0x80) {
      gb18030Found = true;
      continue;
    }
    if (*ps=='"' || *ps=='\\') {
      strncpy(pd, ps_last, ps-ps_last);
      pd += ps - ps_last;
      *(pd++) = '\\';
      *(pd++) = *ps;
      ps_last = ps + 1;
    }
  }
  strcpy(pd, ps_last);
}

} // namespace UBCService
