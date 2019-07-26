/* Object code handler header file */

extern void emit(byte b);
extern void emit2(word w);
extern void emit4(doubleword d);
extern byte getcode(unsigned at_lc);
extern void writecode(int objfile);
extern void set_lc(word newlc);
extern void reloc_req(unsigned at_lc);

extern int lineno;
