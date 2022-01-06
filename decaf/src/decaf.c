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

const char* tokenize(const char* str) {
	static char buf[MAX_IDENTIFIER_SIZE];
	assert(strlen(str) < MAX_IDENTIFIER_SIZE && "identifier too big");
	strncpy(buf, str, MAX_IDENTIFIER_SIZE);
	return buf;
}

int main (int argc, char* argv[])
{
    struct params parameters = parse_args(argc, argv);

    FILE* fo = fopen(parameters.output_file, "w");
    if (!fo) {
	    perror("Cannot open output file for writing");
	    exit(EXIT_FAILURE);
    }
    
    // Contexte super-global
    ctx_pushctx();
    struct typelist* one_int = typelist_new();
    ctx_newname(tokenize("WriteInt"))->type = typedesc_make_function(BT_INT, one_int);

    yydebug = parameters.debug_mode;
    int r = yyparse();
    if (r == EXIT_FAILURE)
	    exit(EXIT_FAILURE);

    size_t sz = 0;
    struct quad* quads = get_all_quads(&sz);

    genasm("MIPS", quads, sz, fo);

    return EXIT_SUCCESS;
}
