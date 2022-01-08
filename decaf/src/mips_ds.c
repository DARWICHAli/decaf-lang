/**
 * DATA SEGMENT
 */

#include "mips.h"
#include "genasm.h"

#include <assert.h>
#include <stdio.h>

const char* MIPS_types[MIPS_TYPES_NB] = { ".word" };
size_t MIPS_types_size[MIPS_TYPES_NB] = { 4 };
/*
 * Type MIPS derrière un tupedesc
 */
void MIPS_type(const struct typedesc* td, char buf[MAX_TYPE_SIZE]) {
	assert(td && "typedesc must not be null");
	assert((typedesc_is_var(td) || typedesc_is_tab(td)) && "Only var or tabs can be global");

	if (typedesc_is_var(td)) {
		for (int i = 0; i < MIPS_TYPES_NB; ++i) {
			if (MIPS_types_size[i] == td_sizeof(td)) {
				int n = snprintf(buf, MAX_TYPE_SIZE, "%s", MIPS_types[i]);
				assert(n > 0 && n < MAX_TYPE_SIZE && "snprintf error");
				return;
			}
		}
		assert(0 && "Type size is not allowed");
	} else {
		int n = snprintf(buf, MAX_TYPE_SIZE, ".space %lu", td_sizeof(td));
		assert(n > 0 && n < MAX_TYPE_SIZE && "snprintf error");
	}
}


void MIPS_data_segment() {
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

