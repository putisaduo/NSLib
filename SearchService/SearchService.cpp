// ---------------------------------------------------------------------
// pion:  a Boost C++ framework for building lightweight HTTP interfaces
// ---------------------------------------------------------------------
// Copyright (C) 2007-2014 Splunk Inc.  (https://github.com/splunk/pion)
//
// Distributed under the Boost Software License, Version 1.0.
// See http://www.boost.org/LICENSE_1_0.txt
//

#include "SearchService.hpp"
#include <boost/bind.hpp>
#include <pion/algorithm.hpp>
#include <pion/http/response_writer.hpp>
#include <pion/user.hpp>

#include <locale>
#include <sys/time.h>
#include <chrono>

using namespace std;
using namespace chrono;

using namespace pion;

namespace pion {        // begin namespace pion
namespace plugins {     // begin namespace plugins

/// used by handle_request to write dictionary terms
void writeDictionaryTerm(http::response_writer_ptr& writer,
                       const ihash_multimap::value_type& val)
{
  // text is copied into writer text cache
  writer << val.first << http::types::HEADER_NAME_VALUE_DELIMITER
         << val.second << http::types::STRING_CRLF;
}


// SearchService member functions

/// handles requests for SearchService
void SearchService::operator()(http::request_ptr& http_request_ptr, 
               tcp::connection_ptr& tcp_conn)
{
  using namespace pion::http;
  
  cerr << UBCService::Worker::getTimeString() << "Received a connection " << endl;
  UBCService::Headers h(http_request_ptr);
  m_worker->search(h, http_request_ptr, tcp_conn);
}


}   // end namespace plugins
}   // end namespace pion


/// creates new SearchService objects
extern "C" PION_PLUGIN pion::plugins::SearchService *pion_create_SearchService( )
{
  std::cerr << "Loading plugin SearchService." << std::endl;
  return new pion::plugins::SearchService();
}

/// destroys SearchService objects
extern "C" PION_PLUGIN void pion_destroy_SearchService(
                                 pion::plugins::SearchService *service_ptr)
{
  delete service_ptr;
}
