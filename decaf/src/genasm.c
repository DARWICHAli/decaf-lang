/*
 * Génération du code assemleur
 */

#include "genasm.h"
#include "quadops.h"
#include "context.h"
#include "typedesc.h"
#include "entry.h"

#include <string.h> // strcmp
#include <stdio.h> // fwrite
#include <stdlib.h> // exit
#include <assert.h> // assert

#define MAX_OPERAND_SIZE MAX_IDENTIFIER_SIZE

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

/*
 * Récupère un moyen d'accéder à l'entrée
 */
void ctx_get_access(const struct entry* ent, char out[MAX_OPERAND_SIZE]) {
	assert(ent && "ctx_get_access expects non-NULL entry");
	assert(ent->ctx && "Entry must have context");
	assert(ent->ctx->parent && "No variable allowed in super-global table");
	if (ent->ctx->parent->parent == NULL) { // racine -> global
		assert(ctx_search(ent->id, ent->ctx) && "Entry exists in its context");
		char* dst = strncpy(out, ent->id, MAX_OPERAND_SIZE);
		assert(dst[MAX_OPERAND_SIZE-1] == '\0' && "identifer too big");
	} else if (contains_func(ent->ctx->parent)) { // recherche du décalage pour argument
		int decal = ctx_byte_idx(ent);
		int n = snprintf(out, MAX_OPERAND_SIZE, "%d($fp)", decal);
		assert(n >= 0 && (n < MAX_OPERAND_SIZE || out[MAX_OPERAND_SIZE-1] == '\0') && "accesseur overflow");
	} else { // variale locale
		int decal = ctx_byte_idx(ent);
		const struct context* it = ent->ctx;
		while (!contains_func(it->parent->parent)) {
			decal += ctx_count_bytes(it->parent);
			assert(it->parent != NULL && "Code not allowed outside of a function");
			it = it->parent;
		}

		int w = snprintf(out, MAX_OPERAND_SIZE, "%d($sp)", decal);
		assert(w >= 0 && (w < MAX_OPERAND_SIZE || out[MAX_OPERAND_SIZE-1] == '\0') && "accesseur overflow");
	}
}

#define REQ_LHS_SIZE 8
#define REQ_RHS_SIZE 6
#define REQ_RES_SIZE 8
enum Q_OP req_lhs[REQ_LHS_SIZE] = {Q_ADD, Q_AFF, Q_DIV, Q_MOD, Q_MUL, Q_NEG, Q_SUB, Q_IFG};
enum Q_OP req_rhs[REQ_RHS_SIZE] = {Q_ADD, Q_DIV, Q_MOD, Q_MUL, Q_SUB, Q_IFG};
enum Q_OP req_res[REQ_RES_SIZE] = {Q_ADD, Q_AFF, Q_DIV, Q_MOD, Q_MUL, Q_NEG, Q_SUB, Q_CST};

int is_in(enum Q_OP op, enum Q_OP* tab, size_t size) {
	for (size_t i = 0; i < size; ++i) {
		if (op == tab[i])
			return 1;
	}
	return 0;
}

/*
 * Wrapper autour de fprintf
 */
void write_instruction(FILE* out, const char* op, const char* one, const char* two, const char* three, size_t nb_op, const char* com) {
	assert(nb_op <= 3 && "unknown number of operands");
	assert(op && strcmp(op, "") != 0 && "op must not be null");
#ifndef NDEBUG
	switch (nb_op) {
		case 3:
			assert(three && strcmp(three, "") != 0 && "third operand must not be null");
		// fall through
		case 2:
			assert(two && strcmp(two, "") != 0 && "second operand must not be null");
		// fall through
		case 1:
			assert(one && strcmp(one, "") != 0 && "first operand must not be null");
	}
#endif

	switch (nb_op) {
		case 0:
			fprintf(out, "%s", op);
			break;
		case 1:
			fprintf(out, "%s %s", op, one);
			break;
		case 2:
			fprintf(out, "%s %s %s", op, one, two);
			break;
		case 3:
			fprintf(out, "%s %s %s %s", op, one, two, three);
			break;
	}
	if (com != NULL && strcmp(com, "") != 0)
		fprintf(out, " # %s", com);
	fprintf(out, "\n");
}

#define MAX_INT_LEN 64

const char* int_to_str(int v) {
	static char buf[MAX_INT_LEN];
	int n = snprintf(buf, MAX_INT_LEN, "%d", v);
	assert((n < MAX_INT_LEN || buf[MAX_INT_LEN-1] == '\0') && "int too big");
	return buf;
}

#define MAX_COM_SIZE 128

/*
 * Traduit une instruction en MIPS
 */
void MIPS_translate(const struct quad q, FILE* out) {
	char buf_lhs[MAX_OPERAND_SIZE];
	char buf_rhs[MAX_OPERAND_SIZE];
	char buf_res[MAX_OPERAND_SIZE];
	char buf_com[MAX_COM_SIZE];
	int hi, lo;

	// récupération des moyens d'accès aux opérandes
	if (is_in(q.op, req_lhs, REQ_LHS_SIZE)) {
		assert(q.lhs && "Operator requires lhs");
		ctx_get_access(q.lhs, buf_lhs);
		write_instruction(out, "lw", "$a0", buf_lhs, "", 2, q.lhs->id);
	}
	if (is_in(q.op, req_rhs, REQ_RHS_SIZE)) {
		assert(q.rhs && "Operator requires rhs");
		ctx_get_access(q.rhs, buf_rhs);
		write_instruction(out, "lw", "$a1", buf_rhs, "", 2, q.rhs->id);
	}

	switch (q.op) {
		/* 1-2-1 */
		case Q_ADD:
			write_instruction(out, "add", "$v0", "$a0", "$a1", 3, "+");
			break;
		case Q_SUB:
			write_instruction(out, "sub", "$v0", "$a0", "$a1", 3, "-");
			break;
		case Q_MUL:
			write_instruction(out, "mult", "$a0", "$a1", "", 2, "*");
			write_instruction(out, "mflo", "$v0", "", "", 1, "res <- LO");
			break;
		case Q_DIV:
			write_instruction(out, "div", "$a2", "$a0", "$a1", 3, "DIV");
			write_instruction(out, "mflo", "$v0", "", "", 1, "res <- LO");
			break;
		case Q_MOD:
			write_instruction(out, "div", "$a2", "$a0", "$a1", 3, "MOD");
			write_instruction(out, "mfhi", "$v0", "", "", 1, "res <- HI");
			break;
		case Q_AFF:
			write_instruction(out, "move", "$v0", "$a0", "", 2, "=");
			break;
		case Q_NEG:
			write_instruction(out, "and", "$a1", "$a1", "$0", 3, "a1 = 0");
			write_instruction(out, "sub", "$v0", "$a1", "$a0", 3, "0-x");
			break;
		case Q_CST:
			lo = q.val & 0xffff;
			snprintf(buf_com, MAX_OPERAND_SIZE, "= %d (16b)", q.val); // commentaire
			write_instruction(out, "ori", "$v0", "$v0", int_to_str(lo), 3, buf_com);
			if (lo != q.val) { // 32 bits
				hi = (q.val & (0xffff << 16)) >> 16;
				snprintf(buf_com, MAX_OPERAND_SIZE, "= %d (+16b)", q.val); // commentaire
				write_instruction(out, "lui", "$v0", int_to_str(hi), "", 2, buf_com);
			}
			break;
		case Q_END:
			write_instruction(out, "jr", "$ra", "", "", 1, "endproc");
			break;
		// LCOV_EXCL_START
		default:
			assert(0 && "Not implemented");
		// LCOV_EXCL_STOP
	}

	if (is_in(q.op, req_res, REQ_RES_SIZE)) {
		assert(q.res && "Operator requires res");
		assert(q.res->ctx && q.res->ctx->parent && q.res->ctx->parent->parent && "Cannot affect to global var");
		ctx_get_access(q.res, buf_res);
		write_instruction(out, "sw", "$v0", buf_res, "", 2, q.res->id);
	}
}

const struct context* ctx_root_ctx() {
	const struct context* it = ctx_currentctx();

	assert(it && "ctx_currentctx is NULL");
	assert(it->parent != NULL && "ctx_currentctx cannot be super-global context");
	while (it->parent->parent != NULL)
		it = it->parent;

	return it;
}

#define MAX_TYPE_SIZE 64
/*
 * Type MIPS derrière un tupedesc
 */
void MIPS_type(const struct typedesc* td, char buf[MAX_TYPE_SIZE]) {
	assert(td && "typedesc must not be null");
	assert((typedesc_is_var(td) || typedesc_is_tab(td)) && "Only var or tabs can be global");
	if (typedesc_is_var(td)) {
		switch (td_sizeof(td)) {
			case 4:
				snprintf(buf, MAX_TYPE_SIZE, ".word");
				break;
			// LCOV_EXCL_START
			default:
				assert(0 && "Don't know how this bt_size translate to MIPS");
			// LCOV_EXCL_STOP
		}
	} else if (typedesc_is_tab(td)) {
		assert(snprintf(buf, MAX_TYPE_SIZE, ".space %lu", td_sizeof(td)) < MAX_TYPE_SIZE);
	}
}

/*
 * Génère le segment data
 */
void MIPS_data_segment(FILE* out) {
	const struct context* root = ctx_root_ctx();

	fprintf(out, ".data\n\n");

	char buf[MAX_TYPE_SIZE];
	for (size_t i = 0; i < ctx_count_entries(root); ++i) {
		const struct entry* ent = ctx_nth(root, i);
		switch (ent->type.mtype) {
			case MT_VAR:
				MIPS_type(&ent->type, buf);
				fprintf(out, "%s: %s %s # TODO\n", ent->id, buf, "0");
				break;
			case MT_TAB:
				MIPS_type(&ent->type, buf);
				fprintf(out, "%s: %s\n", ent->id, buf);
				break;
			case MT_FUN:
				break;
			// LCOV_EXCL_START
			// ces lignes sont testées et exécutées !
			default:
				assert(0 && "Unexpected mtype");
			//LCOV_EXCL_STOP
		}
	}
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

void MIPS_alloc_stack(size_t size, FILE* outfile) {
	if (size == 0)
		return;

	char buf[MAX_OPERAND_SIZE];
	int n = snprintf(buf, MAX_OPERAND_SIZE, "-%lu", size);
	assert(n > 0 && n < MAX_OPERAND_SIZE && "snprintf error");

	write_instruction(outfile, "addi", "$sp", "$sp", buf, 3, "");
}

/*
 * Crée le label de la fonction
 * IMPORTANT: on suppose l'ordre respecté
 */
void make_fct(const struct context* args_ctx, FILE* outfile) {
	assert(args_ctx && args_ctx->parent && "Bad args context");
	static int idx = 0;
	const struct context* root = args_ctx->parent;
	assert(root && root->parent && (root->parent->parent == NULL) && "args context too low");

	const struct entry* fct = ctx_nth_function(root, idx);

	fprintf(outfile, "%s: # args:", fct->id);

	// args
	const struct entry* ent;
	for (size_t i=0; i < ctx_count_entries(args_ctx); ++i) {
		ent = ctx_nth(args_ctx, i);
		fprintf(outfile, " %s[%s]", ent->id, ent->type.btype == BT_INT ? "int" : "bool");
	}
	fprintf(outfile, "\n");

	// header
	size_t to_alloc = ctx_longest_path(args_ctx) - ctx_count_bytes(args_ctx); // args not alloc
	MIPS_alloc_stack(to_alloc, outfile);
}

/* Génère point d'entrée */
void MIPS_start(FILE* outfile) {
	(void)outfile;
	//inutile si on utilise spim -exception
	/*fprintf(outfile, "__start:\n");
	write_instruction(outfile, "call", "main", "", "", 1, "");
	write_instruction(outfile, "move", "$a0", "$v0", "", 2, "store main return");
	write_instruction(outfile, "li", "$v0", "17", "", 2, "");
	write_instruction(outfile, "syscall", "", "", "", 0, "");
	fprintf(outfile, "\n");*/
}

/*
 * Génère segment text
 */
void MIPS_text_segment(const struct quad* qlist, size_t liste_size, FILE* outfile) {
	assert(liste_size > 0 && qlist && "Expected non-null qlist");
	assert(ctx_lookup(tokenize("main")) && typedesc_is_function(&ctx_lookup(tokenize("main"))->type) && "main function required");
	fprintf(outfile, "\n.text\n");
	MIPS_start(outfile);

	const struct context* last_args_ctx = NULL, *cur;
	for (size_t i = 0; i < liste_size; ++i) {
		assert(qlist[i].ctx && "Quad must have a context");
		cur = ctx_argsfun(qlist[i].ctx);
		if (last_args_ctx != cur) { // changement de fonction
			make_fct(cur, outfile);
		}
		MIPS_translate(qlist[i], outfile);
		last_args_ctx = cur;
	}
}


/*
 * Génère un fichier MIPS
 */
void genMIPS(const struct quad* qlist, size_t liste_size, FILE* outfile) {
	MIPS_data_segment(outfile);

	MIPS_text_segment(qlist, liste_size, outfile);
}

void genasm(const char* to_lang, const struct quad* qlist, size_t liste_size, FILE* outfile) {
	assert(qlist && liste_size > 0 && "Empty list not allowed");
	assert(outfile && "Bad file");
	if (strcmp(to_lang, "MIPS") == 0) {
		genMIPS(qlist, liste_size, outfile);
	} else {
		fprintf(stderr, "Unknown lang: %s\n", to_lang);
		exit(EXIT_FAILURE);
	}
}
