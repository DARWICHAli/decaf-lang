/**
 * QUADS
 */

#include "mips.h"

void mips_Q_ADD(const struct entry* res, const struct entry* lhs, const struct entry* rhs) {
	struct Mips_loc rl = entry_to_reg(lhs);
	struct Mips_loc rh = entry_to_reg(rhs);
	struct Mips_loc rr = available_register(res);
	instr(ADD, rr, rl, rh);
}

void mips_Q_SUB(const struct entry* res, const struct entry* lhs, const struct entry* rhs) {
	struct Mips_loc rl = entry_to_reg(lhs);
	struct Mips_loc rh = entry_to_reg(rhs);
	struct Mips_loc rr = available_register(res);
	instr(SUB, rr, rl, rh);
}

void mips_Q_MUL(const struct entry* res, const struct entry* lhs, const struct entry* rhs) {
	struct Mips_loc rl = entry_to_reg(lhs);
	struct Mips_loc rh = entry_to_reg(rhs);
	struct Mips_loc rr = available_register(res);
	instr(MUL, rr, rl, rh);
}

void mips_Q_DIV(const struct entry* res, const struct entry* lhs, const struct entry* rhs) {
	struct Mips_loc rl = entry_to_reg(lhs);
	struct Mips_loc rh = entry_to_reg(rhs);
	struct Mips_loc rr = available_register(res);
	instr(DIV, rr, rl, rh);
}

void mips_Q_MOD(const struct entry* res, const struct entry* lhs, const struct entry* rhs) {
	struct Mips_loc rl = entry_to_reg(lhs);
	struct Mips_loc rh = entry_to_reg(rhs);
	struct Mips_loc rr = available_register(res);
	instr(REM, rr, rl, rh);
}

void mips_Q_AFF(const struct entry* res, const struct entry* val) {
	struct Mips_loc rv = entry_to_reg(val);
	struct Mips_loc rr = available_register(res);
	instr(MOVE, rr, rv);
}

void mips_Q_NEG(const struct entry* res, const struct entry* val) {
	struct Mips_loc rv = entry_to_reg(val);
	struct Mips_loc rr = available_register(res);
	instr(NEGU, rr, rv);
}

void mips_Q_CST(const struct entry* res, int cst) {
	struct Mips_loc rr = available_register(res);
	instr(LI, rr, imm(cst));
}

void mips_Q_END(const struct context* ctx) {
	function_footer(ctx);
	instr(JR, reg(RA));
	clear_reg_tmp();
}

void mips_Q_RET(const struct context* ctx, const struct entry* result) {
	function_footer(ctx);
	if (entry_in_tmp(result)) { // intelligent load
		instr(MOVE, reg(V0), entry_to_reg(result));
	} else {
		instr(LW, reg(V0), entry_loc(result));
	}
	instr(JR, reg(RA));
	clear_reg_tmp();
}

void mips_Q_PAR(const struct entry* arg) {
	push_stack(entry_to_reg(arg), td_sizeof(&arg->type));
}

void mips_Q_CAL(const struct entry* res, const struct entry* fct) {
	save_reg_tmp();
	clear_reg_tmp();
	if (fct->ctx->parent == NULL) { // builtin
		call_builtin(fct->id);
	} else {
		instr(JAL, entry_loc(fct));
		instr(MOVE, available_register(res), reg(V0));
	}
	pop_stack_args(fct);
}

void mips_Q_PRO(const struct entry* pro) {
	save_reg_tmp();
	clear_reg_tmp();
	if (pro->ctx->parent == NULL) { // builtin
		call_builtin(pro->id);
	} else {
		instr(JAL, entry_loc(pro));
	}
	pop_stack_args(pro);
}

void mips_Q_GOT(quad_id_t dst) {
	save_reg_tmp();
	clear_reg_tmp();
	instr(B, quad_loc(dst));
}

static inline void eswap(const struct entry** lhs, const struct entry** rhs) {
	const struct entry* tmp = *lhs;
	*lhs = *rhs;
	*rhs = tmp;
}

void mips_Q_IFG(const struct entry* lhs, enum CMP_OP cop, const struct entry* rhs, quad_id_t dst) {
	save_reg_tmp();
	clear_reg_tmp();

	switch (cop) {
		case CMP_EQ:
			instr(BEQ, entry_to_reg(lhs), entry_to_reg(rhs), quad_loc(dst));
			break;
		case CMP_NQ:
			instr(BNE, entry_to_reg(lhs), entry_to_reg(rhs), quad_loc(dst));
			break;

		case CMP_GE:
			eswap(&rhs, &lhs); // a >= b eq b <= a
			// fall through
		case CMP_LE:
			instr(BLE, entry_to_reg(lhs), entry_to_reg(rhs), quad_loc(dst));
			break;

		case CMP_GT:
			eswap(&rhs, &lhs); // a >= b eq b <= a
			// fall through
		case CMP_LT:
			instr(BLT, entry_to_reg(lhs), entry_to_reg(rhs), quad_loc(dst));
			break;
	}
}
