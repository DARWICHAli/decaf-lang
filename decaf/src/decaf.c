#include "argparse.h"
#include "gencode.h"
#include "genasm.h"
#include "quadops.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "symbols.h" // TDS - TDS entries - descripteur de type - liste de type

extern int yyparse();
extern int yydebug;

int main(int argc, char* argv[])
{
	struct params parameters = parse_args(argc, argv);

	FILE* fo;
	if (!parameters.no_gen) {
		fo = fopen(parameters.output_file, "w");
		if (!fo) {
			perror("Cannot open output file for writing");
			exit(EXIT_FAILURE);
		}
	}

	yydebug = parameters.debug_mode;
	int r = yyparse();

	size_t sz = 0;
	quad_id_t* quads = get_all_quads(&sz);

	if (parameters.ir_outfile) {
		FILE* fir = fopen(parameters.ir_outfile, "w");
		for (size_t i = 0; i < sz; ++i) {
			quad_fprint(fir, getquad(quads[i]));
		}
	}

	if (r == EXIT_FAILURE)
		exit(EXIT_FAILURE);

	struct asm_params asmp = { .generate_entrypoint = parameters.generate_entrypoint };

	if (!parameters.no_gen)
		genasm("MIPS", quads, sz, fo, &asmp);

	return EXIT_SUCCESS;
}
