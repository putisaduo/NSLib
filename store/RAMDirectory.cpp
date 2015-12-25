#include "StdHeader.h"
#include "RAMDirectory.h"

#include "Lock.h"
#include "Directory.h"
#include "util/VoidMap.h"
#include "util/Arrays.h"
#include "util/CharConverter.h"


namespace NSLib{ namespace store{

  RAMFile::RAMFile()
  {
     length = 0;
     lastModified = NSLib::util::Misc::currentTimeMillis();
     buffers.setDoDelete(NSLib::util::DELETE_TYPE_DELETE_ARRAY);
  }
  RAMFile::~RAMFile(){
  }




  RAMLock::RAMLock(const fchar_t* name, RAMDirectory* dir):
    fname ( fstringDuplicate(name) ),
    directory(dir)
  {
  }
  RAMLock::~RAMLock()
  {
    delete[] fname;
  }
  bool RAMLock::obtain(){
    LOCK_MUTEX(directory->files_mutex);
    if (!directory->fileExists(fname)) {
        OutputStream& tmp = directory->createFile(fname);
        tmp.close();
        delete &tmp;
        
        UNLOCK_MUTEX(directory->files_mutex);
      return true;
    }
    UNLOCK_MUTEX(directory->files_mutex);
    return false;
  }

  void RAMLock::release(){
    directory->deleteFile(fname);
  }



  RAMOutputStream::~RAMOutputStream(){
  }
  RAMOutputStream::RAMOutputStream(RAMFile& f):file(f) {
    pointer = 0;
  }


  /** output methods: */
  void RAMOutputStream::flushBuffer(const l_byte_t* src, const int len) {
    uint bufferNumber = pointer/NSLIB_STREAM_BUFFER_SIZE;
    int bufferOffset = pointer%NSLIB_STREAM_BUFFER_SIZE;
    int bytesInBuffer = NSLIB_STREAM_BUFFER_SIZE - bufferOffset;
    int bytesToCopy = bytesInBuffer >= len ? len : bytesInBuffer;

    if (bufferNumber == file.buffers.size())
      file.buffers.push_back( new l_byte_t[NSLIB_STREAM_BUFFER_SIZE] );

    l_byte_t* buffer = file.buffers.at(bufferNumber);
    NSLib::util::Arrays::arraycopy(src, 0, buffer, bufferOffset, bytesToCopy);

    if (bytesToCopy < len) {        // not all in one buffer
      int srcOffset = bytesToCopy;
      bytesToCopy = len - bytesToCopy;      // remaining bytes
      bufferNumber++;
      if (bufferNumber == file.buffers.size())
        file.buffers.push_back( new l_byte_t[NSLIB_STREAM_BUFFER_SIZE]);
      buffer = file.buffers.at(bufferNumber);
      NSLib::util::Arrays::arraycopy(src, srcOffset, buffer, 0, bytesToCopy);
    }
    pointer += len;
    if (pointer > file.length)
      file.length = pointer;

    file.lastModified = NSLib::util::Misc::currentTimeMillis();
  }

  void RAMOutputStream::close() {
    OutputStream::close();
  }

  /** Random-at methods */
  void RAMOutputStream::seek(const long_t pos){
    OutputStream::seek(pos);
    pointer = (int)pos;
  }
  long_t RAMOutputStream::Length() {
    return file.length;
  };





  RAMInputStream::RAMInputStream(RAMFile& f):file(f) {
    pointer = 0;
    length = f.length;
  }
  RAMInputStream::RAMInputStream(RAMInputStream& clone):
    file(clone.file),
    InputStream(clone)
  {
    pointer = clone.pointer;
    length = clone.length;
  }
  RAMInputStream::~RAMInputStream(){
  }
  InputStream& RAMInputStream::clone()
  {
    RAMInputStream* ret = new RAMInputStream(*this);
    return *ret;
  }

  /** InputStream methods */
  void RAMInputStream::readInternal(l_byte_t* dest, const int idestOffset, const int len) {
    //TODO2: check this... this may be a bug....
    //if pointer is less than buffersize, and bigger than 0
    //remainder will be greater than 0 after first loop,
    //so it will try and read bufferNumber 1.
    int remainder = len - (pointer<NSLIB_STREAM_BUFFER_SIZE?pointer:0);
    int start = pointer;
    int destOffset = idestOffset;
    while (remainder != 0) {
      int bufferNumber = start/NSLIB_STREAM_BUFFER_SIZE;
      int bufferOffset = start%NSLIB_STREAM_BUFFER_SIZE;
      int bytesInBuffer = NSLIB_STREAM_BUFFER_SIZE - bufferOffset;
      int bytesToCopy = bytesInBuffer >= remainder ? remainder : bytesInBuffer;
      l_byte_t* buffer = file.buffers.at(bufferNumber);
      NSLib::util::Arrays::arraycopy(buffer, bufferOffset, dest, destOffset, bytesToCopy);

      destOffset += bytesToCopy;
      start += bytesToCopy;
      remainder -= bytesToCopy; //todo2:
    }
    pointer += len;
  }

  void RAMInputStream::close() {
    InputStream::close();
  }

  /** Random-at methods */
  void RAMInputStream::seekInternal(const long_t pos) {
    pointer = (int)pos;
  }






  /** Returns an array of strings, one for each file in the directory. */
  void RAMDirectory::list(char_t**& list, int& size) {
    size = 0;
    list = new char_t*[files.size()];

    map<const char_t*,RAMFile*,NSLib::util::charCompare>::iterator itr = files.begin();
    while (itr != files.end()){
      list[size] = (char_t*)itr->first;
      itr++;
      size++;
    }
  }

  RAMDirectory::RAMDirectory(){
    files.setDoDelete(true,NSLib::util::DELETE_TYPE_DELETE_ARRAY);
    files.setDoDelete(false,NSLib::util::DELETE_TYPE_DELETE);
  }
  RAMDirectory::~RAMDirectory(){
  }
         
  /** Returns true iff the named file exists in this directory. */
  bool RAMDirectory::fileExists(const fchar_t* name) {
    return files.exists(F_TO_CHAR_T(name));
  }

  /** Returns the time the named file was last modified. */
  long_t RAMDirectory::fileModified(const fchar_t* name) {
    return files.get(F_TO_CHAR_T(name))->lastModified;
  }

  /** Returns the length in bytes of a file in the directory. */
  long_t RAMDirectory::fileLength(const fchar_t* name) {
    return files.get(F_TO_CHAR_T(name))->length;
  }

  /** Removes an existing file in the directory. */
  void RAMDirectory::deleteFile(const fchar_t* name, const bool throwError) {
    files.remove(F_TO_CHAR_T(name));
  }

  /** Removes an existing file in the directory. */
  void RAMDirectory::renameFile(const fchar_t* from, const fchar_t* to) {
    RAMFile* file = files.get(F_TO_CHAR_T(from));
    //files.dv = false; //temporary disable delete value
    files.remove(F_TO_CHAR_T(from),false,true);
    //files.dv = true;
    files.put(F_TO_CHAR_T(to), file);
  }

  /** Creates a new, empty file in the directory with the given name.
    Returns a stream writing this file. */
  OutputStream& RAMDirectory::createFile(const fchar_t* name) {
    RAMFile& file = *new RAMFile();
    char_t* n = F_TO_CHAR_T(name);
    files.put( n, &file);
    OutputStream* ret = new RAMOutputStream(file);
    return *ret;
  }

  /** Returns a stream reading an existing file. */
  InputStream& RAMDirectory::openFile(const fchar_t* name) {
    RAMFile* file = files.get(F_TO_CHAR_T(name));
    return *new RAMInputStream( *file );
  }

  void RAMDirectory::close(){
    files.clear();
  }

  /** Construct a {@link Lock}.
  * @param name the name of the lock file
  */
  NSLock* RAMDirectory::makeLock(const fchar_t* name) {
    return new RAMLock(name,this);
  }
    
}}
