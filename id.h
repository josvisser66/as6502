/* Symbol table management header file */

#define EXTERNALSYM (1)
#define PUBLICSYM (2)
#define STARTSYM (4)
#define ABSOLUTE_EXP (8)

extern void *getid(const char *id);
extern void setflags(void *id, int flags);
extern void setvalue(void *id, long value);
extern long getvalue(void *id);
extern int getflags(void *id);
extern void dumptable();
extern void external_req(void *id, word at_lc);
extern void writeid(int objfile);
