/*
 * getxattr.c
 *
 * Copyright (C) 1993 Alain Knaff
 */
#define _LARGEFILE64_SOURCE
#define _GNU_SOURCE

#include "sysincludes.h"

#ifdef HAVE_GETXATTR
ssize_t getxattr(__const char *file_name, const char *name, 
		 void *value, size_t size)
{
  int st;
  char newname[MAXPATHLEN + MAXEXTLEN + 1];

  _zlibc_init();
  st=zlib_real_getxattr(file_name, name, value, size);

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
  return zlib_real_getxattr(newname, name, value, size);
}

#endif
