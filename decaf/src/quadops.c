#include "quadops.h"
#include "gencode.h"
#include "entry.h"
#include "incomplete.h"

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

struct quad quad_acc(const struct entry* res, const struct entry* tab, const struct entry* idx)
{
	assert(res && "quad_aff expecting NON null result entry");
	assert(tab && "quad_aff expecting NON null tab entry");
	assert(idx && "quad_aff expecting NON null idx entry");
	struct quad q = { .op = Q_ACC, .res = res, .lhs = tab, .rhs = idx };
	return q;
}

struct quad quad_aft(const struct entry* tab, const struct entry* idx, const struct entry* val)
{
	assert(tab && "quad_aff expecting NON null tab entry");
	assert(idx && "quad_aff expecting NON null idx entry");
	assert(val && "quad_aff expecting NON null value entry");
	struct quad q = { .op = Q_AFT, .res = tab, .lhs = idx, .rhs = val };
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

struct quad quad_access(const struct entry* res, const struct entry* tab, const struct entry* idx) {
	assert(res && "quad_access expecting NON null result entry");
	assert(tab && typedesc_is_tab(&tab->type) && "quad_access tab must be an array");
	assert(idx && "quad_acess expecting NON null index entry");
	struct quad q = { .op = Q_ACC, .res = res, .lhs = tab, .rhs = idx };
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

const char* binop_str(enum Q_OP op) {
	switch (op) {
		case Q_ADD:
			return "+";
		case Q_SUB:
			return "-";
		case Q_MUL:
			return "*";
		case Q_DIV:
			return "/";
		case Q_MOD:
			return "%";
		default:
			return "UNKNOWN BINOP";
	}
}

const char* cmp_str(enum CMP_OP cmp) {
	switch (cmp) {
		case CMP_EQ:
			return "==";
		case CMP_NQ:
			return "!=";
		case CMP_GE:
			return ">=";
		case CMP_LE:
			return "<=";
		case CMP_GT:
			return ">";
		case CMP_LT:
			return "<";
		default:
			return "UNKNOWN RELOP";
	}
}

void quad_fprint(FILE* fo, const struct quad* q) {
	switch(q->op) {
		case Q_ADD:
		case Q_SUB:
		case Q_MUL:
		case Q_DIV:
		case Q_MOD:
			fprintf(fo, "%s = %s %s %s\n", q->res->id, q->lhs->id, binop_str(q->op), q->rhs->id);
			break;
		case Q_CST:
			fprintf(fo, "%s = %d\n", q->res->id, q->val);
			break;
		case Q_AFF:
			fprintf(fo, "%s = %s\n", q->res->id, q->lhs->id);
			break;
		case Q_END:
			fprintf(fo, "ENDPROC\n");
			break;
		case Q_RET:
			fprintf(fo, "RETURN %s\n", q->lhs->id);
			break;
		case Q_PAR:
			fprintf(fo, "PARAM %s\n", q->lhs->id);
			break;
		case Q_PRO:
			fprintf(fo, "PROC %s\n", q->lhs->id);
			break;
		case Q_CAL:
			fprintf(fo, "%s = CALL %s\n", q->res->id, q->lhs->id);
			break;
		case Q_NEG:
			fprintf(fo, "%s = -%s\n", q->res->id, q->lhs->id);
			break;
		case Q_ACC:
			fprintf(fo, "%s = %s[%s]\n", q->res->id, q->lhs->id, q->rhs->id);
			break;
		case Q_AFT:
			fprintf(fo, "%s[%s] = %s\n", q->res->id, q->lhs->id, q->rhs->id);
			break;
		case Q_GOT:
			fprintf(fo, "GOTO %lu\n", q->dst+1);
			break;
		case Q_IFG:
			fprintf(fo, "IF %s %s %s GOTO %lu\n", q->lhs->id, cmp_str(q->cmp), q->rhs->id, q->dst+1);
			break;
	}
}
