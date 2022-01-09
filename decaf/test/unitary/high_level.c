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

#include <stdlib.h>
#include <string.h>
#include <assert.h>

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

	struct typelist* two_int = typelist_new();
	typelist_append(typelist_append(two_int, BT_INT), BT_INT);
	ctx_newname(tokenize("main"))->type = typedesc_make_function(BT_INT, two_int);

	ctx_pushctx(); // main args
	ctx_newname(tokenize("argc"))->type = typedesc_make_var(BT_INT);
	ctx_newname(tokenize("argv"))->type = typedesc_make_var(BT_INT);

	ctx_pushctx(); // contenu du main


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
	struct asm_params ap = {.generate_entrypoint = 1 };

	genasm("MIPS", q_all, sz, fo, &ap);

	return 1;
}

int gencode_func(void* data)
{
	(void) data;

	/* int foo(int x, int y) {
	 * 	int res;
	 * 	res = x * y;
	 * 	return res;
	 * }
	 * void main() {
	 * 	int res;
	 * 	res = foo(x, y);
	 * }
	 */

	// foo
	struct typelist* two_int = typelist_new();
	typelist_append(typelist_append(two_int, BT_INT), BT_INT);
	ctx_newname(tokenize("foo"))->type = typedesc_make_function(BT_INT, two_int);

	ctx_pushctx(); // foo args
	ctx_newname(tokenize("x"))->type = typedesc_make_var(BT_INT);
	ctx_newname(tokenize("y"))->type = typedesc_make_var(BT_INT);

	ctx_pushctx(); // foo body

	ctx_newname(tokenize("res"))->type = typedesc_make_var(BT_INT);

	gencode(quad_arith(ctx_lookup(tokenize("res")), ctx_lookup(tokenize("x")), Q_MUL,
			   ctx_lookup(tokenize("y"))));
	gencode(quad_return(ctx_lookup(tokenize("res"))));

	ctx_popctx(); // end foo body
	ctx_popctx(); // end foo args

	// main
	struct typelist* empty = typelist_new();
	ctx_newname(tokenize("main"))->type = typedesc_make_function(BT_VOID, empty);

	ctx_pushctx(); // main args
	ctx_pushctx(); //main body

	ctx_newname(tokenize("res"))->type = typedesc_make_var(BT_INT);

	gencode(quad_call(ctx_lookup(tokenize("res")), ctx_lookup(tokenize("foo"))));

	size_t sz = 0;
	const struct quad* q_all = get_all_quads(&sz);

	FILE* fo = fopen("/tmp/gencode_hl.mips", "w+");
	struct asm_params ap = {.generate_entrypoint = 0 };

	genasm("MIPS", q_all, sz, fo, &ap);

	return 1;

}


int main()
{
	struct test_suite hl;

	hl = make_ts("High level code generation", setup, teardown);
	add_test(&hl, gencode_arith_add, "Can generate one addition all the way without crash");
	add_test(&hl, gencode_func, "Can generate two functions and calls");

	return exec_ts(&hl) ? EXIT_SUCCESS : EXIT_FAILURE;
}
