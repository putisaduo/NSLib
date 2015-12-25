/**
 * Defines a class for parsing Uniform Resource Locators (URLs) as according to
 * RFC 1738.
 *
 * protocol://username:password@authority:port/resource?param1&param2=value2#reference
 *
 * EXAMPLES
 * ========
 *
 * Parsing a standard URL
 * ----------------------
 *
 * An URL can be parsed in the constructor, or by calling the 'parse' member function:
 *
 *  Tools::URL url("http://username:password@www.website.com/index.html?param1=name1#anchor");
 *  url.parse("http://username:password@www.website.com/index.html?param1=name1#anchor");
 *
 */

#ifndef _TOOLS_URL_H_
#define _TOOLS_URL_H_

#include <boost/regex.hpp>
#include <vector>
#include <string>
#include "StringParam.h"

namespace UBCService{

class URL
{
 public:

  URL();

  URL( ConstStringParam url, bool reparseAsFileProtocol = true );

  void clear();

  // Parses the given url, returning true if it was valid, false otherwise.
  bool parse( ConstStringParam url, bool reparseAsFileProtocol = true );

  bool is_valid() const;
  
  const std::string& get_url() const;

  std::string get_protocol( bool _decode = true ) const;

  std::string get_username( bool _decode = true ) const;

  std::string get_password( bool _decode = true ) const;

  std::string get_authority( bool _decode = true ) const;

  std::string get_port( bool _decode = true ) const;

  std::string get_resource( bool _decode = true ) const;

  std::string get_reference( bool _decode = true ) const;

  std::string get_params( bool _decode = true ) const;

  size_t get_param_count() const;

  std::string get_param( size_t i, bool _decode = true ) const;

  // Determines if the given parameters was a name-value pair (ie: of the form name=value)
  bool param_is_nvp( size_t i ) const;

  std::string get_param_name( size_t i, bool _decode = true ) const;
  std::string get_param_value( size_t i, bool _decode = true ) const;

  /**
   * Splits an NVP, returning the name and the value in provided strings. Returns true if the
   * parameter was a NVP, false otherwise (in which case, name stores the whole parameter, and value
   * is cleared).
   */
  bool split_param( size_t i, std::string& name, std::string& value, bool _decode = true ) const;
 
  std::string get_authority_to_params( bool _decode = true ) const;

  std::string get_resource_to_params( bool _decode = true ) const;

  std::string get_filename( bool _decode = true ) const;

  int get_port_int() const;

  bool has_port_int() const;

  bool has_username() const;

  bool has_password() const;

  bool has_port() const;

  bool has_resource() const;

  bool has_params() const;

  bool has_reference() const;

  static std::string decode( ConstStringParam s );

  // URL encodes a string.
  static std::string encode( ConstStringParam s );

private:

  // Used internally by parse().  Only parses a strict RFC compliant URL.
  bool _parse( ConstStringParam url );

  bool _valid;
  std::string _url;
  int _port;
  std::vector< std::string > _parsed;
  std::vector<std::string> _params;
};

} // namespace Tools

#endif
