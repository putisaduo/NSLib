#include "StdHeader.h"
#ifndef _NSLib_store_Directory
#define _NSLib_store_Directory

#include "store/Lock.h"
#include "util/VoidList.h"

namespace NSLib{ namespace store{

    //Java's filesystem API is not used directly, but rather through these
    //classes.  This permits:
    //  . implementation of RAM-based indices, useful for summarization, etc.;
    //  . implementation of an index as a single file.
    //
    //
    //A Directory is a flat list of files.  Files may be written once,
    //when they are created.  Once a file is created it may only be opened for
    //read, or deleted.  Random access is permitted when reading and writing.
    //
    //  @author Doug Cutting
  class Directory {
  public:
    virtual ~Directory(){ };

    // Returns an array of strings, one for each file in the directory. 
    virtual void list(char_t**& list, int& size) = 0;
           
    // Returns true iff a file with the given name exists. 
    virtual bool fileExists(const fchar_t* name) = 0;

    // Returns the time the named file was last modified. 
    virtual long_t fileModified(const fchar_t* name) = 0;

    // Removes an existing file in the directory. 
    virtual void deleteFile(const fchar_t* name, const bool throwError = true) = 0;

    // Renames an existing file in the directory.
    //  If a file already exists with the new name, then it is replaced.
    //  This replacement should be atomic. 
    virtual void renameFile(const fchar_t* from, const fchar_t* to) = 0;

    // Returns the length of a file in the directory. 
    virtual long_t fileLength(const fchar_t* name) = 0;

    // Creates a new, empty file in the directory with the given name.
    //  Returns a stream writing this file. 
    virtual OutputStream& createFile(const fchar_t* name) = 0;

    // Returns a stream reading an existing file. 
    virtual InputStream& openFile(const fchar_t* name) = 0;

    // Construct a {@link Lock}.
    // @param name the name of the lock file
    virtual NSLock* makeLock(const fchar_t* name) = 0;

    // Closes the store. 
    virtual void close() = 0;
  };
}}
#endif


