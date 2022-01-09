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

struct quad_list qlist_new()
{
	struct quad_list qlst = { .used = 0, .quads = { INCOMPLETE_QUAD_ID } };
	return qlst;
}

struct quad_list* qlist_append(struct quad_list* qlst, quad_id_t qid)
{
	assert(qlst->used < QUADLIST_MAX_SIZE && "QUADLIST_MAX_SIZE reached!");
	qlst->quads[qlst->used] = qid;
	qlst->used++;
	return qlst;
}

struct quad_list qlist_concat(struct quad_list* ql1, struct quad_list* ql2)
{
	struct quad_list new = qlist_new();
	assert((ql1->used + ql2->used) < QUADLIST_MAX_SIZE && "Final list would be too big");

	// append ql1 and ql2 with qlist_append
	for (size_t i = 0; i < ql1->used; i++)
		qlist_append(&new, ql1->quads[i]);

	for (size_t i = 0; i < ql2->used; i++)
		qlist_append(&new, ql2->quads[i]);

	return new;
}

void qlist_complete(struct quad_list* qlst, quad_id_t qid)
{
	assert(qid != INCOMPLETE_QUAD_ID && "Can't complete jump destination to INCOMPLETE_QUAD_ID");
	assert(getquad(qid) && "The quad to complete must exists");
	assert((getquad(qid)->op == Q_IFG || getquad(qid)->op == Q_GOT) && "can't complete non-jump instructions");

	for (size_t i = 0; i < qlst->used; i++) {
		struct quad* q = getquad(qlst->quads[i]);
		q->dst = qid;
	}
	return;
}
