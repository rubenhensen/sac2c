/*
 *
 * $Log$
 * Revision 1.1  1995/03/10 17:26:51  sbs
 * Initial revision
 *
 *
 */

#define GetNextId(res, ex)                                                               \
    {                                                                                    \
        DBUG_ASSERT ((ex->nodetype == N_exprs), "wrong icm-arg: N_exprs expected");      \
        DBUG_ASSERT ((ex->node[1]->nodetype == N_id), "wrong icm-arg: N_id expected");   \
        res = ex->node[1]->info.id;                                                      \
        exprsp = ex->node[0];                                                            \
    }

#define GetNextInt(res, ex)                                                              \
    {                                                                                    \
        DBUG_ASSERT ((ex->nodetype == N_exprs), "wrong icm-arg: N_exprs expected");      \
        DBUG_ASSERT ((ex->node[1]->nodetype == N_num), "wrong icm-arg: N_num expected"); \
        res = ex->node[1]->info.cint;                                                    \
        exprsp = ex->node[0];                                                            \
    }

#define GetShape(dim, v, ex)                                                             \
    {                                                                                    \
        int i;                                                                           \
        v = (char **)malloc (size (char *) * dim);                                       \
        for (i = 0; i < dim; i++)                                                        \
            GetNextId (v[i], ex);                                                        \
    }

#define ICM_DEF(prf)
#define ICM_STR(name) GetNextId (name, exprs);
#define ICM_INT(name) GetNextInt (name, exprs);
#define ICM_VAR(dim, name) GetShape (dim, name, exprs);
#define ICM_END

#include "icm.data"
