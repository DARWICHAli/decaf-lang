/** 
 * TMP
 */

#include "mips.h"

#include <string.h>
#include <assert.h>

const char* Mips_reg_str[INVALID_REGISTER] = { "0",  "v0", "a0", "a1", "a2", "sp", "fp", "ra",
					       "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7" };
enum Mips_reg Mips_reg_temp[MIPS_REG_TMP_NB] = { T0, T1, T2, T3, T4, T5, T6, T7 };
const struct entry* Mips_reg_storing[MIPS_REG_TMP_NB] = { NULL };
int Mips_tmp_last_used[MIPS_REG_TMP_NB] = { 0 };
int Mips_tmp_reserved[MIPS_REG_TMP_NB] = { 0 };

struct Mips_loc entry_to_reg(const struct entry* ent)
{
	for (int i = 0; i < MIPS_REG_TMP_NB; ++i) {
		if (Mips_reg_storing[i] == ent)
			return reg(Mips_reg_temp[i]);
	}
	struct Mips_loc ret = available_register(ent);
	instr(LW, ret, entry_loc(ent));
	return ret;
}

struct Mips_loc available_register(const struct entry* ent)
{
	assert(ent);
	// Pas de doublons
	for (int i = 0; i < MIPS_REG_TMP_NB; ++i)
		if (Mips_reg_storing[i] == ent)
			return alloc_tmp_register(ent, Mips_reg_temp[i]);

	int max = -1, idx = -1;
	for (int i = 0; i < MIPS_REG_TMP_NB; ++i) {
		if ((Mips_reg_storing[i] == NULL) && !Mips_tmp_reserved[i])
			return alloc_tmp_register(ent, Mips_reg_temp[i]);
		int last_used = Mips_tmp_last_used[i];
		if (!Mips_tmp_reserved[i] && (max < last_used)) {
			max = last_used;
			idx = i;
		}
	}
	assert(idx >= 0 && "all registers reserved");
	return alloc_tmp_register(ent, Mips_reg_temp[idx]);
}

enum Mips_reg reserve_tmp_register()
{
	int max = -1, idx = -1;
	for (int i = 0; i < MIPS_REG_TMP_NB; ++i) {
		if ((Mips_reg_storing[i] == NULL) && !Mips_tmp_reserved[i]) {
			idx = i;
			break;
		}
		int last_used = Mips_tmp_last_used[i];
		if (!Mips_tmp_reserved[i] && max < last_used) {
			max = last_used;
			idx = i;
		}
	}
	assert(idx >= 0 && "all registers reserved");
	Mips_tmp_reserved[idx] = 1;
	return Mips_reg_temp[idx];
}

void free_tmp_register(enum Mips_reg reg)
{
	for (int i = 0; i < MIPS_REG_TMP_NB; ++i) {
		if (Mips_reg_temp[i] == reg) {
			assert(Mips_tmp_reserved[i] && "double free register");
			Mips_tmp_reserved[i] = 0;
			return;
		}
	}
	assert(0 && "Not a temp register");
}

struct Mips_loc alloc_tmp_register(const struct entry* ent, enum Mips_reg t)
{
	assert(ent);
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

int entry_in_tmp(const struct entry* ent)
{
	assert(ent);
	for (int i = 0; i < MIPS_REG_TMP_NB; ++i) {
		if (Mips_reg_storing[i] == ent)
			return 1;
	}
	return 0;
}

void save_reg_to_entry(enum Mips_reg r, const struct entry* ent)
{
	instr(SW, reg(r), entry_loc(ent));
}

void save_reg_tmp()
{
	for (int i = 0; i < MIPS_REG_TMP_NB; ++i) {
		if (Mips_reg_storing[i] != NULL)
			save_reg_to_entry(Mips_reg_temp[i], Mips_reg_storing[i]);
	}
}

void clear_reg_tmp()
{
	memset(Mips_reg_storing, 0, sizeof(const struct entry*) * MIPS_REG_TMP_NB);
	memset(Mips_tmp_last_used, 0, sizeof(int) * MIPS_REG_TMP_NB);
}
