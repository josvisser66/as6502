/* Utility procedures definition file */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "jos.h"
#include "as6502.h"

#define MAXERRS 10

PRIVATE int errptr;
PRIVATE char *errtxt[MAXERRS];
PUBLIC int total_errs = 0;

extern int pass;
extern int lineno;
extern FILE *listfile;

PUBLIC int yywrap() { return 1; }

PUBLIC void yyerror(s) char *s;
{}

PRIVATE int digit(d) char d;
{
  if (d >= '0' && d <= '9')
    return (int)(d - '0');
  else if (d >= 'a' && d <= 'z')
    return 10 + (int)(d - 'a');
  else if (d >= 'A' && d <= 'Z')
    return 10 + (int)(d - 'A');
  else
    return 0;
}

PUBLIC long calcnum(text, base) char text[];
int base;
{
  register int i;
  long result = 0L;

  for (i = 1; text[i]; i++) {
    result = result * base + digit(text[i]);
  }

  return result;
}

PUBLIC void fatal(char *s, ...) {
  va_list args;
  char line[132];

  va_start(args, s);
  vsprintf(line, s, args);
  va_end(args);
  printf("FATAL: %s\n", line);
  exit(999);
}

PUBLIC void error(char *s, ...) {
  if (pass == 1)
    return;

  if (errptr == MAXERRS)
    return;

  va_list args;
  char line[132];

  vsprintf(line, s, args);
  va_end(args);
  errtxt[errptr] = strdup(line);
  errptr++;
  total_errs++;
}

PUBLIC void dequeue_errs() {
  int i, j;

  for (i = 0; i < errptr; i++) {
    if (listfile)
      fprintf(listfile, "**** %s\n", errtxt[i]);

    printf("at line %4d: %s\n", lineno, errtxt[i]);
    free(errtxt[i]);
  }

  errptr = 0;
}
