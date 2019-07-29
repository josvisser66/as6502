/* Turbo C compatibility routines */

#include <ctype.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "compat.h"
#include "jos.h"

PUBLIC char *strupr(char *s) {
  char *t;

  for (t = s; *t; ++t) {
    *t = toupper(*t);
  }

  return s;
}

PUBLIC void fnsplit(const char *filename, char *drive, char *dir, char *file,
                    char *ext) {
  char *fname;

  if (drive) {
    *drive = 0;
  }

  if (dir) {
    fname = strdup(filename);
    strcpy(dir, dirname(fname));
    free(fname);
  }

  if (file) {
    fname = strdup(filename);
    strcpy(file, basename(fname));
    free(fname);
  }

  char *dot = strrchr(file, '.');

  if (!dot) {
    if (ext) {
      *ext = 0;
    }
  } else {
    if (ext) {
      strcpy(ext, dot + 1);
    }
    *dot = 0;
  }
}

PUBLIC void fnmerge(char *filename, const char *drive, const char *dir,
                    const char *file, const char *ext) {
  sprintf(filename, "%s/%s.%s", dir, file, ext);
}

PUBLIC size_t filelength(int fhandle) {
  struct stat buf;

  if (fstat(fhandle, &buf) < 0) {
    perror("Cannot stat file handle");
    exit(1);
  }

  return buf.st_size;
}
