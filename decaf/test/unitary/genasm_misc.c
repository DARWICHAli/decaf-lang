/*
 * Test de fonctions annexes
 */

#include "genasm.h"

#include "quadops.h"
#include "context.h"
#include "entry.h"

#include "test_suite.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern enum Q_OP req_rhs[7];
extern size_t next;
extern quad_id_t global_qids[GLOBAL_QUADS_SIZE];

int ii_setup(void** data) {
	(void)(data);
	return 1;
}

int ii_teardown(void** data) {
	(void)(data);
	return 1;
}

int ii_in(void* data) {
	(void)(data);
	ASSERT_TRUE(is_in(Q_ADD, req_rhs, 7));
	return 1;
}

int ii_out(void* data) {
	(void)(data);
	ASSERT_TRUE(!is_in(Q_NEG, req_rhs, 7));
	return 1;
}

struct gmd {
	FILE* fo;
	struct quad quads[1];
	struct asm_params ap;
	size_t sz;
};

int genasm_setup(void** data) {
	*data = malloc(sizeof(struct gmd));
	if (!data)
		return 0;

	struct gmd* dt = *data;
	dt->ap.generate_entrypoint = 1;
	dt->fo = fopen("/tmp/genasm_gmd.mips", "w+");

	return dt->fo ? 1 : 0;
}

int genasm_teardown(void** data) {
	struct gmd* dt = *data;
	fclose(dt->fo);
	free(*data);
	return 1;
}

int genasm_err_lang(void* data) {
	struct gmd* dt = data;
	next = 1;
	*getquad(0) = quad_endproc();
	genasm("", get_all_quads(&dt->sz), 1, dt->fo, &dt->ap);
	return 0;
}

int genasm_err_quads(void* data) {
	struct gmd* dt = data;
	genasm("MIPS", NULL, 1, dt->fo, &dt->ap);
	return 0;
}

int genasm_err_size(void* data) {
	struct gmd* dt = data;
	genasm("MIPS", get_all_quads(&dt->sz), 0, dt->fo, &dt->ap);
	return 0;
}

int genasm_err_file(void* data) {
	struct gmd* dt = data;
	genasm("MIPS", get_all_quads(&dt->sz), 1, NULL, &dt->ap);
	return 0;
}

int genasm_err_params(void* data) {
	struct gmd* dt = data;
	genasm("MIPS", get_all_quads(&dt->sz), 1, dt->fo, NULL);
	return 0;
}


int genasm_err_unknown_instr(void* data) {
	struct gmd* dt = data;
	struct context root;
	root.used = 3;
	root.entries[0].type.mtype = MT_VAR;
	root.entries[0].type.btype = BT_INT;
	strcpy(root.entries[0].id, "g1");
	root.entries[1].type.mtype = MT_VAR;
	root.entries[1].type.btype = BT_INT;
	strcpy(root.entries[1].id, "g2");
	root.entries[2].type.mtype = MT_FUN;
	root.entries[2].type.btype = BT_INT;
	strcpy(root.entries[2].id, "main");

	struct context main;
	main.used = 2;
	main.parent = &root;
	main.entries[0].type.mtype = MT_VAR;
	main.entries[0].type.btype = BT_INT;
	strcpy(main.entries[0].id, "argc");
	main.entries[1].type.mtype = MT_VAR;
	main.entries[1].type.btype = BT_INT;
	strcpy(main.entries[1].id, "argv");

	struct context main_i;
	main_i.used = 1;
	main_i.parent = &main;
	main_i.entries[0].type.mtype = MT_VAR;
	main_i.entries[0].type.btype = BT_INT;
	strcpy(main_i.entries[0].id, "res");

	dt->quads[0].lhs = &root.entries[0];
	dt->quads[0].rhs = &root.entries[1];
	dt->quads[0].res = &main_i.entries[0];
	dt->quads[0].op = (enum Q_OP) -1;

	next = 1;
	*getquad(0) = dt->quads[0];

	genasm("MIPS", get_all_quads(&dt->sz), 1, dt->fo, &dt->ap);
	return 0;
}


int main() {
	struct test_suite ii, gm;

	ii = make_ts("is_in", ii_setup, ii_teardown);
	add_test(&ii, ii_in, "is_in correctly assert if inside");
	add_test(&ii, ii_out, "is_in correctly assert if outside");

	gm = make_ts("genasm", genasm_setup, genasm_teardown);
	add_test_failure(&gm, genasm_err_lang, "Error if unknown lang");
	add_test_assert(&gm, genasm_err_quads, "Vérifie le tableau de quads");
	add_test_assert(&gm, genasm_err_size, "Vérifie la taille du tableau de quads");
	add_test_assert(&gm, genasm_err_file, "Vérifie le fichier de sortie");
	add_test_assert(&gm, genasm_err_params, "Vérifie les paramètres");
	add_test_assert(&gm, genasm_err_unknown_instr, "Instruction inconnue délenche assert");

	return exec_ts(&ii) && exec_ts(&gm) ? EXIT_SUCCESS : EXIT_FAILURE;
}

