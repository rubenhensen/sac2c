/*
 *
 * $Log$
 * Revision 3.4  2002/07/01 16:42:14  dkr
 * GetNextIcm(): some spaces added in output
 *
 * Revision 3.3  2002/06/02 21:38:14  dkr
 * support for TAGGED_ARRAYS added
 *
 * Revision 3.2  2002/03/07 20:13:34  dkr
 * - Support for ICMs arguments of type N_icm (H-ICMs with str-, int-, var- or
 *   varint-arguments only) added (ICM_ICM).
 *   This feature is not just yet, so it might contain several bugs...
 * - Some macros replaced by functions
 *
 * Revision 3.1  2000/11/20 18:01:23  sacbase
 * new release made
 *
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
 */

#define ICM_DEF(prf, trf)                                                                \
    void Print##prf (node *exprs, node *arg_info)                                        \
    {                                                                                    \
        DBUG_ENTER ("Print" #prf);
#define ICM_ICM(name) exprs = GetNextIcm (&name, exprs);
#define ICM_STR(name) exprs = GetNextId (&name, exprs);
#define ICM_INT(name) exprs = GetNextInt (&name, exprs);
#define ICM_VAR(cnt, name)                                                               \
    if (cnt > 0) {                                                                       \
        exprs = GetNextVar (&name, NULL, cnt, exprs);                                    \
    }
#define ICM_VARINT(cnt, name)                                                            \
    if (cnt > 0) {                                                                       \
        exprs = GetNextVarInt (&name, cnt, exprs);                                       \
    }
#define ICM_END(prf, args)                                                               \
    ICMCompile##prf args;                                                                \
    DBUG_VOID_RETURN;                                                                    \
    }

/* forward declaration */
static node *GetNextVar (char ***ret, int *ret_len, int cnt, node *exprs);

/* static */
node *
GetNextIcm (char **ret, node *exprs)
{
    node *expr;
    int cnt, len;
    int i;
    char **v;

    DBUG_ENTER ("GetNextIcm");

    DBUG_ASSERT ((ret != NULL), "no return value found!");

    DBUG_ASSERT ((NODE_TYPE (exprs) == N_exprs), "wrong icm-arg: N_exprs expected");
    expr = EXPRS_EXPR (exprs);

    DBUG_ASSERT ((NODE_TYPE (expr) == N_icm), "wrong icm-arg: N_icm expected");

    cnt = CountExprs (ICM_ARGS (expr));

    GetNextVar (&v, &len, cnt, ICM_ARGS (expr));
    len += strlen (ICM_NAME (expr));
    len += 5 + 2 * cnt;

    (*ret) = (char *)Malloc (len * sizeof (char));
    (*ret)[0] = '\0';
    strcat ((*ret), ICM_NAME (expr));
    strcat ((*ret), "( ");
    if (cnt > 0) {
        strcat ((*ret), v[0]);
    }
    for (i = 1; i < cnt; i++) {
        strcat ((*ret), ", ");
        strcat ((*ret), v[i]);
        v[i] = Free (v[i]);
    }
    strcat ((*ret), ")");

    v = Free (v);

    DBUG_PRINT ("PRINT", ("icm-arg found: %s", (*ret)));

    exprs = EXPRS_NEXT (exprs);

    DBUG_RETURN (exprs);
}

static node *
GetNextId (char **ret, node *exprs)
{
    node *expr;

    DBUG_ENTER ("GetNextId");

    DBUG_ASSERT ((ret != NULL), "no return value found!");

    DBUG_ASSERT ((NODE_TYPE (exprs) == N_exprs), "wrong icm-arg: N_exprs expected");
    expr = EXPRS_EXPR (exprs);

    DBUG_ASSERT ((NODE_TYPE (expr) == N_id), "wrong icm-arg: N_id expected");
    (*ret) = StringCopy ((ID_NT_TAG (expr) != NULL) ? ID_NT_TAG (expr) : ID_NAME (expr));

    DBUG_PRINT ("PRINT", ("icm-arg found: %s", (*ret)));

    exprs = EXPRS_NEXT (exprs);

    DBUG_RETURN (exprs);
}

static node *
GetNextString (char **ret, node *exprs)
{
    node *expr;

    DBUG_ENTER ("GetNextString");

    DBUG_ASSERT ((ret != NULL), "no return value found!");

    DBUG_ASSERT ((NODE_TYPE (exprs) == N_exprs), "wrong icm-arg: N_exprs expected");
    expr = EXPRS_EXPR (exprs);

    DBUG_ASSERT ((NODE_TYPE (expr) == N_str), "wrong icm-arg: N_str expected");
    (*ret) = Malloc (strlen (STR_STRING (expr)) + 3);
    sprintf ((*ret), "\"%s\"", STR_STRING (expr));

    DBUG_PRINT ("PRINT", ("icm-arg found: %s", (*ret)));

    exprs = EXPRS_NEXT (exprs);

    DBUG_RETURN (exprs);
}

static node *
GetNextInt (int *ret, node *exprs)
{
    node *expr;

    DBUG_ENTER ("GetNextInt");

    DBUG_ASSERT ((ret != NULL), "no return value found!");

    DBUG_ASSERT ((NODE_TYPE (exprs) == N_exprs), "wrong icm-arg: N_exprs expected");
    expr = EXPRS_EXPR (exprs);

    DBUG_ASSERT ((NODE_TYPE (expr) == N_num), "wrong icm-arg: N_num expected");
    (*ret) = NUM_VAL (expr);

    DBUG_PRINT ("PRINT", ("icm-arg found: %d", (*ret)));

    exprs = EXPRS_NEXT (exprs);

    DBUG_RETURN (exprs);
}

static node *
GetNextChar (char *ret, node *exprs)
{
    node *expr;

    DBUG_ENTER ("GetNextChar");

    DBUG_ASSERT ((ret != NULL), "no return value found!");

    DBUG_ASSERT ((NODE_TYPE (exprs) == N_exprs), "wrong icm-arg: N_exprs expected");
    expr = EXPRS_EXPR (exprs);

    DBUG_ASSERT ((NODE_TYPE (expr) == N_char), "wrong icm-arg: N_char expected");
    (*ret) = CHAR_VAL (expr);

    DBUG_PRINT ("PRINT", ("icm-arg found: %d", (*ret)));

    exprs = EXPRS_NEXT (exprs);

    DBUG_RETURN (exprs);
}

static node *
GetNextBool (bool *ret, node *exprs)
{
    node *expr;

    DBUG_ENTER ("GetNextBool");

    DBUG_ASSERT ((ret != NULL), "no return value found!");

    DBUG_ASSERT ((NODE_TYPE (exprs) == N_exprs), "wrong icm-arg: N_exprs expected");
    expr = EXPRS_EXPR (exprs);

    DBUG_ASSERT ((NODE_TYPE (expr) == N_bool), "wrong icm-arg: N_bool expected");
    (*ret) = BOOL_VAL (expr);

    DBUG_PRINT ("PRINT", ("icm-arg found: %d(bool)", (*ret)));

    exprs = EXPRS_NEXT (exprs);

    DBUG_RETURN (exprs);
}

static node *
GetNextFloat (float *ret, node *exprs)
{
    node *expr;

    DBUG_ENTER ("GetNextFloat");

    DBUG_ASSERT ((ret != NULL), "no return value found!");

    DBUG_ASSERT ((NODE_TYPE (exprs) == N_exprs), "wrong icm-arg: N_exprs expected");
    expr = EXPRS_EXPR (exprs);

    DBUG_ASSERT ((NODE_TYPE (expr) == N_float), "wrong icm-arg: N_float expected");
    (*ret) = FLOAT_VAL (expr);

    DBUG_PRINT ("PRINT", ("icm-arg found: %d", (*ret)));

    exprs = EXPRS_NEXT (exprs);

    DBUG_RETURN (exprs);
}

static node *
GetNextDouble (double *ret, node *exprs)
{
    node *expr;

    DBUG_ENTER ("GetNextDouble");

    DBUG_ASSERT ((ret != NULL), "no return value found!");

    DBUG_ASSERT ((NODE_TYPE (exprs) == N_exprs), "wrong icm-arg: N_exprs expected");
    expr = EXPRS_EXPR (exprs);

    DBUG_ASSERT ((NODE_TYPE (expr) == N_double), "wrong icm-arg: N_double expected");
    (*ret) = DOUBLE_VAL (expr);

    DBUG_PRINT ("PRINT", ("icm-arg found: %d", (*ret)));

    exprs = EXPRS_NEXT (exprs);

    DBUG_RETURN (exprs);
}

static node *
GetNextVar (char ***ret, int *ret_len, int cnt, node *exprs)
{
    node *expr;
    int ival;
    char cval;
    bool bval;
    float fval;
    double dval;
    int i;
    int len = 0;

    DBUG_ENTER ("GetNextVar");

    (*ret) = (char **)Malloc (cnt * sizeof (char *));

    DBUG_ASSERT ((NODE_TYPE (exprs) == N_exprs), "wrong icm-arg: N_exprs expected");

    len = 0;
    for (i = 0; i < cnt; i++) {
        expr = EXPRS_EXPR (exprs);
        switch (NODE_TYPE (expr)) {
        case N_id:
            exprs = GetNextId (&((*ret)[i]), exprs);
            break;
        case N_str:
            exprs = GetNextString (&((*ret)[i]), exprs);
            break;
        case N_num:
            (*ret)[i] = (char *)Malloc (sizeof (char) * 50);
            exprs = GetNextInt (&ival, exprs);
            sprintf ((*ret)[i], "%d", ival);
            break;
        case N_char:
            (*ret)[i] = (char *)Malloc (sizeof (char) * 5);
            exprs = GetNextChar (&cval, exprs);
            sprintf ((*ret)[i], "%d", cval);
            break;
        case N_bool:
            (*ret)[i] = (char *)Malloc (sizeof (char) * 6);
            exprs = GetNextBool (&bval, exprs);
            if (bval) {
                sprintf ((*ret)[i], "true");
            } else {
                sprintf ((*ret)[i], "false");
            }
            break;
        case N_float:
            exprs = GetNextFloat (&fval, exprs);
            (*ret)[i] = Float2String (fval);
            break;
        case N_double:
            exprs = GetNextDouble (&dval, exprs);
            (*ret)[i] = Double2String (dval);
            break;
        default:
            DBUG_PRINT ("PRINT",
                        ("found icm_arg of type: %s", mdb_nodetype[NODE_TYPE (expr)]));
            DBUG_ASSERT (0, "wrong icm-arg in var_arg_list");
        }
        len += strlen ((*ret)[i]);
    }

    if (ret_len != NULL) {
        (*ret_len) = len;
    }

    DBUG_RETURN (exprs);
}

/* static */
node *
GetNextVarInt (int **ret, int cnt, node *exprs)
{
    node *expr;
    int i;

    DBUG_ENTER ("GetNextVarInt");

    (*ret) = (int *)Malloc (sizeof (int) * cnt);

    DBUG_ASSERT ((NODE_TYPE (exprs) == N_exprs), "wrong icm-arg: N_exprs expected");
    expr = EXPRS_EXPR (exprs);

    for (i = 0; i < cnt; i++) {
        DBUG_ASSERT ((NODE_TYPE (expr) == N_num), "wrong icm-arg: N_num expected");
        exprs = GetNextInt (&((*ret)[i]), exprs);
    }

    DBUG_RETURN (exprs);
}

#include "icm.data"

#undef ICM_DEF
#undef ICM_ICM
#undef ICM_STR
#undef ICM_INT
#undef ICM_VAR
#undef ICM_VARINT
#undef ICM_END
