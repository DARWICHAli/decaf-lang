/**
 * MEMORY
 */

#include "mips.h"
#include "typelist.h"
#include "genasm.h"

#include <assert.h>

void alloc_stack(int size) {
	if (size == 0)
		return;
	instr(ADDI, reg(SP), reg(SP), imm(-size));
}

void push_stack(struct Mips_loc loc, int size) {
	assert(size == MIPS_WORD && "Only 1 word push available for now");
	if (size == 0)
		return;

	alloc_stack(size);
	instr(SW, loc, imr(0, SP));
}

void pop_stack_args(const struct entry* fonction) {
	assert(typedesc_is_function(&fonction->type) && "expected function");
	alloc_stack(-typelist_bytesize(typedesc_function_args(&fonction->type)));
}


size_t allocated_for_func(const struct context* body_ctx) {
	const struct context* args_ctx = ctx_argsfun(body_ctx);
	return ctx_longest_path(args_ctx) - ctx_count_bytes(args_ctx) + MIPS_RESERVED;
}

void function_header(const struct context* body_ctx) {
	int allocated = allocated_for_func(body_ctx);
	alloc_stack(allocated);
	instr(SW, reg(RA), imr(allocated-MIPS_WORD, SP)); // save $ra
	instr(SW, reg(FP), imr(allocated - 2 * MIPS_WORD, SP)); // save $fp
	instr(MOVE, reg(FP), reg(SP)); // $fp indique le sommet statique
}

void function_footer(const struct context* body_ctx) {
	int allocated = allocated_for_func(body_ctx);
	save_reg_tmp();
	instr(LW, reg(RA), imr(allocated-MIPS_WORD, FP)); // restaure $ra
	instr(LW, reg(FP), imr(allocated - 2 * MIPS_WORD, FP)); // restaure $fp
	alloc_stack(-allocated);
}
