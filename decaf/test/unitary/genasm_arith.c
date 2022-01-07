/*
 * Test de genasm uniquement pour l'arithmétique
 */
#include "test_suite.h"

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

void make_var(struct context* ctx, const char* id) {
	size_t idx = ctx->used;
	strcpy(ctx->entries[idx].id, id);
	ctx->entries[idx].type.mtype = MT_VAR;
	ctx->entries[idx].type.btype = BT_INT;
	ctx->entries[idx].ctx = ctx;
	++ctx->used;
}

int setup(void** data) {
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
	dt->ap.generate_entrypoint = 1;

	return dt->fo ? 1 : 0;
}

int teardown(void** data) {
	struct data* dt = *data;
	co_used = 0;
	sommet = NULL;
	free(*data);
	return fclose(dt->fo) == 0;
}

void set_quad(struct quad quads[MAX_Q], size_t i, enum Q_OP op, struct entry* res, struct entry* lhs, struct entry* rhs) {
	quads[i].op = op;
	quads[i].res = res;
	quads[i].lhs = lhs;
	quads[i].rhs = rhs;
	quads[i].ctx = res ? res->ctx : NULL;
}

int check_file(FILE* f, const char* exp) {
	fseek(f, 0, SEEK_SET);
	char buf[1024];

	size_t read = fread(buf, sizeof(char), 1024, f);
	size_t n_exp = strlen(exp);
	assert(n_exp < 1024);
	fclose(f);

	size_t ib, ie;
	for (ib = 0, ie = 0; ib < read && ie < n_exp;) {
		if (exp[ie] != buf[ib]) {
			if (buf[ib] == '#') {
				for (; buf[ib] != '\n'; ++ib);
			} else if (buf[ib] == ' ' && ib + 1 < read && buf[ib+1] == '#') {
				++ib;
			} else {
				fprintf(stderr, "Not matching...\n\n---[expected]---\n%s\n\n---[got]---\n%s\n\nError on char : %lu\n\n[UNTIL]\n", exp, buf, ib);
				for (size_t i = 0; i < ib; ++i)
					fprintf(stderr, "%c", buf[i]);
				return 0;
			}
		} else {
			++ie;
			++ib;
		}
	}
	if (ie != n_exp) {
		fprintf(stderr, "Not matching...\n\n---[expected]---\n%s\n\n---[got]---\n%s\n\nError on char : %lu\n\n[UNTIL]\n", exp, buf, ib);
				for (size_t i = 0; i < ib; ++i)
					fprintf(stderr, "%c", buf[i]);
				return 0;
	}
	return ie == n_exp;
}

int arith_add(void* data) {
	struct data* dt = data;

	set_quad(dt->ql, 0, Q_ADD, dt->res, dt->lhs, dt->rhs);

	const char* expected = ".data\n\n"
				"global: .word 0\n"
				"\n.text\n"
				"__start:\n"
				"call main\n"
                                "move $a0 $v0\n"
                                "li $v0 17\n"
                                "syscall\n"
                                "\n"
				"main:\n"
				"addi $sp $sp -12\n"
				"lw $a0 4($sp)\n"
				"lw $a1 8($sp)\n"
				"add $v0 $a0 $a1\n"
				"sw $v0 0($sp)\n";


	genasm("MIPS", dt->ql, 1, dt->fo, &dt->ap);

	return check_file(dt->fo, expected);
}

int arith_all_binary(void* data) {
	struct data* dt = data;

	set_quad(dt->ql, 0, Q_ADD, dt->res, dt->lhs, dt->rhs);
	set_quad(dt->ql, 1, Q_SUB, dt->res, dt->lhs, dt->rhs);
	set_quad(dt->ql, 2, Q_MUL, dt->res, dt->lhs, dt->rhs);
	set_quad(dt->ql, 3, Q_DIV, dt->res, dt->lhs, dt->rhs);
	set_quad(dt->ql, 4, Q_MOD, dt->res, dt->lhs, dt->rhs);

	const char* expected = ".data\n\n"
			       "global: .word 0\n"
			       "\n.text\n"
				"__start:\n"
				"call main\n"
                                "move $a0 $v0\n"
                                "li $v0 17\n"
                                "syscall\n"
                                "\n"
			       "main:\n"
			       "addi $sp $sp -12\n"
			       "lw $a0 4($sp)\n"
			       "lw $a1 8($sp)\n"
			       "add $v0 $a0 $a1\n"
			       "sw $v0 0($sp)\n"
			       "lw $a0 4($sp)\n"
			       "lw $a1 8($sp)\n"
			       "sub $v0 $a0 $a1\n"
			       "sw $v0 0($sp)\n"
			       "lw $a0 4($sp)\n"
			       "lw $a1 8($sp)\n"
			       "mult $a0 $a1\n"
			       "mflo $v0\n"
			       "sw $v0 0($sp)\n"
			       "lw $a0 4($sp)\n"
			       "lw $a1 8($sp)\n"
			       "div $a2 $a0 $a1\n"
			       "mflo $v0\n"
			       "sw $v0 0($sp)\n"
			       "lw $a0 4($sp)\n"
			       "lw $a1 8($sp)\n"
			       "div $a2 $a0 $a1\n"
			       "mfhi $v0\n"
			       "sw $v0 0($sp)\n";

	genasm("MIPS", dt->ql, 5, dt->fo, &dt->ap);

	return check_file(dt->fo, expected);
}

int arith_aff_neg(void* data) {
	struct data* dt = data;

	set_quad(dt->ql, 0, Q_AFF, dt->res, dt->lhs, NULL);
	set_quad(dt->ql, 1, Q_NEG, dt->res, dt->rhs, NULL);

	const char* expected = ".data\n\n"
			       "global: .word 0\n"
			       "\n.text\n"
				"__start:\n"
				"call main\n"
                                "move $a0 $v0\n"
                                "li $v0 17\n"
                                "syscall\n"
                                "\n"
			       "main:\n"
			       "addi $sp $sp -12\n"
                               "lw $a0 4($sp)\n"
                               "move $v0 $a0\n"
                               "sw $v0 0($sp)\n"
                               "lw $a0 8($sp)\n"
                               "and $a1 $a1 $0\n"
                               "sub $v0 $a1 $a0\n"
                               "sw $v0 0($sp)\n";

	genasm("MIPS", dt->ql, 2, dt->fo, &dt->ap);

	return check_file(dt->fo, expected);
}

int arith_cst_small(void* data) {
	struct data* dt = data;

	set_quad(dt->ql, 0, Q_CST, dt->res, NULL, NULL);
	dt->ql[0].val = 42;

	const char* expected = ".data\n\n"
			       "global: .word 0\n"
			       "\n.text\n"
				"__start:\n"
				"call main\n"
                                "move $a0 $v0\n"
                                "li $v0 17\n"
                                "syscall\n"
                                "\n"
			       "main:\n"
			       "addi $sp $sp -12\n"
                               "ori $v0 $v0 42\n"
                               "sw $v0 0($sp)\n";

	genasm("MIPS", dt->ql, 1, dt->fo, &dt->ap);

	return check_file(dt->fo, expected);
}

int arith_cst_big(void* data) {
	struct data* dt = data;

	set_quad(dt->ql, 0, Q_CST, dt->res, NULL, NULL);
	dt->ql[0].val = 65535 << 4;

	const char* expected = ".data\n\n"
			       "global: .word 0\n"
			       "\n.text\n"
				"__start:\n"
				"call main\n"
                                "move $a0 $v0\n"
                                "li $v0 17\n"
                                "syscall\n"
                                "\n"
			       "main:\n"
			       "addi $sp $sp -12\n"
                               "ori $v0 $v0 65520\n"
                               "lui $v0 15\n"
                               "sw $v0 0($sp) # res\n";

	genasm("MIPS", dt->ql, 1, dt->fo, &dt->ap);

	return check_file(dt->fo, expected);
}

int arith_no_locals(void* data) {
	struct data* dt = data;

	set_quad(dt->ql, 0, Q_AFF, &dt->main->entries[0], &dt->main->entries[1], NULL);
	dt->ql[0].ctx = dt->main_int;
	dt->main_int->used = 0; // no locals

	const char* expected = ".data\n\n"
			       "global: .word 0\n"
			       "\n.text\n"
				"__start:\n"
				"call main\n"
                                "move $a0 $v0\n"
                                "li $v0 17\n"
                                "syscall\n"
                                "\n"
			       "main:\n"
			       "lw $a0 4($fp)\n"
			       "move $v0 $a0\n"
                               "sw $v0 0($fp)\n";

	genasm("MIPS", dt->ql, 1, dt->fo, &dt->ap);

	return check_file(dt->fo, expected);
}


int err_cst_glob(void* data) {
	struct data* dt = data;

	set_quad(dt->ql, 0, Q_CST, &dt->root->entries[0], NULL, NULL);
	dt->ql[0].val = 42;
	dt->ql[0].ctx = dt->main_int;

	genasm("MIPS", dt->ql, 1, dt->fo, &dt->ap);

	return 0;
}

int err_aff_glob(void* data) {
	struct data* dt = data;

	set_quad(dt->ql, 0, Q_AFF, &dt->root->entries[0], dt->lhs, NULL);
	dt->ql[0].ctx = dt->main_int;

	genasm("MIPS", dt->ql, 1, dt->fo, &dt->ap);

	return 0;
}

int err_neg_glob(void* data) {
	struct data* dt = data;

	set_quad(dt->ql, 0, Q_NEG, &dt->root->entries[0], dt->lhs, NULL);
	dt->ql[0].ctx = dt->main_int;

	genasm("MIPS", dt->ql, 1, dt->fo, &dt->ap);

	return 0;
}

int empty_main(void* data) {
	struct data* dt = data;

	set_quad(dt->ql, 0, Q_END, NULL, NULL, NULL);
	dt->ql[0].ctx = dt->main_int;
	dt->main_int->used = 0; // no locals

	const char* expected = ".data\n\n"
			       "global: .word 0\n"
			       "\n.text\n"
				"__start:\n"
				"call main\n"
                                "move $a0 $v0\n"
                                "li $v0 17\n"
                                "syscall\n"
                                "\n"
			       "main:\n"
			       "jr $ra\n";

	genasm("MIPS", dt->ql, 1, dt->fo, &dt->ap);

	return check_file(dt->fo, expected);
}

int main() {
	struct test_suite ts = make_ts("Traduction MIPS d'expressions arithmetiques", setup, teardown);
	add_test(&ts, arith_add, "Opérateur d'addition");
	add_test(&ts, arith_all_binary, "Tous les opérateurs binaires");
	add_test(&ts, arith_aff_neg, "Affectation et négation");
	add_test(&ts, arith_cst_small, "Affectation de petites constantes (<=16b)");
	add_test(&ts, arith_cst_big, "Affectation de grandes constantes (>16b)");
	add_test(&ts, arith_no_locals, "Fonction sans variable locales");
	add_test(&ts, empty_main, "Main vide");
	add_test_assert(&ts, err_cst_glob, "Impossible d'affecter une constante à une variable globale");
	add_test_assert(&ts, err_aff_glob, "Impossible d'affecter une variable à une variable globale");
	add_test_assert(&ts, err_neg_glob, "Impossible d'affecter une négation à une variable globale");

	return exec_ts(&ts) ? EXIT_SUCCESS : EXIT_FAILURE;
}
