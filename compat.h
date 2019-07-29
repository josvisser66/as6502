/* Functions that were there in Turbo C but not in Linux or MacOS */

extern char *strupr(char *s);
extern void fnsplit(const char *filename, char *drive, char *dir, char *file,
                    char *ext);
extern void fnmerge(char *filename, const char *drive, const char *dir,
                    const char *file, const char *ext);
extern size_t filelength(int fhandle);

#include <limits.h>

#define MAXDRIVE 1
#define MAXEXT PATH_MAX
#define MAXDIR PATH_MAX
#define MAXFILE PATH_MAX
#define MAXPATH PATH_MAX
