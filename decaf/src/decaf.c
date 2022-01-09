#include "argparse.h"
#include "gencode.h"
#include "genasm.h"

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
    struct context* ctx;
	FILE* fo;
	if (!parameters.no_gen) {
		fo = fopen(parameters.output_file, "w");
		if (!fo) {
			perror("Cannot open output file for writing");
			exit(EXIT_FAILURE);
		}
	}

	// Contexte super-global, factoriser quelque part
	ctx = ctx_pushctx();
	struct typelist* one_int = typelist_new();
	typelist_append(one_int, BT_INT);
	ctx_newname(tokenize("WriteInt"))->type = typedesc_make_function(BT_INT, one_int);

	yydebug = parameters.debug_mode;
	int r = yyparse();
	if (r == EXIT_FAILURE)
		exit(EXIT_FAILURE);

	size_t sz = 0;
	quad_id_t* quads = get_all_quads(&sz);
	struct asm_params asmp = { .generate_entrypoint = parameters.generate_entrypoint };

	if(parameters.print_table){
        ctx_fprintf(stdout, ctx->next);
    }

	if (!parameters.no_gen)
		genasm("MIPS", quads, sz, fo, &asmp);

    
	return EXIT_SUCCESS;
}
