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
int is_in(enum Q_OP op, enum Q_OP* tab, size_t size);
void write_instruction(FILE* out, const char* op, const char* one, const char* two, const char* three, size_t nb_op, const char* com);

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

#define wid_BUF_SIZE 512

struct wid {
	FILE* fo;
	char buf[wid_BUF_SIZE];
};

int wi_setup(void** data) {
	*data = malloc(sizeof(struct wid));
	if (!data)
		return 0;

	struct wid* dt = *data;
	dt->fo = fopen("/tmp/genasm_wid.mips", "w+");

	return dt->fo ? 1 : 0;
}

int wi_teardown(void** data) {
	struct wid* dt = *data;
	fclose(dt->fo);
	free(*data);
	return 1;
}

int wi_1(void* data) {
	struct wid* dt = data;
	write_instruction(dt->fo, "OP", "one", "", "", 1, "");

	fseek(dt->fo, 0, SEEK_SET);
	fread(dt->buf, sizeof(char), wid_BUF_SIZE, dt->fo);

	ASSERT_EQSTR(dt->buf, "OP one\n");
	return 1;
}

int wi_2(void* data) {
	struct wid* dt = data;
	write_instruction(dt->fo, "OP", "a", "two", "", 2, "");

	fseek(dt->fo, 0, SEEK_SET);
	fread(dt->buf, sizeof(char), wid_BUF_SIZE, dt->fo);

	ASSERT_EQSTR(dt->buf, "OP a two\n");
	return 1;
}

int wi_3(void* data) {
	struct wid* dt = data;
	write_instruction(dt->fo, "OP", "a", "b", "three", 3, "");

	fseek(dt->fo, 0, SEEK_SET);
	fread(dt->buf, sizeof(char), wid_BUF_SIZE, dt->fo);

	ASSERT_EQSTR(dt->buf, "OP a b three\n");
	return 1;
}

int wi_err(void* data) {
	struct wid* dt = data;
	write_instruction(dt->fo, "OP", "one", "lm", "po", 72, "");
	return 0;
}

int wi_empty_op(void* data) {
	struct wid* dt = data;
	write_instruction(dt->fo, "", "one", "lm", "po", 3, "");
	return 0;
}

int wi_empty_1(void* data) {
	struct wid* dt = data;
	write_instruction(dt->fo, "OP", "", "", "", 1, "");
	return 0;
}

int wi_empty_2(void* data) {
	struct wid* dt = data;
	write_instruction(dt->fo, "OP", "pl", "", "", 2, "");
	return 0;
}

int wi_empty_3(void* data) {
	struct wid* dt = data;
	write_instruction(dt->fo, "OP", "pl", "", "ol", 3, "");
	return 0;
}

int wi_com(void* data) {
	struct wid* dt = data;
	write_instruction(dt->fo, "OP", "a", "b", "three", 3, "com with spaces");

	fseek(dt->fo, 0, SEEK_SET);
	fread(dt->buf, sizeof(char), wid_BUF_SIZE, dt->fo);

	ASSERT_EQSTR(dt->buf, "OP a b three # com with spaces\n");
	return 1;
}

struct gmd {
	FILE* fo;
	struct quad quads[1];
};

int genasm_setup(void** data) {
	*data = malloc(sizeof(struct gmd));
	if (!data)
		return 0;

	struct gmd* dt = *data;
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
	genasm("", dt->quads, 1, dt->fo);
	return 0;
}

int genasm_err_quads(void* data) {
	struct gmd* dt = data;
	genasm("MIPS", NULL, 1, dt->fo);
	return 0;
}

int genasm_err_size(void* data) {
	struct gmd* dt = data;
	genasm("MIPS", dt->quads, 0, dt->fo);
	return 0;
}

int genasm_err_file(void* data) {
	struct gmd* dt = data;
	genasm("MIPS", dt->quads, 1, NULL);
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

	genasm("MIPS", dt->quads, 1, dt->fo);
	return 0;
}


int main() {
	struct test_suite ii, wi, gm;

	ii = make_ts("is_in", ii_setup, ii_teardown);
	add_test(&ii, ii_in, "is_in correctly assert if inside");
	add_test(&ii, ii_out, "is_in correctly assert if outside");

	wi = make_ts("write_instruction", wi_setup, wi_teardown);
	add_test(&wi, wi_1, "Instruction with 1 operand");
	add_test(&wi, wi_2, "Instruction with 2 operand");
	add_test(&wi, wi_3, "Instruction with 3 operand");
	add_test_assert(&wi, wi_err, "Instruction with wrong number of operands fails");
	add_test_assert(&wi, wi_empty_op, "Instruction with empty operator crash");
	add_test_assert(&wi, wi_empty_1, "Instruction with empty operand crash (1)");
	add_test_assert(&wi, wi_empty_2, "Instruction with empty operand crash (2)");
	add_test_assert(&wi, wi_empty_3, "Instruction with empty operand crash (3)");
	add_test(&wi, wi_com, "Comments are appended correctly");

	gm = make_ts("genasm", genasm_setup, genasm_teardown);
	add_test_failure(&gm, genasm_err_lang, "Error if unknown lang");
	add_test_assert(&gm, genasm_err_quads, "Vérifie le tableau de quads");
	add_test_assert(&gm, genasm_err_size, "Vérifie la taille du tableau de quads");
	add_test_assert(&gm, genasm_err_file, "Vérifie le fichier de sortie");
	add_test_assert(&gm, genasm_err_unknown_instr, "Instruction inconnue délenche assert");

	return exec_ts(&ii) && exec_ts(&wi) && exec_ts(&gm) ? EXIT_SUCCESS : EXIT_FAILURE;
}

