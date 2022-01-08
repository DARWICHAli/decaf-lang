#include "mips.h"
#include "genasm.h"

#include <string.h>
#include <assert.h>
#include <stdarg.h>

/*
 * Variables
 */


FILE* out = NULL;
const struct asm_params* global_params = NULL;

void set_output(FILE* fd) {
	assert(fd && "MIPS: bad file");
	out = fd;
}

void set_parameters(const struct asm_params* params) {
	assert(params && "MIPS: bad file");
	global_params = params;
}


void mips_translate(const struct quad q) {
	check_quad(q);
	switch (q.op) {
		case Q_ADD:
			mips_Q_ADD(q.res, q.lhs, q.rhs);
			break;
		case Q_SUB:
			mips_Q_SUB(q.res, q.lhs, q.rhs);
			break;
		case Q_MUL:
			mips_Q_MUL(q.res, q.lhs, q.rhs);
			break;
		case Q_DIV:
			mips_Q_DIV(q.res, q.lhs, q.rhs);
			break;
		case Q_MOD:
			mips_Q_MOD(q.res, q.lhs, q.rhs);
			break;
		case Q_AFF:
			mips_Q_AFF(q.res, q.lhs);
			break;
		case Q_NEG:
			mips_Q_NEG(q.res, q.lhs);
			break;
		case Q_CST:
			mips_Q_CST(q.res, q.val);
			break;
		case Q_END:
			mips_Q_END(q.ctx);
			break;
		case Q_PAR:
			mips_Q_PAR(q.lhs);
			break;
		case Q_PRO:
			mips_Q_PRO(q.lhs);
			break;
		case Q_CAL:
			mips_Q_CAL(q.res, q.lhs);
			break;
		case Q_RET:
			mips_Q_RET(q.ctx, q.lhs);
			break;
		// LCOV_EXCL_START
		default:
			assert(0 && "Not implemented");
		// LCOV_EXCL_STOP
	}
}

void MIPS_start() {
	if (global_params->generate_entrypoint) { //inutile si on utilise spim -exception
		mips_label("__start", "entrypoint");
		instr(JAL, sym("main"));
		instr(MOVE, reg(A0), reg(V0));
		instr(LI, reg(V0), imm(17));
		instr(SYSCALL);
		fprintf(out, "\n");
	}
}
/*
 * Crée le label de la fonction
 * IMPORTANT: on suppose l'ordre respecté
 */
void make_fct(const struct context* args_ctx) {
	assert(args_ctx && args_ctx->parent && "Bad args context");
	static int idx = 0;
	const struct context* root = args_ctx->parent;
	assert(root && root->parent && (root->parent->parent == NULL) && "args context too low");

	const struct entry* fct = ctx_nth_function(root, idx);

	mips_label(fct->id, "");

	// args
	/*const struct entry* ent;
	for (size_t i=0; i < ctx_count_entries(args_ctx); ++i) {
		ent = ctx_nth(args_ctx, i);
		fprintf(outfile, " %s[%s]", ent->id, ent->type.btype == BT_INT ? "int" : "bool");
	}
	fprintf(outfile, "\n");*/

	function_header(args_ctx);

	++idx;
}


void MIPS_text_segment(const struct quad* qlist, size_t liste_size) {
	assert(liste_size > 0 && qlist && "Expected non-null qlist");
	assert(ctx_lookup(tokenize("main")) && typedesc_is_function(&ctx_lookup(tokenize("main"))->type) && "main function required");
	fprintf(out, "\n.text\n");
	MIPS_start();

	const struct context* last_args_ctx = NULL, *cur;
	for (size_t i = 0; i < liste_size; ++i) {
		assert(qlist[i].ctx && "Quad must have a context");
		cur = ctx_argsfun(qlist[i].ctx);
		if (last_args_ctx != cur) { // changement de fonction
			make_fct(cur);
		}
		mips_translate(qlist[i]);
		last_args_ctx = cur;
	}
}

void genMIPS(const struct quad* qlist, size_t liste_size, FILE* outfile, const struct asm_params* genp) {
	assert(outfile && genp);
	set_output(outfile);
	set_parameters(genp);

	MIPS_data_segment();

	MIPS_text_segment(qlist, liste_size);
}
