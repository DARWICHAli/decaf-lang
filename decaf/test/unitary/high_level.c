/*
 * Génération de code haut-niveau
 *
 * Voir si les contextes, gencode et genasm sont interincompatibles
 */

#include "test_suite.h"

#include "gencode.h"
#include "quadops.h"
#include "context.h"
#include "genasm.h"
#include "typelist.h"

#include "stdlib.h"

extern size_t co_used;
extern size_t next;
extern size_t used;
extern struct context* sommet;

char* tokenize(const char* str)
{
	static char buf[MAX_IDENTIFIER_SIZE];
	int n = snprintf(buf, MAX_IDENTIFIER_SIZE, "%s", str);
	ASSERT_TRUE(n > 0 && n < MAX_IDENTIFIER_SIZE);
	return buf;
}

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

int gencode_arith_add(void* data)
{
	(void)(data);

	ctx_newname(tokenize("res"))->type = typedesc_make_var(BT_INT);
	ctx_newname(tokenize("lhs"))->type = typedesc_make_var(BT_INT);
	ctx_newname(tokenize("rhs"))->type = typedesc_make_var(BT_INT);

	gencode(quad_cst(ctx_lookup(tokenize("lhs")), 42));
	gencode(quad_cst(ctx_lookup(tokenize("rhs")), 69));

	gencode(quad_arith(ctx_lookup(tokenize("res")), ctx_lookup(tokenize("lhs")), Q_MUL,
			   ctx_lookup(tokenize("rhs"))));

	size_t sz = 0;
	const struct quad* q_all = get_all_quads(&sz);

	ASSERT_EQ(sz, 3);

	FILE* fo = fopen("/tmp/gencode_hl.mips", "w+");

	genasm("MIPS", q_all, sz, fo);

	return 1;
}

int main()
{
	struct test_suite hl;

	hl = make_ts("High level code generation", setup, teardown);
	add_test(&hl, gencode_arith_add, "Can generate one addition all the way without crash");

	return exec_ts(&hl) ? EXIT_SUCCESS : EXIT_FAILURE;
}
