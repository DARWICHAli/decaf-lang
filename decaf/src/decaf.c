#include "argparse.h"
#include "gencode.h"
#include "genasm.h"

#include <stdio.h>
#include <stdlib.h>

#include "symbols.h" // TDS - TDS entries - descripteur de type - liste de type

extern int yyparse();
extern int yydebug;

int main (int argc, char* argv[])
{
    struct params parameters = parse_args(argc, argv);

    FILE* fo = fopen(parameters.output_file, "w");
    if (!fo) {
	    perror("Cannot open output file for writing");
	    exit(EXIT_FAILURE);
    }

    yydebug = parameters.debug_mode;
    int r = yyparse();
    if (r == EXIT_FAILURE)
	    exit(EXIT_FAILURE);

    size_t sz = 0;
    struct quad* quads = get_all_quads(&sz);

    genasm("MIPS", quads, sz, fo);

    return EXIT_SUCCESS;
}
