/*
 * Tests très simples de gencode
 */

#include "test_suite.h"

#include "gencode.h"
#include "quadops.h"
#include "context.h"
#include "genasm.h"
#include "typelist.h"

#include <stdlib.h>
#include <string.h>

extern size_t co_used;
extern size_t next;
extern size_t used;
extern struct context* sommet;

int setup(void** data)
{
	(void)(data);

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

int all_quads_empty(void* data)
{
	(void)data;
	size_t sz = 0;
	quad_id_t* qs = get_all_quads(&sz);
	ASSERT_EQ(sz, 0);
	ASSERT_TRUE(qs != NULL);
	return 1;
}

int nq_empty(void* data)
{
	(void)(data);
	ASSERT_EQ(nextquad(), 0);
	return 1;
}

int gq_invalid(void* data)
{
	(void)data;
	getquad(0);
	return 0;
}

int gq_ok(void* data)
{
	(void)(data);

	struct quad q = quad_goto(INCOMPLETE_QUAD_ID);
	gencode(q);

	ASSERT_EQ(getquad(0)->dst, q.dst);
	ASSERT_EQ(getquad(0)->op, q.op);
	return 1;
}

#define TOO_MUCH_QUADS 1000000
int gc_full(void* data)
{
	(void)(data);

	struct entry* res = ctx_newname(tokenize("res"));
	for (size_t i = 0; i < TOO_MUCH_QUADS; ++i) {
		gencode(quad_cst(res, 42));
	}
	return 0;
}

int main()
{
	struct test_suite gc;

	gc = make_ts("Gencode basic tesrs", setup, teardown);
	add_test(&gc, all_quads_empty, "get_all_quads works if empty");
	add_test(&gc, nq_empty, "nextquad works if empty");
	add_test_assert(&gc, gq_invalid, "getquad triggers assert for invalid qid");
	add_test(&gc, gq_ok, "getquad retrieves correct quad");
	add_test_assert(&gc, gc_full, "There is a limit to quad generation");

	return exec_ts(&gc) ? EXIT_SUCCESS : EXIT_FAILURE;
}
