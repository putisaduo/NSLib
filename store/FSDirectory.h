#ifndef _NSLib_store_FSDirectory_
#define _NSLib_store_FSDirectory_

#include <sys/timeb.h>
#include <vector>

#ifdef _WIN32

# include <io.h>
# include <direct.h>                       
# include <sys/stat.h>
#include "util/dirent.h"

#else
 
# include <dirent.h>

#endif

// the follow 2 defintions are temporary fix by fzhao@July 2010
#define HAVE_UNISTD_H 1
#define HAVE_SYS_STAT_H 1

#if HAVE_UNISTD_H
# include <sys/types.h>
# include <unistd.h>
# define O_RANDOM 0
# ifdef O_BINARY
#  undef O_BINARY //reduces compile errors
# endif
# define O_BINARY 0
# define _close ::close
# define _lseek lseek
# define _read read
# define _write write
# define _tell(fhandle) lseek(fhandle, 0, SEEK_CUR)
#endif

#if HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif

#include "StdHeader.h"
#include "Directory.h"
#include "Lock.h"
#include "util/VoidMap.h"
#include "util/StringBuffer.h"

using namespace std;
namespace NSLib{ namespace store{
//static util::VoidMap< const char_t*,Directory*> DIRECTORIES(true,util::DELETE_TYPE_DELETE_ARRAY,false,util::DELETE_TYPE_NONE);
static map< const string,Directory*> DIRECTORIES;
//static DEFINE_MUTEX(DIRECTORIES_MUTEX);

class FSInputStream:public InputStream {
protected:
  int fhandle;
  DEFINE_MUTEX(file_mutex);

public:
  bool isClone;

  FSInputStream(const fchar_t* path);
  FSInputStream(FSInputStream& clone);
  ~FSInputStream();

  InputStream& clone();
  void close();

protected:
  void seekInternal(const long_t position) {}
  // InputStream methods 
  void readInternal(l_byte_t* b, const int offset, const int len);
};


class FSOutputStream: public OutputStream {
private:
  int fhandle;
public:
  FSOutputStream(const fchar_t* path);
  ~FSOutputStream();

  OutputStream& clone();

  // output methods: 
  void flushBuffer(const l_byte_t* b, const int size);
  void close();

  // Random-access methods 
  void seek(const long_t pos);
  long_t Length();
};

class FSDirectory: public Directory{
private:
  fchar_t directory[MAX_PATH];
  int refCount;
  
  DEFINE_MUTEX (FSDIR_CREATE);
  void create();
  
  void priv_getFN(fchar_t* buffer, const fchar_t* name);
  int priv_getStat(const fchar_t* name, struct Struct_Stat* ret);

public:
  FSDirectory(const fchar_t* path, const bool createDir);
  ~FSDirectory();

  // Returns true iff a file in the directory. 
  bool fileExists(const fchar_t* name);

  fchar_t* getDirName();
  
  virtual void list(char_t**& list, int& size) {}

  // Returns the directory instance for the named location.
  // 
  // <p>Directories are cached, so that, for a given canonical path, the same
  // FSDirectory instance will always be returned.  This permits
  // synchronization on directories.
  // 
  // @param file the path to the directory.
  // @param create if true, create, or erase any existing contents.
  // @return the FSDirectory for the named file.
  static FSDirectory& getDirectory(const fchar_t* file, const bool create);

  // Returns the time the named file was last modified.
  long_t fileModified(const fchar_t* name);
         
  // Returns the time the named file was last modified.
  //static
  static long_t fileModified(const fchar_t* dir, const fchar_t* name);

  // Returns the length in bytes of a file in the directory. 
  //static
  long_t fileLength(const fchar_t* name);

  // Removes an existing file in the directory. 
  void deleteFile(const fchar_t* name, const bool throwError=true);

  DEFINE_MUTEX(FSDIR_RENAME);
  // Renames an existing file in the directory. 
  void renameFile(const fchar_t* from, const fchar_t* to);

  // Creates a new, empty file in the directory with the given name.
  //  Returns a stream writing this file. 
  OutputStream& createFile(const fchar_t* name);

  // Returns a stream reading an existing file. 
  InputStream& openFile(const fchar_t* name);

  // Construct a {@link Lock}.
  // @param name the name of the lock file
  NSLock* makeLock(const fchar_t* name);

  void refInc();

  DEFINE_MUTEX(FSDIR_CLOSE);
  // Closes the store to future operations. 
  void close();

  DEFINE_MUTEX(LOCK_MUTEX);
};

class FSLock:public virtual NSLock{
public:
  // const char_t* fname;
  fchar_t* fname; // removed const because the free() wouldn't compile otherwise. --RGR 
  FSLock ( const fchar_t* name );
  ~FSLock();
  bool obtain();
  void release();
};

}}
#endif
