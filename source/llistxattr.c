/*
 * llistxattr.c
 *
 * Copyright (C) 1993 Alain Knaff
 */
#define _LARGEFILE64_SOURCE
#define _GNU_SOURCE

#include "sysincludes.h"

#ifdef HAVE_GETXATTR
int llistxattr(__const char *file_name, char *list, size_t size)
{
  int st;
  char newname[MAXPATHLEN + MAXEXTLEN + 1];

  _zlibc_init();
  st=zlib_real_llistxattr(file_name, list, size);

  if ( st >= 0 || errno != ENOENT )
    return st;

  zlib_initialise();
  if ( zlib_mode & CM_DISAB )
    return st;
  if ( (zlib_getfiletype(file_name,-1) & PM_READ_MASK) == PM_LEAVE_COMPR)
    return st;
  
  if ( zlib_mode & CM_VERBOSE )
    fprintf(stderr,"Getxattr %s\n",file_name);
  
  strncpy(newname,file_name,1024);
  strcat(newname,zlib_ext);
  
  errno = 0;
  return zlib_real_llistxattr(newname, list, size);
}

#endif
