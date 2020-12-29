#ifndef __FS_UTILS_H__
#define __FS_UTILS_H__
#include <stdio.h>

extern char *exePath;

FILE *fopen_home(const char *filename, const char *mode);
FILE *fopen_conf(const char *filename, const char *mode);
FILE *fopen_save(const char *filename, const char *mode);

#endif /* __FS_UTILS_H__ */