#include "StdHeader.h"
#ifndef _NSLib_store_RAMDirectory_
#define _NSLib_store_RAMDirectory_

#include "Lock.h"
#include "Directory.h"
#include "util/VoidMap.h"
#include "util/Arrays.h"

namespace NSLib{ namespace store{
  class RAMDirectory; //predefined

  class RAMFile {
  public:
    NSLib::util::VoidList<l_byte_t*> buffers;
    long_t length;
    long_t lastModified;

#ifdef _DEBUG
    fchar_t* filename;
#endif

    RAMFile();
    ~RAMFile();
  };

  class RAMLock: public virtual NSLock{
  private:
    RAMDirectory* directory;
    const fchar_t* fname;
  public:
    RAMLock(const fchar_t* name, RAMDirectory* dir);
    ~RAMLock();
    bool obtain();
    
    void release();
  };

  class RAMOutputStream: public OutputStream {
  protected:
    RAMOutputStream(RAMOutputStream& clone);
  public:
    RAMFile& file;
    int pointer;

    RAMOutputStream(RAMFile& f);
    ~RAMOutputStream();

    OutputStream& clone();

    // output methods: 
    void flushBuffer(const l_byte_t* src, const int len);

    void close();

    // Random-at methods 
    void seek(const long_t pos);
    long_t Length();
  };


  class RAMInputStream:public InputStream {
  private:
    RAMFile& file;
    int pointer;

  public:
    RAMInputStream(RAMFile& f);
    RAMInputStream(RAMInputStream& clone);
    ~RAMInputStream();
    InputStream& clone();

    // InputStream methods 
    void readInternal(l_byte_t *dest, const int idestOffset, const int len);

    void close();

    // Random-at methods 
    void seekInternal(const long_t pos);

    RAMInputStream& operator=( const RAMInputStream& str )
    {
      throw "CAN'T ASSIGN THIS OBJECT";
    }
  };




  
  class RAMDirectory:public Directory{
  private:
    NSLib::util::VoidMap<const char_t*,RAMFile*> files;
  public:
    DEFINE_MUTEX(files_mutex);

    // Returns an array of strings, one for each file in the directory. 
    void list(char_t**& list, int& size);

    RAMDirectory();
    ~RAMDirectory();
           
    // Returns true iff the named file exists in this directory. 
    bool fileExists(const fchar_t* name);

    // Returns the time the named file was last modified. 
    long_t fileModified(const fchar_t* name);

    // Returns the length in bytes of a file in the directory. 
    long_t fileLength(const fchar_t* name);

    // Removes an existing file in the directory. 
    void deleteFile(const fchar_t* name, const bool throwError = true);

    // Removes an existing file in the directory. 
    void renameFile(const fchar_t* from, const fchar_t* to);

    // Creates a new, empty file in the directory with the given name.
    //  Returns a stream writing this file. 
    OutputStream& createFile(const fchar_t* name);

    // Returns a stream reading an existing file. 
    InputStream& openFile(const fchar_t* name);

    void close();

    // Construct a {@link Lock}.
    // @param name the name of the lock file
    NSLock* makeLock(const fchar_t* name);
  };
}}
#endif
