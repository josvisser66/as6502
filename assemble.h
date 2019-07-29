/* Header file for the assembler module */

typedef enum {
  IMPLIED,
  ACCUMULATOR,
  ABSOLUTE,
  ZERO_PAGE,
  IMMEDIATE,
  ABSX,
  ABSY,
  INDX,
  INDY,
  ZERO_PAGEX,
  ZERO_PAGEY,
  RELATIVE,
  INDIRECT
} adrmode;

extern void assemble(const char *mnem, adrmode adrmode, int external, void *id,
                     long num, int absolute);
