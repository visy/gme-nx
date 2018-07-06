#ifndef ZLIBC_DIRECT_SYS_H
#define ZLIBC_DIRECT_SYS_H

#if defined RTLD_NEXT && defined __ELF__

extern int (*zlib_real_access)(const char *, int);
extern int (*zlib_real_chmod) (const char *, mode_t);
extern int (*zlib_real_chown) (const char *, uid_t, gid_t);
extern int (*zlib_real_getdents) (unsigned int, struct dirent *, unsigned int);

#ifdef SYS_getdents64
extern int (*zlib_real_getdents64) (unsigned int, struct dirent64 *, unsigned int);
#endif

extern int (*zlib_real_link) (const char *, const char *);

extern int (*zlib_real_open) (const char *, int, mode_t);
#ifdef linux
extern FILE* (*zlib_real_fopen) (const char *, const char *mode);
extern FILE* (*zlib_real_fopen64) (const char *, const char *mode);
#endif
extern int (*zlib_real_prev_stat) (const char *, struct stat *);
extern struct dirent *(*zlib_real_readdir) (DIR *);
# ifdef HAVE_READDIR64
extern struct dirent64 *(*zlib_real_readdir64) (DIR *);
# endif
extern int (*zlib_real_readlink) (const char *, char *, size_t);
extern int (*zlib_real_rename) (const char *, const char *);

# ifdef HAVE___XSTAT
extern int (*zlib_real_xstat) (int vers, const char *, struct stat *);
extern int (*zlib_real_lxstat) (int vers, const char *, struct stat *);
# else
extern int (*zlib_real_stat) (const char *, struct stat *);
extern int (*zlib_real_lstat) (const char *, struct stat *);
# endif

# ifdef HAVE___XSTAT64
extern int (*zlib_real_xstat64) (int vers, const char *, struct stat64 *);
extern int (*zlib_real_lxstat64) (int vers, const char *, struct stat64 *);
# elif defined HAVE_STAT64
extern int (*zlib_real_stat64) (const char *, struct stat64 *);
extern int (*zlib_real_lstat64) (const char *, struct stat64 *);
# endif


extern int (*zlib_real_symlink) (const char *, const char *);
extern int (*zlib_real_unlink) (const char *);
extern int (*zlib_real_utime) (const char *, struct utimbuf *);
extern int (*zlib_real_utimes) (const char *, struct timeval *);

#ifdef HAVE_GETXATTR
extern ssize_t (*zlib_real_getxattr) (const char *, const char *, void *, size_t);
extern ssize_t (*zlib_real_lgetxattr) (const char *, const char *, void *, size_t);
extern int (*zlib_real_setxattr) (const char *, const char *, const void *, 
				  size_t, int);
extern int (*zlib_real_lsetxattr) (const char *, const char *, const void *, 
				   size_t, int);
extern ssize_t (*zlib_real_listxattr) (const char *, char *, size_t);
extern ssize_t (*zlib_real_llistxattr) (const char *, char *, size_t);
extern int (*zlib_real_removexattr) (const char *, const char *);
extern int (*zlib_real_lremovexattr) (const char *, const char *);
#endif

#else /* RTLD_NEXT */

#ifdef aix

#include "aix-syscall.h"

#else /* aix */

#define zlib_real_access(fn, mode)	(syscall(SYS_access, (fn), (mode)))
#define zlib_real_chmod(fn, mode)	(syscall(SYS_chmod,(fn), (mode)))
#define zlib_real_chown(fn, owner, group)	(syscall(SYS_chown,(fn),(owner),(group)))

#define zlib_real_getdents(fd, dirp, count)	(syscall(SYS_getdents, (fd), (dirp), (count)))

#ifdef SYS_getdents64
#define zlib_real_getdents64(fd, dirp, count)	(syscall(SYS_getdents64, (fd), (dirp), (count)))
#endif

#define zlib_real_link(fn1, fn2)		(syscall(SYS_link, (fn1), (fn2)))

#define zlib_real_lstat(fn, buf )		(syscall(SYS_lstat, (fn), (buf)))
#define zlib_real_open(fn,flags,mode)	(syscall(SYS_open, (fn), (flags), (mode)))

#ifdef linux
struct dirent *__libc_readdir(DIR * dir);
#define zlib_real_readdir(dir)		(__libc_readdir(dirp))
#else
#define zlib_real_readdir(dirp)		((struct dirent *)syscall(SYS_readdir,(dirp)))
/* if needed define SYS_readdir so that readdir gets compiled */
#endif

#define zlib_real_readlink(fn,buf,len)	(syscall(SYS_readlink, (fn), (buf), (len)))
#define zlib_real_rename(fn1, fn2)	(syscall(SYS_rename, (fn1), (fn2)))
#define zlib_real_stat(fn, buf )	(syscall(SYS_stat, (fn), (buf)))
#define zlib_real_symlink(fn1, fn2)	(syscall(SYS_symlink, (fn1), (fn2)))
#define zlib_real_unlink(fn)		(syscall(SYS_unlink, (fn)))
#define zlib_real_utime(fn, buf)	(syscall(SYS_utime, (fn), (buf)))
#define zlib_real_utimes(fn, buf)	(syscall(SYS_utimes, (fn), (buf)))


#ifdef HAVE_GETXATTR
#define zlib_real_getxattr(fn,name,value,size)	(syscall(SYS_getxattr, (fn), (name), (value), (size)))
#define zlib_real_lgetxattr(fn,name,value,size)	(syscall(SYS_lgetxattr, (fn), (name), (value), (size)))
#define zlib_real_setxattr(fn,name,value,size,flags)	(syscall(SYS_setxattr, (fn), (name), (value), (size), (flags)))
#define zlib_real_lsetxattr(fn,name,value,size,flags)	(syscall(SYS_lsetxattr, (fn), (name), (value), (size), (flags)))
#define zlib_real_listxattr(fn,list,size)	(syscall(SYS_listxattr, (fn), (list), (size)))
#define zlib_real_llistxattr(fn,list,size)	(syscall(SYS_llistxattr, (fn), (list), (size)))
#define zlib_real_removexattr(fn,name)	(syscall(SYS_removexattr,(fn),(name)))
#define zlib_real_lremovexattr(fn,name)	(syscall(SYS_lremovexattr,(fn),(name)))
#endif

#ifdef linux
/* java */
int __libc_read();
int __libc_close();

#define read(a,b,c) __libc_read(a,b,c)
#define close(a) __libc_close(a)
#define write(a,b,c) __libc_read(a,b,c)
#endif /* linux */


#endif /* aix */

#endif /* RTLD_NEXT */

#endif /* ZLIBC_DIRECT_SYS_H */
