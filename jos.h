#define PRIVATE         static
#define PUBLIC

typedef int             boolean;
#define TRUE            1
#define FALSE           0

#include <sys/types.h>

#define GETCORE(x)      ((x *)malloc(sizeof(x)))
