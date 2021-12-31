#include "test_suite.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>


struct test_suite make_ts(const char* name, int (*setup)(void**), int (*teardown)(void**)) {
	assert(strlen(name) < MAX_NAME);
	struct test_suite ts;
	strcpy(ts.nom, name);
	ts.setup = setup;
	ts.teardown = teardown;
	ts.nb_tests = 0;
	return ts;
}

int test_assert(int (*f)(void*), void* data) {
	int pid, status;
	switch (pid = fork()) {
		case -1:
			perror("fork failed");
			return 0;
		case 0: // child
			f(data);
			return 0; // assert expected to abort before
		default: // parent
			if (wait(&status) == -1) {
				perror("wait failed");
				return 0;
			}
			return WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT; // expect assert to trigger
	}
}

int test_std(int (*f)(void*), void* data) {
	return f(data);
}


void add_test(struct test_suite* ts, int (*test)(void*), const char* nom) {
	assert(strlen(nom) < MAX_NAME);
	assert(ts->nb_tests < MAX_TESTS);
	ts->tests[ts->nb_tests] = test;
	strcpy(ts->nom_test[ts->nb_tests], nom);
	ts->test_call[ts->nb_tests] = test_std;
	++ts->nb_tests;
}

void add_test_assert(struct test_suite* ts, int (*test)(void*), const char* nom) {
	add_test(ts, test, nom);
	ts->test_call[ts->nb_tests-1] = test_assert;
}


int exec_ts(struct test_suite* ts) {
	fprintf(stderr, "--Executing TS %s--\n", ts->nom);
	for (size_t i = 0; i < ts->nb_tests; ++i) {
		fprintf(stderr, "Executing test %s %lu/%lu : ", ts->nom_test[i], i+1, ts->nb_tests);

		ts->setup(&ts->data);
		ts->results[i] = ts->test_call[i](ts->tests[i], ts->data);
		ts->teardown(&ts->data);

		fprintf(stderr, "\n%s\n", (ts->results[i]) ? GRE "Success" DEF : RED "Failure" DEF);
	}

	fprintf(stderr, "\n--Summary TS %s--\n", ts->nom);
	size_t suc = 0;
	for (size_t i = 0; i < ts->nb_tests; ++i) {
		fprintf(stderr, "%s : %s\n", ts->nom_test[i],  (ts->results[i]) ? GRE "Success" DEF : RED "Failure" DEF);
		if (ts->results[i])
			++suc;
	}
	fprintf(stderr, "\nTotal: %s%lu/%lu tests passed %s\n\n", (suc == ts->nb_tests) ? GRE : RED, suc, ts->nb_tests, DEF);

	return suc == ts->nb_tests;
}
