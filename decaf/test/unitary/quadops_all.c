#include "quadops.h"
#include "context.h"
#include "entry.h"
#include "incomplete.h"
#include "test_suite.h"
#include <stdlib.h>

struct donnees {
	struct entry lhs, rhs, res;
};

int setup(void** data)
{
	struct donnees* dt = malloc(sizeof(struct donnees));
	if (!dt)
		return 0;

	*data = dt;
	return 1;
}

int teardown(void** data)
{
	free(*data);
	*data = NULL;
	return 1;
}

int quad_arith_null_res(void* data)
{
	struct donnees* dt = data;
	quad_arith(NULL, &dt->res, Q_ADD, &dt->rhs);
	return 0;
}

int quad_arith_null_lhs(void* data)
{
	struct donnees* dt = data;
	quad_arith(&dt->res, NULL, Q_ADD, &dt->rhs);
	return 0;
}

int quad_arith_null_rhs(void* data)
{
	struct donnees* dt = data;
	quad_arith(&dt->res, &dt->lhs, Q_ADD, NULL);
	return 0;
}

int quad_aff_res(void* data)
{
	struct donnees* dt = data;
	quad_aff(NULL, &dt->lhs);
	return 0;
}

int quad_aff_val(void* data)
{
	struct donnees* dt = data;
	quad_aff(&dt->res, NULL);
	return 0;
}

int quad_neg_res(void* data)
{
	struct donnees* dt = data;
	quad_neg(NULL, &dt->lhs);
	return 0;
}

int quad_neg_val(void* data)
{
	struct donnees* dt = data;
	quad_neg(&dt->res, NULL);
	return 0;
}

int quad_ret_bad(void* data)
{
	(void)data;
	quad_return(NULL);
	return 0;
}

extern int is_arith(enum Q_OP);

int quad_arith_bad_op(void* data) {
	struct donnees* dt = data;
	ASSERT_EQ(is_arith(Q_IFG), 0);
	quad_arith(&dt->res, &dt->lhs, Q_GOT, &dt->rhs);
	return 0;
}


int spec_ok_arith(void* data) {
	struct donnees* dt = data;
	struct quad q = quad_arith(&dt->res, &dt->lhs, Q_ADD, &dt->rhs);
	ASSERT_EQ(q.op, Q_ADD);
	ASSERT_EQ(q.res, &dt->res);
	ASSERT_EQ(q.lhs, &dt->lhs);
	ASSERT_EQ(q.rhs, &dt->rhs);
	return 1;
}

int spec_ok_aff(void* data) {
	struct donnees* dt = data;
	struct quad q = quad_aff(&dt->res, &dt->lhs);
	ASSERT_EQ(q.op, Q_AFF);
	ASSERT_EQ(q.res, &dt->res);
	ASSERT_EQ(q.lhs, &dt->lhs);
	return 1;
}

int spec_ok_neg(void* data) {
	struct donnees* dt = data;
	struct quad q = quad_neg(&dt->res, &dt->lhs);
	ASSERT_EQ(q.op, Q_NEG);
	ASSERT_EQ(q.res, &dt->res);
	ASSERT_EQ(q.lhs, &dt->lhs);
	return 1;
}

int spec_ok_cst(void* data) {
	struct donnees* dt = data;
	struct quad q = quad_cst(&dt->res, 42);
	ASSERT_EQ(q.op, Q_CST);
	ASSERT_EQ(q.res, &dt->res);
	ASSERT_EQ(q.val, 42);
	return 1;
}

int spec_ok_goto(void* data) {
	(void)(data);
	struct quad q = quad_goto(INCOMPLETE_QUAD_ID);
	ASSERT_EQ(q.op, Q_GOT);
	ASSERT_EQ(q.dst, INCOMPLETE_QUAD_ID);
	return 1;
}

int spec_ok_ifgoto(void* data) {
	struct donnees* dt = data;
	struct quad q = quad_ifgoto(&dt->lhs, CMP_EQ, &dt->rhs, INCOMPLETE_QUAD_ID);
	ASSERT_EQ(q.op, Q_IFG);
	ASSERT_EQ(q.lhs, &dt->lhs);
	ASSERT_EQ(q.cmp, CMP_EQ);
	ASSERT_EQ(q.rhs, &dt->rhs);
	ASSERT_EQ(q.dst, INCOMPLETE_QUAD_ID);
	return 1;
}

int spec_ok_param(void* data) {
	struct donnees* dt = data;
	struct quad q = quad_param(&dt->lhs);
	ASSERT_EQ(q.op, Q_PAR);
	ASSERT_EQ(q.lhs, &dt->lhs);
	return 1;
}

int spec_ok_proc(void* data) {
	struct donnees* dt = data;
	struct quad q = quad_proc(&dt->lhs);
	ASSERT_EQ(q.op, Q_PRO);
	ASSERT_EQ(q.lhs, &dt->lhs);
	return 1;
}

int spec_ok_call(void* data) {
	struct donnees* dt = data;
	struct quad q = quad_call(&dt->res, &dt->lhs);
	ASSERT_EQ(q.op, Q_CAL);
	ASSERT_EQ(q.res, &dt->res);
	ASSERT_EQ(q.lhs, &dt->lhs);
	return 1;
}

int spec_ok_return(void* data) {
	struct donnees* dt = data;
	struct quad q = quad_return(&dt->res);
	ASSERT_EQ(q.op, Q_RET);
	ASSERT_EQ(q.lhs, &dt->res);
	return 1;
}

int spec_ok_endproc(void* data) {
	(void)data;
	struct quad q = quad_endproc();
	ASSERT_EQ(q.op, Q_END);
	return 1;
}

int is_arith_test(void* data) {
	(void)data;
	ASSERT_EQ(is_arith(Q_CAL), 0);
	ASSERT_EQ(is_arith(Q_ADD), 1);
	return 1;
}



int main(void)
{
	struct test_suite ts_qo = make_ts("Quadops helpers", setup, teardown);
	add_test_assert(&ts_qo, quad_arith_null_lhs, "erreur si quad_arith: res == NULL");
	add_test_assert(&ts_qo, quad_arith_null_lhs, "erreur si quad_arith: lhs == NULL");
	add_test_assert(&ts_qo, quad_arith_null_rhs, "erreur si quad_arith: rhs == NULL");
	add_test_assert(&ts_qo, quad_arith_bad_op, "erreur si quad_arith: mauvais operateur");
	add_test_assert(&ts_qo, quad_aff_res, "erreur si quad_aff: res == NULL");
	add_test_assert(&ts_qo, quad_aff_res, "erreur si quad_aff: val == NULL");
	add_test_assert(&ts_qo, quad_ret_bad, "erreur si quad_return: ret == NULL");

	add_test(&ts_qo, spec_ok_arith, "quad_arith conforme");
	add_test(&ts_qo, spec_ok_aff, "quad_aff conforme");
	add_test(&ts_qo, spec_ok_neg, "quad_neg conforme");
	add_test(&ts_qo, spec_ok_cst, "quad_cst conforme");
	add_test(&ts_qo, spec_ok_goto, "quad_goto conforme");
	add_test(&ts_qo, spec_ok_ifgoto, "quad_ifgoto conforme");
	add_test(&ts_qo, spec_ok_param, "quad_param conforme");
	add_test(&ts_qo, spec_ok_proc, "quad_proc conforme");
	add_test(&ts_qo, spec_ok_call, "quad_call conforme");
	add_test(&ts_qo, spec_ok_return, "quad_return conforme");
	add_test(&ts_qo, spec_ok_endproc, "quad_endproc conforme");

	add_test(&ts_qo, is_arith_test, "Fonction annexe is_arith");

	return exec_ts(&ts_qo) ? EXIT_SUCCESS : EXIT_FAILURE;
}
