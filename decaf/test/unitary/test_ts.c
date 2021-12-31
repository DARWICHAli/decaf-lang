/*
 * Test de test_suite
 */
#include "test_suite.h"
#include <stdlib.h>
#include <stdio.h>

int setup(void** d) {
	int* t = malloc(sizeof(int)*50);
	if (!t)
		return 0;

	*d = t;
	t[0] = 4096;
	t[1] = 2048;
	t[2] = 1024;
	t[3] = 512;
	t[4] = 256;
	t[5] = 128;
	t[6] = 64;
	t[7] = 32;
	t[8] = 16;
	t[9] = 8;
	t[10] = 4;
	t[11] = 2;
	t[12] = 1;

	return 1;
}

int teardown(void** d) {
	free(*d);
	return 1;
}

int test_num(void* d) {
	int* t = d;
	int k = t[0];
	int i = 1;
	fprintf(stderr, "k = %d\n", k);
	while (k > 1) {
		int tmp = t[i];
		fprintf(stderr, "tmp = %d\n", tmp);
		ASSERT_TRUE(tmp == k/2);
		k = tmp;
		++i;
	}
	return 1;
}

int test_2(void* d) {
	int* t = d;
	return t[12] == 1;
}

int main() {
	struct test_suite test_suite = make_ts("test_ts", setup, teardown);

	add_test(&test_suite, test_num, "Test setup");
	add_test(&test_suite, test_2, "Test répetabilite");

	return exec_ts(&test_suite) ? EXIT_SUCCESS : EXIT_FAILURE;
}
