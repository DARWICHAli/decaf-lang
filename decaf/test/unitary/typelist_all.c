#include "typelist.h"
#include "typedesc.h"
#include "test_suite.h"
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

struct donnees {
	struct typelist* lhs;
	struct typelist* rhs;
	struct typelist* diff;
	struct typelist* size;
};

int setup(void** data)
{
	struct donnees* dt = malloc(sizeof(struct donnees));
	if (!dt)
		return 0;

	dt->lhs = typelist_new();
	dt->rhs = typelist_new();
	dt->diff = typelist_new();
	dt->size = typelist_new();

	*data = dt;
	return 1;
}

int teardown(void** data)
{
	free(*data);
	*data = NULL;
	return 1;
}

int null_entry(void* data)
{
	(void)(data);
	typelist_append(NULL, BT_BOOL);
	return 0;
}

int test_append_1(void* data)
{
	struct donnees* dt = data;

	typelist_append(dt->lhs, BT_BOOL);
	typelist_append(dt->lhs, BT_INT);

	ASSERT_EQ(dt->lhs->btypes[0], BT_BOOL);
	ASSERT_EQ(dt->lhs->btypes[1], BT_INT);
	return 1;
}

int test_append_2(void* data)
{
	struct donnees* dt = data;

	typelist_append(typelist_append(dt->lhs, BT_INT), BT_BOOL);

	ASSERT_EQ(dt->lhs->btypes[0], BT_INT);
	ASSERT_EQ(dt->lhs->btypes[1], BT_BOOL);
	return 1;
}

int test_append_oversize(void* data)
{
	struct donnees* dt = data;

	for (int i = 0; i < MAX_TYPELIST_SIZE; i++)
		typelist_append(dt->lhs, BT_BOOL);

	typelist_append(dt->lhs, BT_BOOL);
	return 0;
}

struct tla_data {
	struct typelist* lhs;
	struct typelist* rhs;
	struct typelist* diff;
	struct typelist* size;
};

int tla_setup(void** data)
{
	struct tla_data* dt = malloc(sizeof(struct tla_data));
	if (!dt)
		return 0;

	dt->lhs = typelist_new();
	dt->rhs = typelist_new();
	dt->diff = typelist_new();
	dt->size = typelist_new();

	typelist_append(dt->lhs, BT_BOOL);
	typelist_append(dt->lhs, BT_BOOL);
	typelist_append(dt->rhs, BT_BOOL);
	typelist_append(dt->rhs, BT_BOOL);
	typelist_append(dt->diff, BT_BOOL);
	typelist_append(dt->diff, BT_INT);
	typelist_append(dt->size, BT_BOOL);

	*data = dt;
	return 1;
}

int tla_teardown(void** data)
{
	free(*data);
	*data = NULL;
	return 1;
}

int tl_rhs_NULL_entry_fail(void* data)
{
	struct tla_data* dt = data;
	typelist_equals(NULL, dt->rhs);
	return 0;
}

int tl_lhs_NULL_entry_fail(void* data)
{
	struct tla_data* dt = data;
	typelist_equals(dt->lhs, NULL);
	return 0;
}

int not_equal(void* data)
{
	struct tla_data* dt = data;
	ASSERT_EQ(typelist_equals(dt->lhs, dt->diff), 0);
	return 1;
}

int equal(void* data)
{
	struct tla_data* dt = data;
	ASSERT_TRUE(typelist_equals(dt->lhs, dt->rhs));
	return 1;
}

int not_same_size(void* data)
{
	struct tla_data* dt = data;
	ASSERT_EQ(typelist_equals(dt->lhs, dt->size), 0);
	return 1;
}

int max_tl(void* data)
{
	(void)(data);
	struct typelist* tl;
	for (int i = 0; i < MAX_TOTAL_TYPELIST + 1; ++i) {
		tl = typelist_new();
	}
	ASSERT_EQ(tl, NULL);
	return 1;
}

int nothing(void** data)
{
	(void)data;
	return 1;
}

int main(void)
{
	struct test_suite ts_tl = make_ts("typelist_append", setup, teardown);
	add_test_assert(&ts_tl, null_entry, "erreur si typelist == NULL");
	add_test(&ts_tl, test_append_1, "erreur si valeur pas correctement ajouté: test_append_1 ");
	add_test_assert(&ts_tl, test_append_oversize, "erreur si crash pas du au oversize de arglist ");
	add_test(&ts_tl, test_append_2, "erreur si valeur pas correctement ajouté: test_append_2 ");

	struct test_suite tla = make_ts("typelist_equals", tla_setup, tla_teardown);
	add_test_assert(&tla, tl_lhs_NULL_entry_fail, "erreur si typelist lhs == NULL");
	add_test_assert(&tla, tl_rhs_NULL_entry_fail, "erreur si typelist rhs == NULL");
	add_test(&tla, not_equal, "erreur si retourne true au lieu de false: not_equal ");
	add_test(&tla, equal, "erreur si retourne false au lieu de true: equal ");
	add_test(&tla, not_same_size, "erreur si retourne true au lieu de false: not_same_size ");

	struct test_suite tl_alloc = make_ts("Allocation de typelist", nothing, nothing);
	add_test(&tl_alloc, max_tl, "NULL returned after too much typelist allocated");
	return exec_ts(&ts_tl) && exec_ts(&tla) && exec_ts(&tl_alloc) ? EXIT_SUCCESS : EXIT_FAILURE;
}
