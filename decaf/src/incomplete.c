#include <stdlib.h>
#include "incomplete.h"
#include "gencode.h"
#include "quadops.h"
#include <assert.h>
#include <string.h>

struct quad_list qlist_new()
{
    struct quad_list qlst;
    memset(qlst.quads,0,sizeof(quad_id_t)*QUADLIST_MAX_SIZE);
	qlst.used = 0;
    return qlst;
}



struct quad_list* qlist_append(struct quad_list* qlst, quad_id_t qid)
{
    assert(qlst->used+1 <= QUADLIST_MAX_SIZE && "Max Reached!");
    assert(getquad(qid) && "quad n'existe pas!");
    qlst->quads[qlst->used++] = qid;
	return qlst;
}

void qlist_complete(struct quad_list* qlst, quad_id_t qid)
{
    for (size_t i = 0; i < qlst->used; i++) {
        if((getquad(qlst->quads[i])->op == Q_IFG) && !(getquad(qlst->quads[i])->res) ){
            getquad(qlst->quads[i])->dst = qid;
        }
    }
    return;
}
