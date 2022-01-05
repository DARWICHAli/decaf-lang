#include "test_suite.h"

#include "genasm.h"
#include "entry.h"
#include "context.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

struct donnees {
	struct context sg;
	struct context root;
	struct context main;
	struct context maini;
	struct context forb;
	struct context forj;
	struct context bigb;
	struct context bigb_ext;
	struct entry ent;
	char buf[MAX_IDENTIFIER_SIZE];
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
	struct donnees* dt = malloc(sizeof(struct donnees));
	if (!dt)
		return 0;
	*data = dt;

	dt->sg.parent = NULL;
	dt->sg.used = 1;
	strcpy(dt->sg.entries[0].id, "WriteInt");
	dt->sg.entries[0].type.mtype = MT_FUN;

	dt->root.parent = &dt->sg;
	dt->main.parent = &dt->root;
	dt->maini.parent = &dt->main;
	dt->forb.parent = &dt->maini;
	dt->forj.parent = &dt->forb;

	dt->bigb.parent = &dt->main;

	// var globale
	dt->root.used = 0;
	make_var(&dt->root, "globale");

	// fonction main
	make_var(&dt->root, "main");
	dt->root.entries[1].type.mtype = MT_FUN;

	// argc, argv
	dt->main.used = 0;
	make_var(&dt->main, "argc");
	make_var(&dt->main, "argv");

	// var locale
	dt->maini.used = 0;
	make_var(&dt->maini, "loc1");
	make_var(&dt->maini, "loc2");

	// cpt boucle
	dt->forb.used = 0;
	make_var(&dt->forb, "i");
	
	dt->forj.used = 0;
	make_var(&dt->forj, "j");

	// entry à part
	dt->ent.ctx = &dt->root;
	dt->ent.type.mtype = MT_VAR;
	dt->ent.type.btype = BT_BOOL;
	strcpy(dt->ent.id, "unknwown");

	// très gros bloc
	dt->bigb.used = 0;
	char buf[32];
	for (size_t i = 0; i < DEF_ENTRIES_PER_CTX; ++i) {
		sprintf(buf, "tmp_%lu", i);
		make_var(&dt->bigb, buf);
	}
	dt->bigb.next = &dt->bigb_ext;
	for (size_t i = 0; i < 4; ++i) {
		dt->bigb.used++;
		dt->bigb_ext.entries[i].ctx = &dt->bigb;
		dt->bigb_ext.entries[i].type.mtype = MT_VAR;
		dt->bigb_ext.entries[i].type.btype = BT_INT;
		sprintf(dt->bigb_ext.entries[i].id, "tmp_%lu", i + DEF_ENTRIES_PER_CTX);
	}

	return 1;
}

int teardown(void** data) {
	free(*data);
	*data = NULL;
	return 1;
}

void ctx_get_access(const struct entry* ent, char out[MAX_IDENTIFIER_SIZE]);

int cta_null_entry_fail(void* data) {
	struct donnees* dt = data;

	ctx_get_access(NULL, dt->buf);
	return 0;
}

int cta_ok_decal_zero(void* data) {
	struct donnees* dt = data;
	ctx_get_access(&dt->maini.entries[0], dt->buf);
	ASSERT_EQSTR(dt->buf, "0($sp)");
	return 1;
}

int cta_ok_decal_more(void* data) {
	struct donnees* dt = data;
	ctx_get_access(&dt->maini.entries[1], dt->buf);
	ASSERT_EQSTR(dt->buf, "4($sp)");
	return 1;
}

int cta_not_found_crash(void* data) {
	struct donnees* dt = data;

	ctx_get_access(&dt->ent, dt->buf);
	return 0;
}

int cta_correctly_nested_blocks(void* data) {
	struct donnees* dt = data;
	ctx_get_access(&dt->forb.entries[0], dt->buf);
	ASSERT_EQSTR(dt->buf, "8($sp)");
	return 1;
}

int cta_correctly_nested_blocks_double(void* data) {
	struct donnees* dt = data;
	ctx_get_access(&dt->forj.entries[0], dt->buf);
	ASSERT_EQSTR(dt->buf, "12($sp)");
	return 1;
}

int cta_big_ctx(void* data) {
	struct donnees* dt = data;
	ctx_get_access(&dt->bigb_ext.entries[3], dt->buf);
	char buf[64];
	sprintf(buf, "%d($sp)", (3+DEF_ENTRIES_PER_CTX)*4);
	ASSERT_EQSTR(dt->buf, buf);
	return 1;
}

int cta_global(void* data) {
	struct donnees* dt = data;
	ctx_get_access(&dt->root.entries[0], dt->buf);
	ASSERT_EQSTR(dt->buf, "globale");
	return 1;
}

int cta_args_zero(void* data) {
	struct donnees* dt = data;
	ctx_get_access(&dt->main.entries[0], dt->buf);
	ASSERT_EQSTR(dt->buf, "0($fp)");
	return 1;
}

int cta_args_second(void* data) {
	struct donnees* dt = data;
	ctx_get_access(&dt->main.entries[1], dt->buf);
	ASSERT_EQSTR(dt->buf, "4($fp)");
	return 1;
}


int main() {
	struct test_suite ts = make_ts("ctx_get_access", setup, teardown);
	add_test_assert(&ts, cta_null_entry_fail, "Entry == NULL fails");
	add_test(&ts, cta_ok_decal_zero, "0 is first var");
	add_test(&ts, cta_ok_decal_more, "correct offset for non-first vars");
	add_test_assert(&ts, cta_not_found_crash, "Entry not in context triggers assert");
	add_test(&ts, cta_correctly_nested_blocks, "Nested blocks offset are correctly calculated (1)");
	add_test(&ts, cta_correctly_nested_blocks_double, "Nested blocks offset are correctly calculated (2)");
	add_test(&ts, cta_big_ctx, "Accesseur ok with extended context");
	add_test(&ts, cta_global, "Global labels are detected");
	add_test(&ts, cta_args_zero, "Acces aux arguments de fonction (premier arg)");
	add_test(&ts, cta_args_second, "Acces aux arguments de fonction (deuxieme arg)");

	return exec_ts(&ts) ? EXIT_SUCCESS : EXIT_FAILURE;
}
