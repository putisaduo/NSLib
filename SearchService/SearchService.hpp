// ---------------------------------------------------------------------
// pion:  a Boost C++ framework for building lightweight HTTP interfaces
// ---------------------------------------------------------------------
// Copyright (C) 2007-2014 Splunk Inc.  (https://github.com/splunk/pion)
//
// Distributed under the Boost Software License, Version 1.0.
// See http://www.boost.org/LICENSE_1_0.txt
//

#ifndef __PION_ECHOSERVICE_HEADER__
#define __PION_ECHOSERVICE_HEADER__

#include <pion/http/plugin_service.hpp>
#include "Worker.h"

namespace pion {        // begin namespace pion
namespace plugins {     // begin namespace plugins

class SearchService : public pion::http::plugin_service
{
public:
  SearchService(void) : m_worker(new UBCService::Worker("/home/index/")) {
    std::cerr << "Loading SearchService." << std::endl;
  }

  virtual ~SearchService() {}
  virtual void operator()(pion::http::request_ptr& http_request_ptr,
                          pion::tcp::connection_ptr& tcp_conn);

  UBCService::Worker::Pointer m_worker;
};

}   // end namespace plugins
}   // end namespace pion

#endif
