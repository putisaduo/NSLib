#ifndef _TOOLS_STRING_PARAM_H_
#define _TOOLS_STRING_PARAM_H_

#include <iostream>
#include <string.h>

namespace UBCService{

/*
 * A simple object that allows us to pass string parameters as
 * std::string objects OR char objects.
 *
 * Behaves as a 'const char *'.
 */
class ConstStringParam
{
public:
  typedef ConstStringParam self_type;

  // CONSTRUCTORS
  ConstStringParam(): _string(NULL)
  { }

  ConstStringParam(const self_type& string): _string(string._string)
  { }

  // We need to duplicate this object because it may only live on the
  // stack, and be destroyed right after constructing us.
  ConstStringParam( const std::string& string ): _string(string.c_str())
  { }

  ConstStringParam( const char* string ): _string(string)
  { }

  // LENGTH
  inline size_t length() const
  { return strlen(_string); }

  // DEREFERENCING STUFF

  inline operator const char * () const
  { return _string; }

  inline const char & operator*() const
  { return _string[0]; }

  inline const char & operator[]( int i ) const
  { return _string[i]; }

  inline const char * get() const
  { return _string; }

  // ASSIGNMENT STUFF

  inline self_type& operator=( const self_type& string )
  { _string = string._string; return (*this); }

  inline self_type& operator=( const std::string& string )
  { _string = string.c_str(); return (*this); }

  inline self_type operator=( const char* string )
  { _string = string; return (*this); }

  // COMPARISON STUFF

  inline operator bool () const
  { return _string != NULL; }

  inline bool operator==(self_type string){return _string==string._string;}
  inline bool operator!=(self_type string){return _string!=string._string;}
  inline bool operator<=(self_type string){return _string<=string._string;}
  inline bool operator>=(self_type string){return _string>=string._string;}
  inline bool operator<(self_type string){return _string<string._string;}
  inline bool operator>(self_type string){return _string>string._string;}

private:
  const char* _string;
};
 
// STREAM STUFF
inline std::ostream& operator<<( std::ostream& os, ConstStringParam s )
{
  os << s.get();
  return os;
}

} // namespace Tools

#endif
