/*
 *
 * $Log$
 * Revision 1.2  1995/04/03 13:58:57  sbs
 * first "complete" version
 *
 * Revision 1.1  1995/03/10  17:26:51  sbs
 * Initial revision
 *
 *
 */

#define GetNextId(res, ex)                                                               \
    {                                                                                    \
        DBUG_ASSERT ((ex->nodetype == N_exprs), "wrong icm-arg: N_exprs expected");      \
        DBUG_ASSERT ((ex->node[0]->nodetype == N_id), "wrong icm-arg: N_id expected");   \
        res = ex->node[0]->info.ids->id;                                                 \
        DBUG_PRINT ("PRINT", ("icm-arg found: %s", res));                                \
        ex = ex->node[1];                                                                \
    }

#define GetNextInt(res, ex)                                                              \
    {                                                                                    \
        DBUG_ASSERT ((ex->nodetype == N_exprs), "wrong icm-arg: N_exprs expected");      \
        DBUG_ASSERT ((ex->node[0]->nodetype == N_num), "wrong icm-arg: N_num expected"); \
        res = ex->node[0]->info.cint;                                                    \
        DBUG_PRINT ("PRINT", ("icm-arg found: %d", res));                                \
        ex = ex->node[1];                                                                \
    }

#define GetShape(dim, v, ex)                                                             \
    {                                                                                    \
        int i, num;                                                                      \
        v = (char **)malloc (sizeof (char *) * dim);                                     \
        for (i = 0; i < dim; i++)                                                        \
            switch (ex->node[0]->nodetype) {                                             \
            case N_id:                                                                   \
                GetNextId (v[i], ex);                                                    \
                break;                                                                   \
            case N_num:                                                                  \
                GetNextInt (num, ex);                                                    \
                v[i] = (char *)malloc (sizeof (char) * 32);                              \
                sprintf (v[i], "%d", num);                                               \
                break;                                                                   \
            default:                                                                     \
                DBUG_PRINT ("PRINT", ("found icm_arg of type: %s",                       \
                                      mdb_nodetype[ex->node[0]->nodetype]));             \
                DBUG_ASSERT (0, "wrong icm-arg in var_arg_list");                        \
            };                                                                           \
    }

#define ICM_DEF(prf)
#define ICM_STR(name) GetNextId (name, exprs);
#define ICM_INT(name) GetNextInt (name, exprs);
#define ICM_VAR(dim, name) GetShape (dim, name, exprs);
#define ICM_END(prf)

#include "icm.data"

#undef ICM_DEF
#undef ICM_STR
#undef ICM_INT
#undef ICM_VAR
#undef ICM_END
