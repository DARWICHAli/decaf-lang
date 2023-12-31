/**
 * @file gencode.c
 * @author Tarek Kunze <tarek.kunze@etu.unistra.fr
 * @version 1.0
 * @brief Génération de code et quadruplets
 * @ingroup IR
 */

#include "gencode.h"
#include "context.h"

#include <assert.h>

quad_id_t next = 0;
struct quad global_quads[GLOBAL_QUADS_SIZE];
quad_id_t global_qids[GLOBAL_QUADS_SIZE];

quad_id_t* get_all_quads(size_t* size)
{
	*size = next;
	return global_qids;
}

quad_id_t nextquad()
{
	return next;
}

struct quad* getquad(quad_id_t id)
{
	assert(id < next && "Invalid quad_id_t");
	return &global_quads[id];
}

quad_id_t gencode(struct quad quadop)
{
	assert(next < GLOBAL_QUADS_SIZE && "Can't alloc more quads");
	quadop.ctx = ctx_currentctx();
	global_quads[next] = quadop;
	global_qids[next] = next;
	return next++;
}
