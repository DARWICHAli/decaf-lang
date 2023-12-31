#include "typedesc.h"
#include "typelist.h"
#include <assert.h> //assert
#include <string.h> // strcmp

// Impossible d'instancier une constante avec un appel de fonction (typedesc_make_var)
// donc j'initialise à la main la structure
const struct typedesc td_var_int = { .btype = BT_INT, .mtype = MT_VAR };
const struct typedesc td_var_bool = { .btype = BT_BOOL, .mtype = MT_VAR };

size_t bt_sizeof(const enum BTYPE btype)
{
	switch (btype) {
	case BT_BOOL:
	case BT_STR:
	case BT_INT:
		return 4;
		break;
	// LCOV_EXCL_START
	default:
		assert(0 && "Unknown BTYPE");
		// LCOV_EXCL_STOP
	}
}

size_t td_sizeof(const struct typedesc* td)
{
	assert(td && "error expecting non NULL entry");

	switch (typedesc_meta_type(td)) {
	case MT_VAR:
		return bt_sizeof(typedesc_var_type(td));
	case MT_TAB:
		return bt_sizeof(typedesc_tab_type(td)) * typedesc_tab_size(td);
	case MT_FUN:
		return 0; // Une fonction fait 0 octets sur la pile
	// LCOV_EXCL_START
	default:
		assert(0 && "Unknown meta-type");
		// LCOV_EXCL_STOP
	}
}

struct typedesc typedesc_make_function(enum BTYPE ret_type, const struct typelist* arg_list)
{
	assert(arg_list && "typedesc_make_function expects non-NULL entry for arg_list");
	struct typedesc ret = { .mtype = MT_FUN, .btype = ret_type, .dist.arg_list = arg_list };
	return ret;
}

struct typedesc typedesc_make_tab(enum BTYPE elem_type, size_t nb_elem)
{
	struct typedesc ret = { .mtype = MT_TAB, .btype = elem_type, .dist.size = nb_elem };
	return ret;
}

struct typedesc typedesc_make_var(enum BTYPE btype)
{
	struct typedesc ret = { .mtype = MT_VAR, .btype = btype };
	return ret;
}

int typedesc_equals(const struct typedesc* lhs, const struct typedesc* rhs)
{
	assert(lhs && "typedesc_equals expects non-NULL entry");
	assert(rhs && "typedesc_equals expects non-NULL entry");

	return (typedesc_meta_type(lhs) == typedesc_meta_type(rhs)) && (lhs->btype == rhs->btype) &&
	       (!typedesc_is_function(lhs) || (typelist_equals(lhs->dist.arg_list, rhs->dist.arg_list))) &&
	       (!typedesc_is_tab(lhs) || lhs->dist.size == rhs->dist.size);
}

int typedesc_is_function(const struct typedesc* td)
{
	assert(td && "Expected non-null typedesc");
	return typedesc_meta_type(td) == MT_FUN;
}

int typedesc_is_var(const struct typedesc* td)
{
	assert(td && "Expected non-null typedesc");
	return typedesc_meta_type(td) == MT_VAR;
}

int typedesc_is_tab(const struct typedesc* td)
{
	assert(td && "Expected non-null typedesc");
	return typedesc_meta_type(td) == MT_TAB;
}

enum BTYPE typedesc_var_type(const struct typedesc* td)
{
	assert(td && "Expected non-null typedesc");
	assert(typedesc_is_var(td) && "typedesc is not a variable");
	return td->btype;
}

enum BTYPE typedesc_function_type(const struct typedesc* td)
{
	assert(td && "Expected non-null typedesc");
	assert(typedesc_is_function(td) && "typedesc is not a function");
	return td->btype;
}

const struct typelist* typedesc_function_args(const struct typedesc* td)
{
	assert(td && "Expected non-null typedesc");
	assert(typedesc_is_function(td) && "typedesc is not a function");
	
	return td->dist.arg_list;
}

enum BTYPE typedesc_tab_type(const struct typedesc* td)
{
	assert(td && "Expected non-null typedesc");
	assert(typedesc_is_tab(td) && "typedesc is not a tab");
	return td->btype;
}

size_t typedesc_tab_size(const struct typedesc* td)
{
	assert(td && "Expected non-null typedesc");
	assert(typedesc_is_tab(td) && "typedesc is not a tab");
	return td->dist.size;
}

enum MTYPE typedesc_meta_type(const struct typedesc* td)
{
	assert(td && "Expected non-null typedesc");
	return td->mtype;
}

const char* bt_str(enum BTYPE type)
{
	switch (type) {
	case BT_BOOL:
		return "bool";

	case BT_INT:
		return "int";

	case BT_STR:
		return "string";

	case BT_VOID:
		return "(void)";
		
	default:
		assert(0 && "Unknown BTYPE");
	}
}

void print_var(FILE* fd, const struct typedesc* td)
{
	enum BTYPE type;
	type = typedesc_var_type(td);
	fprintf(fd, "%s ", bt_str(type));
}

void print_tab(FILE* fd, const struct typedesc* td)
{
	enum BTYPE type;
	size_t size;
	type = typedesc_tab_type(td);
	size = typedesc_tab_size(td);
	fprintf(fd, "(%s)[%ld]  ", bt_str(type), size);
}

void print_fct(FILE* fd, const struct typedesc* td)
{
	enum BTYPE type;
	const struct typelist* tl;
	size_t size_arglist;

	type = typedesc_function_type(td);
	tl = typedesc_function_args(td);
	if(tl != NULL)
		size_arglist = typelist_size(tl);
	else 
		size_arglist = 0;
		
	fprintf(fd, "fonction: (");
	for (size_t i = 0; i < size_arglist; i++) {
		if (i != size_arglist - 1) {
			fprintf(fd, "%s, ", bt_str(tl->btypes[i]));
		} else {
			fprintf(fd, "%s", bt_str(tl->btypes[size_arglist - 1]));
		}
	}
	fprintf(fd, ") -> %s", bt_str(type));
}

void td_fprintf(FILE* fd, const struct typedesc* td)
{
	assert(td && "td_fprintf expecting NON null entry");
	assert(fd && "td_fprintf expecting NON null entry");

	switch (typedesc_meta_type(td)) {
	case MT_VAR:
		print_var(fd, td);
		break;
	case MT_TAB:
		print_tab(fd, td);
		break;
	case MT_FUN:
		print_fct(fd, td);
		break;
	// LCOV_EXCL_START
	default:
		assert(0 && "Unknown meta-type");
		// LCOV_EXCL_STOP
	}
}

int typedesc_is_cstring(const struct typedesc* td)
{
	assert(td && "Expected non-null typedesc");
	return typedesc_is_var(td) && typedesc_var_type(td) == BT_STR;
}
