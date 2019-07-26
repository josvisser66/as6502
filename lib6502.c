/* Library manager for the 6502 set */

#include "jos.h"
#include "as6502.h"
#include "obj65.h"
#include "compat.h"

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

struct lib {
  char modname[13];
  unsigned filelen;
  void *thefile;
  struct lib *next;
  int deleted;
};

char *libname = NULL;
char *flagL = NULL;
struct lib *root = NULL;

void fatal(s, args) char *s;
char args;
{
  char line[132];

  vsprintf(line, s, &args);
  printf("FATAL: %s\n", line);
  exit(999);
}

#define READ(a, b, c)                                                          \
  if (read(a, b, c) != c)                                                      \
  fatal("Library corrupt")
#define WRITE(a, b, c)                                                         \
  if (write(a, b, c) != c)                                                     \
  fatal("Error writing library")

void writeout2(libmem, handle) struct lib *libmem;
int handle;
{
  if (!libmem)
    return;
  else
    writeout2(libmem->next, handle);

  if (libmem->deleted)
    return;

  WRITE(handle, libmem->modname, 13);
  WRITE(handle, (char *)&libmem->filelen, 2);
  WRITE(handle, libmem->thefile, libmem->filelen);
}

void writeout() {
  int handle = open(libname, O_WRONLY | O_TRUNC);

  writeout2(root, handle);
  close(handle);
}

void bringin() {
  struct lib *work;
  int readd;
  int handle = open(libname, O_RDONLY | O_CREAT , 255);
  char modname[13];

  if (handle == -1) {
    perror("Library open error");
    exit(999);
  }

  while (1) {
    readd = read(handle, modname, 13);

    if (readd == 0)
      break;

    if (readd != 13)
      fatal("Library corrupt");

    work = GETCORE(struct lib);
    work->next = root;
    root = work;
    work->deleted = 0;
    strcpy(work->modname, modname);
    READ(handle, (char *)&work->filelen, 2);
    work->thefile = malloc(work->filelen);

    READ(handle, work->thefile, work->filelen);
  }

  close(handle);
}

struct lib *bringin2a(modname) char *modname;
{
  struct lib *work;
  char drive[MAXDRIVE];
  char dir[MAXDIR];
  char file[MAXFILE];
  char newpath[MAXPATH];
  char ext[MAXEXT];
  int handle = open(modname, O_RDONLY );

  work = GETCORE(struct lib);

  work->filelen = work->deleted = 0;
  work->next = work->thefile = NULL;
  fnsplit(modname, drive, dir, file, ext);
  fnmerge(newpath, NULL, NULL, file, ext);
  strcpy(work->modname, newpath);

  if (handle != -1) {
    work->filelen = filelength(handle);
    work->thefile = malloc(work->filelen);

    if (read(handle, work->thefile, work->filelen) != work->filelen)
      fatal("Object file %s corrupt", modname);

    close(handle);
  }

  return work;
}

struct lib *bringin2(modname) char *modname;
{
  char drive[MAXDRIVE];
  char dir[MAXDIR];
  char file[MAXFILE];
  char newpath[MAXPATH];
  char ext[MAXEXT];
  struct lib *work;

  work = bringin2a(modname);

  if (work->thefile)
    return work;

  fnsplit(modname, drive, dir, file, ext);

  if (ext[0])
    return work;

  fnmerge(newpath, drive, dir, file, ".O65");
  free(work);
  work = bringin2a(newpath);
  return work;
}

struct lib *search(name) char *name;
{
  struct lib *walk;

  walk = root;

  while (walk) {
    if (strcmp(walk->modname, name) == 0 && !walk->deleted)
      return walk;

    walk = walk->next;
  }

  return NULL;
}

#define EXISTS(a)                                                              \
  if (!((a)->thefile))                                                         \
  fatal("Object file %s not found", (a)->modname)

void oadd(obj) struct lib *obj;
{
  EXISTS(obj);

  if (search(obj->modname))
    fatal("Module %s already in library", obj->modname);

  obj->next = root;
  root = obj;
}

void oupdate(obj) struct lib *obj;
{
  struct lib *work;

  EXISTS(obj);
  work = search(obj->modname);

  if (!work)
    oadd(obj);

  work->filelen = obj->filelen;
  free(work->thefile);
  work->thefile = obj->thefile;
  free(obj);
}

void odelete(obj) struct lib *obj;
{
  struct lib *work = search(obj->modname);

  if (!work)
    fatal("Module %s not in library", obj->modname);

  work->deleted = 1;
}

void ocopy(obj) struct lib *obj;
{
  struct lib *work = search(obj->modname);
  int handle;

  if (!work)
    fatal("Module %s not in library", obj->modname);

  handle = open(obj->modname, O_WRONLY | O_CREAT | O_TRUNC , 255);

  if (write(handle, work->thefile, work->filelen) != work->filelen)
    fatal("Error copying module %s from library", obj->modname);

  close(handle);
}

void perform(obj, action) struct lib *obj;
char *action;
{
  if (strcmp(action, "+ ") == 0)
    oadd(obj);
  else if (strcmp(action, "- ") == 0)
    odelete(obj);
  else if (strcmp(action, "* ") == 0)
    ocopy(obj);
  else if (strcmp(action, "+-") == 0)
    oupdate(obj);
  else
    fatal("Invalid action %s on %s", action, obj);
}

void doit(argc, argv) int argc;
char *argv[];
{
  char drive[MAXDRIVE];
  char dir[MAXDIR];
  char file[MAXFILE];
  char newpath[MAXPATH];
  char ext[MAXEXT];
  int i;
  char *par;
  char action[3];
  struct lib *obj;

  if (argc == 1) {
    puts("Usage: LIB6502 <libfile> [([+][-][+-]<objfile>)*] [-l]");
    exit(1);
  }

  fnsplit(argv[1], drive, dir, file, ext);

  if (ext[0] == '\0')
    strcpy(ext, ".L65");

  fnmerge(newpath, drive, dir, file, ext);
  libname = strdup(newpath);
  bringin();

  for (i = 2; i < argc; i++) {
    par = argv[i];
    strupr(par);

    if (strcmp(par, "-L") == 0) {
      fnsplit(libname, drive, dir, file, ext);
      fnmerge(newpath, drive, dir, file, ".LST");
      flagL = strdup(newpath);
    } else {
      strcpy(action, "  ");
      action[0] = *par;

      if (*par == '+' && *(par + 1) == '-') {
        par++;
        action[1] = '-';
      }

      if (*par == '-' && *(par + 1) == '+') {
        par++;
        action[0] = '+';
        action[1] = '-';
      }

      obj = bringin2(++par);
      perform(obj, action);
    }
  }

  writeout();
}

void main(argc, argv) int argc;
char *argv[];
{
  puts("LIB6502 -- The 6502 library manager");
  puts("           (c) Copyright 1990-2019  Jos Visser\n");

  doit(argc, argv);
}
