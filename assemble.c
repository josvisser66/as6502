/* Definition file for the module that takes care of the assembly */

#include <string.h>

#include "as6502.h"
#include "assemble.h"
#include "code.h"
#include "id.h"
#include "jos.h"
#include "utils.h"

#define NR_OF_MNEMS (56)

struct instr {
  char mnem[4];
  int opcode[INDIRECT + 1];
};

extern word lc;
extern int pass;

PRIVATE struct instr astab[NR_OF_MNEMS] = {
    {"ADC",
     {-1, -1, 0x6d, 0x65, 0x69, 0x7d, 0x79, 0x61, 0x71, 0x75, -1, -1, -1}},
    {"AND",
     {-1, -1, 0x2d, 0x25, 0x29, 0x3d, 0x39, 0x21, 0x31, 0x35, -1, -1, -1}},
    {"ASL", {-1, 0x0a, 0x0e, 0x06, -1, 0x1e, -1, -1, -1, 0x16, -1, -1, -1}},
    {"BCC", {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x90, -1}},
    {"BCS", {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x80, -1}},
    {"BEQ", {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0xf0, -1}},
    {"BIT", {-1, -1, 0x2c, 0x24, -1, -1, -1, -1, -1, -1, -1, -1, -1}},
    {"BMI", {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x30, -1}},
    {"BNE", {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0xD0, -1}},
    {"BPL", {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x10, -1}},
    {"BRK", {0x00, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}},
    {"BVC", {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x50, -1}},
    {"BVS", {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x70, -1}},
    {"CLC", {0x18, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}},
    {"CLD", {0xd8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}},
    {"CLI", {0x58, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}},
    {"CLV", {0xb8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}},
    {"CMP",
     {-1, -1, 0xcd, 0xc5, 0xc9, 0xdd, 0xd9, 0xc1, 0xd1, 0xd5, -1, -1, -1}},
    {"CPX", {-1, -1, 0xec, 0xe4, 0xe0, -1, -1, -1, -1, -1, -1, -1, -1}},
    {"CPY", {-1, -1, 0xcc, 0xc4, 0xc0, -1, -1, -1, -1, -1, -1, -1, -1}},
    {"DEC", {-1, -1, 0xce, 0xc6, -1, 0xde, -1, -1, -1, 0xd6, -1, -1, -1}},
    {"DEX", {0xca, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}},
    {"DEY", {0x88, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}},
    {"EOR",
     {-1, -1, 0x4d, 0x45, 0x49, 0x5d, 0x59, 0x41, 0x51, 0x55, -1, -1, -1}},
    {"INC", {-1, -1, 0xee, 0xe6, -1, 0xfe, -1, -1, -1, 0xf6, -1, -1, -1}},
    {"INX", {0xe8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}},
    {"INY", {0xc8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}},
    {"JMP", {-1, -1, 0x4c, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x6c}},
    {"JSR", {-1, -1, 0x20, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}},
    {"LDA",
     {-1, -1, 0xad, 0xa5, 0xa9, 0xbd, 0xb9, 0xa1, 0xb1, 0xb5, -1, -1, -1}},
    {"LDX", {-1, -1, 0xae, 0xa6, 0xa2, -1, 0xbe, -1, -1, -1, 0xb6, -1, -1}},
    {"LDY", {-1, -1, 0xac, 0xa4, 0xa0, 0xbc, -1, -1, -1, 0xb4, -1, -1, -1}},
    {"LSR", {-1, 0x4a, 0x4e, 0x46, -1, 0x5e, -1, -1, -1, 0x56, -1, -1, -1}},
    {"NOP", {0xea, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}},
    {"ORA",
     {-1, -1, 0x0d, 0x05, 0x09, 0x1d, 0x19, 0x01, 0x11, 0x15, -1, -1, -1}},
    {"PHA", {0x48, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}},
    {"PHP", {0x08, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}},
    {"PLA", {0x68, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}},
    {"PLP", {0x28, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}},
    {"ROL", {-1, 0x2a, 0x2e, 0x26, -1, 0x3e, -1, -1, -1, 0x36, -1, -1, -1}},
    {"ROR", {-1, 0x6a, 0x6e, 0x66, -1, 0x7e, -1, -1, -1, 0x76, -1, -1, -1}},
    {"RTI", {0x40, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}},
    {"RTS", {0x60, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}},
    {"SBC",
     {-1, -1, 0xed, 0xe5, 0xe9, 0xfd, 0xf9, 0xe1, 0xf1, 0xf5, -1, -1, -1}},
    {"SEC", {0x38, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}},
    {"SED", {0xf8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}},
    {"SEI", {0x78, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}},
    {"STA", {-1, -1, 0x8d, 0x85, -1, 0x9d, 0x99, 0x81, 0x91, 0x95, -1, -1, -1}},
    {"STX", {-1, -1, 0x8e, 0x86, -1, -1, -1, -1, -1, -1, 0x96, -1, -1}},
    {"STY", {-1, -1, 0x8c, 0x84, -1, -1, -1, -1, -1, 0x94, -1, -1, -1}},
    {"TAX", {0xaa, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}},
    {"TAY", {0xa8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}},
    {"TSX", {0xba, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}},
    {"TXA", {0x8a, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}},
    {"TXS", {0x9a, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}},
    {"TYA", {0x98, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}}};

PRIVATE void bad_adrmode() { error("Illegal addressing mode"); }

#define CHECK_ADRMODE  \
  if (opcode == -1) {  \
    bad_adrmode();     \
    return;            \
  }

PUBLIC void assemble(char *mnem, adrmode adrmode, boolean external, void *id,
                     long num, boolean absolute) {
  register int i;
  int displ;
  struct instr *ip;
  int opcode;
  int true_adrmode;
  int sec;

  for (i = 0; strcmp(mnem, astab[i].mnem) != 0 && i < NR_OF_MNEMS; i++)
    ;

  if (i == NR_OF_MNEMS)
    fatal("Unknown mnemonic %s passed lexical analysis", mnem);

  ip = &astab[i];
  opcode = ip->opcode[adrmode];

  if (external)
    external_req(id, lc + 1);

  if (ip->opcode[RELATIVE] != -1) { /* branch instr */
    if (adrmode != ABSOLUTE) {
      bad_adrmode();
      return;
    } else {
      if (external) {
        error("Target of branch instruction may not be externally defined");
        return;
      }
    }
  } else {
    emit(ip->opcode[RELATIVE]);
    displ = (int)(num - (long)lc + 1L);

    if (displ > 127 || displ < -128)
      error("Target of branch out of reach");

    emit(displ & 255);
    return;
  }

  switch (adrmode) {
  case IMPLIED:
  case ACCUMULATOR:
    CHECK_ADRMODE;
    emit(opcode);
    return;

  case INDIRECT:
    CHECK_ADRMODE;
    emit(opcode);
    emit2(num);
    return;

  case IMMEDIATE:
  case INDX:
  case INDY:
    CHECK_ADRMODE;
    emit(opcode);
    emit(num);
    return;

  case ABSOLUTE:
    sec = ZERO_PAGE;
    break;

  case ABSX:
    sec = ZERO_PAGEX;
    break;

  case ABSY:
    sec = ZERO_PAGEY;

  default:
    fatal("Unexpected addressing mode: ", adrmode);
  }

  true_adrmode = adrmode;

  if (!external) {
    if (pass == 1) {
      if (num < 256) {
        true_adrmode = sec;
      }
    } else {
      opcode = getcode(lc);

      if (opcode == ip->opcode[sec]) {
        true_adrmode = sec;
      } else if (opcode != ip->opcode[adrmode]) {
        fatal("Internal error in assemble()");
      }
    }

    emit(ip->opcode[true_adrmode]);

    if (!absolute) {
      reloc_req(lc);
		}

    if (adrmode == true_adrmode) {
      emit2(num);
    } else {
      emit(num % 256);
		}
  }
}
