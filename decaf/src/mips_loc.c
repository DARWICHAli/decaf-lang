/*
 * MIPS LOC
 */

#include "mips.h"
#include "genasm.h"
#include "incomplete.h"

#include <assert.h>

struct Mips_loc reg(enum Mips_reg reg) { struct Mips_loc ret = {.adr = REG, .reg = reg }; return ret; }
struct Mips_loc imr(int imm, enum Mips_reg reg) { struct Mips_loc ret = {.adr = IMR, .imm = imm, .reg = reg }; return ret; }
struct Mips_loc imm(int imm) { struct Mips_loc ret = {.adr = IMM, .imm = imm }; return ret; }
struct Mips_loc sym(const char* sym) { struct Mips_loc ret = {.adr = SYM, .sym = sym }; return ret; }


void print_loc(struct Mips_loc loc) {
	switch (loc.adr) {
		case REG:
			assert(loc.reg > 0 && loc.reg < INVALID_REGISTER && "MIPS: Unknown register");
			fprintf(out, "$%s", Mips_reg_str[loc.reg]);
			break;
		case IMR:
			assert(loc.reg > 0 && loc.reg < INVALID_REGISTER && "MIPS: Unknown register");
			fprintf(out, "%d($%s)", loc.imm, Mips_reg_str[loc.reg]);
			break;
		case IMM:
			fprintf(out, "%d", loc.imm);
			break;
		case SYM:
			assert((loc.sym || loc.qid) && "Bad symbol");
			if (loc.sym) {
				fprintf(out, "%s", loc.sym);
			} else {
				fprintf(out, LBL_QUAD_FMT, loc.qid);
			}
			break;
		// LCOV_EXCL_START
		default:
			assert(0 && "MIPS: bad operand type");
		// LCOV_EXCL_STOP
	}
}

struct Mips_loc entry_loc(const struct entry* ent) {
	assert(ent && "ctx_get_access expects non-NULL entry");
	assert(ent->ctx && "Entry must have context");
	assert((typedesc_is_function(&ent->type) || ent->ctx->parent) && "No variable allowed in super-global table");
	if (ent->ctx->parent == NULL || ent->ctx->parent->parent == NULL) { // racine ou super-global
		assert(ctx_search(ent->id, ent->ctx) && "Entry exists in its context");
		return sym(ent->id);
	} else if (contains_func(ent->ctx->parent)) { // recherche du décalage pour argument
		int decal = ctx_byte_idx(ent); // arguments empilés à l'envers, donc arg1 au sommet !
		return imr(decal + allocated_for_func(ent->ctx), FP);
	} else { // variale locale
		int decal = ctx_byte_idx(ent);
		const struct context* it = ent->ctx;
		while (!contains_func(it->parent->parent)) {
			decal += ctx_count_bytes(it->parent);
			assert(it->parent != NULL && "Code not allowed outside of a function");
			it = it->parent;
		}
		return imr(decal, FP);
	}
}

struct Mips_loc quad_loc(quad_id_t qid) {
	assert(qid != INCOMPLETE_QUAD_ID && "Unexpected incomplete quad");
	struct Mips_loc ret = { .adr = SYM, .sym = NULL, .qid = qid };
	return ret;
}
