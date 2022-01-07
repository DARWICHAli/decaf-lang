#include "typedesc.h"
#include "typelist.h"
#include "test_suite.h"
#include <stdlib.h>
#include <stdio.h>

int bt_setup(void** data)
{
	(void)(data);
	return 1;
}

int bt_teardown(void** data)
{
	(void)(data);
	return 1;
}

int right_size(void* data)
{
	(void)(data);
	ASSERT_EQ(bt_sizeof(BT_INT), 4);
	ASSERT_EQ(bt_sizeof(BT_BOOL), 4);
	ASSERT_EQ(bt_sizeof(BT_BOOL), bt_sizeof(BT_BOOL));
	return 1;
}

int bt_bad_type(void* data)
{
	(void)(data);
	bt_sizeof((enum BTYPE) - 1);
	return 0;
}

struct data_td {
	struct typedesc lhs_b;
	struct typedesc rhs_b;
	struct typedesc lhs_i;
	struct typedesc rhs_i;
	struct typedesc var_int;
	struct typedesc var_bool;
	struct typedesc lst_tab;
	struct typedesc lst_func;
	struct typedesc fct_int_void;
	struct typedesc fct_bool;
	struct typedesc fct_bool2;
	struct typedesc fct_bool2b;
	struct typelist* tl;
	size_t size;
};

int setup(void** data)
{
	struct data_td* dt = malloc(sizeof(struct data_td));
	if (!dt)
		return 0;

	dt->lhs_b = typedesc_make_var(BT_BOOL);
	dt->rhs_b = typedesc_make_var(BT_BOOL);
	dt->lhs_i = typedesc_make_var(BT_INT);
	dt->rhs_i = typedesc_make_var(BT_INT);

	dt->tl = typelist_new();
	typelist_append(typelist_append(dt->tl, BT_BOOL), BT_INT);
	dt->lst_func = typedesc_make_function(BT_INT, dt->tl);
	dt->var_bool = typedesc_make_var(BT_BOOL);
	dt->var_int = typedesc_make_var(BT_INT);

	dt->size = 15;
	dt->lst_tab = typedesc_make_tab(BT_BOOL, dt->size);

	struct typelist* tlvoid = typelist_new();
	dt->fct_int_void = typedesc_make_function(BT_INT, tlvoid);

	struct typelist* tlint1 = typelist_new();
	typelist_append(tlint1, BT_INT);
	dt->fct_bool = typedesc_make_function(BT_BOOL, tlint1);

	struct typelist* tlint2 = typelist_new();
	typelist_append(tlint2, BT_INT);
	typelist_append(tlint2, BT_INT);
	dt->fct_bool2 = typedesc_make_function(BT_BOOL, tlint2);

	struct typelist* tlint2b = typelist_new();
	typelist_append(tlint2b, BT_INT);
	typelist_append(tlint2b, BT_BOOL);
	dt->fct_bool2b = typedesc_make_function(BT_BOOL, tlint2b);

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
	td_sizeof(NULL);
	return 0;
}

int check_size_var(void* data)
{
	struct data_td* dt = data;
	ASSERT_EQ(td_sizeof(&dt->var_int), bt_sizeof(BT_INT));
	ASSERT_EQ(td_sizeof(&dt->var_bool), bt_sizeof(BT_BOOL));
	return 1;
}

int check_size_tab(void* data)
{
	struct data_td* dt = data;
	ASSERT_EQ(td_sizeof(&dt->lst_tab), (bt_sizeof(BT_BOOL) * dt->size));
	return 1;
}

int check_size_func(void* data)
{
	struct data_td* dt = data;
	ASSERT_EQ(td_sizeof(&dt->lst_func), 0);
	return 1;
}

int tde_rhs_NULL_entry_fail(void* data)
{
	struct data_td* dt = data;
	typedesc_equals(NULL, &dt->rhs_b);

	return 0;
}

int tde_lhs_NULL_entry_fail(void* data)
{
	struct data_td* dt = data;
	typedesc_equals(&dt->lhs_b, NULL);

	return 0;
}

int tde_different_types_var(void* data)
{
	(void)(data);
	ASSERT_EQ(typedesc_equals(&td_var_int, &td_var_bool), 0);
	return 1;
}

int tde_same_types_var(void* data)
{
	struct data_td* dt = data;
	ASSERT_TRUE(typedesc_equals(&td_var_int, &dt->var_int));
	return 1;
}

int tde_different_types_func(void* data)
{
	struct data_td* dt = data;
	ASSERT_EQ(typedesc_equals(&dt->fct_bool, &dt->fct_int_void), 0);
	return 1;
}

int tde_same_types_and_different_arglist_func(void* data)
{
	struct data_td* dt = data;
	ASSERT_EQ(typedesc_equals(&dt->fct_bool2, &dt->fct_bool2b), 0);
	return 1;
}

int tde_same_types_and_arglist_func(void* data)
{
	struct data_td* dt = data;
	struct typelist* tlint1 = typelist_new();
	typelist_append(tlint1, BT_INT);
	struct typedesc fct_bool_bis = typedesc_make_function(BT_BOOL, tlint1);
	ASSERT_TRUE(typedesc_equals(&dt->fct_bool, &fct_bool_bis));
	return 1;
}

int NULL_entry_fail(void* data)
{
	struct data_td* dt = data;
	dt->lst_func = typedesc_make_function(BT_BOOL, NULL);

	return 0;
}

int td_var(void* data)
{
	(void)(data);
	struct typedesc tmp = typedesc_make_var(BT_INT);
	ASSERT_TRUE(typedesc_is_var(&tmp));
	ASSERT_EQ(typedesc_var_type(&tmp), BT_INT);
	return 1;
}

int td_tab(void* data)
{
	(void)(data);
	struct typedesc tmp = typedesc_make_tab(BT_INT, 64);
	ASSERT_TRUE(typedesc_is_tab(&tmp));
	ASSERT_EQ(typedesc_tab_type(&tmp), BT_INT);
	ASSERT_EQ(typedesc_tab_size(&tmp), 64);
	return 1;
}

int td_func(void* data)
{
	struct data_td* dt = data;
	struct typedesc tmp = typedesc_make_function(BT_INT, dt->tl);
	ASSERT_TRUE(typedesc_is_function(&tmp));
	ASSERT_EQ(typedesc_function_type(&tmp), BT_INT);
	ASSERT_TRUE(typelist_equals(typedesc_function_args(&tmp), dt->tl));
	return 1;
}

int td_fprintf_null_entry_fd(void *data)
{
	struct data_td* dt = data;
	td_fprintf(NULL,&dt->fct_bool2b);
	return 0;
}

int td_fprintf_null_entry_td()
{
	
	FILE* fd = tmpfile();
	if(fd == NULL)
		fprintf(stderr,"td_fprintf_1: erreur creation de fichier temporaire\n");
	
	td_fprintf(fd,NULL);
	fclose(fd);
	return 0;
}

int td_fprintf_1(void *data)
{
	struct data_td* dt = data;
	FILE* fd = tmpfile();
	if(fd == NULL)
		fprintf(stderr,"td_fprintf_1: erreur creation de fichier temporaire\n");
	td_fprintf(fd,&dt->fct_bool2b);
	char c[1];
	size_t nread;
	int i = 0;
	char expected[] = "function: (int, bool) -> bool";
	while((nread = fread(c, 1, sizeof(c), fd))){
		if(c[0] != expected[i] ){
			fprintf(stderr,"td_fprintf_1: mauvais affichage");
			return 0;
		}
		i++;
	}
	fclose(fd);
	return 1;
}

int td_fprintf_2(void *data)
{
	struct data_td* dt = data;
	FILE* fd = tmpfile();
	if(fd == NULL)
		fprintf(stderr,"td_fprintf_2: erreur creation de fichier temporaire\n");
	td_fprintf(fd,&dt->fct_bool);
	char c[1];
	size_t nread;
	int i = 0;
	char expected[] = "function: (int) -> bool";
	while((nread = fread(c, 1, sizeof(c), fd))){
		if(c[0] != expected[i]){
			fprintf(stderr,"td_fprintf_2: mauvais affichage");
			return 0;
		}
		i++;
	}
	fclose(fd);
	return 1;
}

int td_fprintf_3(void *data)
{
	struct data_td* dt = data;
	FILE* fd = tmpfile();
	if(fd == NULL)
		fprintf(stderr,"td_fprintf_3: erreur creation de fichier temporaire\n");
	td_fprintf(fd,&dt->lst_tab);
	char c[1];
	size_t nread;
	int i = 0;
	char expected[] = "tab: (bool)[15] ";
	while((nread = fread(c, 1, sizeof(c), fd))){
		if (ferror(fd)) {
			return 0;
    	}
		if(c[0] != expected[i]){
			fprintf(stderr,"td_fprintf_3: mauvais affichage");
			return 0;
		}
		i++;
	}
	fclose(fd);


	return 1;
}

int td_fprintf_4(void *data)
{
	struct data_td* dt = data;
	FILE* fd = tmpfile();
	if(fd == NULL)
		fprintf(stderr,"td_fprintf_4: erreur creation de fichier temporaire\n");
	td_fprintf(fd,&dt->lhs_b);
	char c[1];
	size_t nread;
	int i = 0;
	char expected[] = "variable: int ";
	while((nread = fread(c, 1, sizeof(c), fd))){
		if (ferror(fd)) {
			return 0;
    	}
		if(c[0] != expected[i]){
			fprintf(stderr,"td_fprintf_4: mauvais affichage");
			return 0;
		}
		i++;
	}
	fclose(fd);
	return 1;
}

int main(void)
{
	struct test_suite bt, td;
	bt = make_ts("bt_sizeof", setup, teardown);
	add_test(&bt, right_size, "Bonne tailles pour les types primaires");
	add_test_assert(&bt, bt_bad_type, "Mauvais type primaire plante");

	td = make_ts("typedesc", setup, teardown);
	add_test_assert(&td, null_entry, "erreur si entree null");
	add_test(&td, check_size_var, "Bonne taille pour variable");
	add_test(&td, check_size_tab, "Bonne taille pour tab");
	add_test(&td, check_size_func, "Bonne taille pour fonction");

	add_test_assert(&td, tde_lhs_NULL_entry_fail, "erreur si typedesc lhs == NULL");
	add_test_assert(&td, tde_rhs_NULL_entry_fail, "erreur si typedesc rhs == NULL");
	add_test(&td, tde_different_types_var, "comparaison variables different type");
	add_test(&td, tde_same_types_var, "comparaison variables meme type");
	add_test(&td, tde_different_types_func, "fonctoons different types");
	add_test(&td, tde_same_types_and_arglist_func, "fonctions memes types et memes params");
	add_test(&td, tde_same_types_and_different_arglist_func, "fonctions memes types differents params");

	add_test_assert(&td, NULL_entry_fail, "erreur si arglist == NULL");

	add_test(&td, td_var, "typedesc var behaves correctly");
	add_test(&td, td_func, "typedesc func behaves correctly");
	add_test(&td, td_tab, "typedesc tab behaves correctly");
	add_test(&td, td_fprintf_1, "mauvais affichage fonction");
	add_test(&td, td_fprintf_2, "mauvais affichage fonction");
	add_test(&td, td_fprintf_3, "mauvais affichage tab");
	add_test(&td, td_fprintf_4, "mauvais affichage variable");


	add_test_assert(&td, td_fprintf_null_entry_fd, "erreru si fd == NULL");
	add_test_assert(&td, td_fprintf_null_entry_fd, "erreru si td == NULL");

	return exec_ts(&bt) && exec_ts(&td) ? EXIT_SUCCESS : EXIT_FAILURE;
}
