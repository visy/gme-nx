/*
 * utimes.c
 *
 * Copyright (C) 1993 Alain Knaff
 */
#define _LARGEFILE64_SOURCE
#define _GNU_SOURCE

#include "sysincludes.h"


#ifdef SYS_UTIMES

int utimes(__const char *file_name, struct timeval *buf)
{
  int st;
  char newname[MAXPATHLEN + MAXEXTLEN + 1];

  _zlibc_init();
  st=zlib_real_utimes(file_name, buf);

  if ( st >= 0 || errno != ENOENT )
    return st;

  zlib_initialise();
  if ( zlib_mode & CM_DISAB )
    return st;
  if ( (zlib_getfiletype(file_name,-1) & PM_READ_MASK) == PM_LEAVE_COMPR)
    return st;
  
  if ( zlib_mode & CM_VERBOSE )
    fprintf(stderr,"Utiming %s\n",file_name);
  
  strncpy(newname,file_name,1024);
  strcat(newname,zlib_ext);
  
  errno = 0;
  st=zlib_real_utimes(newname, buf);
  if ( st < 0 && errno == EINVAL )
    errno = ENOENT;
  return st;
}

#endif
