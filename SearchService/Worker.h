#ifndef UBCService_Worker_h_included_
#define UBCService_Worker_h_included_

#include <boost/enable_shared_from_this.hpp>

#include <vector>
#include <string>
#include <queue>
#include <mutex>
#include <thread>
#include <map>

#include "search/IndexSearcher.h"
#include "queryParser/QueryParser.h"

#include "Headers.h"

using namespace std;
using namespace chrono;
using namespace pion::http;
using namespace NSLib::queryParser;

namespace UBCService {

class Worker : public boost::enable_shared_from_this<Worker>
{
public:
  //  :: Type Information
  typedef vector<string> StringVec;
  typedef vector<string>::iterator StrVecIter;
  typedef vector<std::thread*> ThreadVec;
  typedef boost::shared_ptr<Worker> Pointer;
  typedef struct {
    Headers header;
    pion::http::request_ptr http_request_ptr;
    pion::tcp::connection_ptr tcp_conn;
  } Job;


public:
  Worker(string indexPath);
  virtual ~Worker() { fprintf(stderr, "Exiting"); }
  
  static string getTimeString() {
    char tmp[100];
    time_t rawtime;
    struct tm* tminfo;
    struct timeval st;
    time(&rawtime);
    tminfo = localtime (&rawtime);
    gettimeofday(&st, NULL);
    sprintf(tmp, "%d-%d-%d %d:%d:%d.%03d ",
          1900+tminfo->tm_year, tminfo->tm_mon, tminfo->tm_mday,
          tminfo->tm_hour, tminfo->tm_min, tminfo->tm_sec, (int)(st.tv_usec/1000));
    string timeStr = tmp;

    return timeStr;
  }
  
  void search();
  void search(Headers header, request_ptr request_ptr, tcp::connection_ptr tcp_conn);
  void searchInDatabase(string database, Query* query,
             u16string wgroupby, WStringVec& showFields, int numResults);

  string m_indexBase;
  map<string, IndexSearcher*> searcherMap;
  map<string, string> resultMap;

  queue<Job> jobQueue;
  mutex queueMutex;

private:
  void prv_readDatabaseNames();
  void prv_addEscapeChar(char* src, char* des);

  string m_index;
  StringVec databaseNames;

  queue<Job> queryQueue;
  thread* searchThread;
};

} // namespace UBCService

#endif
