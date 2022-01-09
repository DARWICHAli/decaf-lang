/*
 * Test de genasm uniquement pour l'arithmétique
 */
#include "test_suite.h"
#include "mips_check.h"

#include "genasm.h"
#include "quadops.h"
#include "context.h"
#include "gencode.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

extern size_t co_used;
extern struct context* sommet;

extern quad_id_t next;

#define MAX_Q 256

struct data {
	struct context *sg, *root, *main, *main_int;
	struct entry *res, *rhs, *lhs;
	FILE* fo;
	struct asm_params ap;
	size_t sz;
};

int setup(void** data)
{
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
	ent->type.mtype = MT_FUN;

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

	dt->fo = fopen("/tmp/tst_genasm_arith.mips", "w+");
	dt->ap.generate_entrypoint = 0;

	return dt->fo ? 1 : 0;
}

int teardown(void** data)
{
	struct data* dt = *data;
	co_used = 0;
	next = 0;
	sommet = NULL;
	free(*data);
	return fclose(dt->fo) == 0;
}

int arith_add(void* data)
{
	struct data* dt = data;
	
	gencode(quad_arith(dt->res, dt->lhs, Q_ADD, dt->rhs));

	genasm("MIPS", get_all_quads(&dt->sz), 1, dt->fo, &dt->ap);

	return print_file(dt->fo);
}

int arith_all_binary(void* data)
{
	struct data* dt = data;

	gencode(quad_arith(dt->res, dt->lhs, Q_ADD, dt->rhs));
	gencode(quad_arith(dt->res, dt->lhs, Q_SUB, dt->rhs));
	gencode(quad_arith(dt->res, dt->lhs, Q_MUL, dt->rhs));
	gencode(quad_arith(dt->res, dt->lhs, Q_DIV, dt->rhs));
	gencode(quad_arith(dt->res, dt->lhs, Q_MOD, dt->rhs));

	genasm("MIPS", get_all_quads(&dt->sz), 5, dt->fo, &dt->ap);

	return print_file(dt->fo);
}

int arith_aff_neg(void* data)
{
	struct data* dt = data;

	gencode(quad_aff(dt->res, dt->lhs));
	gencode(quad_neg(dt->res, dt->rhs));

	genasm("MIPS", get_all_quads(&dt->sz), 2, dt->fo, &dt->ap);

	return print_file(dt->fo);
}

int arith_cst_small(void* data)
{
	struct data* dt = data;

	gencode(quad_cst(dt->res, 42));

	genasm("MIPS", get_all_quads(&dt->sz), 1, dt->fo, &dt->ap);

	return print_file(dt->fo);
}

int arith_cst_big(void* data)
{
	struct data* dt = data;

	gencode(quad_cst(dt->res, 65535 << 4));

	genasm("MIPS", get_all_quads(&dt->sz), 1, dt->fo, &dt->ap);

	return print_file(dt->fo);
}

int arith_no_locals(void* data)
{
	struct data* dt = data;

	gencode(quad_aff(&dt->main->entries[0], &dt->main->entries[1]));
	getquad(0)->ctx = dt->main_int;
	dt->main_int->used = 0; // no locals

	genasm("MIPS", get_all_quads(&dt->sz), 1, dt->fo, &dt->ap);

	return print_file(dt->fo);
}

int cst_glob(void* data)
{
	struct data* dt = data;

	gencode(quad_cst(&dt->root->entries[0], 42));

	genasm("MIPS", get_all_quads(&dt->sz), 1, dt->fo, &dt->ap);

	return print_file(dt->fo);
}

int empty_main(void* data)
{
	struct data* dt = data;

	gencode(quad_endproc());
	dt->main_int->used = 0; // no locals

	genasm("MIPS", get_all_quads(&dt->sz), 1, dt->fo, &dt->ap);

	return print_file(dt->fo);
}

int main()
{
	struct test_suite ts = make_ts("Traduction MIPS d'expressions arithmetiques", setup, teardown);
	add_test(&ts, arith_add, "Opérateur d'addition");
	add_test(&ts, arith_all_binary, "Tous les opérateurs binaires");
	add_test(&ts, arith_aff_neg, "Affectation et négation");
	add_test(&ts, arith_cst_small, "Affectation de petites constantes (<=16b)");
	add_test(&ts, arith_cst_big, "Affectation de grandes constantes (>16b)");
	add_test(&ts, arith_no_locals, "Fonction sans variable locales");
	add_test(&ts, empty_main, "Main vide");
	add_test(&ts, cst_glob, "Affectation d'une constante à une variable globale");

	return exec_ts(&ts) ? EXIT_SUCCESS : EXIT_FAILURE;
}
