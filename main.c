/* The MAIN file of the 6502 assembler */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include "jos.h"
#include "as6502.h"
#include "id.h"
#include "code.h"
#include "compat.h"
#include "utils.h"

PUBLIC FILE *listfile = NULL;
PUBLIC int objfile = 0;
PUBLIC int pass;

PRIVATE char *flagL = NULL;
PRIVATE char *objname;

extern void yyparse();
extern FILE *yyin;
extern int total_errs;
extern void reset_input();

void scanargs(argc, argv) int argc;
char *argv[];
{
  char drive[MAXDRIVE];
  char dir[MAXDIR];
  char file[MAXFILE];
  char newpath[MAXPATH];
  int i;

  if (argc == 1) {
    puts("Usage: AS6502 <inputfile> [-l]");
    exit(1);
  }

  fnsplit(argv[1], drive, dir, file, NULL);
  fnmerge(newpath, drive, dir, file, "o65");
  objname = strdup(newpath);

  for (i = 2; i < argc; i++) {
    strupr(argv[i]);

    if (strcmp(argv[i], "-L") == 0) {
      fnmerge(newpath, drive, dir, file, "lst");
      flagL = strdup(newpath);
    } else
      fatal("Unknown flag %s", argv[i]);
  }
}

int main(argc, argv) int argc;
char *argv[];
{
  puts("as6502 -- A 6502 Assembler");
  puts("          (c) Copyright 1990-2019  Jos Visser\n");

  scanargs(argc, argv);
  yyin = fopen(argv[1], "rt");

  if (!yyin)
    perror("Cannot open input file:");

  puts("Pass 1");
  pass = 1;
  set_lc(0);
  yyparse();

  reset_input();

  if (flagL) {
    listfile = fopen(flagL, "wt");

    if (!listfile) {
      perror("Can't open list file");
      puts("Continuing without a list file");
    }
  }

  puts("Pass 2");
  pass = 2;
  set_lc(0);
  yyparse();

  fclose(yyin);
  dumptable();

  printf("\nAssembly done: %d error%s\n", total_errs,
         (total_errs == 1) ? "" : "s");

  if (listfile) {
    fprintf(listfile, "\nAssembly done: %d error%s\n", total_errs,
            (total_errs == 1) ? "" : "s");
    fclose(listfile);
  }
  unlink(objname);

  if (total_errs == 0) {
    objfile = open(objname, O_WRONLY | O_CREAT);

    if (objfile == -1)
      perror("Cannot open object file");

    writeid(objfile);
    writecode(objfile);
    close(objfile);
  }

	return 0;
}
