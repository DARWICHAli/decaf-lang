/**
 * MIPS INSTRUCTIONS
 */

#include "mips.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

int Mips_op_nums[INVALID] = {3, 3, 3, 3, 2, 3, 3, 3, 
	2, 
	2, 2, 2,
	1, 1,
	0 };

enum Mips_adr Mips_op_fmt[INVALID][3] = { {REG, REG, REG}, {REG, REG, IMM}, {REG, REG, REG}, {REG, REG, REG}, {REG, REG, -1}, {REG, REG, REG}, {REG, REG, REG}, {REG, REG, REG}, 
	{REG, IMM, -1},
	{REG, IMR, -1},{REG, IMR, -1},{REG, REG, -1},
	{SYM, -1, -1}, {REG, -1, -1},
	{-1, -1, -1}};

const char* Mips_op_str[INVALID] = { "add", "addi", "div", "mul", "negu", "rem", "sub", "xor", 
	"li",
	"lw", "sw", "move",
	"jal", "jr",
	"syscall" };

void instr(enum Mips_op op, ...) {
	assert(op >= 0 && op < INVALID && "Unknown MIPS operator");
	
	fprintf(out, "%s", Mips_op_str[op]);

	va_list vl;
	va_start(vl, op);
	struct Mips_loc loc;
	for (int i = 0; i < Mips_op_nums[op]; ++i) {
		loc = va_arg(vl, struct Mips_loc);
		assert(loc.adr == Mips_op_fmt[op][i] && "MIPS: bad operand");
		fprintf(out, ", ");
		print_loc(loc);
	}
	va_end(vl);

	fprintf(out, "\n");
}

void instrc(enum Mips_op op, ...) {
	assert(op >= 0 && op < INVALID && "Unknown MIPS operator");
	
	fprintf(out, "%s", Mips_op_str[op]);

	va_list vl;
	va_start(vl, op);
	struct Mips_loc loc;
	for (int i = 0; i < Mips_op_nums[op]; ++i) {
		loc = va_arg(vl, struct Mips_loc);
		assert(loc.adr == Mips_op_fmt[op][i] && "MIPS: bad operand");
		fprintf(out, " ");
		print_loc(loc);
	}

	const char* com = va_arg(vl, const char*);
	fprintf(out, " # %s\n", com);

	va_end(vl);
}

void mips_label(const char* lbl, const char* com) {
	if (strcmp(com, "") == 0) {
		fprintf(out, "%s:\n", lbl);
	} else {
		fprintf(out, "%s: #%s\n", lbl, com);
	}
}
