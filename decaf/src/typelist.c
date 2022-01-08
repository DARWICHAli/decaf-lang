#include "typelist.h"
#include <assert.h>

struct typelist global_tl[MAX_TOTAL_TYPELIST];
size_t used = 0;

struct typelist* typelist_new()
{
	if (used >= MAX_TOTAL_TYPELIST) {
		return NULL;
	}
	struct typelist* ret = &global_tl[used++];
	ret->used = 0;
	return ret;
}

struct typelist* typelist_append(struct typelist* lst, enum BTYPE new_type)
{
	assert(lst && "typelist_append expecting non NULL entry");
	assert(lst->used < MAX_TYPELIST_SIZE && "typelist maximum typelist size reached");

	lst->btypes[lst->used] = new_type;
	lst->used++;
	return lst;
}

int typelist_equals(const struct typelist* lhs, const struct typelist* rhs)
{
	assert(lhs && "typelist_equals expecting non NULL entry");
	assert(rhs && "typelist_equals expecting non NULL entry");
	if (lhs->used != rhs->used) {
		return 0;
	}
	for (size_t i = 0; i <= typelist_size(lhs); i++) {
		/* Incertitude avec la comparaison */
		if (lhs->btypes[i] != rhs->btypes[i])
			return 0;
	}
	return 1;
}

size_t typelist_size(const struct typelist* tl)
{
	assert(tl && "Expected non-NULL typelist");
	return tl->used;
}

size_t typelist_bytesize(const struct typelist* tl) {
	assert(tl && "Expected non-NULL typelist");
	size_t res = 0;
	for (size_t i = 0; i < typelist_size(tl); ++i) {
		res += bt_sizeof(tl->btypes[i]);
	}
	return res;
}
