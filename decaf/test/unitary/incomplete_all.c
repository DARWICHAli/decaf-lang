#include "quadops.h"
#include "context.h"
#include "entry.h"
#include "incomplete.h"
#include "gencode.h"
#include "test_suite.h"
#include <stdlib.h>
#include <stdio.h>





struct data_qlst {

    struct quad_list qlst;
    struct quad  quadTab[QUADLIST_MAX_SIZE+1];
    struct entry lhs;
    struct entry rhs;
    struct entry res;
};

int setup(void** data)
{
    (void) data;
	struct data_qlst* qlst = malloc(sizeof(struct quad_list)+(sizeof(struct quad)*QUADLIST_MAX_SIZE+1)+3*sizeof(struct entry));
	if (!qlst)
		return 0;
    ctx_pushctx();
    qlst->qlst = qlist_new();

    for (size_t i = 0; i < QUADLIST_MAX_SIZE+1; i++) {
        switch (i%8) {
            case (0):{qlst->quadTab[i] =quad_arith( &qlst->res,  &qlst->lhs,Q_ADD, &qlst->rhs); break;}
            case (1):{qlst->quadTab[i] =quad_aff(&qlst->res, &qlst->rhs); break;}
            case (2):{qlst->quadTab[i] =quad_neg(&qlst->res,&qlst->rhs); break;}
            case (3):{qlst->quadTab[i] =quad_goto(i); break;}
            case (4):{qlst->quadTab[i] =quad_ifgoto(&qlst->lhs, CMP_EQ, &qlst->rhs, i); break;}
            case (5):{qlst->quadTab[i] =quad_param(&qlst->rhs); break;}
            case (6):{qlst->quadTab[i] =quad_call(&qlst->res, &qlst->rhs); break;}
            case (7):{qlst->quadTab[i] =quad_proc(&qlst->rhs); break;}
            case (8):{qlst->quadTab[i] =quad_cst(&qlst->rhs, i); break;}
        }
        gencode(qlst->quadTab[i]);
    }




	*data = qlst;

	return 1;
}

int teardown(void** data)
{
	free(*data);
	*data = NULL;
	return 1;
}


int append_max(void *data)
{
    struct data_qlst* dt = data;
    for (size_t i = 0; i < QUADLIST_MAX_SIZE; i++) {
        qlist_append(&dt->qlst,i);
    }
    return 1;
}


int append_max_assert(void *data)
{
    struct data_qlst* dt = data;
    ASSERT_TRUE(qlist_append(&dt->qlst,QUADLIST_MAX_SIZE+1));
    return 1;
}

int append_nonexist_assert(void *data)
{
    struct data_qlst* dt = data;
    ASSERT_TRUE(qlist_append(&dt->qlst,QUADLIST_MAX_SIZE+2));
    return 1;
}
int tt_complete(void *data)
{
    (void )data;
    return 1;
}
int main(void)
{
	struct test_suite qlst;

	qlst = make_ts("incomplete", setup, teardown);
    add_test(&qlst, append_max, "erreur ");
    add_test_assert(&qlst, append_max_assert, "erreur si append apres sizemax");
    add_test_assert(&qlst, append_nonexist_assert, "erreur si append quad n'existe pas");
    add_test(&qlst, tt_complete, "erreur ");


	return exec_ts(&qlst) ? EXIT_SUCCESS : EXIT_FAILURE;
}
