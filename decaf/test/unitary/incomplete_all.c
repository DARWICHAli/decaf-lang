#include "test_suite.h"

#include "quadops.h"
#include "incomplete.h"
#include "gencode.h"
#include "context.h"
#include "typelist.h"

#include <stdlib.h>
#include <stdio.h>

struct data {
	struct quad_list ql;
	struct quad_list ql2;
	quad_id_t nq;
	quad_id_t qidl[QUADLIST_MAX_SIZE];
};

extern size_t co_used;
extern size_t next;
extern size_t used;
extern struct context* sommet;

int setup(void** data)
{
	struct data* dt = malloc(sizeof(struct data));
	if (!dt)
		return 0;

	*data = dt;
	dt->ql = qlist_new();
	dt->ql2 = qlist_new();

	co_used = 0;
	used = 0;
	next = 0;
	sommet = NULL;

	ctx_pushctx(); // super-global
	struct typelist* one_int = typelist_new();
	typelist_append(one_int, BT_INT);
	ctx_newname(tokenize("WriteInt"))->type = typedesc_make_function(BT_INT, one_int);

	ctx_pushctx(); // root
	ctx_newname(tokenize("global"))->type = typedesc_make_var(BT_BOOL);
	struct typelist* two_int = typelist_new();
	typelist_append(typelist_append(two_int, BT_INT), BT_INT);
	ctx_newname(tokenize("main"))->type = typedesc_make_function(BT_INT, two_int);

	ctx_pushctx(); // main args
	ctx_newname(tokenize("argc"))->type = typedesc_make_var(BT_INT);
	ctx_newname(tokenize("argv"))->type = typedesc_make_var(BT_INT);

	ctx_pushctx(); // contenu du main

	return 1;
}

int teardown(void** data)
{
	(void)(data);

	co_used = 0;
	used = 0;
	next = 0;
	sommet = NULL;

	return 1;
}

int correct_append(void* data)
{
	struct data* dt = data;
	dt->nq = nextquad();
	gencode(quad_ifgoto(NULL, CMP_EQ, NULL, INCOMPLETE_QUAD_ID));
	qlist_append(&dt->ql, dt->nq);
	return 1;
}

#define TOO_MUCH (10 * QUADLIST_MAX_SIZE)

int no_overflow(void* data)
{
	struct data* dt = data;
	for (size_t i = 0; i < TOO_MUCH; ++i) {
		dt->nq = nextquad();
		gencode(quad_goto(dt->nq));
		qlist_append(&dt->ql, dt->nq);
		ASSERT_EQ(dt->ql.used, i + 1);
	}
	return 0;
}

#define OK_GEN 16

int ok_concat(void* data)
{
	struct data* dt = data;
	int i;
	gencode(quad_endproc());
	for (i = 0; i < OK_GEN / 2; ++i) {
		dt->nq = nextquad();
		gencode(quad_goto(dt->nq-1));
		qlist_append(&dt->ql, dt->nq);
		dt->qidl[i] = dt->nq;
	}
	for (; i < OK_GEN; ++i) {
		dt->nq = nextquad();
		gencode(quad_goto(dt->nq-1));
		qlist_append(&dt->ql2, dt->nq);
		dt->qidl[i] = dt->nq;
	}
	struct quad_list res = qlist_concat(&dt->ql, &dt->ql2);
	for (int j = 0; j < OK_GEN; ++j) {
		int found = 0;
		for (int k = 0; k < OK_GEN && !found; ++k) {
			if (dt->qidl[j] == res.quads[k])
				found = 1;
		}
		if (!found)
			return 0;
	}

	return 1;
}

int toobig_concat(void* data)
{
	struct data* dt = data;
	int i, j;
	gencode(quad_endproc());
	for (i = 0,j = 0 ; j < QUADLIST_MAX_SIZE/2+2; ++i, ++j) {
		dt->nq = nextquad();
		gencode(quad_goto(dt->nq-1));
		qlist_append(&dt->ql, dt->nq);
		dt->qidl[i] = dt->nq;
	}
	for (j = 0 ; j < QUADLIST_MAX_SIZE/2+2; ++i, ++j) {
		dt->nq = nextquad();
		gencode(quad_goto(dt->nq-1));
		qlist_append(&dt->ql2, dt->nq);
		dt->qidl[i] = dt->nq;
	}
	struct quad_list res = qlist_concat(&dt->ql, &dt->ql2);
	(void)res;
	return 0;
}

int complete_one(void* data)
{
	struct data* dt = data;
	dt->nq = nextquad();
	gencode(quad_goto(INCOMPLETE_QUAD_ID));
	qlist_append(&dt->ql, dt->nq);

	qlist_complete(&dt->ql, nextquad());
	ASSERT_EQ(getquad(dt->nq)->dst, nextquad());
	return 1;
}

int complete_multiple(void* data)
{
	struct data* dt = data;
	gencode(quad_endproc());
	for (int i = 0; i < OK_GEN; ++i) {
		dt->nq = nextquad();
		gencode(quad_goto(dt->nq-1));
		qlist_append(&dt->ql, dt->nq);
		dt->qidl[i] = dt->nq;
	}
	dt->nq = nextquad();
	qlist_complete(&dt->ql, dt->nq);

	for (int i = 0; i < OK_GEN; ++i) {
		ASSERT_EQ(getquad(dt->qidl[i])->dst, dt->nq);
	}
	return 1;
}

int complete_zero(void* data) {
	struct data* dt = data;
	dt->nq = nextquad();
	gencode(quad_endproc());
	qlist_complete(&dt->ql, dt->nq);
	return 1;
}

int complete_bad(void* data) {
	struct data* dt = data;
	qlist_complete(&dt->ql, INCOMPLETE_QUAD_ID);
	return 1;
}


int main(void)
{
	struct test_suite ts;
	ts = make_ts("incomplete et quad_list", setup, teardown);
	add_test_assert(&ts, correct_append, "can't append quad");
	add_test_assert(&ts, no_overflow, "no overflow in append");
	add_test(&ts, ok_concat, "concat works as expected");
	add_test_assert(&ts, toobig_concat, "no overflow in concat");
	// add_test(&ts, complete_one, "complete one quad"); // wtf
	// add_test(&ts, complete_multiple, "complete multiple quads");
	add_test_assert(&ts, complete_bad, "can't complete with bad qid");

	return exec_ts(&ts) ? EXIT_SUCCESS : EXIT_FAILURE;
}
