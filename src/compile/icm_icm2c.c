/*
 *
 * $Log$
 * Revision 2.1  1999/02/23 12:42:48  sacbase
 * new release made
 *
 * Revision 1.2  1998/06/23 12:51:18  cg
 * implemented new ICM argument type VARINT for a variable number
 * of integer arguments.
 *
 * Revision 1.1  1998/04/25 16:21:33  sbs
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

#define GetNextString(res, ex)                                                           \
    {                                                                                    \
        DBUG_ASSERT ((ex->nodetype == N_exprs), "wrong icm-arg: N_exprs expected");      \
        DBUG_ASSERT ((ex->node[0]->nodetype == N_str), "wrong icm-arg: N_str expected"); \
        res = Malloc (strlen (ex->node[0]->info.id) + 3);                                \
        sprintf (res, "\"%s\"", ex->node[0]->info.id);                                   \
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

#define GetNextChar(res, ex)                                                             \
    {                                                                                    \
        DBUG_ASSERT ((ex->nodetype == N_exprs), "wrong icm-arg: N_exprs expected");      \
        DBUG_ASSERT ((ex->node[0]->nodetype == N_char),                                  \
                     "wrong icm-arg: N_char expected");                                  \
        res = ex->node[0]->info.cchar;                                                   \
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
        v = (char **)Malloc (sizeof (char *) * dim);                                     \
        DBUG_ASSERT ((ex->nodetype == N_exprs), "wrong icm-arg: N_exprs expected");      \
        for (i = 0; i < dim; i++)                                                        \
            switch (ex->node[0]->nodetype) {                                             \
            case N_id:                                                                   \
                GetNextId (v[i], ex);                                                    \
                break;                                                                   \
            case N_str:                                                                  \
                GetNextString (v[i], ex);                                                \
                break;                                                                   \
            case N_num:                                                                  \
                GetNextInt (num, ex);                                                    \
                v[i] = (char *)Malloc (sizeof (char) * 32);                              \
                sprintf (v[i], "%d", num);                                               \
                break;                                                                   \
            case N_char:                                                                 \
                GetNextChar (num, ex);                                                   \
                v[i] = (char *)Malloc (sizeof (char) * 5);                               \
                sprintf (v[i], "%d", num);                                               \
                break;                                                                   \
            case N_bool:                                                                 \
                GetNextBool (num, ex);                                                   \
                v[i] = (char *)Malloc (sizeof (char) * 6);                               \
                if (num)                                                                 \
                    sprintf (v[i], "true");                                              \
                else                                                                     \
                    sprintf (v[i], "false");                                             \
                break;                                                                   \
            case N_float:                                                                \
                GetNextFloat (cfloat, ex);                                               \
                v[i] = Float2String (cfloat);                                            \
                break;                                                                   \
            case N_double:                                                               \
                GetNextDouble (cdbl, ex);                                                \
                v[i] = Double2String (cdbl);                                             \
                break;                                                                   \
            default:                                                                     \
                DBUG_PRINT ("PRINT", ("found icm_arg of type: %s",                       \
                                      mdb_nodetype[ex->node[0]->nodetype]));             \
                DBUG_ASSERT (0, "wrong icm-arg in var_arg_list");                        \
            };                                                                           \
    }

#define GetVarInt(dim, v, ex)                                                            \
    {                                                                                    \
        int i, num;                                                                      \
        v = (int *)Malloc (sizeof (int) * dim);                                          \
        DBUG_ASSERT ((ex->nodetype == N_exprs), "wrong icm-arg: N_exprs expected");      \
        for (i = 0; i < dim; i++) {                                                      \
            DBUG_ASSERT ((ex->node[0]->nodetype == N_num),                               \
                         "wrong icm-arg: N_num expected");                               \
            GetNextInt (num, ex);                                                        \
            v[i] = num;                                                                  \
        }                                                                                \
    }

#define ICM_DEF(prf, trf)                                                                \
    void Print##prf (node *exprs, node *arg_info)                                        \
    {                                                                                    \
        DBUG_ENTER ("Print" #prf);
#define ICM_STR(name) GetNextId (name, exprs);
#define ICM_INT(name) GetNextInt (name, exprs);
#define ICM_VAR(dim, name)                                                               \
    if (dim > 0)                                                                         \
        GetShape (dim, name, exprs);
#define ICM_VARINT(dim, name)                                                            \
    if (dim > 0)                                                                         \
        GetVarInt (dim, name, exprs);
#define ICM_END(prf, args)                                                               \
    ICMCompile##prf args;                                                                \
    DBUG_VOID_RETURN;                                                                    \
    }

#include "icm.data"

#undef ICM_DEF
#undef ICM_STR
#undef ICM_INT
#undef ICM_VAR
#undef ICM_VARINT
#undef ICM_END
