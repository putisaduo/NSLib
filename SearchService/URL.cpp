#include "URL.h"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <ctype.h>

namespace UBCService{

URL::URL():
  _valid(false),
  _url(),
  _port(-1),
  _parsed(),
  _params()
{
}

URL::URL(
  ConstStringParam url,
  bool reparseAsFileProtocol ):
  _valid(false),
  _url(),
  _port(-1),
  _parsed(),
  _params()
{
  parse(url,reparseAsFileProtocol);
}

void
URL::clear()
{
  _valid = false;
  _url.clear();
  _port = -1;
  _parsed.clear();
  _params.clear();
}

#define protocol_id             1
#define username_id             2
#define password_id             3
#define authority_to_params_id  4
#define authority_id            5
#define port_id                 6
#define resource_to_params_id   7
#define resource_id             8
#define params_id               9
#define reference_id           10

bool URL::parse( ConstStringParam url, bool reparseAsFileProtocol )
{
  if ( url.get() == NULL )
  {
    clear();
    return false;
  }

  // Try normally first, then try as a file:// type url.
  if ( _parse(url) ) return true;
  if ( !reparseAsFileProtocol ) return false;

  // stdin or stdout? Don't URL encode it, as we may wish
  // to provide file type hints in the parameters.
  if ( url.get()[0] == '-' && ( url.get()[1] == '?' || url.get()[1] == '#' ) )
    return _parse( std::string("file://") + url.get() );
  else
    return _parse( std::string("file://") + encode(url.get()) );
}

bool
URL::_parse(
  ConstStringParam url )
{
  static const boost::regex rurl(
    "^([^:/]+)://" // 1: protocol
    "(?:([^:@]*)(?:\\:([^@]*))?@)?" // 2:username / 3:password
    "(" // 4:authority_port_resource_params
      "([^:/?#]*)" // 5:authority
      "(?:\\:([0-9]*))?" // 6:port
      "(" // 7:resource_params
        "(/[^?#]*)?" // 8:resource
        "(?:\\?([^#]*))?" // 9:params
      ")"
    ")"
    "(?:#(.*))?$", // 10:reference
    boost::regex::icase );
 
  clear();
  _url = url.get();

  boost::cmatch m;
  if ( !boost::regex_match(url.get(),m,rurl) )
    return false;
  _valid = true;

  _parsed.push_back("");
  for ( size_t i = 1; i < m.size(); ++i )
    _parsed.push_back( m.str(i) );

  if ( has_port() )
  {
    _port = atoi( get_port().c_str() );
  }
  else
  {
    // use default ports for known protocols
    // TODO: Could always add more ports here!
    std::string prot = get_protocol();
    if ( prot == "ftp" ) _port = 21;
    else if ( prot == "sftp" ) _port = 22;
    else if ( prot == "http" ) _port = 80;
    else if ( prot == "https" ) _port = 443;
  }

  // Process the params
  if ( has_params() )
  {
    std::string s = get_params(false);
    boost::algorithm::split( _params, s, boost::algorithm::is_any_of("&") );
  }

  return true;
}

bool
URL::is_valid() const
{
  return _valid;
}

const std::string& URL::get_url() const { return _url; }
#define URL_GET(lc) \
  std::string URL::get_ ## lc ( bool _decode ) const \
  { \
    if ( _valid ) \
    { \
      if ( _decode ) return decode( _parsed[ lc ## _id ] ); \
      else return _parsed[ lc ## _id ]; \
    } \
    else \
      return std::string(); \
  }
URL_GET(protocol);
URL_GET(username);
URL_GET(password);
URL_GET(authority);
URL_GET(port);
URL_GET(resource);
URL_GET(params);
URL_GET(reference);
URL_GET(authority_to_params);
URL_GET(resource_to_params);
#undef URL_GET
int find_equal_sign( const std::string& s )
{
  if ( s.length() < 2 ) return 0;
  for ( size_t j = 0; j < s.length(); ++j )
    if ( s[j] == '=' )
      return j;
  return 0;
}
bool URL::param_is_nvp( size_t i ) const
{
  if ( i >= _params.size() ) return false;
  return find_equal_sign(_params[i]) > 0;
}
std::string URL::get_param_name( size_t i, bool _decode ) const
{
  std::string s;
  if ( i >= _params.size() ) return s;
  int j = find_equal_sign(_params[i]);
  if ( j == 0 ) return s;
  s = _params[i].substr(0,j);
  if ( _decode )
    return decode(s);
  else
    return s;
}
std::string URL::get_param_value( size_t i, bool _decode ) const
{
  std::string s;
  if ( i >= _params.size() ) return s;
  int j = find_equal_sign(_params[i]);
  if ( j == 0 ) return s;
  s = _params[i].substr(j+1);
  if ( _decode )
    return decode(s);
  else
    return s;
}
bool URL::split_param( size_t i, std::string& name, std::string& value, bool _decode ) const
{
  if ( i >= _params.size() ) return false;
  int j = find_equal_sign(_params[i]);
  if ( j == 0 )
  {
    if ( _decode )
      name = decode(_params[i]);
    else
      name = _params[i];
    return false;
  }
  name = _params[i].substr(0,j);
  value = _params[i].substr(j+1);
  if ( _decode )
  {
    name = decode(name);
    value = decode(value);
  }
  return true;
}
std::string URL::get_filename( bool _decode ) const
{
  std::string fn = get_authority() + get_resource();
  if ( _decode )
    return decode(fn);
  else
    return fn;
}
size_t URL::get_param_count() const { return _params.size(); }
std::string URL::get_param( size_t i, bool _decode ) const
{
  if ( _decode ) return decode( _params[i] );
  else return _params[i];
}
int URL::get_port_int() const { return _port; }
bool URL::has_port_int() const { return _port != -1; }

#define URL_HAS(lc) \
  bool URL::has_ ## lc () const \
  { \
    if ( _valid ) \
      return _parsed[lc ## _id].length() > 0; \
    else \
      return false; \
  }
URL_HAS(username);
URL_HAS(password);
URL_HAS(port);
URL_HAS(resource);
URL_HAS(params);
URL_HAS(reference);
#undef URL_HAS

int base16tobase10( int c )
{
  if ( c >= '0' && c <= '9' ) return c-'0';
  if ( c >= 'a' && c <= 'f' ) return (c-'a'+10);
  if ( c >= 'A' && c <= 'F' ) return (c-'A'+10);
  return -1;
}

std::string URL::decode( ConstStringParam s )
{
  size_t i = 0;
  std::string o;
  while ( i < s.length()) {
    // Found a %xx?
    if (s[i]=='%' && i+2<s.length() && isxdigit(s[i+1]) && isxdigit(s[i+2])) {
      int c = base16tobase10(s[i+1])*16 + base16tobase10(s[i+2]);
      o.push_back(c);
      i += 3;
    } else {
      o.push_back(s[i]);
      ++i;
    }
  }
  return o;
}

bool needs_encode( char c )
{
  switch ( c )
  {
    case '%':
    case '?':
    case '#':
    case '@':
    case ':':
      return true;

    default:
      return false;
  }
}

int base10tobase16( int i )
{
  if ( i < 10 ) return i + '0';
  return i - 10 + 'a';
}

std::string URL::encode( ConstStringParam s )
{
  std::string o;
  for ( size_t i = 0; i < s.length(); ++i )
  {
    if ( needs_encode(s[i]) )
    {
      o.push_back('%');
      o.push_back( base10tobase16( (s[i]>>4)&0xf ) );
      o.push_back( base10tobase16( s[i]&0xf ) );
    }
    else
    {
      o.push_back(s[i]);
    }
  }
  return o;
}

} // namespace Tools
