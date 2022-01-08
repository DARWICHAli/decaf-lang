/*
 * Génération du code assemleur
 */

#include "genasm.h"
#include "mips.h"

#include "quadops.h"
#include "context.h"
#include "typedesc.h"
#include "entry.h"

#include <string.h> // strcmp
#include <stdio.h> // fwrite
#include <stdlib.h> // exit
#include <assert.h> // assert

#define REQ_LHS_SIZE 12
#define REQ_RHS_SIZE 6
#define REQ_RES_SIZE 9

enum Q_OP req_lhs[REQ_LHS_SIZE] = {Q_ADD, Q_AFF, Q_DIV, Q_MOD, Q_MUL, Q_NEG, Q_SUB, Q_IFG, Q_PAR, Q_RET, Q_CAL, Q_PRO};
enum Q_OP req_rhs[REQ_RHS_SIZE] = {Q_ADD, Q_DIV, Q_MOD, Q_MUL, Q_SUB, Q_IFG};
enum Q_OP req_res[REQ_RES_SIZE] = {Q_ADD, Q_AFF, Q_DIV, Q_MOD, Q_MUL, Q_NEG, Q_SUB, Q_CST, Q_CAL};

struct geninfos qinfos = { .init = 0, .is_dst = {0} };

int is_in(enum Q_OP op, enum Q_OP* tab, size_t size) {
	for (size_t i = 0; i < size; ++i) {
		if (op == tab[i])
			return 1;
	}
	return 0;
}


int check_quad(const struct quad* q) {
	/*
	 * lhs, rhs, res checks
	 */
	assert((is_in(q->op, req_lhs, REQ_LHS_SIZE) ? q->lhs != NULL : 1) && "Operator requires lhs");
	assert((is_in(q->op, req_rhs, REQ_RHS_SIZE) ? q->rhs != NULL : 1) && "Operator requires rhs");
	assert((is_in(q->op, req_res, REQ_RES_SIZE) ? q->res != NULL : 1) && "Operator requires res");

	/*
	 * Function and procedures
	 */
	assert((q->op == Q_CAL || q->op == Q_PRO ) ? typedesc_is_function(&q->lhs->type) : 1 && "Trying to call a non-function");
	assert((q->op == Q_CAL ) ? typedesc_function_type(&q->lhs->type) != BT_VOID : 1 && "Trying to retrieves result of void function");

	return 1;
}

/*
 * Regarde si la table des symboles contient une fonction
 */
int contains_func(const struct context* ctx) {
	assert(ctx && "contains_func expects non-NULL ctx");

	for (size_t i = 0; i < ctx_count_entries(ctx); ++i) {
		if (typedesc_is_function(&ctx_nth(ctx,i)->type))
			return 1;
	}
	return 0;
}

const struct context* ctx_root_ctx() {
	const struct context* it = ctx_currentctx();

	assert(it && "ctx_currentctx is NULL");
	assert(it->parent != NULL && "ctx_currentctx cannot be super-global context");
	while (it->parent->parent != NULL)
		it = it->parent;

	return it;
}

/*
 * Récupère le contexte le plus haut du corps de fonction
 */
const struct context* ctx_argsfun(const struct context* ctx) {
	assert(ctx && "Non null context expected");
	assert(ctx->parent && "Super-global context have no body");
	assert(ctx->parent->parent && "Global context have no body");

	const struct context* it = ctx;
	while (!contains_func(it->parent)) {
		assert(it->parent && "Reached super-global context without finding function body");
		it = it->parent;
	}

	return it;
}

void genasm(const char* to_lang, const quad_id_t* qlist, size_t liste_size, FILE* outfile, const struct asm_params* genp) {
	assert(qlist && liste_size > 0 && "Empty list not allowed");
	assert(genp && "Parameters cannot be NULL");
	assert(outfile && "Bad file");
	if (strcmp(to_lang, "MIPS") == 0) {
		genMIPS(qlist, liste_size, outfile, genp);
	} else {
		fprintf(stderr, "Unknown lang: %s\n", to_lang);
		exit(EXIT_FAILURE);
	}
}

void first_pass(const quad_id_t* qlist, size_t liste_size) {
	assert(liste_size < MAX_IS_DST_SIZE && "first_pass can't handle this many quads while having is_quad_dst being O(1))");

	for (size_t i = 0; i < liste_size; ++i) {
		const struct quad* q = getquad(qlist[i]);
		if (q->op == Q_GOT || q->op == Q_IFG) {
			assert(q->dst != INCOMPLETE_QUAD_ID && "Incomplete quad in final IR");
			assert(getquad(q->dst) && "Bad quad in destination");
			assert(q->dst < MAX_IS_DST_SIZE && "quad_id too big???");
			qinfos.is_dst[q->dst] = 1;
		}
	}

	qinfos.init = 1;
}

int is_quad_dst(quad_id_t qid) {
	assert(qinfos.init && "First pass need to be done before calling this function");
	assert((qid < MAX_IS_DST_SIZE && qid != INCOMPLETE_QUAD_ID) && "bad qid");
	return qinfos.is_dst[qid];
}
