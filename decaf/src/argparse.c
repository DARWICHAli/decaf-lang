#include "argparse.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define VERSION "V6-final"

#define NB_ARGS 9
static const char* args_str[NB_ARGS][3] = { { "-t", "-tos", "Affiche la table des symboles" },
					    { "-v", "-version", "Version du programme et noms des auteurs" },
					    { "-o", "-out", "Nom du fichier de sortie" },
					    { "-i", "-ir", "Print intermediate representation" },
					    { "-d", "-debug", "Active le mode debug du compilateur" },
					    { "-n", "-nogen", "Ne fait que le parsing" },
					    { "-e", "-entrypoint", "Génère un point d'entrée dans l'assembleur final" },
					    { "-m", "-mips-verbose", "Ajoute des commentaires au MIPS" },
					    { "-h", "-help", "Affiche ce message" } };
static int args_need[NB_ARGS] = { 0, 0, 1, 1, 0, 0, 0, 0, 0};
static int (*args_fct[NB_ARGS])(struct params* p, char* const args[]) = { arg_tos,   arg_version,    arg_out, arg_ir, arg_debug,
									  arg_nogen, arg_entrypoint, arg_mv, arg_help };

struct params default_args()
{
	struct params ret;
	ret.debug_mode = 0;
	ret.print_table = 0;
	ret.no_gen = 0;
	ret.generate_entrypoint = 0;
	ret.output_file = "out.mips";
	ret.ir_outfile = NULL;
	ret.mips_verbose = 0;
	ret.infile = NULL;
	return ret;
}

int arg_mv(struct params* p, char* const args[])
{
	(void)args;
	p->mips_verbose = 1;
	return 1;
}

int arg_nogen(struct params* p, char* const args[])
{
	(void)args;
	p->no_gen = 1;
	return 1;
}

int arg_entrypoint(struct params* p, char* const args[])
{
	(void)args;
	p->generate_entrypoint = 1;
	return 1;
}

int arg_tos(struct params* p, char* const args[])
{
	(void)args;
	p->print_table = 1;
	return 1;
}

int arg_version(struct params* p, char* const args[])
{
	(void)p;
	(void)args;

	fprintf(stderr, "decaf version: " VERSION "\n"
			"auteurs:\n"
			"- Kunze Tarek (SDSC)\n"
			"- Chéneau Léo (SDSC)\n"
			"- Damoi Neiko (SDSC)\n"
			"- Darwich Ali (SDSC)\n");

	exit(EXIT_SUCCESS);
	return 1;
}

int arg_help(struct params* p, char* const args[])
{
	(void)p;
	(void)args;

	fprintf(stderr, "usage: decaf [arg] [<] <file>\n\n"
			"args:\n");

	for (int i = 0; i < NB_ARGS; ++i) {
		fprintf(stderr, "%s / %s ", args_str[i][0], args_str[i][1]);
		for (int j = 0; j < args_need[i]; ++j) {
			fprintf(stderr, "<ARG%d> ", j);
		}
		fprintf(stderr, ": %s\n", args_str[i][2]);
	}

	exit(p != NULL ? EXIT_SUCCESS : EXIT_FAILURE);
	return 1;
}

int arg_out(struct params* p, char* const args[])
{
	assert(args && args[0] && "Expected arg");
	p->output_file = args[0]; // argv alloué, ok
	return 2;
}

int arg_ir(struct params* p, char* const args[])
{
	assert(args && args[0] && "Expected arg");
	p->ir_outfile = args[0]; // argv alloué, ok
	return 2;
}

int arg_debug(struct params* p, char* const args[])
{
	(void)args;
	p->debug_mode = 1;
	return 1;
}

// remove leading char
const char* strip(const char* str, char c)
{
	size_t len = strlen(str);
	if (len >= 2 && str[0] == c && str[1] == c)
		return &str[1];

	return str;
}

int arg(struct params* p, const char* arg, char* const argv_left[], int left)
{
	arg = strip(arg, '-');
	for (int i = 0; i < NB_ARGS; ++i) {
		if ((strcmp(args_str[i][0], arg) == 0) // short
		    || (strcmp(args_str[i][1], arg) == 0)) {
			if (left < args_need[i]) // pas asez d'arguments
				return 0;
			return args_fct[i](p, argv_left);
		}
	}
	return 0;
}

int name(struct params* p, const char* arg, char* const argv_left[], int left)
{
	(void) argv_left;
	if (left != 0) {
		fprintf(stderr, "Program name must be last arg");
		return 0;
	}
	p->infile = arg;
	return 1;
}

struct params parse_args(int argc, char* const argv[])
{
	assert(argc >= 1 && "argc is always at least 1");
	struct params ret = default_args();

	int read = 1;
	int left = argc - 1;
	for (int i = 1; i < argc; ++i) {
		if (argv[i][0] == '-') { // on est sûr que c'est un cstring
			read = arg(&ret, &argv[i][0], left > 1 ? &argv[i + 1] : NULL, left - 1);
		} else {
			read = name(&ret, &argv[i][0], left > 1 ? &argv[i + 1] : NULL, left - 1);
		}
		if (read == 0) {
			fprintf(stderr, "error parsing command line argument: %s\n", argv[i]);
			arg_help(NULL, NULL);
		}
		assert(read <= left && "Erreur in arg or name");
		left -= read;
		i += read - 1;
	}

	return ret;
}
