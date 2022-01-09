#include "quadops.h"
#include "gencode.h"
#include "entry.h"

#include <assert.h>

#define NUM_QOP_BIN 5
// quads arithmétique
int is_arith(enum Q_OP qop)
{
	static enum Q_OP arith_quads[NUM_QOP_BIN] = { Q_ADD, Q_SUB, Q_MUL, Q_DIV, Q_MOD };
	for (int i = 0; i < NUM_QOP_BIN; ++i) {
		if (qop == arith_quads[i])
			return 1;
	}
	return 0;
}

struct quad quad_arith(const struct entry* res, const struct entry* lhs, enum Q_OP qop, const struct entry* rhs)
{
	assert(is_arith(qop) && "Expected binary operator");
	assert(res && "quad_arith expecting NON null result entry");
	assert(lhs && "quad_arith expecting NON null lhs entry");
	assert(rhs && "quad_arith expecting NON null rhs entry");
	struct quad q = { .op = qop, .res = res, .lhs = lhs, .rhs = rhs };
	return q;
}

struct quad quad_aff(const struct entry* res, const struct entry* val)
{
	assert(res && "quad_aff expecting NON null result entry");
	assert(val && "quad_aff expecting NON null value entry");
	struct quad q = { .op = Q_AFF, .res = res, .lhs = val };
	return q;
}

struct quad quad_neg(const struct entry* res, const struct entry* val)
{
	assert(res && "quad_neg expecting NON null result entry");
	assert(val && "quad_neg expecting NON null value entry");
	struct quad q = { .op = Q_NEG, .res = res, .lhs = val };
	return q;
}

struct quad quad_goto(quad_id_t qid)
{
	assert((qid == INCOMPLETE_QUAD_ID || getquad(qid) != NULL) && "Expected either incomplete or good destination");
	struct quad q = { .op = Q_GOT, .dst = qid };
	return q;
}

struct quad quad_ifgoto(const struct entry* lhs, enum CMP_OP cmp_op, const struct entry* rhs, quad_id_t qid)
{
	assert(lhs && "quad_ifgoto expecting NON null lhs entry");
	assert(rhs && "quad_ifgoto expecting NON null rhs entry");
	assert((qid == INCOMPLETE_QUAD_ID || getquad(qid) != NULL) && "Expected either incomplete or good destination");
	struct quad q = { .op = Q_IFG, .lhs = lhs, .cmp = cmp_op, .rhs = rhs, .dst = qid };
	return q;
}

struct quad quad_param(const struct entry* var)
{
	assert(var && "quad_param expecting NON null var entry");
	struct quad q = { .op = Q_PAR, .lhs = var };
	return q;
}

struct quad quad_call(const struct entry* res, const struct entry* fct)
{
	assert(res && "quad_call expecting NON null result entry");
	assert(fct && "quad_call expecting NON null function entry");
	struct quad q = { .op = Q_CAL, .res = res, .lhs = fct };
	return q;
}

struct quad quad_proc(const struct entry* proc)
{
	assert(proc && "quad_proc expecting NON null procedure entry");
	struct quad q = { .op = Q_PRO, .lhs = proc };
	return q;
}

struct quad quad_cst(const struct entry* var, int cst)
{
	assert(var && "quad_cst expecting NON null var entry");
	struct quad q = { .op = Q_CST, .res = var, .val = cst };
	return q;
}

struct quad quad_return(const struct entry* ret)
{
	assert(ret && "quad_return expecting NON null return entry");
	struct quad q = { .op = Q_RET, .lhs = ret };
	return q;
}

struct quad quad_endproc()
{
	struct quad q = { .op = Q_END };
	return q;
}
