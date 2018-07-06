/* commands with one filename and one "other" argument */
#define ALIAS(alias, type) \
int alias(char *filename, type var);

ALIAS(zlib_real_access, int)
ALIAS(zlib_real_chmod, int)

ALIAS(zlib_real_stat, struct stat*)
ALIAS(zlib_real_lstat, struct stat*)

#ifdef SYS_UTIME
ALIAS(zlib_real_utime, struct utimbuf *)
#endif

#ifdef SYS_UTIMES
int utimes();
ALIAS(zlib_real_utimes, struct timeval *)
#endif

#undef ALIAS


/* commands with two filenames */
#define ALIAS(alias) \
int alias(const char *filename, const char *filename2);

ALIAS(zlib_real_link);
ALIAS(zlib_real_symlink);
ALIAS(zlib_real_rename);

#undef ALIAS

int zlib_real_chown(const char *filename, int o, int g);
int zlib_real_open(const char *pathname, int flags, mode_t mode);
int zlib_real_unlink(const char *pathname);
int zlib_real_readlink(const char *path, char *buf, size_t bufsiz);
