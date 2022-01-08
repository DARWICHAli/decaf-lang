/**
 * MIPS BUILTINS
 */

#include "mips.h"

#include <string.h>

const char* Mips_builtins_decaf[MIPS_BUILTINS_NB] = { "WriteInt", "WriteString", "ReadInt", "ReadString", "Exit" };

enum Mips_builtins Mips_builtins_syscall[MIPS_BUILTINS_NB] = { PRINT_INT, PRINT_STRING, READ_INT, READ_STRING, EXIT };

/*
 * fonctions
 */

void call_builtin(const char* builtin) {
	for (int i = 0; i < MIPS_BUILTINS_NB; ++i) {
		if (strcmp(Mips_builtins_decaf[i], builtin) == 0) {
			instr(LW, reg(A0), imr(0, SP));
			instr(LI, reg(V0), imm(Mips_builtins_syscall[i]));
			instrc(SYSCALL, Mips_builtins_decaf[i]);
			return;
		}
	}
}

