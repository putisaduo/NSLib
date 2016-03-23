#include "StdHeader.h"
#include "FSDirectory.h"
#include <iostream>
#include "util/CharConverter.h"

namespace NSLib{ namespace store{
using namespace std;
using namespace util;

FSInputStream::FSInputStream(const fchar_t* path) 
  : isClone(false)
{
  filePath = path;
  InputStreamMap::iterator it = global_input_stream_map.find(filePath);
  if (it==global_input_stream_map.end()) {
    fhandle  = openFile(path, O_BINARY | O_RANDOM | O_RDONLY, _S_IREAD );
    if ( fhandle == -1  ) {
      cerr << "File IO Open error " << path << " " << errno << endl;
      _THROWC( "File IO Open error");
    }
    length = _filelength(fhandle);
    global_file_length[filePath] = length;
    cerr << " FSInputStream opening " << path << " with " << length << " bytes" << endl;

    l_byte_t* native_buffer = new l_byte_t[length+1];      // allocate buffer lazily
    _lseek(fhandle, 0, SEEK_SET);
    long l = _read(fhandle, native_buffer, length);
    _close(fhandle);
    global_input_stream_map[filePath] = native_buffer;
    cerr << "global_input_stream_map[" << filePath << "] has " << l << " bytes " 
         << (long)native_buffer << endl;
  } else {
    length = global_file_length[filePath];
    cerr << " FSInputStream " << path << " with " << length << " bytes" << endl;
  }
  it = global_input_stream_map.find(filePath);
  if (it==global_input_stream_map.end())
    cerr << "ERROR: missing " << filePath << " " << length << " bytes" << endl;
}
FSInputStream::FSInputStream(FSInputStream& clone):
    InputStream(clone)
{
  length = clone.length;
  filePath = clone.filePath;
  isClone = true;
}
FSInputStream::~FSInputStream(){
  close();
}

InputStream& FSInputStream::clone()
{
  //cerr << filePath << ": Clone my BufferLength=" << bufferLength << endl;
  return *new FSInputStream(*this);
}
void FSInputStream::close()  {
  InputStream::close();
}

/** InputStream methods */
void FSInputStream::readInternal(l_byte_t* b, const int offset, const int len) {
  long_t position = getFilePointer();
  int blen = (std::min)(int(length-position), len);
  l_byte_t* native_buffer = global_input_stream_map[filePath];
  //cerr << "FSInputStream::readInternal " << filePath << " " << position << ", " << blen 
  //     << "(" << (long)native_buffer << ")" << (long)b << endl;
  memcpy(b, native_buffer+position, blen);
}

FSOutputStream::FSOutputStream(const fchar_t* path){
  //_O_BINARY - Opens file in binary (untranslated) mode
  //_O_CREAT - Creates and opens new file for writing. Has no effect if file specified by filename exists
  //_O_RANDOM - Specifies that caching is optimized for, but not restricted to, random access from disk. 
  //_O_WRONLY - Opens file for writing only; 
  if ( util::Misc::dir_Exists(path) )
    fhandle  = openFile( path, O_BINARY | O_RANDOM | O_RDWR, _S_IREAD | _S_IWRITE);
  else
    fhandle  = openFile( path, O_BINARY | O_CREAT  | O_RANDOM | O_RDWR, _S_IREAD | _S_IWRITE);

  if ( fhandle == -1 )
    _THROWC( "File IO Open error");
}

FSOutputStream::~FSOutputStream(){
  if ( fhandle != 0 ){
    try{
      flush();
    }catch(...){
    }
  }
}

/** output methods: */
void FSOutputStream::flushBuffer(const l_byte_t* b, const int size) {  
  if ( size > 0 && _write(fhandle,b,size) != size )
    _THROWC( "File IO Write error");
}
void FSOutputStream::close() {
  try{
    OutputStream::close();
  }catch(...){}

  if ( _close(fhandle) != 0 )
    _THROWC( "File IO Close error");
  else
    fhandle = 0;
}

/** Random-access methods */
void FSOutputStream::seek(const long_t pos) {
  OutputStream::seek(pos);
  if ( _lseek((long_t)fhandle,pos,SEEK_SET) != 0 )
    _THROWC( "File IO Seek error");
}
long_t FSOutputStream::Length(){
  return _filelength(fhandle);
}






FSDirectory::FSDirectory(const fchar_t* path, const bool createDir)
{
  fstringCopy(directory,path);
  refCount=0;
  if (createDir)
    create();

  if (!util::Misc::dir_Exists(directory)){
#ifndef _F_UNICODE
    //char_t* wpath = util::CharConverter::charToWide(path, LANG_ENG);
#else
    size_t len = strlen(path);
    char_t* wpath = new char_t[len+1];
    fstringCopy(wpath, path);
#endif

    cerr << "ERROR: " << path << " " << " is not a directory" << endl;
///*
    //StringBuffer e(wpath);
    StringBuffer e(_T(""));
    e.append(_T(" not a directory"));
    //delete[] wpath; 
    _THROWX( e.getBuffer());
//*/
  }
}

void FSDirectory::create(){
  LOCK_MUTEX(FSDIR_CREATE);
  if ( !util::Misc::dir_Exists(directory) )
    if ( makeDirectory(directory) == -1 ){
      StringBuffer e(_T("Cannot create directory: "));
      UNLOCK_MUTEX(FSDIR_CREATE);
      _THROWX( e.getBuffer() );
    }

#ifdef _WIN32
  DIR* dir = opendir(directory); 
  fchar_t path[MAX_PATH];
  fstringCopy(path,directory);
  fstringCat(path,PATH_DELIMITER);
  fchar_t* pathP = path + fstringLength(path);
#else
#ifdef _F_UNICODE
  char* directory_temp = util::CharConverter::wideToChar(directory, "8859-1");
  char* PATH_DELIMITER_temp = util::CharConverter::wideToChar(PATH_DELIMITER, "8859-1");
  DIR* dir = opendir(directory_temp); 
  char path[MAX_PATH];
  strcpy(path,directory_temp);
  strcat(path, PATH_DELIMITER_temp);
  char* pathP = path + strlen(path);
#else
  char* directory_temp = directory;
  DIR* dir = opendir(directory_temp); 
  char path[MAX_PATH];
  strcpy(path,directory_temp);
  strcat(path, PATH_DELIMITER);
  char* pathP = path + strlen(path);
#endif
#endif
  struct dirent* fl = readdir(dir);
  struct Struct_Stat buf;

  while ( fl != NULL ){
#ifdef _WIN32
    fstringCat(pathP,fl->d_name);
    int ret = Cmd_Stat(path,&buf);
#else
    strcat(pathP, fl->d_name);
    int ret = stat(path,&buf);
#endif
    if ( buf.st_mode & S_IFDIR ) {
#ifdef _WIN32
      if ( (fstringCompare(fl->d_name, CONST_STRING("."))) && (fstringCompare(fl->d_name, CONST_STRING(".."))) ) {
        fchar_t buf[MAX_PATH];
        fstringPrintF(buf, CONST_STRING("%s/%s"),directory,fl->d_name);
        if ( unlinkFile( buf ) == -1 )
#else
      if ( (strcmp(fl->d_name, CONST_STRING("."))) && (strcmp(fl->d_name, CONST_STRING(".."))) ) {
        char buf[MAX_PATH];
        sprintf(buf, "%s/%s",directory_temp,fl->d_name);
        if ( unlink( buf ) == -1 )
#endif
        UNLOCK_MUTEX(FSDIR_CREATE);
        closedir(dir);
        _THROWC( "Couldn't delete file ");
      }
    }
    fl = readdir(dir);
  }
  closedir(dir);
  UNLOCK_MUTEX(FSDIR_CREATE);
#ifndef _WIN32
  delete directory_temp;
#ifdef _F_UNICODE
  delete PATH_DELIMITER_temp;
#endif
#endif
}

void FSDirectory::priv_getFN(fchar_t* buffer, const fchar_t* name){
  buffer[0] = 0;
  fstringCopy(buffer,directory);
  fstringCat(buffer, PATH_DELIMITER );
  fstringCat(buffer,name);
}

int FSDirectory::priv_getStat(const fchar_t* name, struct Struct_Stat* ret){
  fchar_t buffer[MAX_PATH];
  priv_getFN(buffer,name);
  return Cmd_Stat( buffer, ret );
}

FSDirectory::~FSDirectory(){
  //DIRECTORIES.remove( F_TO_CHAR_T(getDirName()));
  DIRECTORIES.erase( getDirName());
}

bool FSDirectory::fileExists(const fchar_t* name){
  fchar_t fl[MAX_PATH];
  priv_getFN(fl, name);
  return util::Misc::dir_Exists( fl );
}

fchar_t* FSDirectory::getDirName(){
  return directory;
}

//static
FSDirectory& FSDirectory::getDirectory(const fchar_t* file, const bool create){
  LOCK_MUTEX(DIRECTORIES_MUTEX);
  if ( DIRECTORIES.count(file) ==1  ){
    FSDirectory* itm = (FSDirectory*)DIRECTORIES[file];
    if ( create )
      itm->create();
    
    LOCK_MUTEX(itm->LOCK_MUTEX);
    itm->refInc();
    UNLOCK_MUTEX(itm->LOCK_MUTEX);
    
    UNLOCK_MUTEX(DIRECTORIES_MUTEX);
    return *itm;
  }
  UNLOCK_MUTEX(DIRECTORIES_MUTEX);
  
  FSDirectory* fs = new FSDirectory(file, create);
  
  LOCK_MUTEX(fs->LOCK_MUTEX);
  fs->refInc();
  UNLOCK_MUTEX(fs->LOCK_MUTEX);

  DIRECTORIES[file]= fs;
  return *fs;
}

long_t FSDirectory::fileModified(const fchar_t* name) {
  struct Struct_Stat buf;
  if ( priv_getStat(name,&buf) == -1 )
    return 0;
  else
    return buf.st_mtime;
} 
      
//static
long_t FSDirectory::fileModified(const fchar_t* dir, const fchar_t* name){
  struct Struct_Stat buf;
  fchar_t buffer[MAX_PATH];
  fstringCopy(buffer,dir);
  fstringCat(buffer,PATH_DELIMITER);
  fstringCat(buffer,name);
  Cmd_Stat( buffer, &buf );
  return buf.st_mtime;
}

long_t FSDirectory::fileLength(const fchar_t* name){
  struct Struct_Stat buf;
  if ( priv_getStat(name, &buf) == -1 )
    return 0;
  else
    return buf.st_size;
}

void FSDirectory::deleteFile(const fchar_t* name, const bool throwError)  {
  fchar_t fl[MAX_PATH];
  priv_getFN(fl, name);
  if ( unlinkFile(fl) == -1 && throwError){
    char_t buffer[200];
    stringPrintF(buffer, _T("couldn't delete %s"),name);
    _THROWX( buffer );
  }
  fchar_t fLen[MAX_PATH];
  fstringCopy(fLen,name);
  fstringCat(fLen,CONST_STRING(".len"));
  unlinkFile(fLen);
}

void FSDirectory::renameFile(const fchar_t* from, const fchar_t* to){
  LOCK_MUTEX(FSDIR_RENAME);
  fchar_t old[MAX_PATH];
  priv_getFN(old, from);

  fchar_t nu[MAX_PATH];
  priv_getFN(nu, to);

  fchar_t oldLen[MAX_PATH];
  fchar_t nuLen[MAX_PATH];
  fstringCopy(oldLen,old);
  fstringCat(oldLen,CONST_STRING(".len"));
  fstringCopy(nuLen,nu);
  fstringCat(nuLen,CONST_STRING(".len"));

  /* This is not atomic.  If the program crashes between the call to
  delete() and the call to renameTo() then we're screwed, but I've
  been unable to figure out how else to do this... */
  
  if ( util::Misc::dir_Exists(nu) )
    if( unlinkFile(nu) != 0 ){
      char_t buffer[200];
      stringPrintF(buffer, _T("couldn't delete %s"),to);
      
      UNLOCK_MUTEX(FSDIR_RENAME);
      _THROWX( buffer );
    }
    unlinkFile(nuLen);
  if ( fileRename(old,nu) != 0 ){
      char_t buffer[200];
      stringPrintF(buffer, _T("couldn't rename %s to %s"), from, to );
      UNLOCK_MUTEX(FSDIR_RENAME);
    _THROWX( buffer );
  }
  fileRename(oldLen,nuLen);
  UNLOCK_MUTEX(FSDIR_RENAME);
}

OutputStream& FSDirectory::createFile(const fchar_t* name) {
  fchar_t fl[MAX_PATH];
  priv_getFN(fl, name);

  return *new FSOutputStream( fl );
}

InputStream& FSDirectory::openFile(const fchar_t* name) {
  fchar_t fl[MAX_PATH];
  priv_getFN(fl, name);
  return *new FSInputStream( fl );
}

NSLock* FSDirectory::makeLock(const fchar_t* name) {
  fchar_t dr[MAX_PATH];
  priv_getFN(dr,name);
  return new FSLock( dr );
}

void FSDirectory::refInc(){
  refCount ++;
}

void FSDirectory::close() {
  LOCK_MUTEX(FSDIR_CLOSE);
  if (--refCount <= 0) {
    LOCK_MUTEX(DIRECTORIES_MUTEX);
    Directory* d = DIRECTORIES[getDirName()];
    if ( d != NULL ){
      DIRECTORIES.erase( getDirName());
      delete d;
    }
    UNLOCK_MUTEX(DIRECTORIES_MUTEX);
  }else{
    UNLOCK_MUTEX(FSDIR_CLOSE);
  }
}




FSLock::FSLock ( const fchar_t* name ):
  fname( fstringDuplicate(name) )
{
}
FSLock::~FSLock(){
  delete[] fname;
}
bool FSLock::obtain() {
  if ( util::Misc::dir_Exists(fname) ) {
    std::cerr << " ERROR: " << fname << " exists already!" << endl;
    return false;
  }
  
  int r = openFile(fname,  O_RDWR | O_CREAT, _S_IREAD | _S_IWRITE);
  if ( r == -1 )
    return false;

  _close(r);
  return true;
}
void FSLock::release() {
  unlinkFile( fname );
}

}}

