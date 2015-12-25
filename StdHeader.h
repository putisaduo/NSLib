#ifndef _NSLib_StdHeader_
# define _NSLib_StdHeader_

#define _NOTMULTITHREAD //if multi-threading not required

//Data types: 
//-------------------------------

//STD Includes
#include <ctype.h>
#include <map>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <vector>
#include <stdexcept>
#include <fcntl.h> //file constants, also will include io.
#include <stdio.h> 

#include <string>

//misc options
#define NSLIB_STREAM_BUFFER_SIZE  1024
#define NSLIB_WRITER_INDEX_INTERVAL 128
#define NSLIB_SCORE_CACHE_SIZE 32
#define CHAR_RADIX 36
//#define USE_INFO_STREAM 1

#define SUPPORT_4_BYTE
#define SUPPORT_TIBETAN
#define SUPPORT_MONGOLIAN
//#define SUPPORT_ALL_WESTEN
//#define _F_UNICODE
#define _UNICODE

#define fStringArray NSLib::util::VoidList<const fchar_t*>
#define fStringArrayConst NSLib::util::VoidList<const fchar_t*>
#define fStringArrayIterator vector<const fchar_t*>::const_iterator
#define fStringArrayConstIterator vector<const fchar_t*>::iterator

//Common data types
#define l_byte_t unsigned char
#define uint unsigned long
#define floatSquareRoot sqrt
#define floatLog log

typedef std::map<std::string, l_byte_t*> InputStreamMap;
static InputStreamMap global_input_stream_map;
static std::map<std::string, long long> global_file_length;

//windows attributes - avoids having to include windows.h
//todo2: this should be defined on a per platform basis???
#ifndef MAX_PATH
# ifdef PATH_MAX
#  define MAX_PATH PATH_MAX
# else
#  define MAX_PATH 256    /* Should be safe for any weird systems that do not define it */
# endif
#endif

#ifndef FILE_ATTRIBUTE_DIRECTORY
#define FILE_ATTRIBUTE_DIRECTORY  16
#endif

//error catching
#define _TRY try
#define _FINALLY(x) catch(...){ x; throw; } x //note, doesn't code in x if a normal return occurs

# define _THROWC(y) throw THROW_TYPE(y)

//Multi-tread support
#ifndef _NOTMULTITHREAD
    struct mutex{
      int locked;
      mutex(){
        locked=0;
      }
    };
    #define DEFINE_MUTEX mutex
    #define LOCK_MUTEX(x) while ( x.locked != 0 );x.locked = 1 //wait till unlocked, then lock
    #ifdef _DEBUG
      #define UNLOCK_MUTEX(x) if ( x.locked == 1 ) x.locked = 0; else _THROWC( "Mutex wasn't locked")
    #else
      #define UNLOCK_MUTEX(x) x.locked = 0
    #endif
#else
    #define DEFINE_MUTEX(x) int x
    #define LOCK_MUTEX(x)
    #define UNLOCK_MUTEX(x)
#endif

#ifdef _WIN32
# include "StdHeaderWindows.h"
#else
# include "StdHeaderLinux.h"
#endif

//shortcut defines
#define StringArray NSLib::util::VoidList<const char_t*>
#define StringArrayConst NSLib::util::VoidList<const char_t*>
#define StringArrayIterator vector<const char_t*>::const_iterator
#define StringArrayConstIterator vector<const char_t*>::iterator
#define _DELETE(x) delete x; x=NULL;

std::string ws2str(const char16_t* src);

//for pre-compiled header in msvc
#include "util/Misc.h"
#include "store/InputStream.h"
#include "store/OutputStream.h"

#endif
