/* Manage the assembler symbol table */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "jos.h"
#include "as6502.h"
#include "obj65.h"
#include "id.h"
#include "utils.h"

extern FILE *listfile;
extern int pass;

#define ITEMS_PER_SEGMENT (50)

#define ID ((struct item *)id)

struct extreq {
  word at_lc;
  struct extreq *next;
};

struct item {
  char id[IDLEN];
  long value;
  int flags;
  boolean assigned;
  struct extreq *root;
};

struct segment {
  int nrofitems;
  struct item item[ITEMS_PER_SEGMENT];
  struct segment *next;
};

PRIVATE struct segment *root = NULL;

PRIVATE struct segment *newseg() {
  struct segment *work = GETCORE(struct segment);

  if (work == NULL)
    fatal("Can't enlarge symbol table");

  work->nrofitems = 0;
  work->next = NULL;

  return work;
}

PRIVATE void findid(id, seg, item) char *id;
struct segment **seg;
struct item **item;
{
  register struct segment *work = root;
  struct segment *last = NULL;
  register int i;

  while (work != NULL) {
    last = work;

    for (i = 0; i < work->nrofitems; i++)
      if (strcmp(id, work->item[i].id) == 0) {
        *seg = work;
        *item = &work->item[i];
        return;
      }

    work = work->next;
  }

  *seg = last;
  *item = NULL;
}

PRIVATE struct item *addid(id, seg) char *id;
struct segment *seg;
{
  struct item *it;

  if (seg == NULL)
    seg = root = newseg();

  if (seg->nrofitems >= ITEMS_PER_SEGMENT) {
    seg->next = newseg();
    seg = seg->next;
  }

  it = &seg->item[seg->nrofitems];
  strcpy(it->id, id);
  it->value = (long)0xFFFF;
  it->flags = 0;
  it->assigned = FALSE;
  it->root = NULL;
  seg->nrofitems++;

  return it;
}

PUBLIC void setflags(id, flags) void *id;
int flags;
{
  ID->flags |= flags;

  if (ID->flags & PUBLICSYM && ID->flags & EXTERNALSYM) {
    error("Attempted assignment of conflicting attributes to symbol %s",
          ID->id);
    ID->flags = 0;
  }
}

PUBLIC void setvalue(id, value) void *id;
long value;
{
  if (ID->flags & EXTERNALSYM)
    error("Cannot assign a value to external symbol %s", ID->id);
  else {
    ID->value = value;
    ID->assigned = TRUE;
  }
}

PRIVATE void defined_id(id) struct item *id;
{
  if (pass == 1)
    return;

  if (!id->assigned && !(id->flags & EXTERNALSYM)) {
    error("Unknown symbol %s", id->id);
    id->assigned = TRUE;
  }
}

PUBLIC long getvalue(id) void *id;
{
  defined_id(id);
  return ID->value;
}

PUBLIC int getflags(id) void *id;
{
  defined_id(id);
  return ID->flags;
}

PUBLIC void dumptable() {
  register struct segment *work = root;
  register int i;

  if (!listfile)
    return;

  fputs("\nDump of symbol table:\n", listfile);
  work = root;

  while (work != NULL) {
    for (i = 0; i < work->nrofitems; i++)
      fprintf(listfile, "%-32s = %08lX\n", work->item[i].id,
              work->item[i].value);

    work = work->next;
  }
}

PUBLIC void *getid(id) char *id;
{
  struct segment *seg;
  struct item *item;

  findid(id, &seg, &item);

  if (item == NULL)
    item = addid(id, seg);

  return item;
}

PUBLIC void external_req(id, at_lc) void *id;
word at_lc;
{
  struct extreq *work;

  if (pass == 1)
    return;

  work = GETCORE(struct extreq);
  work->at_lc = at_lc;
  work->next = ID->root;
  ID->root = work;
}

PUBLIC void writeid(objfile) int objfile;
{
  register struct segment *work = root;
  register struct item *ip;
  int i;
  struct extreq *req;
  struct obj65rec orec;

  work = root;

  while (work != NULL) {
    for (i = 0; i < work->nrofitems; i++) {
      ip = &work->item[i];

      if (ip->flags & STARTSYM) {
        orec.type = STARTREC;
        orec.rec.at_lc = ip->value;
        write(objfile, &orec, sizeof(orec.rec.at_lc) + sizeof(orec.type));
      } else if (ip->flags & PUBLICSYM) {
        orec.type = PUBLICREC;
        strcpy(orec.rec.sym.id, ip->id);
        orec.rec.sym.at_lc = ip->value;
        write(objfile, &orec, sizeof(orec.rec.sym) + sizeof(orec.type));
      } else if (ip->flags & EXTERNALSYM) {
        orec.type = EXTREQREC;
        strcpy(orec.rec.sym.id, ip->id);
        req = ip->root;

        while (req) {
          orec.rec.sym.at_lc = req->at_lc;
          write(objfile, &orec, sizeof(orec.rec.sym) + sizeof(orec.type));
          req = req->next;
        }
      }
    }

    work = work->next;
  }
}
