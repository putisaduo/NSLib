#include "StdHeader.h"
#ifdef REQUIRE_DIRENT
#include <io.h>
#include "dirent.h"

namespace NSLib{ namespace util {

#ifdef _WIN32
/*
* opendir
*
* Returns a pointer to a DIR structure appropriately filled in to begin
* searching a directory.
*/
DIR * opendir (const fchar_t *szPath)
{
  DIR *nd;
  fchar_t szFullPath[MAX_PATH];

  errno = 0;

  if (!szPath) {
    errno = EFAULT;
    return NULL;
  }

  if (szPath[0] == '\0') {
    errno = ENOTDIR;
    return NULL;
  }

  /* Attempt to determine if the given path really is a directory. */
  struct Struct_Stat rcs;
  if ( Cmd_Stat(szPath,&rcs) == -1) {
    /* call GetLastError for more error info */
    errno = ENOENT;
    return NULL;
  }
  if (!(rcs.st_mode & _S_IFDIR)) {
    /* Error, entry exists but not a directory. */
    errno = ENOTDIR;
    return NULL;
  }

  /* Make an absolute pathname.  */
  fileFullName (szFullPath, szPath, MAX_PATH);

  /* Allocate enough space to store DIR structure and the complete
  * directory path given. */
  //nd = (DIR *) malloc (sizeof (DIR) + fstringLength (szFullPath) + fstringLength (DIRENT_SLASH) +
  //          fstringLength (DIRENT_SEARCH_SUFFIX)+1);
  nd = new DIR;

  if (!nd) {
    /* Error, out of memory. */
    errno = ENOMEM;
    return NULL;
  }

  /* Create the search expression. */
  fstringCopy (nd->dd_name, szFullPath);

  /* Add on a slash if the path does not end with one. */
  if (nd->dd_name[0] != '\0' &&
    nd->dd_name[fstringLength (nd->dd_name) - 1] != '/' &&
    nd->dd_name[fstringLength (nd->dd_name) - 1] != '\\')
  {
    fstringCat (nd->dd_name, DIRENT_SLASH);
  }

  /* Add on the search pattern */
  fstringCat (nd->dd_name, DIRENT_SEARCH_SUFFIX);

  /* Initialize handle to -1 so that a premature closedir doesn't try
  * to call _findclose on it. */
  nd->dd_handle = -1;

  /* Initialize the status. */
  nd->dd_stat = 0;

  /* Initialize the dirent structure. ino and reclen are invalid under
  * Win32, and name simply points at the appropriate part of the
  * findfirst_t structure. */
  //nd->dd_dir.d_ino = 0;
  //nd->dd_dir.d_reclen = 0;
  nd->dd_dir.d_namlen = 0;
  nd->dd_dir.d_name = nd->dd_dta.name;

  return nd;
}


/*
* readdir
*
* Return a pointer to a dirent structure filled with the information on the
* next entry in the directory.
*/
struct dirent * readdir (DIR * dirp)
{
  errno = 0;

  /* Check for valid DIR struct. */
  if (!dirp) {
    errno = EFAULT;
    return NULL;
  }

  if (dirp->dd_dir.d_name != dirp->dd_dta.name) {
    /* The structure does not seem to be set up correctly. */
    errno = EINVAL;
    return NULL;
  }

  if (dirp->dd_stat < 0) {
    /* We have already returned all files in the directory
    * (or the structure has an invalid dd_stat). */
    return NULL;
  } else if (dirp->dd_stat == 0) {
    /* We haven't started the search yet. */
    /* Start the search */
    dirp->dd_handle = CmdFindFirst (dirp->dd_name, &(dirp->dd_dta));

    if (dirp->dd_handle == -1) {
      /* Whoops! Seems there are no files in that
      * directory. */
      dirp->dd_stat = -1;
    } else {
      dirp->dd_stat = 1;
    }
  } else {
    /* Get the next search entry. */
    if (CmdFindNext (dirp->dd_handle, &(dirp->dd_dta))) {
      /* We are off the end or otherwise error. */
      _findclose (dirp->dd_handle);
      dirp->dd_handle = -1;
      dirp->dd_stat = -1;
    } else {
      /* Update the status to indicate the correct
      * number. */
      dirp->dd_stat++;
    }
  }

  if (dirp->dd_stat > 0) {
    /* Successfully got an entry. Everything about the file is
    * already appropriately filled in except the length of the
    * file name. */
    dirp->dd_dir.d_namlen = fstringLength (dirp->dd_dir.d_name);

    if ( dirp->dd_dir.d_name[0] == '.' &&
      (dirp->dd_dir.d_name[1] == 0 || 
        (dirp->dd_dir.d_name[1] == '.' && dirp->dd_dir.d_name[2] == 0)))
        return readdir(dirp);

    struct _stat buf;
    fchar_t buffer[MAX_PATH];
    int bl = fstringLength(dirp->dd_name)-fstringLength(DIRENT_SEARCH_SUFFIX);
    fstringNCopy(buffer,dirp->dd_name,bl);
    buffer[bl]=0;
    fstringCat(buffer, dirp->dd_dir.d_name);
    if ( Cmd_Stat(buffer,&buf) == -1 )
      return readdir(dirp);

    return &dirp->dd_dir;
  }

  return NULL;
}


/*
* closedir
*
* Frees up resources allocated by opendir.
*/
int closedir (DIR * dirp)
{
  int rc;

  errno = 0;
  rc = 0;

  if (!dirp) {
    errno = EFAULT;
    return -1;
  }

  if (dirp->dd_handle != -1) {
    rc = _findclose (dirp->dd_handle);
  }

  /* Delete the dir structure. */
  delete dirp;

  return rc;
}

#endif

}}
#endif

