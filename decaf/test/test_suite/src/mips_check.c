#include "mips_check.h"

#include <string.h>
#include <assert.h>

int check_file(FILE* f, const char* exp) {
	fseek(f, 0, SEEK_SET);
	char buf[1024];

	size_t read = fread(buf, sizeof(char), 1024, f);
	assert(read < 1024);
	buf[read] = '\0';
	size_t n_exp = strlen(exp);
	assert(n_exp < 1024);
	fclose(f);

	size_t ib, ie;
	for (ib = 0, ie = 0; ib < read && ie < n_exp;) {
		if (exp[ie] != buf[ib]) {
			if (buf[ib] == '#') {
				for (; buf[ib] != '\n' && buf[ib] != '\0'; ++ib);
			} else if (buf[ib] == ' ' && ib + 1 < read && buf[ib+1] == '#') {
				++ib;
			} else {
				fprintf(stderr, "Not matching...\n\n---[expected]---\n%s\n\n---[got]---\n%s\n\nError on char : %lu\n\n[UNTIL]\n", exp, buf, ib);
				for (size_t i = 0; i < ib; ++i)
					fprintf(stderr, "%c", buf[i]);
				return 0;
			}
		} else {
			++ie;
			++ib;
		}
	}
	if (ie != n_exp || ib < read) {
		fprintf(stderr, "Not matching...\n\n---[expected]---\n%s\n\n---[got]---\n%s\n\nError on char : %lu\n\n[UNTIL]\n", exp, buf, ib);
				for (size_t i = 0; i < ib; ++i)
					fprintf(stderr, "%c", buf[i]);
				return 0;
	}
	return ie == n_exp;
}

int print_file(FILE* f) {
	assert(f);
	fseek(f, 0, SEEK_SET);
	char buf[1024];
	size_t read;

	fprintf(stderr, "\n---[Generated]---\n");
	while ((read = fread(buf, sizeof(char), 1023, f)) > 0) {
		buf[read] = '\0';
		fprintf(stderr, "%s", buf);
	}
	return 1;
}


