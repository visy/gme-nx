/**
 * Stat wrappers for zlibc's own usage, i.e. when zlibc itself wants
 * to probe existence or attributes of a file, rather than just
 * proxying a request from an application
 */
static inline int ___zlibc_testexistence(__const char *name) {
  struct stat buf;
#ifdef HAVE___XSTAT
  return zlib_real_lxstat(_STAT_VER, name, &buf);
#else
  return zlib_real_lstat(file_name, &buf);
#endif
}

static inline int ___zlibc_nativestat(__const char *name, struct stat *buf) {
#ifdef HAVE___XSTAT
  return zlib_real_xstat(_STAT_VER, name, buf);
#else
  return zlib_real_stat(file_name, buf);
#endif
}
