/*
 * MIPS data segement
 */

#include "test_suite.h"

#include "genasm.h"
#include "context.h"
#include "entry.h"
#include "context.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define BUF_SIZE 4096

struct donnees {
	struct context* sg;
	struct context* root;
	struct context* main;
	struct context* maini;
	FILE* fo;
	char buf[BUF_SIZE];
};

extern size_t co_used;
extern struct context* sommet;

void strip_com(char* str) {
	int len = strlen(str);
	for (int i = 0; i < len && str[i] != '\0'; ++i) {
		if (str[i] == '#') {
			int cpt = 0;
			for (; str[i+cpt] != '\n' && str[i+cpt] != '\0'; ++cpt);
			for (int j = 0; j < cpt; ++j) {
				for (int k = i; k < len; ++k) {
					str[k] = str[k+1];
				}
			}
		}
	}
}

int setup(void** data) {
	struct donnees* dt = malloc(sizeof(struct donnees));
	if (!dt)
		return 0;
	*data = dt;

	co_used = 0;
	sommet = NULL;

	dt->sg = ctx_pushctx();
	struct entry* ent = ctx_newname(tokenize("WriteInt"));
	ent->type.mtype = MT_FUN;

	dt->root = ctx_pushctx();
	ent = ctx_newname(tokenize("globale"));
	ent->type = typedesc_make_var(BT_INT);
	ent = ctx_newname(tokenize("main"));
	ent->type.mtype = MT_FUN;
	ent = ctx_newname(tokenize("g_tab"));
	ent->type = typedesc_make_tab(BT_INT, 64);


	dt->main = ctx_pushctx();
	ctx_newname(tokenize("argc"))->type = typedesc_make_var(BT_INT);
	ctx_newname(tokenize("argv"))->type = typedesc_make_var(BT_INT);

	dt->maini = ctx_pushctx();
	ctx_newname(tokenize("loc1"))->type = typedesc_make_var(BT_INT);
	ctx_newname(tokenize("loc2"))->type = typedesc_make_var(BT_INT);

	dt->fo = fopen("/tmp/genasm_mips_ds.mips", "w+");
	if (!dt->fo)
		return 0;

	return 1;
}

int teardown(void** data) {
	struct donnees* dt = *data;
	fclose(dt->fo);
	free(*data);
	*data = NULL;
	sommet = NULL;
	co_used = 0;
	return 1;
}

void MIPS_data_segment(FILE* out);

int ds_one_var(void* data) {
	struct donnees* dt = data;
	dt->root->used = 1;

	MIPS_data_segment(dt->fo);

	fseek(dt->fo, 0, SEEK_SET);

	fread(dt->buf, 1, 4096, dt->fo);
	strip_com(dt->buf);
	ASSERT_EQSTR(dt->buf, ".data\n\n"
			"globale: .word 0 \n");

	return 1;
}

int ds_two_glob(void* data) {
	struct donnees* dt = data;

	MIPS_data_segment(dt->fo);

	fseek(dt->fo, 0, SEEK_SET);

	fread(dt->buf, 1, 4096, dt->fo);
	strip_com(dt->buf);
	ASSERT_EQSTR(dt->buf, ".data\n\n"
			"globale: .word 0 \n"
			"g_tab: .space 256\n");

	return 1;
}

int ds_unknown_mtype(void* data) {
	struct donnees* dt = data;
	dt->root->entries[dt->root->used].ctx = dt->root;
	dt->root->entries[dt->root->used].type.mtype = (enum MTYPE) -1;
	strcpy(dt->root->entries[dt->root->used].id, "bad_entryM");
	dt->root->used++;

	MIPS_data_segment(dt->fo);

	return 0;
}


int main() {
	struct test_suite ts = make_ts("MIPS data segment", setup, teardown);
	
	add_test(&ts, ds_one_var, "Segment data avec une variable globale simple");
	add_test(&ts, ds_two_glob, "Two globals with one tab");
	add_test_assert(&ts, ds_unknown_mtype, "Unknown mtype crash");

	return exec_ts(&ts) ? EXIT_SUCCESS : EXIT_FAILURE;
}
