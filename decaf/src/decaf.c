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

void check()
{
	struct entry* main = ctx_lookup(tokenize("main"));
	if(!ctx_lookup(tokenize("main")))
		exit(EXIT_FAILURE);
	if (!typedesc_is_function(&main->type) || typelist_size(typedesc_function_args(&main->type)) != 0)
		exit(EXIT_FAILURE);
}



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

	if (parameters.infile != NULL) { // to stdin
		FILE* fi = freopen(parameters.infile, "r", stdin);
		if (!fi) {
			perror("Cannot open input file");
			exit(EXIT_FAILURE);
		}
	}

	yydebug = parameters.debug_mode;
	int r = yyparse();

	check();

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

	struct asm_params asmp = { .generate_entrypoint = parameters.generate_entrypoint, .verbose = parameters.mips_verbose};

	if (!parameters.no_gen)
		genasm("MIPS", quads, sz, fo, &asmp);

	return EXIT_SUCCESS;
}
