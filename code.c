/* Object code handler */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "jos.h"
#include "as6502.h"
#include "obj65.h"
#include "utils.h"

struct reloc {
  unsigned at_lc;
  struct reloc *next;
};

PRIVATE byte *code = NULL;
PRIVATE word codeseglen = 0;
PRIVATE word highwater = 1;
PRIVATE struct reloc *reloc_root = NULL;

PUBLIC word lc = 0;

extern int lineno;
extern char listline[];
extern FILE *listfile;
extern int pass;

PRIVATE void newpage() {
  register int i;
  register unsigned newlen = codeseglen + 256;

  if (code)
    code = realloc(code, newlen);
  else
    code = malloc(newlen);

  if (!code)
    fatal("Not enough memory (code table)");

  for (i = 0; i < 256; i++)
    *(code + codeseglen + i) = (byte)0;

  codeseglen = newlen;
}

PUBLIC void emit(b) unsigned char b;
{
  register int i;
  char s[4];

  while (lc >= codeseglen)
    newpage();

  *(code + lc) = b;
  lc++;

  if (lc >= highwater)
    highwater = lc + 1;

  if (!listfile)
    return;

  sprintf(s, "%02X", (int)b);

  for (i = 7; i < 18; i += 3)
    if (listline[i] == ' ') {
      listline[i] = s[0];
      listline[i + 1] = s[1];
      return;
    }

  fputs(listline, listfile);
  sprintf(listline, "%04X   %02X          %4d\n", lc - 1, (int)b, lineno);
}

PUBLIC void emit2(w) word w;
{
  emit(w % 256);
  emit(w / 256);
}

PUBLIC void emit4(d) doubleword d;
{
  emit2(d % 65536);
  emit2(d / 65536);
}

PUBLIC byte getcode(at_lc) word at_lc;
{
  while (lc >= codeseglen)
    newpage();

  return *(code + at_lc);
}

PUBLIC void writecode(objfile) int objfile;
{
  struct reloc *work = reloc_root;
  struct obj65rec orec;

  orec.type = RELOCREC;

  while (work) {
    orec.rec.at_lc = work->at_lc;
    write(objfile, &orec, sizeof(orec.rec.at_lc) + sizeof(orec.type));
    work = work->next;
  }

  orec.type = CODEREC;
  orec.rec.len = highwater - 1;
  write(objfile, &orec, sizeof(orec.rec.len) + sizeof(orec.type));
  write(objfile, code, highwater - 1);
}

PUBLIC void set_lc(newlc) word newlc;
{
  lc = newlc;

  if (lc >= highwater)
    highwater = lc + 1;
}

PUBLIC void reloc_req(at_lc) unsigned at_lc;
{
  struct reloc *work;

  if (pass == 1)
    return;

  work = GETCORE(struct reloc);
  work->at_lc = at_lc;
  work->next = reloc_root;
  listline[5] = '*';
  reloc_root = work;
}
