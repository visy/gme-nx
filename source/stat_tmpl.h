static inline int STAT(___zlibc_lx)(int ver, const char *file_name,
				    STATTYPE *buf)
{
#ifdef USE_XSTAT
  return STAT(zlib_real_lx)(ver, file_name, buf);
#else
  return STAT(zlib_real_l)(file_name, buf);   
#endif
}


static inline int STAT(___zlibc_x)(int ver, const char *file_name,
				   STATTYPE *buf)
{
#ifdef USE_XSTAT
  return STAT(zlib_real_x)(ver, file_name, buf);
#else
  return STAT(zlib_real_)(file_name, buf );
#endif
}
