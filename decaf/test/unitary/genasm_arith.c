/*
 * Test de genasm uniquement pour l'arithmétique
 */
#include "test_suite.h"
#include "mips_check.h"

#include "genasm.h"
#include "quadops.h"
#include "context.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

extern size_t co_used;
extern struct context* sommet;

#define MAX_Q 256

struct data {
	struct context *sg, *root, *main, *main_int;
	struct quad ql[MAX_Q];
	struct entry *res, *rhs, *lhs;
	FILE* fo;
	struct asm_params ap;
};

void make_var(struct context* ctx, const char* id)
{
	size_t idx = ctx->used;
	strcpy(ctx->entries[idx].id, id);
	ctx->entries[idx].type.mtype = MT_VAR;
	ctx->entries[idx].type.btype = BT_INT;
	ctx->entries[idx].ctx = ctx;
	++ctx->used;
}

int setup(void** data)
{
	struct data* dt = malloc(sizeof(struct data));
	if (!dt)
		return 0;
	*data = dt;

	co_used = 0;
	sommet = NULL;

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
	sommet = NULL;
	free(*data);
	return fclose(dt->fo) == 0;
}

void set_quad(struct quad quads[MAX_Q], size_t i, enum Q_OP op, struct entry* res, struct entry* lhs, struct entry* rhs)
{
	quads[i].op = op;
	quads[i].res = res;
	quads[i].lhs = lhs;
	quads[i].rhs = rhs;
	quads[i].ctx = res ? res->ctx : NULL;
}

int arith_add(void* data)
{
	struct data* dt = data;

	set_quad(dt->ql, 0, Q_ADD, dt->res, dt->lhs, dt->rhs);

	genasm("MIPS", dt->ql, 1, dt->fo, &dt->ap);

	return print_file(dt->fo);
}

int arith_all_binary(void* data)
{
	struct data* dt = data;

	set_quad(dt->ql, 0, Q_ADD, dt->res, dt->lhs, dt->rhs);
	set_quad(dt->ql, 1, Q_SUB, dt->res, dt->lhs, dt->rhs);
	set_quad(dt->ql, 2, Q_MUL, dt->res, dt->lhs, dt->rhs);
	set_quad(dt->ql, 3, Q_DIV, dt->res, dt->lhs, dt->rhs);
	set_quad(dt->ql, 4, Q_MOD, dt->res, dt->lhs, dt->rhs);

	genasm("MIPS", dt->ql, 5, dt->fo, &dt->ap);

	return print_file(dt->fo);
}

int arith_aff_neg(void* data)
{
	struct data* dt = data;

	set_quad(dt->ql, 0, Q_AFF, dt->res, dt->lhs, NULL);
	set_quad(dt->ql, 1, Q_NEG, dt->res, dt->rhs, NULL);

	genasm("MIPS", dt->ql, 2, dt->fo, &dt->ap);

	return print_file(dt->fo);
}

int arith_cst_small(void* data)
{
	struct data* dt = data;

	set_quad(dt->ql, 0, Q_CST, dt->res, NULL, NULL);
	dt->ql[0].val = 42;

	genasm("MIPS", dt->ql, 1, dt->fo, &dt->ap);

	return print_file(dt->fo);
}

int arith_cst_big(void* data)
{
	struct data* dt = data;

	set_quad(dt->ql, 0, Q_CST, dt->res, NULL, NULL);
	dt->ql[0].val = 65535 << 4;

	genasm("MIPS", dt->ql, 1, dt->fo, &dt->ap);

	return print_file(dt->fo);
}

int arith_no_locals(void* data)
{
	struct data* dt = data;

	set_quad(dt->ql, 0, Q_AFF, &dt->main->entries[0], &dt->main->entries[1], NULL);
	dt->ql[0].ctx = dt->main_int;
	dt->main_int->used = 0; // no locals

	genasm("MIPS", dt->ql, 1, dt->fo, &dt->ap);

	return print_file(dt->fo);
}

int cst_glob(void* data)
{
	struct data* dt = data;

	set_quad(dt->ql, 0, Q_CST, &dt->root->entries[0], NULL, NULL);
	dt->ql[0].val = 42;
	dt->ql[0].ctx = dt->main_int;

	genasm("MIPS", dt->ql, 1, dt->fo, &dt->ap);

	return print_file(dt->fo);
}

int empty_main(void* data)
{
	struct data* dt = data;

	set_quad(dt->ql, 0, Q_END, NULL, NULL, NULL);
	dt->ql[0].ctx = dt->main_int;
	dt->main_int->used = 0; // no locals

	genasm("MIPS", dt->ql, 1, dt->fo, &dt->ap);

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
