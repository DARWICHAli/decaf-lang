#include "context.h"
#include "typedesc.h"
#include "typelist.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#define CO_LEN 1000
struct context global_context[CO_LEN];
size_t co_used = 0; // nombre totale des contextes

struct context* sommet = NULL;

struct context* alloc_ctx()
{
	assert(co_used < CO_LEN && "Impossible d'allouer un nouveau contexte");

	struct context* new_context = &global_context[co_used];
	new_context->used = 0;
	new_context->next = NULL;
	co_used++;

	return new_context;
}

struct entry* ctx_search(const char* str, const struct context* ctx)
{
	assert(ctx && "non-NULL context expected");
	const struct entry* ret;
	for (size_t i = 0; i < ctx_count_entries(ctx); i++) {
		ret = ctx_nth(ctx, i);
		if (strcmp(ret->id, str) == 0) {
			return (struct entry*)ret;
		}
	}
	return NULL;
}

void rand_string(char str[MAX_IDENTIFIER_SIZE])
{
	static size_t i = 0;
	int n = snprintf(str, MAX_IDENTIFIER_SIZE, "tmp_%lu", i++);
	assert(n > 0 && "snprintf error");
	assert(n < MAX_IDENTIFIER_SIZE && "snprintf overflow");
}

struct context* ctx_pushctx()
{
	struct context* new_context = alloc_ctx();
	new_context->parent = sommet;
	sommet = new_context;
	return sommet;
}

struct context* ctx_popctx()
{
	assert(sommet && "Pas de contexte empilé!");

	sommet = (struct context*)sommet->parent; // const
	return sommet;
}

struct context* ctx_currentctx()
{
	assert(sommet && "pas de contexte");
	return sommet;
}

struct entry* ctx_addtoctx(struct context* ctx, const char id[MAX_IDENTIFIER_SIZE]) {
	struct context* curr = ctx;
	assert(curr && "No current context");

	if (ctx_search(&id[0], curr)) {
		return NULL;
	}

	size_t idx = ctx_count_entries(curr);
	struct context* it = curr;
	while (idx > DEF_ENTRIES_PER_CTX) {
		assert(it && "Expected extended context allocated at this point");
		it = it->next;
		idx -= DEF_ENTRIES_PER_CTX;
	}
	if (idx == DEF_ENTRIES_PER_CTX) { // alloc
		assert(it->next == NULL && "Already allocated extension");
		it->next = alloc_ctx();
		it = it->next;
		idx = 0;
	}

	struct entry* newe = &it->entries[idx];
	newe->ctx = curr;
	char* str = strncpy(newe->id, id, MAX_IDENTIFIER_SIZE);
	assert(str && str[MAX_IDENTIFIER_SIZE - 1] == '\0' && "strncpy overflow");
	curr->used++;

	return newe;
}


struct entry* ctx_newname(const char id[MAX_IDENTIFIER_SIZE])
{
	return ctx_addtoctx(ctx_currentctx(), id);
}

struct entry* ctx_make_temp(enum BTYPE btype)
{
	struct context* curr = ctx_currentctx();
	assert(curr && "current context is NULL");
	char str[MAX_IDENTIFIER_SIZE];
	//si on a nommer des var tmp_X
	do {
		rand_string(str);
	} while (ctx_search(str, curr));
	struct entry* ret = ctx_newname(str);
	ret->type = typedesc_make_var(btype);
	return ret;
}

struct entry* ctx_lookup(const char id[MAX_IDENTIFIER_SIZE])
{
	struct context* curr = ctx_currentctx();
	const struct entry* ent;
	while (curr) {
		ent = ctx_search(id, curr);
		if (ent != NULL) {
			return (struct entry*)ent; // const
		}

		curr = (struct context*)curr->parent; // const
	}
	return NULL;
}

size_t ctx_count_entries(const struct context* ctx)
{
	assert(ctx && "Context expected");
	return ctx->used;
}

size_t ctx_count_bytes(const struct context* ctx)
{
	assert(ctx && "Expects non-NULL context");

	size_t sum = 0;
	for (size_t i = 0; i < ctx_count_entries(ctx); i++) {
		sum += td_sizeof(&ctx_nth(ctx, i)->type);
	}
	return sum;
}

const struct entry* ctx_nth(const struct context* ctx, size_t idx)
{
	assert(ctx && "Expects non-NULL context");
	assert(idx < ctx_count_entries(ctx) && "Index too high");

	const struct context* it = ctx;
	while (idx >= DEF_ENTRIES_PER_CTX) {
		it = it->next;
		assert(it && "Index expects extended context");
		idx -= DEF_ENTRIES_PER_CTX;
	}
	return &it->entries[idx];
}

const struct entry* ctx_nth_function(const struct context* ctx, size_t idx)
{
	assert(ctx && "Expects non-NULL context");
	assert(idx < ctx_count_entries(ctx) && "Index too high");

	const struct entry *ret = NULL, *tmp;
	size_t i, j;
	for (i = 0, j = 0; j <= idx && i < ctx_count_entries(ctx); ++i) {
		tmp = ctx_nth(ctx, i);
		if (tmp->type.mtype == MT_FUN) {
			ret = tmp;
			++j;
		}
	}

	return ret;
}

size_t ctx_byte_idx(const struct entry* ent)
{
	assert(ent && "ctx_byte_idx expects non-NULL entry");
	assert(ent->ctx && "Every entry must have a context");

	const struct context* ctx = ent->ctx;
	int ret = 0;
	const struct entry* cur;
	for (size_t i = 0; i < ctx_count_entries(ctx); ++i) {
		cur = ctx_nth(ctx, i);
		if (cur == ent) // On suppose que les entrées sont identifiées par leurs adresses
			return ret;

		ret += td_sizeof(&cur->type);
	}
	// LCOV_EXCL_START
	assert(0 && "Reached end of context without finding entry !!!"); // Cette assertion est bien testée !
	// LCOV_EXCL_STOP
}

#define MAX(lhs, rhs) ((lhs) < (rhs)) ? (rhs) : (lhs)

// Complexité : (NB_CTX) * (PROFONDEUR) ~ n
size_t ctx_longest_path(const struct context* ctx)
{
	assert(ctx && "Longest past from super-global context makes no sense");

	size_t sz = ctx_count_bytes(ctx);
	size_t max = sz;
	size_t lp;

	// recherche des fils
	for (size_t i = 0; i < co_used; ++i) {
		struct context* potentiel_fils = &global_context[i];
		if (potentiel_fils->parent == ctx) { // fils de ctx
			lp = ctx_longest_path(potentiel_fils);
			max = MAX(max, sz + lp);
		}
	}

	return max;
}

const char* tokenize(const char* str) {
	static char buf[MAX_IDENTIFIER_SIZE];
	assert(strlen(str) < MAX_IDENTIFIER_SIZE && "identifier too big");
	strncpy(buf, str, MAX_IDENTIFIER_SIZE);
	return buf;
}

struct context* ctx_rootctx() {
	assert(co_used >= 2 && "No root context");
	return &global_context[1]; // 0 = super-global
}

char global_cstr[TOTAL_STR_ALLOCATED] = { 0 };
size_t char_used;

struct entry* ctx_register_cstr(const char* str) {
	static int cstr_nb = 0;
	char buf[MAX_IDENTIFIER_SIZE];
	assert(strlen(str) + char_used < TOTAL_STR_ALLOCATED && "c-str buffer is full");
	char* dst = strncpy(&global_cstr[char_used], str, TOTAL_STR_ALLOCATED - char_used);
	char_used += strlen(str);

	struct entry* ret;
	do {
		int n = snprintf(buf, MAX_IDENTIFIER_SIZE, CSTR_FMT, cstr_nb++);
		assert(n > 0 && n < MAX_IDENTIFIER_SIZE && "snprintf overflow");
		ret = ctx_addtoctx(ctx_rootctx(), buf);
	} while (!ret);

	ret->type = typedesc_make_var(BT_STR);
	ret->cstr = dst;
	return ret;
}

void ctx_push_super_global() {
	assert(sommet == NULL && "super-global context already set");
	ctx_pushctx();
	struct typelist* one_int = typelist_new();
	typelist_append(one_int, BT_INT);
	ctx_newname(tokenize("WriteInt"))->type = typedesc_make_function(BT_VOID, one_int);

	struct typelist* one_str = typelist_new();
	typelist_append(one_str, BT_STR);
	ctx_newname(tokenize("WriteString"))->type = typedesc_make_function(BT_VOID, one_str);
}
