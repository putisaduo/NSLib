#pragma once

#include <boost/utility.hpp>
#include <boost/format.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/lexical_cast.hpp>
#include <pion/http/request.hpp>
#include <pion/http/types.hpp>

#include "Encrypt.h"
#include "URL.h"

#include <NSLib.h>

#include <uchar.h>
#include <string>


typedef char16_t wchar_t16;

using namespace pion;

namespace UBCService{

typedef std::vector<std::u16string> WStringVec;

// ---------------------------------------------------------------------------
// Create a Url Key Value Pair with the following guaranteed:
//   - The 'present' flag is set iff the key *and* value were present
//   - the 'key' will be set to the key provided
//   - the 'value' will be url decoded
struct UrlKeyValuePair{ 
  UrlKeyValuePair(const http::request_ptr& http_request,
                  const std::string& urlKey)
    : key(urlKey)
  { 
    pion::ihash_multimap& params = http_request->get_queries();
    pion::ihash_multimap::const_iterator param = params.find(key);
    if(param == params.end() || param->second.empty())
      present = false;
    else {
      present = true;
      value = URL::decode(param->second);
    }
  }

  std::string key;
  bool present;
  std::string value;
};

   
struct Headers
{
  // convenience alias
  typedef pion::http::types HTTPTypes;

  // Default constructor for serialization support
  Headers()
  {}

  // Real constructor from an HTTPRequets, from which we get the (optional)
  // User, CreatorIp and User Agent
  Headers(const pion::http::request_ptr& http_request)
    : creatorIp(prv_getRemoteIp(http_request)),
      userAgent(http_request->get_header(HTTPTypes::HEADER_USER_AGENT)),
      database(getOptionalParameter<std::string>(http_request, "database", "")),
      numResults(getOptionalParameter<int>(http_request, "numResults", 500)),
      query(getOptionalParameter<std::string>(http_request, "q", ""))
  {
    std::cerr << query << std::endl;
    size_t base64_output_length;
    char* base64_output = Base64::decode(query.c_str(), query.length(), 
                                         base64_output_length);
    for (unsigned int i=0; i<base64_output_length; i++)
      std::cerr << std::uppercase << std::hex << ((unsigned short)base64_output[i])%256 << " ";
    std::cerr << std::endl;
    //Base64 b64;
    //char* base64_output = new char[1024*10];
    //size_t base64_output_length = b64.decode(query.c_str(), query.length(), 
    //                                         base64_output);
    unsigned char* l = (unsigned char*)base64_output;

    int len = (((l[0]-'0')*10 + l[2]-'0')*10 + l[4]-'0')*10 + l[6]-'0';
    unsigned char* f = l + sizeof(wchar_t16) * 4;
    
    l = f + len * sizeof(wchar_t16);
    len = (((l[0]-'0')*10 + l[2]-'0')*10 + l[4]-'0')*10 + l[6]-'0';
    *l = '\0';
    searchQuery = (wchar_t16*)f;
    char* str = NSL_wideToChar((wchar_t16*)f);
    std::cerr << " query is " << str << std::endl;
    delete str;
    //searchQuery = convert_u16string( (wchar_t16*)f, len);
    
    f = l + sizeof(wchar_t16) * 4;    
    l = f + len * sizeof(wchar_t16);
    len = (((l[0]-'0')*10 + l[2]-'0')*10 + l[4]-'0')*10 + l[6]-'0';
    *l = '\0';
    field = (wchar_t16*)f;
    str = NSL_wideToChar((wchar_t16*)f);
    std::cerr << " field is " << str << std::endl;
    delete str;
    //field = convert_u16string( (wchar_t16*)f, len);
    
    f = l + sizeof(wchar_t16) * 4;    
    l = f + len * sizeof(wchar_t16);
    len = (((l[0]-'0')*10 + l[2]-'0')*10 + l[4]-'0')*10 + l[6]-'0';
    *l = '\0';
    showfield = (wchar_t16*)f;
    str = NSL_wideToChar((wchar_t16*)f);
    std::cerr << " showfield is " << str << std::endl;
    delete str;
    //showfield = convert_u16string( (wchar_t16*)f, len);

    f = l + sizeof(wchar_t16) * 4;
    if (len>0) {
      wchar_t16* buf = new wchar_t16[len+1];
      memcpy(buf, f, len*sizeof(wchar_t16));
      buf[len]=0;
      groupby= (wchar_t16*)buf;
      char* str = NSL_wideToChar((wchar_t16*)f);
      std::cerr << " groupby is " << str << std::endl;
      delete str;
      //groupby = convert_u16string( (wchar_t16*)f, len);
      delete[] buf;
    }
    delete base64_output;
  }

  virtual ~Headers()
  {}

public:
  std::string dump() const {
    return boost::str(boost::format("%s@%s(via:%s)[%s]")
                   % creatorIp % userAgent % query);
  }
  
  void getShowFields(WStringVec& showFields)
  {
    showFields.clear();
    size_t start = 0;
    while(1) {
      size_t pos = showfield.find(L';', start);
      if (pos == std::string::npos) {
        if (showFields.size() > 0) 
	        showFields.push_back(showfield.substr(start));
	      break;
      } else {
	      showFields.push_back(showfield.substr(start, pos-start));
        start = pos + 1; 
      }
    }
  }

private:
   
  // ---------------------------------------------------------------------------
  // Function to obtain an optional parameter from an Http Request.  Users can
  // (optionally) provide a default value to be used if the parameter is not
  // present.
  template<typename T>
  T getOptionalParameter(const http::request_ptr& http_request,
                     std::string key, T defaultValue)
  {
    UrlKeyValuePair kvp(http_request, key);
    try{
      std::cerr << key << " " << kvp.value << std::endl; 
      return kvp.present ? boost::lexical_cast<T>(kvp.value) : defaultValue;
    } catch(boost::bad_lexical_cast&) {
      std::string InvalidParameter("Invalid value '%s' specified for "
                               "parameter '%s'");
      throw pion::exception(boost::str(
             boost::format(InvalidParameter) % kvp.value % kvp.key));
    }
  }

  std::u16string convert_u16string(wchar_t16* f, int len)
  {
    std::u16string ret;
    char16_t wch;
    for (int i=0; i<len; i++) {
      wch = f[i];
      ret += wch;
    }
    return ret;
  }

  // Parse out the optional "User" parameter
  inline std::string prv_getUsername(
               const http::request_ptr& http_request)
  {
    UrlKeyValuePair usernameParam(http_request, "User");
    return usernameParam.present ? usernameParam.value : "Unknown";
  }

  // Get the RemoteIP, which is either the X_FORWARDED_FOR or the actual
  // remoteIP of the underlying TCP connection.
  inline std::string prv_getRemoteIp( const http::request_ptr& http_request)
  {
    // Initialize the remoteIp to that of the remote end of the underlying TCP
    // connection
    std::string remoteIp(http_request->get_remote_ip().to_string());

    // Iff there is a non-empty HEADER_X_FORWARDED_FOR header, we may be able
    // to get the actual originating IP address
    using namespace pion;
    std::string xForwardedFor(http_request->
                                get_header(HTTPTypes::HEADER_X_FORWARDED_FOR));
    if (! xForwardedFor.empty()) {
      using namespace boost::algorithm;
      std::vector<std::string> forwardsVec;
      boost::split(forwardsVec, xForwardedFor, is_any_of(","), token_compress_on);
      
      // take the first IP address in the list, and ensure that it is actually
      // an IP address.
      try{
        using namespace boost::asio::ip;
        remoteIp = address::from_string(forwardsVec[0]).to_string();
      } catch(const boost::system::system_error&){
        std::fprintf(stderr, "Received Invalid IP address '%s' in \"%s: %s\"",
                    forwardsVec[0].c_str(),
                    HTTPTypes::HEADER_X_FORWARDED_FOR.c_str(),
                    xForwardedFor.c_str());
      }
    }
    
    return remoteIp;
  }

public:
  //  :: -------------------------------------------------------------------
  //  :: Data Members
  
  std::string creatorIp;
  std::string userAgent;
  std::string database;
  int numResults;
  std::string query;
  std::u16string searchQuery;
  std::u16string field;
  std::u16string showfield;
  std::u16string groupby;
  std::u16string orderby;
};

}; // namespace UBCService
