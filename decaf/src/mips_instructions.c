/**
 * MIPS INSTRUCTIONS
 */

#include "mips.h"

#include "incomplete.h"
#include "genasm.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

int Mips_op_nums[INVALID] = {3, 3, 3, 3, 2, 3, 3, 3, 
	2, 2,
	2, 2, 2,
	1, 1,
	1, 3, 3, 3, 3,
	0 };

enum Mips_adr Mips_op_fmt[INVALID][3] = { {REG, REG, REG}, {REG, REG, IMM}, {REG, REG, REG}, {REG, REG, REG}, {REG, REG, -1}, {REG, REG, REG}, {REG, REG, REG}, {REG, REG, REG}, 
	{REG, IMM, -1}, {REG, SYM, -1},
	{REG, IMR, -1},{REG, IMR, -1},{REG, REG, -1},
	{SYM, -1, -1}, {REG, -1, -1},
	{SYM, -1, -1},{REG, REG, SYM},{REG, REG, SYM},{REG, REG, SYM},{REG, REG, SYM},
	{-1, -1, -1}};

const char* Mips_op_str[INVALID] = { "add", "addi", "div", "mul", "negu", "rem", "sub", "xor", 
	"li", "la",
	"lw", "sw", "move",
	"jal", "jr",
	"b", "beq", "bne", "ble", "blt",
	"syscall" };

void instrinfos(enum Mips_op op, va_list args) {
	int toosay = 0;
	for (int i = 0; i < Mips_op_nums[op]; ++i) {
		struct Mips_loc loc = va_arg(args, struct Mips_loc);
		if (loc.adr == REG || loc.adr == IMR) {
			for (int j = 0; j < MIPS_REG_TMP_NB; ++j) {
				if (Mips_reg_temp[j] == loc.reg && Mips_reg_storing[j] != NULL) {
					fprintf(out, "%s%s(%s) ", toosay ? "" : " # ", Mips_reg_str[loc.reg], Mips_reg_storing[j]->id);
					toosay = 1;
					break;
				}
			}
		}
	}
}


void instr(enum Mips_op op, ...) {
	assert(op >= 0 && op < INVALID && "Unknown MIPS operator");
	
	fprintf(out, "%s", Mips_op_str[op]);

	va_list vl, fw;
	va_start(vl, op);
	va_copy(fw, vl);
	struct Mips_loc loc;
	for (int i = 0; i < Mips_op_nums[op]; ++i) {
		loc = va_arg(vl, struct Mips_loc);
		assert(loc.adr == Mips_op_fmt[op][i] && "MIPS: bad operand");
		fprintf(out, (i > 0) ? ", " : " ");
		print_loc(loc);
	}

	if (global_params->verbose) {
		instrinfos(op, fw);
	}
	fprintf(out, "\n");
	va_end(fw);
	va_end(vl);
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
		fprintf(out, (i > 0) ? ", " : " ");
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

void mips_label_quad(quad_id_t qid) {
	assert(qid != INCOMPLETE_QUAD_ID && "can't label incomplete quad");
	fprintf(out, LBL_QUAD_FMT ":\n", qid);
}
