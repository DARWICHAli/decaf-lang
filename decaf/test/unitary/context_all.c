/*
 * Tests du context
 */
#include "context.h"
#include "test_suite.h"
#include "typedesc.h"
#include "typelist.h"


#include <stdlib.h>
#include <memory.h>

#define CO_LEN 1000
extern struct context global_context[CO_LEN];
extern size_t co_used;
extern struct context* sommet;

#define NB_ENTRIES_LOT (DEF_ENTRIES_PER_CTX * 50)
#define ENTRIES_PER_CTX_LOT (NB_ENTRIES_LOT / 10)
#define NB_CTX_LOT (NB_ENTRIES_LOT / ENTRIES_PER_CTX_LOT)

struct data {
	char entries_names[NB_ENTRIES_LOT][MAX_IDENTIFIER_SIZE];
	const struct entry* entries[NB_ENTRIES_LOT];
	const struct entry* fct[NB_ENTRIES_LOT];
	const struct context* expected_ctx[NB_ENTRIES_LOT];
};

int ent_setup(void** data)
{
	co_used = 0;
	sommet = NULL;

	*data = malloc(sizeof(struct data));
	memset(*data, 0, sizeof(struct data));
	memset(&global_context[0], 0, sizeof(struct context) * CO_LEN);

	return *data != NULL;
}

int ent_teardown(void** data)
{
	co_used = 0;
	sommet = NULL;

	free(*data);

	return 1;
}

int empty_add_entry_crash(void* data)
{
	(void)(data);
	ctx_newname(tokenize("nom_test"));
	return 0;
}

int basic_add_entry(void* data)
{
	(void)(data);
	ctx_pushctx();
	ctx_newname(tokenize("nom_test"));
	ASSERT_TRUE(ctx_lookup(tokenize("nom_test")) != NULL);
	return 1;
}

int no_lookup(void* data)
{
	(void)(data);
	ctx_pushctx();
	ctx_newname(tokenize("nom_test"));
	ASSERT_EQ(ctx_lookup(tokenize("noexist")), NULL);
	return 1;
}

int lot_entries_one_ctx(void* data)
{
	struct data* dt = data;
	struct context* ctx = ctx_pushctx();
	// add
	for (size_t i = 0; i < NB_ENTRIES_LOT; ++i) {
		snprintf(dt->entries_names[i], MAX_IDENTIFIER_SIZE, "o%lu", i);
		ASSERT_TRUE((dt->entries[i] = ctx_newname(dt->entries_names[i])) != NULL);
		dt->expected_ctx[i] = ctx;
	}

	ASSERT_EQ(ctx_count_entries(ctx), NB_ENTRIES_LOT);

	// lookup
	const struct entry* found;
	for (size_t i = 0; i < NB_ENTRIES_LOT; ++i) {
		found = ctx_lookup(dt->entries_names[i]);
		ASSERT_TRUE(found != NULL);
		ASSERT_EQ(found, dt->entries[i]);
		ASSERT_EQ(found->ctx, dt->expected_ctx[i]);
	}

	return 1;
}

int lot_entries_multiple_ctx(void* data)
{
	struct data* dt = data;
	struct context* ctx = ctx_pushctx();
	// add
	for (size_t i = 0; i < NB_ENTRIES_LOT; ++i) {
		if (i > 0 && i % ENTRIES_PER_CTX_LOT == 0)
			ctx = ctx_pushctx();
		snprintf(dt->entries_names[i], MAX_IDENTIFIER_SIZE, "m%lu", i);
		ASSERT_TRUE((dt->entries[i] = ctx_newname(dt->entries_names[i])) != NULL);
		dt->expected_ctx[i] = ctx;
	}

	// lookup
	const struct entry* found;
	for (size_t i = 0; i < NB_ENTRIES_LOT; ++i) {
		found = ctx_lookup(dt->entries_names[i]);
		ASSERT_TRUE(found != NULL);
		ASSERT_EQ(found, dt->entries[i]);
		ASSERT_EQ(found->ctx, dt->expected_ctx[i]);
	}

	return 1;
}

int tmp_entries(void* data)
{
	(void)(data);
	ctx_pushctx();
	const struct entry* tmp = ctx_make_temp(BT_INT);
	ASSERT_EQ(ctx_lookup(tmp->id), tmp);
	return 1;
}

int lot_make_tmp_multiple_ctx(void* data)
{
	struct data* dt = data;
	struct context* ctx = ctx_pushctx();
	// add
	for (size_t i = 0; i < NB_ENTRIES_LOT; ++i) {
		if (i > 0 && i % ENTRIES_PER_CTX_LOT == 0)
			ctx = ctx_pushctx();
		ASSERT_TRUE((dt->entries[i] = ctx_make_temp(BT_INT)) != NULL);
		dt->expected_ctx[i] = ctx;
	}

	// lookup
	const struct entry* found;
	for (size_t i = 0; i < NB_ENTRIES_LOT; ++i) {
		found = ctx_lookup(dt->entries[i]->id);
		ASSERT_TRUE(found != NULL);
		ASSERT_EQ(found, dt->entries[i]);
		ASSERT_EQ(found->ctx, dt->expected_ctx[i]);
	}

	return 1;
}

int same_id_simple(void* data)
{
	(void)(data);
	ctx_pushctx();
	ctx_newname(tokenize("test"));
	ASSERT_EQ(ctx_newname(tokenize("test")), NULL);
	return 1;
}

int same_id_lot(void* data)
{
	struct data* dt = data;
	ctx_pushctx();

	for (size_t i = 0; i < NB_ENTRIES_LOT; ++i) {
		snprintf(dt->entries_names[i], MAX_IDENTIFIER_SIZE, "o%lu", i);
		ASSERT_TRUE((dt->entries[i] = ctx_newname(dt->entries_names[i])) != NULL);
	}

	for (size_t i = 0; i < NB_ENTRIES_LOT; ++i) {
		ASSERT_EQ(ctx_newname(dt->entries_names[i]), NULL);
	}

	return 1;
}

int nth(void* data)
{
	struct data* dt = data;
	ctx_pushctx();

	for (size_t i = 0; i < NB_ENTRIES_LOT; ++i) {
		snprintf(dt->entries_names[i], MAX_IDENTIFIER_SIZE, "o%lu", i);
		ASSERT_TRUE((dt->entries[i] = ctx_newname(dt->entries_names[i])) != NULL);
	}

	for (size_t i = 0; i < NB_ENTRIES_LOT; ++i) {
		ASSERT_EQ(ctx_nth(ctx_currentctx(), i), dt->entries[i]);
	}

	return 1;
}

int nth_function(void* data)
{
	struct data* dt = data;
	ctx_pushctx();
	srand(0);
	size_t nb_f = 0;

	for (size_t i = 0; i < NB_ENTRIES_LOT; ++i) {
		snprintf(dt->entries_names[i], MAX_IDENTIFIER_SIZE, "o%lu", i);
		ASSERT_TRUE((dt->entries[i] = ctx_newname(dt->entries_names[i])) != NULL);
		if (rand() % 2 == 0) {
			((struct entry*)dt->entries[i])->type.mtype = MT_FUN;
			dt->fct[nb_f++] = dt->entries[i];
		} else {
			((struct entry*)dt->entries[i])->type.mtype = MT_VAR;
		}
	}

	for (size_t i = 0; i < nb_f; ++i) {
		ASSERT_EQ(ctx_nth_function(ctx_currentctx(), i), dt->fct[i]);
	}

	return 1;
}

int push(void* data)
{
	(void)data;
	ASSERT_TRUE(ctx_pushctx() != NULL);
	return 1;
}

int pop(void* data)
{
	(void)data;
	struct context* ctx = ctx_pushctx();
	ASSERT_TRUE(ctx_pushctx() != NULL);
	ASSERT_EQ(ctx_popctx(), ctx);

	ASSERT_TRUE(ctx_pushctx() != NULL);
	ASSERT_TRUE(ctx_pushctx() != NULL);
	ASSERT_TRUE(ctx_popctx() != NULL);
	ASSERT_TRUE(ctx_pushctx() != NULL);
	ASSERT_TRUE(ctx_popctx() != NULL);
	ASSERT_EQ(ctx_popctx(), ctx);

	return 1;
}

int currentctx(void* data)
{
	(void)data;
	struct context* ctx = ctx_pushctx();
	ASSERT_EQ(ctx_currentctx(), ctx);

	ASSERT_TRUE(ctx_pushctx() != NULL);
	ASSERT_TRUE(ctx_popctx() != NULL);
	ASSERT_EQ(ctx_currentctx(), ctx);

	return 1;
}

int empty_pop_crash(void* data)
{
	(void)data;
	ctx_popctx();
	return 0;
}

int empty_cur_crash(void* data)
{
	(void)data;
	ctx_currentctx();
	return 0;
}

int nothing(void** data)
{
	(void)(data);

	co_used = 0;
	sommet = NULL;

	return 1;
}

int count_bytes_simple(void* data)
{
	(void)(data);
	ctx_pushctx();
	struct entry* ent = ctx_newname(tokenize("test"));
	ent->type.mtype = MT_VAR;
	ent->type.btype = BT_INT;
	ASSERT_EQ(ctx_count_bytes(ctx_currentctx()), bt_sizeof(BT_INT));
	return 1;
}

struct typedesc rdtd()
{
	struct typedesc td;
	switch (rand() % 3) {
	case 0:
		td.mtype = MT_VAR;
		break;
	case 1:
		td.mtype = MT_TAB;
		td.dist.size = (rand() % 1023) + 1;
		break;
	case 2:
		td.mtype = MT_FUN;
		break;
	}
	td.btype = (rand() % 2 == 0) ? BT_INT : BT_BOOL;
	return td;
}

int count_bytes_lot(void* data)
{
	struct data* dt = data;
	size_t expected_size = 0;
	ctx_pushctx();
	srand(0);
	for (size_t i = 0; i < NB_ENTRIES_LOT; ++i) {
		struct typedesc td = rdtd();
		snprintf(dt->entries_names[i], MAX_IDENTIFIER_SIZE, "o%lu", i);
		ASSERT_TRUE((dt->entries[i] = ctx_newname(dt->entries_names[i])) != NULL);
		((struct entry*)dt->entries[i])->type = td;
		expected_size += td_sizeof(&td);
	}
	ASSERT_EQ(expected_size, ctx_count_bytes(ctx_currentctx()));
	return 1;
}

int longest_path_simple(void* data)
{
	(void)(data);
	struct entry* ent;
	// Exemple de la spec
	ctx_pushctx(); // A 16
	ent = ctx_make_temp(BT_INT);
	ent->type.mtype = MT_TAB;
	ent->type.btype = BT_INT;
	ent->type.dist.size = 4;

	ctx_pushctx(); // B 8
	ent = ctx_make_temp(BT_INT);
	ent->type.mtype = MT_VAR;
	ent->type.btype = BT_INT;
	ent = ctx_make_temp(BT_INT);
	ent->type.mtype = MT_VAR;
	ent->type.btype = BT_BOOL;

	ctx_popctx();
	ctx_pushctx(); // C 4
	ent = ctx_make_temp(BT_INT);
	ent->type.mtype = MT_VAR;
	ent->type.btype = BT_INT;

	ctx_pushctx(); // D 8
	ent = ctx_make_temp(BT_INT);
	ent->type.mtype = MT_VAR;
	ent->type.btype = BT_INT;
	ent = ctx_make_temp(BT_INT);
	ent->type.mtype = MT_VAR;
	ent->type.btype = BT_INT;

	ctx_popctx();
	ctx_popctx();

	ASSERT_EQ(ctx_longest_path(ctx_currentctx()), 8 + 4 + 16);
	return 1;
}

#define PATH_LEN 128
#define NB_PATH_MAX 4
#define MIN_DEPTH 10
#define PSTOP 2

#define MAX(lhs, rhs) (((lhs) < (rhs)) ? (rhs) : (lhs))

void create_path(int left, int goodpath)
{
	if (left == 0)
		return;

	if (!goodpath && (rand() % PSTOP) == 0)
		return;

	int nb_childs = rand() % NB_PATH_MAX;
	int gp = rand() % nb_childs;
	for (int i = 0; i < nb_childs; ++i) {
		ctx_pushctx();
		int alloc = MAX(rand() % (PATH_LEN / MIN_DEPTH), left);
		for (int j = 0; j < alloc; ++j) {
			struct entry* e = ctx_make_temp(BT_INT);
			e->type.mtype = MT_VAR;
			e->type.btype = BT_INT;
		}
		create_path(left - alloc, goodpath && i == gp);
		ctx_popctx();
	}
}

int ctx_longest_path_lot(void* data)
{
	(void)(data);
	struct context* ctx = ctx_pushctx();
	srand(0);
	create_path(PATH_LEN, 1);
	ASSERT_EQ(ctx_longest_path(ctx), PATH_LEN * bt_sizeof(BT_INT));
	return 1;
}

/* TS ctx_get_oidx */
int cgo_setup(void** data) {
	*data = malloc(sizeof(struct context));

	if (!data)
		return 0;

	struct context* ctx = *data;

	ctx->entries[0].ctx = ctx;
	ctx->entries[0].type.mtype = MT_VAR;
	ctx->entries[0].type.btype = BT_BOOL;
	strcpy(ctx->entries[0].id, "def_var");
	ctx->used = 1;

	return 1;
}

int cgo_teardown(void** data) {
	free(*data);
	*data = NULL;
	return 1;
}

int cgo_NULL_entry_fail(void* data) {
	(void)(data);
	ctx_byte_idx(NULL);
	return 0;
}

int cgo_error_if_no_entry(void* data) {
	struct context* ctx = data;

	struct entry ent;
	strcpy(ent.id, "introuvable");
	ent.type.mtype = MT_VAR;
	ent.type.btype = BT_INT;
	ent.ctx = ctx;

	ctx_byte_idx(&ent);
	return 0;
}

int cgo_error_if_no_ctx(void* data) {
	struct context* ctx = data;
	ctx->entries[0].ctx = NULL;
	ctx_byte_idx(&ctx->entries[0]);
	return 0;
}

int cgo_good_idx_simple(void* data) {
	struct context* ctx = data;
	ASSERT_EQ(ctx_byte_idx(&ctx->entries[0]), 0);
	return 1;
}

int cgo_good_idx_double(void* data) {
	struct context* ctx = data;

	ctx->entries[1].ctx = ctx;
	ctx->entries[1].type.mtype = MT_VAR;
	ctx->entries[1].type.btype = BT_BOOL;
	strcpy(ctx->entries[1].id, "def_var2");
	ctx->used = 2;

	ASSERT_EQ(ctx_byte_idx(&ctx->entries[1]), bt_sizeof(ctx->entries[0].type.btype));
	return 1;
}

int cgo_big_ctx(void* data) {
	struct context* ctx = data;
	struct context more;
	ctx->next = &more;
	ctx->used = DEF_ENTRIES_PER_CTX+1; // première entrée du second contexte

	for (int i = 1; i < DEF_ENTRIES_PER_CTX; ++i) {
		ctx->entries[i].type.mtype = MT_VAR;
		ctx->entries[i].type.btype = BT_BOOL;
		sprintf(ctx->entries[i].id, "tmp_%d", i);
	}

	more.entries[0].ctx = ctx;
	more.entries[0].type.mtype = MT_VAR;
	more.entries[0].type.btype = BT_INT;
	strcpy(more.entries[0].id, "second_page");

	ASSERT_EQ(ctx_byte_idx(&more.entries[0]), DEF_ENTRIES_PER_CTX*bt_sizeof(BT_BOOL));
	return 1;
}

int ctx_fprintf_null_entry_fd(void* data)
{
	(void) data;
	struct context* global = NULL;
	global = ctx_pushctx();
	ctx_fprintf(NULL, global);
	return 0;
}

int ctx_fprintf_null_entry_ctx(void* data)
{
	(void) data;
	ctx_fprintf(stderr, NULL);
	return 0;
}


int ctx_fprintf_test_1(void* data){

	struct data* dt = data;
	struct context* global = NULL;

	fprintf(stderr,"\n");
	global = ctx_pushctx(); // super-global
	struct typelist* one_int = typelist_new();
	typelist_append(one_int, BT_INT);
	ctx_newname(tokenize("WriteInt"))->type = typedesc_make_function(BT_INT, one_int);
	ctx_pushctx(); // root
	ctx_newname(tokenize("root"))->type = typedesc_make_var(BT_BOOL);
	ctx_pushctx();//main
	struct typelist* two_int = typelist_new();
	typelist_append(typelist_append(two_int, BT_INT), BT_INT);
	ctx_newname(tokenize("main"))->type = typedesc_make_function(BT_INT, two_int); 
	ctx_pushctx(); // main args
	ctx_newname(tokenize("argc"))->type = typedesc_make_var(BT_INT);
	ctx_newname(tokenize("argv"))->type = typedesc_make_var(BT_INT);

	ctx_pushctx(); // contenu du main

	

	for (size_t i = 0; i < 5; ++i) {		
		snprintf(dt->entries_names[i], MAX_IDENTIFIER_SIZE, "o%lu", i);
		ASSERT_TRUE((dt->entries[i] = ctx_newname(dt->entries_names[i])) != NULL);
		((struct entry*)dt->entries[i])->type = typedesc_make_var(BT_BOOL);	

	}
	ctx_newname(tokenize("tes1"))->type = typedesc_make_tab(BT_INT, 10);
	ctx_newname(tokenize("tes2"))->type = typedesc_make_tab(BT_BOOL, 20);
	
	ctx_pushctx();
	ctx_newname(tokenize("main"))->type = typedesc_make_function(BT_INT, two_int); 
	ctx_pushctx(); // main args
	ctx_newname(tokenize("argc"))->type = typedesc_make_var(BT_INT);
	ctx_newname(tokenize("argv"))->type = typedesc_make_var(BT_INT);

	ctx_pushctx(); // contenu du main

	
	for (size_t i = 0; i < 5; ++i) {		
		snprintf(dt->entries_names[i], MAX_IDENTIFIER_SIZE, "o%lu", i);
		ASSERT_TRUE((dt->entries[i] = ctx_newname(dt->entries_names[i])) != NULL);
		((struct entry*)dt->entries[i])->type = typedesc_make_var(BT_BOOL);	
	}

	FILE* fd = tmpfile();
	
	if(fd == NULL){
		fprintf(stderr,"ctx_fprintf_4: erreur creation de fichier temporaire\n");
		return 0;
	}

	ctx_fprintf(stderr, global);

	fclose(fd);
	return 0;
}

int main()
{
	struct test_suite add_lookup, misc, octal, ts_cgo, ctx_print;

	add_lookup = make_ts("Entry add and lookup", ent_setup, ent_teardown);
	add_test_assert(&add_lookup, empty_add_entry_crash, "Cannot add entry before push");
	add_test(&add_lookup, basic_add_entry, "ctx_newname basic");
	add_test(&add_lookup, no_lookup, "lookup of non-existent variable returns NULL");
	add_test(&add_lookup, lot_entries_one_ctx, "lot of entries, 1 context");
	add_test(&add_lookup, lot_entries_multiple_ctx, "lot of entries, lot of context");
	add_test(&add_lookup, tmp_entries, "make_temp works for 1 entry");
	add_test(&add_lookup, lot_make_tmp_multiple_ctx, "make_temp works for lot of entries in multiple context");
	add_test(&add_lookup, same_id_simple, "ctx_newname returns NULL if id exists (simple)");
	add_test(&add_lookup, same_id_lot, "ctx_newname returns NULL if id exists (lot)");
	add_test(&add_lookup, nth, "ctx_nth behaves as expected");
	add_test(&add_lookup, nth_function, "ctx_nth_function behaves as expected");

	misc = make_ts("Push/Pop", nothing, nothing);
	add_test_assert(&misc, empty_pop_crash, "Pop before push crash");
	add_test_assert(&misc, empty_cur_crash, "currentctx before push crash");
	add_test(&misc, push, "push behaves as expected");
	add_test(&misc, pop, "pop behaves as expected");
	add_test(&misc, currentctx, "currentctx behaves as expected");

	octal = make_ts("Bytes", ent_setup, ent_teardown);
	add_test(&octal, count_bytes_simple, "Allocate bytes small context");
	add_test(&octal, count_bytes_lot, "Allocate bytes big context");
	add_test(&octal, longest_path_simple, "Longest path (spec)");
	add_test(&octal, ctx_longest_path_lot, "Longest path (long)");

	ts_cgo = make_ts("ctx_byte_idx", cgo_setup, cgo_teardown);
	add_test(&ts_cgo, cgo_good_idx_simple, "bon idx une seule entry");
	add_test(&ts_cgo, cgo_good_idx_double, "bon idx 2 entry");
	add_test_assert(&ts_cgo, cgo_error_if_no_entry, "erreur si entry introuvable");
	add_test_assert(&ts_cgo, cgo_NULL_entry_fail, "erreur si entry == NULL");
	add_test_assert(&ts_cgo, cgo_error_if_no_ctx, "erreur si entry pas de context");
	add_test(&ts_cgo, cgo_big_ctx, "fonctionne avec context etendu");
	
	ctx_print = make_ts("ctx_fprintf", cgo_setup, cgo_teardown);
	add_test_assert(&ctx_print, ctx_fprintf_null_entry_fd, "erreur si FD == NULL");
	add_test_assert(&ctx_print, ctx_fprintf_null_entry_ctx, "erreur si CTX == NULL");
	add_test(&ctx_print, ctx_fprintf_test_1, "test affichage");

	
	return exec_ts(&misc) && exec_ts(&add_lookup) && exec_ts(&octal) && exec_ts(&ts_cgo) && exec_ts(&ctx_print) ? EXIT_SUCCESS : EXIT_FAILURE;
}
