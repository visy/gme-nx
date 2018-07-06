/*
 * readconfig.h
 *
 * Copyright (C) 1993 Alain Knaff
 */

/* pipe handling */
#define USE_TEMP_FILE 1 /* use a temp file to store uncompressed file */
#define USE_PIPE      2 /* uncompress file to a pipe, but don't show it as a 
			 * pipe to stat */
#define SHOW_PIPE     3 /* uncompress file to a pipe, and show it as pipe to 
			 * stat */
#define DISABLE_ZLIB  4 /* don't use zlib at all */


/* readdir handling */
#define READDIR_COMPR   1 /* readdir shows files with their real name */
#define READDIR_UNCOMPR 2 /* readdir strips of trailing .z's */
extern int readdir_mode;

/* disable package for certain commands */
#define USE_ZLIB      1 /* use package */
#define DONT_USE_ZLIB


