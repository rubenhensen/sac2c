/*
 *
 * $Log$
 * Revision 1.7  1995/07/04 09:27:22  hw
 * - macro GetNextDouble inserted
 * - N_double in GetShape integrated
 *
 * Revision 1.6  1995/06/02  08:46:33  hw
 * - macro GetNextFloat inserted
 * - changed macro GetShape ( N_float added)
 *
 * Revision 1.5  1995/05/29  09:40:29  sbs
 * N_bool inserted
 *
 * Revision 1.4  1995/05/24  15:17:26  sbs
 * one more DBUG_ASSERT
 *
 * Revision 1.3  1995/05/04  11:42:34  sbs
 * trf inserted in ICM-macros
 *
 * Revision 1.2  1995/04/03  13:58:57  sbs
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

#define GetNextBool(res, ex)                                                             \
    {                                                                                    \
        DBUG_ASSERT ((ex->nodetype == N_exprs), "wrong icm-arg: N_exprs expected");      \
        DBUG_ASSERT ((ex->node[0]->nodetype == N_bool),                                  \
                     "wrong icm-arg: N_bool expected");                                  \
        res = ex->node[0]->info.cint;                                                    \
        DBUG_PRINT ("PRINT", ("icm-arg found: %d(bool)", res));                          \
        ex = ex->node[1];                                                                \
    }

#define GetNextFloat(res, ex)                                                            \
    {                                                                                    \
        DBUG_ASSERT ((ex->nodetype == N_exprs), "wrong icm-arg: N_exprs expected");      \
        DBUG_ASSERT ((ex->node[0]->nodetype == N_float),                                 \
                     "wrong icm-arg: N_float expected");                                 \
        res = ex->node[0]->info.cfloat;                                                  \
        DBUG_PRINT ("PRINT", ("icm-arg found: %d", res));                                \
        ex = ex->node[1];                                                                \
    }

#define GetNextDouble(res, ex)                                                           \
    {                                                                                    \
        DBUG_ASSERT ((ex->nodetype == N_exprs), "wrong icm-arg: N_exprs expected");      \
        DBUG_ASSERT ((ex->node[0]->nodetype == N_double),                                \
                     "wrong icm-arg: N_double expected");                                \
        res = ex->node[0]->info.cdbl;                                                    \
        DBUG_PRINT ("PRINT", ("icm-arg found: %d", res));                                \
        ex = ex->node[1];                                                                \
    }

#define GetShape(dim, v, ex)                                                             \
    {                                                                                    \
        int i, num;                                                                      \
        float cfloat;                                                                    \
        double cdbl;                                                                     \
        v = (char **)malloc (sizeof (char *) * dim);                                     \
        DBUG_ASSERT ((ex->nodetype == N_exprs), "wrong icm-arg: N_exprs expected");      \
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
            case N_bool:                                                                 \
                GetNextBool (num, ex);                                                   \
                v[i] = (char *)malloc (sizeof (char) * 6);                               \
                if (num)                                                                 \
                    sprintf (v[i], "true");                                              \
                else                                                                     \
                    sprintf (v[i], "false");                                             \
                break;                                                                   \
            case N_float:                                                                \
                GetNextFloat (cfloat, ex);                                               \
                v[i] = (char *)malloc (sizeof (char) * 40);                              \
                sprintf (v[i], "%16.16g", cfloat);                                       \
                break;                                                                   \
            case N_double:                                                               \
                GetNextDouble (cdbl, ex);                                                \
                v[i] = (char *)malloc (sizeof (char) * 40);                              \
                sprintf (v[i], "%16.16g", cdbl);                                         \
                break;                                                                   \
            default:                                                                     \
                DBUG_PRINT ("PRINT", ("found icm_arg of type: %s",                       \
                                      mdb_nodetype[ex->node[0]->nodetype]));             \
                DBUG_ASSERT (0, "wrong icm-arg in var_arg_list");                        \
            };                                                                           \
    }

#define ICM_DEF(prf, trf)
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
