/*
 * zlibc.h
 *
 * Copyright (C) 1993 Alain Knaff
 */

#ifndef ZLIBC_H
#define ZLIBC_H

#define FA_ALL	    0

#define FA_DIR      1
#define FA_SUBDIR   2
/* FA_DIR and FA_SUBDIR should be specified with their trailing slash */

#define FA_BASENAME 3
#define FA_SUFFIX   4
#define FA_FILENAME 5
#define FA_ALL2     6
#define FA_FS	    7

#define PM_NONE    0

#define PM_USE_TMP_FILE 1
#define PM_HIDE_PIPE	2
#define PM_SHOW_PIPE	3
#define PM_DIR_LEAVE_COMPR	4
#define PM_LEAVE_COMPR 	5
#define PM_READ_MASK    7

#define PM_CREATE_COMPR 0x8
#define PM_NOCREATE_COMPR 0x10
#define PM_CREATE_MASK 0x18

#define PM_APPEND_COMPR 0x20
#define PM_NOAPPEND_COMPR 0x40
#define PM_APPEND_MASK 0x60

#define PM_UNCOMPR_BEFORE_WRITE 0x80
#define PM_NO_UNCOMPR_BEFORE_WRITE 0x100
#define PM_UNCOMPR_MASK 0x180

#define PM_SIZE_COMPR 0x200
#define PM_SIZE_UNCOMPR 0x400
#define PM_SIZE_COMPR_MASK 0x600

typedef struct FilenameActions {
    /* initialized elements.
       MUST be in this order in order to support compiled-in defaults */
    int fa_type;
    char *name;
    int namelength;
    int pipe_mode;

    /* elements that are computed at runtime */
    dev_t dev;
    ino_t ino;
    int is_initialized;
} FilenameActions;

int _zlibc_init(void);

/* the file actions have been read */
#define CM_HAVE_FA 0x1

/* disable package altogether */
#define CM_DISAB 0x2

/* readdir lists compressed files */
#define CM_READDIR_COMPR 0x4

/* print comments for each intercepted syscall */
#define CM_VERBOSE 0x8

/* unlinks compressed files when user programs asks to unlink 
 * uncompressed file */
#define CM_UNLINK 0x10

/* doesn't read run time configuration file */
#define CM_NORTCONF 0x20

/* doesn't read run time configuration file */
#define CM_DISAB_CHILD 0x40

/* all available file-settable modes or'ed together, except NORTCONF */
#define CM_ALL_MODES 0x5f

/* none of these flags */
#define CM_NONE 0

typedef struct CommandActions {
  char *name;
  int cm_type;  
  FilenameActions *actions;
} CommandActions;


extern CommandActions zlib_commandActions[];

#define FREC(a,b,c) { a, b, sizeof(b) - 1, c }

#if 0

#ifdef HAVE___ACCESS
#define access __access
#endif

#ifdef HAVE___CHMOD
#define chmod __chmod
#endif

#ifdef HAVE___CHOWN
#define chown __chown
#endif

#ifdef HAVE___LSTAT
#define lstat __lstat
#endif

#ifdef HAVE___STAT
#define lstat __stat
#endif

#if 0
#ifdef HAVE___OPEN
#define open __open
#endif

#ifdef HAVE___READLINK
#define readlink __readlink
#endif

#ifdef HAVE___UNLINK
#define unlink __unlink
#endif

#ifdef HAVE___LINK
#define link __link
#endif

#ifdef HAVE___SYMLINK
#define symlink __symlink
#endif
#endif

#endif

#ifndef __STDC__
#define signed /**/
int fprintf();
int printf();
int _flsbuf();
int syscall();
int wait4();
int fclose();
int perror();
int getdents();
#endif


void zlib_initialise(void);
int zlib_getfiletype(__const char *name, int fd);
void zlib_getuserconf(char *progname, 
		      FilenameActions **filenameActions, 
		      int *mode, int *modemask);

extern char *zlib_ext;
extern int zlib_extlen;
extern int zlib_mode;
extern char *zlib_tmp;
extern char **zlib_uncompressor;




#ifdef CONFIG_COMPILER
extern char *err_strings[];
extern int generation;

/* configuration file compiler */
typedef struct commands_line {
  char *name;
  int mode, modemask;
  struct commands_line *globalnext, *localnext;
  int generation;
  char class[16];
  int defined;
} commands_line;

int add_prog(commands_line **cmd, char *name);
int add_default(commands_line **cmd);

void print_class(FilenameActions *fa, char *name, int line);
int parse_file(char *progname, 
	       FILE *f,
	       int *line,
	       int *mode,
	       int *modemask,
	       char *classname);
#endif

/**
 * Stat calling stubs.  These stubs are used to allow zlibc to call the
 * libc's original stat functions
 */
#ifdef HAVE___XSTAT
# include <sys/stat.h>
# include "direct_sys.h"

/* 32 bit versions */
# define STAT(x) x ## stat
# define STATTYPE struct stat
# define USE_XSTAT 1
# include "stat_tmpl.h"
# undef USE_XSTAT
# undef STATTYPE
# undef STAT

#else
# include "direct_sys.h"
# define ___zlibc_stat(ver,name,buf) (zlib_real_stat((name), (buf)))
# define ___zlibc_lstat(ver,name,buf) (zlib_real_lstat((name), (buf)))
#endif


/* 64 bit versions */
#ifdef HAVE_STAT64
# define STATTYPE struct stat64
# define STAT(x) x ## stat64
# ifdef HAVE___XSTAT64
#  define USE_XSTAT 1
# endif
# include "stat_tmpl.h"
# undef USE_XSTAT
# undef STAT
# undef STATTYPE
#endif


#ifndef DEFUN
#define AND ,
#define DEFUN(name, arglist, args) name(args)
#endif


#ifndef function_alias
#ifdef linux
#define	function_alias(alias, name, type, args, defun) \
__asm__(".globl " #alias "; " #alias " = " #name);
#else
#define	function_alias(alias, name, type, args, defun) \
type defun { return name args; }
#endif
#endif

#if (defined __ELF__ && defined elf_alias)
#undef function_alias
#define function_alias(alias, name, type, vars, defun) elf_alias(name, alias)
#endif

#define ALIAS1(name, alias, rtype, type1) \
  function_alias(name, alias, rtype, \
		 (var1), \
		 DEFUN(alias, (var1), \
		       type1 var1))

#define ALIAS2(name, alias, rtype, type1, type2) \
  function_alias(alias, name, rtype, \
		 (var1, var2), \
		 DEFUN(alias, (var1, var2), \
		       type1 var1 AND type2 var2))

#define ALIAS3(name, alias, rtype, type1, type2, type3) \
  function_alias(alias, name, rtype, \
		 (var1, var2, var3), \
		 DEFUN(alias, (var1, var2, var3), \
		       type1 var1 AND type2 var2 AND type3 var3))

#define ALIASF0(name, alias) ALIAS1(name, alias, int, CONST char *)
#define ALIASF1(name, alias, type2) \
  ALIAS2(name, alias, int, CONST char *, type2)
#define ALIASF2(name, alias, type2, type3) \
  ALIAS3(name, alias, int, CONST char *, type2, type3)

#define ALIAS2F(name, alias) \
  ALIAS2(name, alias, int, CONST char *, CONST char *)

#endif
