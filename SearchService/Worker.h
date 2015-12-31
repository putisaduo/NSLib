#ifndef UBCService_Worker_h_included_
#define UBCService_Worker_h_included_

#include <boost/enable_shared_from_this.hpp>

#include <vector>
#include <string>
#include <queue>
#include <mutex>
#include <thread>

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
  typedef std::vector<std::string> StringVec;
  typedef boost::shared_ptr<Worker> Pointer;
  typedef struct {
    Headers header;
    pion::http::request_ptr http_request_ptr;
    pion::tcp::connection_ptr tcp_conn;
  } Job;


public:
  Worker(std::string indexPath);
  virtual ~Worker() { std::fprintf(stderr, "Exiting"); }
  
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
  void search(Headers header, request_ptr request_ptr,
                      tcp::connection_ptr tcp_conn);
  void* WSearch(const char* wdir, const char_t* wquery, 
               const char_t* wfield, const char_t* wgroupby);

  string m_indexBase;
  IndexSearcher* searcher;

  queue<Job> jobQueue;
  mutex queueMutex;

private:
  string prv_search(string database, u16string wquery, u16string wfield,
                    u16string wgroupby, WStringVec& showFields, int numResults);

  void prv_addEscapeChar(char* src, char* des);

private:
  //  :: -------------------------------------------------------------------
  //  :: Data Members
  string m_index;

  queue<Job> queryQueue;
  thread* searchThread;
};

} // namespace UBCService

#endif
