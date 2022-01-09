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
    struct quad_list qlst;
	qlst.used = 0;
    return qlst;
}



struct quad_list* qlist_append(struct quad_list* qlst, quad_id_t qid)
{
    assert(qlst->used < QUADLIST_MAX_SIZE && "QUADLIST_MAX_SIZE reached!");
    assert(getquad(qid) && "quad n'existe pas!");
    qlst->quads[qlst->used] = qid;
    qlst->used++;
	return qlst;
}

struct quad_list qlist_concat(struct quad_list* ql1, struct quad_list* ql2)
{
    struct quad_list new = qlist_new();

    // append ql1 and ql2 with qlist_append
    for (size_t i = 0; i < ql1->used; i++)
        qlist_append(&new, ql1->quads[i]);

    for (size_t i = 0; i < ql2->used; i++)
        qlist_append(&new, ql2->quads[i]);

    assert((ql1->used + ql2->used) == new.used && "Final sizes do not match");
    return new;
}

void qlist_complete(struct quad_list* qlst, quad_id_t qid)
{
    // complete vide !!
    // assert(qlst->used > 0);
    // assert(qid > 0 && "complete with neg quad !");
    for (size_t i = 0; i < qlst->used; i++) {
        if(((getquad(qlst->quads[i])->op == Q_IFG) || (getquad(qlst->quads[i])->op == Q_GOT)) && !(getquad(qlst->quads[i])->res))
            getquad(qlst->quads[i])->dst = qid;
    }
}
