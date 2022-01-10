/*
 * Test des fonctions de parsing des arguments
 */

#include "test_suite.h"

#include "argparse.h"

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <assert.h>

#define CHECK(x) do { if ((x) == -1) { fprintf(stdout,  "%s : %s\n", #x, strerror(errno)); return 0; }} while (0)

#define BUF_SIZE 512

struct data {
	int fd;
	char buf[BUF_SIZE];
	FILE* fo;
	fpos_t pos;
};

int fork_pa(int argc, char* const argv[], int* ws) {
	switch(fork()) {
		case -1:
			return -1;
		case 0: // child
			parse_args(argc, argv);
			exit(EXIT_SUCCESS);
			break;
		default: // parent
			if (wait(ws) == -1) {
				assert(0);
				perror("wait error");
				return -1;
			}
			return 1;
	}
}

void rdbuf(struct data* dt) {
	assert(fseek(dt->fo, 0, SEEK_SET) != -1);
	int n = fread(dt->buf, sizeof(char), BUF_SIZE, dt->fo);
	assert(n > 0 && "erreur fread");
	assert(n < BUF_SIZE && "fread overflow");
	dt->buf[n] = '\0';
}

int setup(void** data)
{
	struct data* dt = malloc(sizeof(struct data));
	if (!dt)
		return 0;

	*data = dt;

	fgetpos(stderr, &dt->pos);
	dt->fd = dup(STDERR_FILENO);
	dt->fo = freopen("/tmp/decaf_args.log", "w+", stderr);

	return 1;
}

int teardown(void** data) {
	struct data* dt = *data;
	dup2(dt->fd, STDERR_FILENO);
	close(dt->fd);
	clearerr(stderr);
	fsetpos(stderr, &dt->pos);
	fflush(stderr);
	free(*data);
	return 1;
}

int no_arg(void* data)
{
	(void)data;
	char* argv[] = { "decaf" };
	size_t argc = 1;

	parse_args(argc, argv);
	fflush(stderr);

	return 1;
}

int tos_short(void* data) {
	(void)data;
	char* argv[] = { "decaf", "-t" };
	size_t argc = 2;

	struct params p = parse_args(argc, argv);
	fflush(stderr);
	ASSERT_EQ(p.print_table, 1);

	return 1;
}

int tos_long(void* data) {
	(void)data;
	char* argv[] = { "decaf", "-t" };
	size_t argc = 2;

	struct params p = parse_args(argc, argv);
	fflush(stderr);
	ASSERT_EQ(p.print_table, 1);

	return 1;
}

int tos_extended(void* data) {
	(void)data;
	char* argv[] = { "decaf", "--tos" };

	size_t argc = 2;
	struct params p = parse_args(argc, argv);
	fflush(stderr);
	ASSERT_EQ(p.print_table, 1);

	return 1;
}

int version_extended(void* data) {
	struct data* dt = data;
	char* argv[] = { "decaf", "--version" };

	size_t argc = 2;
	int ws = 0;
	fork_pa(argc, argv, &ws);
	ASSERT_TRUE(WIFEXITED(ws));
	ASSERT_EQ(WEXITSTATUS(ws), EXIT_SUCCESS);

	rdbuf(dt);
	fprintf(stdout, "stderr output:\n %s\n", dt->buf);
	ASSERT_TRUE(strstr(dt->buf, "auteurs") != NULL);

	return 1;
}

int help_extended(void* data) {
	struct data* dt = data;
	char* argv[] = { "decaf", "--help" };

	size_t argc = 2;
	int ws = 0;
	fork_pa(argc, argv, &ws);
	ASSERT_TRUE(WIFEXITED(ws));
	ASSERT_EQ(WEXITSTATUS(ws), EXIT_SUCCESS);

	rdbuf(dt);
	fprintf(stdout, "stderr output:\n %s\n", dt->buf);
	ASSERT_TRUE(strstr(dt->buf, "usage:") != NULL);

	return 1;
}

int debug_extended(void* data) {
	(void)data;
	char* argv[] = { "decaf", "--debug" };

	size_t argc = 2;
	struct params p = parse_args(argc, argv);
	fflush(stderr);
	ASSERT_EQ(p.debug_mode, 1);

	return 1;
}

int parse_extended(void* data) {
	(void)data;
	char* argv[] = { "decaf", "--nogen" };

	size_t argc = 2;
	struct params p = parse_args(argc, argv);
	fflush(stderr);
	ASSERT_EQ(p.no_gen, 1);

	return 1;
}

int entrypoint_extended(void* data) {
	(void)data;
	char* argv[] = { "decaf", "--entrypoint" };

	size_t argc = 2;
	struct params p = parse_args(argc, argv);
	fflush(stderr);
	ASSERT_EQ(p.generate_entrypoint, 1);

	return 1;
}

int bad_arg_fail(void* data) {
	(void)data;
	char* argv[] = { "decaf", "--xxx" };

	size_t argc = 2;
	parse_args(argc, argv);

	return 0;
}

int outfile_short(void* data) {
	(void)data;
	char* argv[] = { "decaf", "-o", "file.mips" };

	size_t argc = 3;
	struct params p = parse_args(argc, argv);
	fflush(stderr);
	ASSERT_EQSTR(p.output_file, "file.mips");

	return 1;
}

int combination(void* data) {
	(void)data;
	char* argv[] = { "decaf", "-o", "file.mips", "-d", "-t", "in.decaf" };

	size_t argc = 6;
	struct params p = parse_args(argc, argv);
	fflush(stderr);
	ASSERT_EQSTR(p.output_file, "file.mips");
	ASSERT_EQ(p.debug_mode, 1);
	ASSERT_EQ(p.print_table, 1);
	ASSERT_EQ(p.generate_entrypoint, 0);

	return 1;
}

int missing_positional(void* data) {
	(void)(data);
	char* argv[] = { "decaf", "-o" };

	size_t argc = 2;
	parse_args(argc, argv);

	return 1;
}

int main() {
	struct test_suite ts;
	ts = make_ts("Arguments ligne de commande", setup, teardown);
	add_test(&ts, no_arg, "empty command line");
	add_test(&ts, tos_short, "-t");
	add_test(&ts, tos_long, "-tos");
	add_test(&ts, tos_extended, "--tos");
	add_test(&ts, debug_extended, "--debug");
	add_test(&ts, parse_extended, "--nogen");
	add_test(&ts, entrypoint_extended, "--entrypoint");
	add_test(&ts, version_extended, "--version works and exit");
	add_test(&ts, tos_extended, "--help works and exit");
	add_test_failure(&ts, bad_arg_fail, "bad argument fails and exit with error");
	add_test(&ts, outfile_short, "-o file.mips");
	add_test_failure(&ts, missing_positional, "-o without file fails");
	add_test(&ts, combination, "-o file.mips -d -t");

	return exec_ts(&ts) ? EXIT_SUCCESS : EXIT_FAILURE;
}


