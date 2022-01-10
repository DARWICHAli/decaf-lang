/**
 * @file gencode.c
 * @author Ali Darwich <ali.darwich@etu.unistra.fr>, Tarek Kunze <tarek.kunze@etu.unistra.fr>
 * @version 1.0
 * @brief Gestion de listes de quadruplets incomplets
 * @ingroup IR
 */

#include <stdlib.h>
#include "incomplete.h"
#include "gencode.h"
#include "quadops.h"
#include <assert.h>
#include <string.h>

#define MAXQL 10000
size_t ql_next = 0;
struct quad_list global_ql[MAXQL];

struct quad_list* qlist_empty()
{
	assert(ql_next < MAXQL && "no ql left");
	struct quad_list* ret = &global_ql[ql_next];
	ret->used = 0;
	++ql_next;
	return ret;
}

struct quad_list* qlist_new(quad_id_t qid) {
	struct quad_list* ret = qlist_empty();
	qlist_append(ret, qid);
	return ret;
}

void qlist_append(struct quad_list* qlst, quad_id_t qid)
{
	assert(qlst->used < QUADLIST_MAX_SIZE && "QUADLIST_MAX_SIZE reached!");
	qlst->quads[qlst->used] = qid;
	qlst->used++;
}

struct quad_list* qlist_concat(const struct quad_list* lhs, const struct quad_list* rhs)
{
	struct quad_list* new = qlist_empty();
	assert((lhs->used + rhs->used) < QUADLIST_MAX_SIZE && "Final list would be too big");

	// append ql1 and ql2 with qlist_append
	for (size_t i = 0; i < lhs->used; i++)
		qlist_append(new, lhs->quads[i]);

	for (size_t i = 0; i < rhs->used; i++)
		qlist_append(new, rhs->quads[i]);

	return new;
}

void qlist_complete(struct quad_list* qlst, quad_id_t qid)
{
	assert(qid != INCOMPLETE_QUAD_ID && "Can't set jump destination to INCOMPLETE_QUAD_ID");
	for (size_t i = 0; i < qlst->used; i++) {
		struct quad* q = getquad(qlst->quads[i]);
		q->dst = qid;
	}
	return;
}
