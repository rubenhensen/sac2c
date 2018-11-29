#include "icm2c_basic.h"

#include "types.h"
#include "free.h"
#include "str.h"
#include "memory.h"
#include "print.h"
#include "tree_compound.h"
#include "tree_basic.h"
#include "convert.h"

#define DBUG_PREFIX "PRINT"
#include "debug.h"

#define ICM_DEF(prf, trf)                                                                \
    void Print##prf (node *exprs, info *arg_info)                                        \
    {                                                                                    \
        DBUG_ENTER ();

#define ICM_ANY(name) exprs = GetNextAny (&name, exprs);

#define ICM_ICM(name) exprs = GetNextIcm (&name, exprs);

#define ICM_NT(name) exprs = GetNextNt (&name, exprs);

#define ICM_ID(name) exprs = GetNextId (&name, exprs);

#define ICM_STR(name) exprs = GetNextString (&name, exprs);

#define ICM_INT(name) exprs = GetNextInt (&name, exprs);

#define ICM_UINT(name) exprs = GetNextUint(&name, exprs);

#define ICM_BOOL(name) exprs = GetNextBool (&name, exprs);

#define ICM_VARANY(cnt, name)                                                            \
    if (cnt > 0) {                                                                       \
        exprs = GetNextVarAny (&name, NULL, cnt, exprs);                                 \
    }

#define ICM_VARNT(cnt, name)                                                             \
    if (cnt > 0) {                                                                       \
        exprs = GetNextVarNt (&name, cnt, exprs);                                        \
    }

#define ICM_VARID(cnt, name)                                                             \
    if (cnt > 0) {                                                                       \
        exprs = GetNextVarId (&name, cnt, exprs);                                        \
    }

#define ICM_VARINT(cnt, name)                                                            \
    if (cnt > 0) {                                                                       \
        exprs = GetNextVarInt (&name, cnt, exprs);                                       \
    }

#define ICM_END(prf, args)                                                               \
    print_comment = 1;                                                                   \
    ICMCompile##prf args;                                                                \
    DBUG_RETURN ();                                                                      \
    }

/* forward declarations */
static node *GetNextVarAny (char ***ret, size_t *ret_len, size_t cnt, node *exprs);

static node *
GetNextIcm (char **ret, node *exprs)
{
    node *expr;
    size_t i,len,cnt;
    char **v, *tmp;

    DBUG_ENTER ();

    DBUG_ASSERT (ret != NULL, "no return value found!");

    DBUG_ASSERT (exprs != NULL, "wrong icm-arg: NULL found!");
    DBUG_ASSERT (NODE_TYPE (exprs) == N_exprs, "wrong icm-arg: N_exprs expected");
    expr = EXPRS_EXPR (exprs);

    DBUG_ASSERT (NODE_TYPE (expr) == N_icm, "wrong icm-arg: N_icm expected");

    cnt = TCcountExprs (ICM_ARGS (expr));

    GetNextVarAny (&v, &len, cnt, ICM_ARGS (expr));

    *ret = STRcatn (4, "SAC_", ICM_NAME (expr), "( ", cnt > 0 ? v[0] : "");

    for (i = 1; i < cnt; i++) {
        tmp = STRcatn (3, *ret, ", ", v[i]);
        *ret = MEMfree (*ret);
        *ret = tmp;
    }

    tmp = STRcat (*ret, ")");
    *ret = MEMfree (*ret);
    *ret = tmp;

    v = MEMfree (v);

    DBUG_PRINT ("icm-arg found: %s", (*ret));

    exprs = EXPRS_NEXT (exprs);

    DBUG_RETURN (exprs);
}

/*
 * N_prf together with tagged arrays is not allowed because both strings
 * have a '(' as first character!!
 */
#if 0
static
node *GetNextPrf( char **ret, node *exprs)
{
  node *expr;
  size_t cnt, len;
  char **v;

  DBUG_ENTER ();

  DBUG_ASSERT (ret != NULL, "no return value found!");

  DBUG_ASSERT (exprs != NULL, "wrong icm-arg: NULL found!");
  DBUG_ASSERT (NODE_TYPE( exprs) == N_exprs, "wrong icm-arg: N_exprs expected");
  expr = EXPRS_EXPR( exprs);

  DBUG_ASSERT (NODE_TYPE( expr) == N_prf, "wrong icm-arg: N_prf expected");

  cnt = TCcountExprs( PRF_ARGS( expr));
  DBUG_ASSERT (cnt == 2, "icm-arg N_prf: only infix notation implemented!");

  GetNextVarAny( &v, &len, cnt, PRF_ARGS( expr));

  *ret = STRcatn( 7, "(", v[0], " ", prf_string[ PRF_PRF( expr)], " ", v[1], ")");

  len += STRlen( prf_symbol[ PRF_PRF( expr)]);
  len += 5;

  v = MEMfree( v);

  DBUG_PRINT ("icm-arg found: %s", (*ret));

  exprs = EXPRS_NEXT( exprs);

  DBUG_RETURN (exprs);
}
#endif

static node *
GetNextNt (char **ret, node *exprs)
{
    node *expr;

    DBUG_ENTER ();

    DBUG_ASSERT (ret != NULL, "no return value found!");

    DBUG_ASSERT (exprs != NULL, "wrong icm-arg: NULL found!");
    DBUG_ASSERT (NODE_TYPE (exprs) == N_exprs, "wrong icm-arg: N_exprs expected");
    expr = EXPRS_EXPR (exprs);

    DBUG_ASSERT (NODE_TYPE (expr) == N_id, "wrong icm-arg: N_id expected");

    /*
     * as the backend creates id nodes without avis nodes, we
     * have to make sure to use the right name
     */
    if ((ID_NAME_OR_ICMTEXT (expr))[0] != '\0') {
        DBUG_ASSERT (ID_NT_TAG (expr) != NULL, "wrong icm-arg: no tag found");
        (*ret) = STRcpy (ID_NT_TAG (expr));
    } else {
        (*ret) = STRcpy ("");
    }

    DBUG_PRINT ("icm-arg found: %s", (*ret));

    exprs = EXPRS_NEXT (exprs);

    DBUG_RETURN (exprs);
}

static node *
GetNextId (char **ret, node *exprs)
{
    node *expr;

    DBUG_ENTER ();

    DBUG_ASSERT (ret != NULL, "no return value found!");

    DBUG_ASSERT (exprs != NULL, "wrong icm-arg: NULL found!");
    DBUG_ASSERT (NODE_TYPE (exprs) == N_exprs, "wrong icm-arg: N_exprs expected");
    expr = EXPRS_EXPR (exprs);

    DBUG_ASSERT (NODE_TYPE (expr) == N_id, "wrong icm-arg: N_id expected");
    DBUG_ASSERT (ID_NT_TAG (expr) == NULL, "wrong icm-arg: tag found");

    /*
     * we may have to use ICMTEXT here, as the backend does not
     * store the name in the avis (NAME), but directly
     * inside of the id node (ICMTEXT)
     */
    (*ret) = STRcpy (ID_NAME_OR_ICMTEXT (expr));

    DBUG_PRINT ("icm-arg found: %s", (*ret));

    exprs = EXPRS_NEXT (exprs);

    DBUG_RETURN (exprs);
}

static node *
GetNextGlobobj (char **ret, node *exprs)
{
    node *expr;

    DBUG_ENTER ();

    DBUG_ASSERT (ret != NULL, "no return value found!");

    DBUG_ASSERT (exprs != NULL, "wrong icm-arg: NULL found!");
    DBUG_ASSERT (NODE_TYPE (exprs) == N_exprs, "wrong icm-arg: N_exprs expected");
    expr = EXPRS_EXPR (exprs);

    DBUG_ASSERT (NODE_TYPE (expr) == N_globobj, "wrong icm-arg: N_globobj expected");

    (*ret) = STRcpy (OBJDEF_NT_TAG (GLOBOBJ_OBJDEF (expr)));

    DBUG_PRINT ("icm-arg found: %s", (*ret));

    exprs = EXPRS_NEXT (exprs);

    DBUG_RETURN (exprs);
}

static node *
GetNextString (char **ret, node *exprs)
{
    node *expr;

    DBUG_ENTER ();

    DBUG_ASSERT (ret != NULL, "no return value found!");

    DBUG_ASSERT (exprs != NULL, "wrong icm-arg: NULL found!");
    DBUG_ASSERT (NODE_TYPE (exprs) == N_exprs, "wrong icm-arg: N_exprs expected");
    expr = EXPRS_EXPR (exprs);

    DBUG_ASSERT (NODE_TYPE (expr) == N_str, "wrong icm-arg: N_str expected");
    (*ret) = (char *)MEMmalloc (STRlen (STR_STRING (expr)) + 3);
    sprintf ((*ret), "\"%s\"", STR_STRING (expr));

    DBUG_PRINT ("icm-arg found: %s", (*ret));

    exprs = EXPRS_NEXT (exprs);

    DBUG_RETURN (exprs);
}

static node *
GetNextByte (char *ret, node *exprs)
{
    node *expr;

    DBUG_ENTER ();

    DBUG_ASSERT (ret != NULL, "no return value found!");

    DBUG_ASSERT (exprs != NULL, "wrong icm-arg: NULL found!");
    DBUG_ASSERT (NODE_TYPE (exprs) == N_exprs, "wrong icm-arg: N_exprs expected");
    expr = EXPRS_EXPR (exprs);

    DBUG_ASSERT (NODE_TYPE (expr) == N_numbyte, "wrong icm-arg: N_numbyte expected");
    (*ret) = NUMBYTE_VAL (expr);

    DBUG_PRINT ("icm-arg found: %d", (*ret));

    exprs = EXPRS_NEXT (exprs);

    DBUG_RETURN (exprs);
}

static node *
GetNextShort (short *ret, node *exprs)
{
    node *expr;

    DBUG_ENTER ();

    DBUG_ASSERT (ret != NULL, "no return value found!");

    DBUG_ASSERT (exprs != NULL, "wrong icm-arg: NULL found!");
    DBUG_ASSERT (NODE_TYPE (exprs) == N_exprs, "wrong icm-arg: N_exprs expected");
    expr = EXPRS_EXPR (exprs);

    DBUG_ASSERT (NODE_TYPE (expr) == N_numshort, "wrong icm-arg: N_numshort expected");
    (*ret) = NUMSHORT_VAL (expr);

    DBUG_PRINT ("icm-arg found: %d", (*ret));

    exprs = EXPRS_NEXT (exprs);

    DBUG_RETURN (exprs);
}

static node *
GetNextInt (int *ret, node *exprs)
{
    node *expr;

    DBUG_ENTER ();

    DBUG_ASSERT (ret != NULL, "no return value found!");

    DBUG_ASSERT (exprs != NULL, "wrong icm-arg: NULL found!");
    DBUG_ASSERT (NODE_TYPE (exprs) == N_exprs, "wrong icm-arg: N_exprs expected");
    expr = EXPRS_EXPR (exprs);

    DBUG_ASSERT ((NODE_TYPE (expr) == N_num) || (NODE_TYPE (expr) == N_numint),
                 "wrong icm-arg: N_num or N_int expected");
    (*ret) = NUM_VAL (expr);

    DBUG_PRINT ("icm-arg found: %d", (*ret));

    exprs = EXPRS_NEXT (exprs);

    DBUG_RETURN (exprs);
}

static node *
GetNextLong (long *ret, node *exprs)
{
    node *expr;

    DBUG_ENTER ();

    DBUG_ASSERT (ret != NULL, "no return value found!");

    DBUG_ASSERT (exprs != NULL, "wrong icm-arg: NULL found!");
    DBUG_ASSERT (NODE_TYPE (exprs) == N_exprs, "wrong icm-arg: N_exprs expected");
    expr = EXPRS_EXPR (exprs);

    DBUG_ASSERT (NODE_TYPE (expr) == N_numlong, "wrong icm-arg: N_numlong expected");
    (*ret) = NUMLONG_VAL (expr);

    DBUG_PRINT ("icm-arg found: %ld", (*ret));

    exprs = EXPRS_NEXT (exprs);

    DBUG_RETURN (exprs);
}

static node *
GetNextLonglong (long long *ret, node *exprs)
{
    node *expr;

    DBUG_ENTER ();

    DBUG_ASSERT (ret != NULL, "no return value found!");

    DBUG_ASSERT (exprs != NULL, "wrong icm-arg: NULL found!");
    DBUG_ASSERT (NODE_TYPE (exprs) == N_exprs, "wrong icm-arg: N_exprs expected");
    expr = EXPRS_EXPR (exprs);

    DBUG_ASSERT (NODE_TYPE (expr) == N_numlonglong,
                 "wrong icm-arg: N_numulonglong expected");
    (*ret) = NUMLONGLONG_VAL (expr);

    DBUG_PRINT ("icm-arg found: %lld", (*ret));

    exprs = EXPRS_NEXT (exprs);

    DBUG_RETURN (exprs);
}

static node *
GetNextUbyte (unsigned char *ret, node *exprs)
{
    node *expr;

    DBUG_ENTER ();

    DBUG_ASSERT (ret != NULL, "no return value found!");

    DBUG_ASSERT (exprs != NULL, "wrong icm-arg: NULL found!");
    DBUG_ASSERT (NODE_TYPE (exprs) == N_exprs, "wrong icm-arg: N_exprs expected");
    expr = EXPRS_EXPR (exprs);

    DBUG_ASSERT (NODE_TYPE (expr) == N_numubyte, "wrong icm-arg: N_numubyte expected");
    (*ret) = NUMUBYTE_VAL (expr);

    DBUG_PRINT ("icm-arg found: %hhu", (*ret));

    exprs = EXPRS_NEXT (exprs);

    DBUG_RETURN (exprs);
}

static node *
GetNextUshort (unsigned short *ret, node *exprs)
{
    node *expr;

    DBUG_ENTER ();

    DBUG_ASSERT (ret != NULL, "no return value found!");

    DBUG_ASSERT (exprs != NULL, "wrong icm-arg: NULL found!");
    DBUG_ASSERT (NODE_TYPE (exprs) == N_exprs, "wrong icm-arg: N_exprs expected");
    expr = EXPRS_EXPR (exprs);

    DBUG_ASSERT (NODE_TYPE (expr) == N_numushort, "wrong icm-arg: N_numushort expected");
    (*ret) = NUMUSHORT_VAL (expr);

    DBUG_PRINT ("icm-arg found: %hu", (*ret));

    exprs = EXPRS_NEXT (exprs);

    DBUG_RETURN (exprs);
}

static node *
GetNextUint (unsigned int *ret, node *exprs)
{
    node *expr;

    DBUG_ENTER ();

    DBUG_ASSERT (ret != NULL, "no return value found!");

    DBUG_ASSERT (exprs != NULL, "wrong icm-arg: NULL found!");
    DBUG_ASSERT (NODE_TYPE (exprs) == N_exprs, "wrong icm-arg: N_exprs expected");
    expr = EXPRS_EXPR (exprs);

    DBUG_ASSERT (NODE_TYPE (expr) == N_numuint, "wrong icm-arg: N_numuint expected");
    (*ret) = NUMUINT_VAL (expr);

    DBUG_PRINT ("icm-arg found: %u", (*ret));

    exprs = EXPRS_NEXT (exprs);

    DBUG_RETURN (exprs);
}

static node *
GetNextUlong (unsigned long *ret, node *exprs)
{
    node *expr;

    DBUG_ENTER ();

    DBUG_ASSERT (ret != NULL, "no return value found!");

    DBUG_ASSERT (exprs != NULL, "wrong icm-arg: NULL found!");
    DBUG_ASSERT (NODE_TYPE (exprs) == N_exprs, "wrong icm-arg: N_exprs expected");
    expr = EXPRS_EXPR (exprs);

    DBUG_ASSERT (NODE_TYPE (expr) == N_numulong, "wrong icm-arg: N_numulong expected");
    (*ret) = NUMULONG_VAL (expr);

    DBUG_PRINT ("icm-arg found: %lu", (*ret));

    exprs = EXPRS_NEXT (exprs);

    DBUG_RETURN (exprs);
}

static node *
GetNextUlonglong (unsigned long long *ret, node *exprs)
{
    node *expr;

    DBUG_ENTER ();

    DBUG_ASSERT (ret != NULL, "no return value found!");

    DBUG_ASSERT (exprs != NULL, "wrong icm-arg: NULL found!");
    DBUG_ASSERT (NODE_TYPE (exprs) == N_exprs, "wrong icm-arg: N_exprs expected");
    expr = EXPRS_EXPR (exprs);

    DBUG_ASSERT (NODE_TYPE (expr) == N_numulonglong,
                 "wrong icm-arg: N_numulonglong expected");
    (*ret) = NUMULONGLONG_VAL (expr);

    DBUG_PRINT ("icm-arg found: %llu", (*ret));

    exprs = EXPRS_NEXT (exprs);

    DBUG_RETURN (exprs);
}

static node *
GetNextChar (char *ret, node *exprs)
{
    node *expr;

    DBUG_ENTER ();

    DBUG_ASSERT (ret != NULL, "no return value found!");

    DBUG_ASSERT (exprs != NULL, "wrong icm-arg: NULL found!");
    DBUG_ASSERT (NODE_TYPE (exprs) == N_exprs, "wrong icm-arg: N_exprs expected");
    expr = EXPRS_EXPR (exprs);

    DBUG_ASSERT (NODE_TYPE (expr) == N_char, "wrong icm-arg: N_char expected");
    (*ret) = CHAR_VAL (expr);

    DBUG_PRINT ("icm-arg found: %d", (*ret));

    exprs = EXPRS_NEXT (exprs);

    DBUG_RETURN (exprs);
}

static node *
GetNextBool (bool *ret, node *exprs)
{
    node *expr;

    DBUG_ENTER ();

    DBUG_ASSERT (ret != NULL, "no return value found!");

    DBUG_ASSERT (exprs != NULL, "wrong icm-arg: NULL found!");
    DBUG_ASSERT (NODE_TYPE (exprs) == N_exprs, "wrong icm-arg: N_exprs expected");
    expr = EXPRS_EXPR (exprs);

    DBUG_ASSERT (NODE_TYPE (expr) == N_bool, "wrong icm-arg: N_bool expected");
    (*ret) = BOOL_VAL (expr);

    DBUG_PRINT ("icm-arg found: %d(bool)", (*ret));

    exprs = EXPRS_NEXT (exprs);

    DBUG_RETURN (exprs);
}

static node *
GetNextFloat (float *ret, node *exprs)
{
    node *expr;

    DBUG_ENTER ();

    DBUG_ASSERT (ret != NULL, "no return value found!");

    DBUG_ASSERT (exprs != NULL, "wrong icm-arg: NULL found!");
    DBUG_ASSERT (NODE_TYPE (exprs) == N_exprs, "wrong icm-arg: N_exprs expected");
    expr = EXPRS_EXPR (exprs);

    DBUG_ASSERT (NODE_TYPE (expr) == N_float, "wrong icm-arg: N_float expected");
    (*ret) = FLOAT_VAL (expr);

    DBUG_PRINT ("icm-arg found: %g", (double)(*ret));

    exprs = EXPRS_NEXT (exprs);

    DBUG_RETURN (exprs);
}

static node *
GetNextFloatvec (floatvec *ret, node *exprs)
{
    node *expr;

    DBUG_ENTER ();

    DBUG_ASSERT (ret != NULL, "no return value found!");

    DBUG_ASSERT (exprs != NULL, "wrong icm-arg: NULL found!");
    DBUG_ASSERT (NODE_TYPE (exprs) == N_exprs, "wrong icm-arg: N_exprs expected");
    expr = EXPRS_EXPR (exprs);

    DBUG_ASSERT (NODE_TYPE (expr) == N_floatvec, "wrong icm-arg: N_float expected");
    (*ret) = FLOATVEC_VAL (expr);

    DBUG_PRINT ("icm-arg found: [%f,...]", (double)(((float *)ret)[0]));

    exprs = EXPRS_NEXT (exprs);

    DBUG_RETURN (exprs);
}

static node *
GetNextDouble (double *ret, node *exprs)
{
    node *expr;

    DBUG_ENTER ();

    DBUG_ASSERT (ret != NULL, "no return value found!");

    DBUG_ASSERT (exprs != NULL, "wrong icm-arg: NULL found!");
    DBUG_ASSERT (NODE_TYPE (exprs) == N_exprs, "wrong icm-arg: N_exprs expected");
    expr = EXPRS_EXPR (exprs);

    DBUG_ASSERT (NODE_TYPE (expr) == N_double, "wrong icm-arg: N_double expected");
    (*ret) = DOUBLE_VAL (expr);

    DBUG_PRINT ("icm-arg found: %g", (*ret));

    exprs = EXPRS_NEXT (exprs);

    DBUG_RETURN (exprs);
}

static node *
GetNextAny (char **ret, node *exprs)
{
    char cval;
    char byval;
    short sval;
    int ival;
    long lval;
    long long llval;
    unsigned char ubyval;
    unsigned short usval;
    unsigned int uival;
    unsigned long ulval;
    unsigned long long ullval;
    bool bval;
    float fval;
    floatvec fvval;
    double dval;
    node *expr;

    DBUG_ENTER ();

    DBUG_ASSERT (exprs != NULL, "wrong icm-arg: NULL found!");
    DBUG_ASSERT (NODE_TYPE (exprs) == N_exprs, "wrong icm-arg: N_exprs expected");
    expr = EXPRS_EXPR (exprs);
    switch (NODE_TYPE (expr)) {
    case N_icm:
        exprs = GetNextIcm (ret, exprs);
        break;
#if 0
    case N_prf:
      exprs = GetNextPrf( ret, exprs);
      break;
#endif
    case N_id:
        if (ID_NT_TAG (expr) != NULL) {
            exprs = GetNextNt (ret, exprs);
        } else {
            exprs = GetNextId (ret, exprs);
        }
        break;
    case N_globobj:
        exprs = GetNextGlobobj (ret, exprs);
        break;
    case N_str:
        exprs = GetNextString (ret, exprs);
        break;
    case N_num:
        (*ret) = (char *)MEMmalloc (sizeof (char) * 50);
        exprs = GetNextInt (&ival, exprs);
        sprintf ((*ret), "%d", ival);
        break;
    case N_numbyte:
        (*ret) = (char *)MEMmalloc (sizeof (char) * 50);
        exprs = GetNextByte (&byval, exprs);
        sprintf ((*ret), "%hhd", byval);
        break;
    case N_numshort:
        (*ret) = (char *)MEMmalloc (sizeof (char) * 50);
        exprs = GetNextShort (&sval, exprs);
        sprintf ((*ret), "%hd", sval);
        break;
    case N_numint:
        (*ret) = (char *)MEMmalloc (sizeof (char) * 50);
        exprs = GetNextInt (&ival, exprs);
        sprintf ((*ret), "%d", ival);
        break;
    case N_numlong:
        (*ret) = (char *)MEMmalloc (sizeof (char) * 50);
        exprs = GetNextLong (&lval, exprs);
        sprintf ((*ret), "%ld", lval);
        break;
    case N_numlonglong:
        (*ret) = (char *)MEMmalloc (sizeof (char) * 50);
        exprs = GetNextLonglong (&llval, exprs);
        sprintf ((*ret), "%lldLL", llval);
        break;
    case N_numubyte:
        (*ret) = (char *)MEMmalloc (sizeof (char) * 50);
        exprs = GetNextUbyte (&ubyval, exprs);
        sprintf ((*ret), "%hhu", ubyval);
        break;
    case N_numushort:
        (*ret) = (char *)MEMmalloc (sizeof (char) * 50);
        exprs = GetNextUshort (&usval, exprs);
        sprintf ((*ret), "%hu", usval);
        break;
    case N_numuint:
        (*ret) = (char *)MEMmalloc (sizeof (char) * 50);
        exprs = GetNextUint (&uival, exprs);
        sprintf ((*ret), "%u", uival);
        break;
    case N_numulong:
        (*ret) = (char *)MEMmalloc (sizeof (char) * 50);
        exprs = GetNextUlong (&ulval, exprs);
        sprintf ((*ret), "%lu", ulval);
        break;
    case N_numulonglong:
        (*ret) = (char *)MEMmalloc (sizeof (char) * 50);
        exprs = GetNextUlonglong (&ullval, exprs);
        sprintf ((*ret), "%lluULL", ullval);
        break;
    case N_char:
        (*ret) = (char *)MEMmalloc (sizeof (char) * 5);
        exprs = GetNextChar (&cval, exprs);
        sprintf ((*ret), "%d", cval);
        break;
    case N_bool:
        (*ret) = (char *)MEMmalloc (sizeof (char) * 6);
        exprs = GetNextBool (&bval, exprs);
        if (bval) {
            sprintf ((*ret), "true");
        } else {
            sprintf ((*ret), "false");
        }
        break;
    case N_float:
        exprs = GetNextFloat (&fval, exprs);
        (*ret) = CVfloat2String (fval);
        break;
    case N_floatvec:
        exprs = GetNextFloatvec (&fvval, exprs);
        (*ret) = CVfloatvec2String (fvval);
        break;

    case N_double:
        exprs = GetNextDouble (&dval, exprs);
        (*ret) = CVdouble2String (dval);
        break;
    default:
        DBUG_UNREACHABLE ("illegal icm-arg found!");
    }

    DBUG_RETURN (exprs);
}

static node *
GetNextVarAny (char ***ret, size_t *ret_len, size_t cnt, node *exprs)
{
    size_t i,len = 0;

    DBUG_ENTER ();

    (*ret) = (char **)MEMmalloc (cnt * sizeof (char *));

    DBUG_ASSERT (exprs != NULL, "wrong icm-arg: NULL found!");
    DBUG_ASSERT (NODE_TYPE (exprs) == N_exprs, "wrong icm-arg: N_exprs expected");

    len = 0;
    for (i = 0; i < cnt; i++) {
        exprs = GetNextAny (&((*ret)[i]), exprs);
        len += STRlen ((*ret)[i]);
    }

    if (ret_len != NULL) {
        (*ret_len) = len;
    }

    DBUG_RETURN (exprs);
}

static node *
GetNextVarNt (char ***ret, size_t cnt, node *exprs)
{
    node *expr;
    size_t i;

    DBUG_ENTER ();

    (*ret) = (char **)MEMmalloc (cnt * sizeof (char *));

    DBUG_ASSERT (exprs != NULL, "wrong icm-arg: NULL found!");
    DBUG_ASSERT (NODE_TYPE (exprs) == N_exprs, "wrong icm-arg: N_exprs expected");
    expr = EXPRS_EXPR (exprs);

    for (i = 0; i < cnt; i++) {
        exprs = GetNextNt (&((*ret)[i]), exprs);
    }

    DBUG_RETURN (exprs);
}

#if 0
static
node *GetNextVarId( char ***ret, int cnt, node *exprs)
{
  node *expr;
  int i;

  DBUG_ENTER ();

  (*ret) = (char **) MEMmalloc( cnt * sizeof( char*));

  DBUG_ASSERT (exprs != NULL, "wrong icm-arg: NULL found!");
  DBUG_ASSERT (NODE_TYPE( exprs) == N_exprs, "wrong icm-arg: N_exprs expected");
  expr = EXPRS_EXPR( exprs);

  for (i = 0; i < cnt; i++) {
    exprs = GetNextId( &((*ret)[i]), exprs);
  }

  DBUG_RETURN (exprs);
}
#endif

static node *
GetNextVarInt (int **ret, size_t cnt, node *exprs)
{
    node *expr;
    size_t i;

    DBUG_ENTER ();

    (*ret) = (int *)MEMmalloc (cnt * sizeof (int));

    DBUG_ASSERT (exprs != NULL, "wrong icm-arg: NULL found!");
    DBUG_ASSERT (NODE_TYPE (exprs) == N_exprs, "wrong icm-arg: N_exprs expected");
    expr = EXPRS_EXPR (exprs);

    for (i = 0; i < cnt; i++) {
        exprs = GetNextInt (&((*ret)[i]), exprs);
    }

    DBUG_RETURN (exprs);
}

#include "icm.data"

#undef ICM_DEF
#undef ICM_ANY
#undef ICM_ICM
#undef ICM_NT
#undef ICM_ID
#undef ICM_STR
#undef ICM_INT
#undef ICM_UINT
#undef ICM_BOOL
#undef ICM_VARANY
#undef ICM_VARNT
#undef ICM_VARID
#undef ICM_VARINT
#undef ICM_END

#undef DBUG_PREFIX
