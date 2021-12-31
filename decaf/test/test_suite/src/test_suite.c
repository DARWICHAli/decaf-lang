#include "test_suite.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>


struct test_suite make_ts(const char* name, int (*setup)(void**), int (*teardown)(void**)) {
	assert(strlen(name) < MAX_NAME);
	struct test_suite ts;
	strcpy(ts.nom, name);
	ts.setup = setup;
	ts.teardown = teardown;
	ts.nb_tests = 0;
	return ts;
}

void add_test(struct test_suite* ts, int (*test)(void*), const char* nom) {
	assert(strlen(nom) < MAX_NAME);
	assert(ts->nb_tests < MAX_TESTS);
	ts->tests[ts->nb_tests] = test;
	strcpy(ts->nom_test[ts->nb_tests], nom);
	++ts->nb_tests;
}

int exec_ts(struct test_suite* ts) {
	fprintf(stderr, "--Executing TS %s--\n", ts->nom);
	for (size_t i = 0; i < ts->nb_tests; ++i) {
		fprintf(stderr, "Executing test %s %lu/%lu : ", ts->nom_test[i], i+1, ts->nb_tests);

		ts->setup(&ts->data);
		ts->results[i] = ts->tests[i](ts->data);
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
