/** 
 * TMP
 */

#include "mips.h"

#include <string.h>

const char* Mips_reg_str[INVALID_REGISTER] = { "0", "v0", "a0", "a1", "a2", "sp", "fp", "ra", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7" };
enum Mips_reg Mips_reg_temp[MIPS_REG_TMP_NB] = { T0, T1, T2, T3, T4, T5, T6, T7 };
const struct entry* Mips_reg_storing[MIPS_REG_TMP_NB] = { NULL };
int Mips_tmp_last_used[MIPS_REG_TMP_NB] = { 0 };

struct Mips_loc entry_to_reg(const struct entry* ent) {
	for (int i = 0; i < MIPS_REG_TMP_NB; ++i) {
		if (Mips_reg_storing[i] == ent)
			return reg(Mips_reg_temp[i]);
	}
	struct Mips_loc rent = available_register(ent);
	instr(LW, rent, entry_loc(ent));
	return rent;
}

struct Mips_loc available_register(const struct entry* ent) {
	// Pas de doublons
	for (int i = 0; i < MIPS_REG_TMP_NB; ++i)
		if (Mips_reg_storing[i] == ent)
			return alloc_tmp_register(ent, Mips_reg_temp[i]);

	int max = 0, idx = 0;
	for (int i = 0; i < MIPS_REG_TMP_NB; ++i) {
		if (Mips_reg_storing[i] == NULL)
			return alloc_tmp_register(ent, Mips_reg_temp[i]);
		int last_used = Mips_tmp_last_used[i];
		if (max < last_used) {
			max = last_used;
			idx = i;
		}
	}
	return alloc_tmp_register(ent, Mips_reg_temp[idx]);
}
	
struct Mips_loc alloc_tmp_register(const struct entry* ent, enum Mips_reg t) {
	for (int i = 0; i < MIPS_REG_TMP_NB; ++i) {
		if (Mips_reg_temp[i] == t) {
			if (Mips_reg_storing[i] != NULL) // sauvegarde
				instr(SW, reg(Mips_reg_temp[i]), entry_loc(Mips_reg_storing[i]));

			Mips_tmp_last_used[i] = 0;
			Mips_reg_storing[i] = ent;
		} else {
			++Mips_tmp_last_used[i];
		}
	}
	return reg(t);
}

int entry_in_tmp(const struct entry* ent) {
	for (int i = 0; i < MIPS_REG_TMP_NB; ++i) {
		if (Mips_reg_storing[i] == ent)
			return 1;
	}
	return 0;
}

void save_reg_tmp() {
	for (int i = 0; i < MIPS_REG_TMP_NB; ++i) {
		if (Mips_reg_storing[i] != NULL)
			instr(SW, reg(Mips_reg_temp[i]), entry_loc(Mips_reg_storing[i]));
	}
}

void clear_reg_tmp() {
	memset(Mips_reg_storing, 0, sizeof(const struct entry*) * MIPS_REG_TMP_NB);
	memset(Mips_tmp_last_used, 0, sizeof(int) * MIPS_REG_TMP_NB);
}
