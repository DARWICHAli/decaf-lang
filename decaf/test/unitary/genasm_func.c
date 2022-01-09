/*
 * Test de genasm uniquement pour les fonctions et procédures
 */
#include "test_suite.h"
#include "mips_check.h"

#include "genasm.h"
#include "quadops.h"
#include "context.h"
#include "typelist.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


extern size_t co_used;
extern struct context* sommet;

extern quad_id_t next;

struct data {
	struct context *sg, *root, *main, *main_int;
	struct entry *res, *rhs, *lhs;
	FILE* fo;
	struct asm_params ap;
	size_t sz;
};

int setup(void** data) {
	struct data* dt = malloc(sizeof(struct data));
	if (!dt)
		return 0;
	*data = dt;

	co_used = 0;
	sommet = NULL;
	next = 0;

	dt->sg = ctx_pushctx();
	struct entry* ent = ctx_newname(tokenize("WriteInt"));
	ent->type.mtype = MT_FUN;

	dt->root = ctx_pushctx();
	ent = ctx_newname(tokenize("global"));
	ent->type = typedesc_make_var(BT_INT);

	ent = ctx_newname(tokenize("main"));
	struct typelist* tl = typelist_new();
	typelist_append(tl, BT_INT);
	typelist_append(tl, BT_INT);
	ent->type = typedesc_make_function(BT_VOID, tl);


	dt->main = ctx_pushctx();
	ctx_newname(tokenize("argc"))->type = typedesc_make_var(BT_INT);
	ctx_newname(tokenize("argv"))->type = typedesc_make_var(BT_INT);

	dt->main_int = ctx_pushctx();
	dt->res = ctx_newname(tokenize("res"));
	dt->res->type = typedesc_make_var(BT_INT);
	dt->lhs = ctx_newname(tokenize("lhs"));
	dt->lhs->type = typedesc_make_var(BT_INT);
	dt->rhs = ctx_newname(tokenize("rhs"));
	dt->rhs->type = typedesc_make_var(BT_INT);

	dt->fo = fopen("/tmp/tst_genasm_func.mips", "w+");
	dt->ap.generate_entrypoint = 0;

	return dt->fo ? 1 : 0;
}

int teardown(void** data) {
	struct data* dt = *data;
	co_used = 0;
	sommet = NULL;
	next = 0;
	free(*data);
	return fclose(dt->fo) == 0;
}

int fun_param_err(void* data) {
	struct data* dt = data;

	gencode(quad_param(NULL));
	genasm("MIPS", get_all_quads(&dt->sz), 1, dt->fo, &dt->ap);

	return 0;
}


int fun_param(void* data) {
	struct data* dt = data;

	gencode(quad_param(dt->lhs));
	genasm("MIPS", get_all_quads(&dt->sz), 1, dt->fo, &dt->ap);

	return print_file(dt->fo);
}

int fun_proc_null(void* data) {
	struct data* dt = data;

	gencode(quad_proc(NULL));
	genasm("MIPS", get_all_quads(&dt->sz), 1, dt->fo, &dt->ap);

	return 0;
}

int fun_proc_notproc(void* data) {
	struct data* dt = data;

	gencode(quad_proc(dt->lhs));
	genasm("MIPS", get_all_quads(&dt->sz), 1, dt->fo, &dt->ap);

	return 0;
}


int fun_proc(void* data) {
	struct data* dt = data;

	gencode(quad_proc(ctx_lookup(tokenize("main"))));
	genasm("MIPS", get_all_quads(&dt->sz), 1, dt->fo, &dt->ap);

	return print_file(dt->fo);
}

int fun_call_null(void* data) {
	struct data* dt = data;

	gencode(quad_call(NULL, NULL));
	genasm("MIPS", get_all_quads(&dt->sz), 1, dt->fo, &dt->ap);

	return 0;
}

int fun_call_nonfunc(void* data) {
	struct data* dt = data;

	gencode(quad_call(dt->res, dt->lhs));
	genasm("MIPS", get_all_quads(&dt->sz), 1, dt->fo, &dt->ap);

	return 0;
}

int fun_call_void(void* data) {
	struct data* dt = data;

	gencode(quad_call(dt->res, ctx_lookup(tokenize("main"))));
	genasm("MIPS", get_all_quads(&dt->sz), 1, dt->fo, &dt->ap);

	return 0;
}

int fun_call(void* data) {
	struct data* dt = data;

	struct entry* foo = &dt->root->entries[dt->root->used++];
	strcpy(foo->id, "foo");
	struct typelist* tl = typelist_new();
	foo->type = typedesc_make_function(BT_INT, tl);
	foo->ctx = dt->root;


	gencode(quad_call(dt->res, ctx_lookup(tokenize("foo"))));
	genasm("MIPS", get_all_quads(&dt->sz), 1, dt->fo, &dt->ap);

	return print_file(dt->fo);
}

int fun_ret_err(void* data) {
	struct data* dt = data;

	gencode(quad_return(NULL));
	genasm("MIPS", get_all_quads(&dt->sz), 1, dt->fo, &dt->ap);

	return 0;
}

int fun_ret(void* data) {
	struct data* dt = data;

	gencode(quad_return(dt->lhs));
	genasm("MIPS", get_all_quads(&dt->sz), 1, dt->fo, &dt->ap);

	return print_file(dt->fo);
}

int fun_endproc(void* data) {
	struct data* dt = data;

	gencode(quad_endproc());
	genasm("MIPS", get_all_quads(&dt->sz), 1, dt->fo, &dt->ap);

	return print_file(dt->fo);

}


int main() {
	struct test_suite ts = make_ts("Traduction MIPS de fonctions et procédures", setup, teardown);
	add_test(&ts, fun_param, "parameter works as intended");
	add_test_assert(&ts, fun_param_err, "can't push NULL parameter");
	add_test(&ts, fun_proc, "proc void function");
	add_test_assert(&ts, fun_proc_null, "proc NULL function fails");
	add_test_assert(&ts, fun_proc_notproc, "proc non-function fails");
	add_test(&ts, fun_call, "call int function");
	add_test_assert(&ts, fun_call_null, "call NULL function fails");
	add_test_assert(&ts, fun_call_nonfunc, "call non-function fails");
	add_test_assert(&ts, fun_call_void, "call void function fails");
	add_test(&ts, fun_ret, "return works as intended");
	add_test_assert(&ts, fun_ret_err, "can't return NULL entry");

	return exec_ts(&ts) ? EXIT_SUCCESS : EXIT_FAILURE;
}
