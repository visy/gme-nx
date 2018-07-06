/*
 * lsetxattr.c
 *
 * Copyright (C) 1993 Alain Knaff
 */
#define _LARGEFILE64_SOURCE
#define _GNU_SOURCE

#include "sysincludes.h"

#ifdef HAVE_SETXATTR
int lsetxattr(__const char *file_name, const char *name, 
	      const void *value, size_t size, int flags)
{
  int st;
  char newname[MAXPATHLEN + MAXEXTLEN + 1];

  _zlibc_init();
  st=zlib_real_lsetxattr(file_name, name, value, size, flags);

  if ( st >= 0 || errno != ENOENT )
    return st;

  zlib_initialise();
  if ( zlib_mode & CM_DISAB )
    return st;
  if ( (zlib_getfiletype(file_name,-1) & PM_READ_MASK) == PM_LEAVE_COMPR)
    return st;
  
  if ( zlib_mode & CM_VERBOSE )
    fprintf(stderr,"Setxattr %s\n",file_name);
  
  strncpy(newname,file_name,1024);
  strcat(newname,zlib_ext);
  
  errno = 0;
  return zlib_real_lsetxattr(newname, name, value, size, flags);
}

#endif
