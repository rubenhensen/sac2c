/*
 *
 * $Log$
 * Revision 3.21  2004/11/29 17:29:49  sah
 * use of new compound macro ID_NAME_OR_SPNAME
 *
 * Revision 3.20  2004/11/25 10:26:46  jhb
 * compile SACdevCamp 2k4
 *
 * Revision 3.19  2004/03/10 00:10:17  dkrHH
 * old backend removed
 *
 * Revision 3.18  2003/11/06 08:29:02  sbs
 * GetNextVarId commented out for new backend
 *
 * Revision 3.17  2002/10/10 23:51:08  dkr
 * ICM_STR added
 *
 * Revision 3.16  2002/10/08 16:38:13  dkr
 * some DBUG_ASSERTs added
 *
 * Revision 3.15  2002/09/11 23:08:50  dkr
 * prf_string replaced by prf_symbol
 *
 * Revision 3.14  2002/08/06 08:57:29  dkr
 * cc warnings eliminated
 *
 * Revision 3.13  2002/07/15 14:47:50  dkr
 * bug in GetNextIcm() fixed: prefix 'SAC_' for icm name added
 *
 * Revision 3.12  2002/07/12 22:39:19  dkr
 * GetNextPrf() added but deactivated
 *
 * Revision 3.11  2002/07/10 20:07:03  dkr
 * GetNextVarId() added
 *
 * Revision 3.10  2002/07/10 19:44:47  dkr
 * bugs in GetNextNt() and GetNextAny() fixed
 *
 * Revision 3.9  2002/07/10 19:24:14  dkr
 * several ICM_... types added and renamed
 *
 * Revision 3.8  2002/07/10 16:24:15  dkr
 * ICM_ANY added, ICM_VAR renamed into ICM_VARANY
 *
 * Revision 3.7  2002/07/08 22:06:40  dkr
 * GetNextVar(): N_icm added
 *
 * Revision 3.6  2002/07/02 13:02:08  dkr
 * icm2c_basic.h included
 *
 * Revision 3.5  2002/07/02 08:58:40  dkr
 * all fundefs are static now
 *
 * Revision 3.4  2002/07/01 16:42:14  dkr
 * GetNextIcm(): some spaces added in output
 *
 * Revision 3.3  2002/06/02 21:38:14  dkr
 * support for new backend added
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

#include "icm2c_basic.h"

#include "types.h"
#include "free.h"
#include "internal_lib.h"
#include "print.h"
#include "tree_compound.h"
#include "tree_basic.h"
#include "convert.h"

#include <string.h>

#define ICM_DEF(prf, trf)                                                                \
    void Print##prf (node *exprs, node *arg_info)                                        \
    {                                                                                    \
        DBUG_ENTER ("Print" #prf);

#define ICM_ANY(name) exprs = GetNextAny (&name, exprs);

#define ICM_ICM(name) exprs = GetNextIcm (&name, exprs);

#define ICM_NT(name) exprs = GetNextNt (&name, exprs);

#define ICM_ID(name) exprs = GetNextId (&name, exprs);

#define ICM_STR(name) exprs = GetNextString (&name, exprs);

#define ICM_INT(name) exprs = GetNextInt (&name, exprs);

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
    DBUG_VOID_RETURN;                                                                    \
    }

/* forward declarations */
static node *GetNextVarAny (char ***ret, int *ret_len, int cnt, node *exprs);

static node *
GetNextIcm (char **ret, node *exprs)
{
    node *expr;
    int cnt, len;
    int i;
    char **v;

    DBUG_ENTER ("GetNextIcm");

    DBUG_ASSERT ((ret != NULL), "no return value found!");

    DBUG_ASSERT ((exprs != NULL), "wrong icm-arg: NULL found!");
    DBUG_ASSERT ((NODE_TYPE (exprs) == N_exprs), "wrong icm-arg: N_exprs expected");
    expr = EXPRS_EXPR (exprs);

    DBUG_ASSERT ((NODE_TYPE (expr) == N_icm), "wrong icm-arg: N_icm expected");

    cnt = TCcountExprs (ICM_ARGS (expr));

    GetNextVarAny (&v, &len, cnt, ICM_ARGS (expr));
    len += strlen (ICM_NAME (expr));
    len += 8 + 2 * cnt;

    (*ret) = (char *)ILIBmalloc (len * sizeof (char));
    (*ret)[0] = '\0';
    strcat ((*ret), "SAC_");
    strcat ((*ret), ICM_NAME (expr));
    strcat ((*ret), "( ");
    if (cnt > 0) {
        strcat ((*ret), v[0]);
    }
    for (i = 1; i < cnt; i++) {
        strcat ((*ret), ", ");
        strcat ((*ret), v[i]);
        v[i] = ILIBfree (v[i]);
    }
    strcat ((*ret), ")");

    v = ILIBfree (v);

    DBUG_PRINT ("PRINT", ("icm-arg found: %s", (*ret)));

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
  int cnt, len;
  char **v;

  DBUG_ENTER( "GetNextPrf");

  DBUG_ASSERT( (ret != NULL), "no return value found!");

  DBUG_ASSERT( (exprs != NULL), "wrong icm-arg: NULL found!");
  DBUG_ASSERT( (NODE_TYPE( exprs) == N_exprs),
               "wrong icm-arg: N_exprs expected");
  expr = EXPRS_EXPR( exprs);

  DBUG_ASSERT( (NODE_TYPE( expr) == N_prf), "wrong icm-arg: N_prf expected");

  cnt = TCcountExprs( PRF_ARGS( expr));
  DBUG_ASSERT( (cnt == 2), "icm-arg N_prf: only infix notation implemented!");

  GetNextVarAny( &v, &len, cnt, PRF_ARGS( expr));
  len += strlen( prf_symbol[ PRF_PRF( expr)]);
  len += 5;

  (*ret) = (char *) ILIBmalloc( len * sizeof( char));
  (*ret)[0] = '\0';
  strcat( (*ret), "(");
  strcat( (*ret), v[0]);
  strcat( (*ret), " ");
  strcat( (*ret), prf_string[ PRF_PRF( expr)]);
  strcat( (*ret), " ");
  strcat( (*ret), v[1]);
  strcat( (*ret), ")");

  v = ILIBfree( v);

  DBUG_PRINT( "PRINT", ("icm-arg found: %s", (*ret)));

  exprs = EXPRS_NEXT( exprs);

  DBUG_RETURN( exprs);
}
#endif

static node *
GetNextNt (char **ret, node *exprs)
{
    node *expr;

    DBUG_ENTER ("GetNextNt");

    DBUG_ASSERT ((ret != NULL), "no return value found!");

    DBUG_ASSERT ((exprs != NULL), "wrong icm-arg: NULL found!");
    DBUG_ASSERT ((NODE_TYPE (exprs) == N_exprs), "wrong icm-arg: N_exprs expected");
    expr = EXPRS_EXPR (exprs);

    DBUG_ASSERT ((NODE_TYPE (expr) == N_id), "wrong icm-arg: N_id expected");

    /*
     * as the backend creates id nodes without avis nodes, we
     * have to make sure to use the right name
     */
    if ((ID_NAME_OR_SPNAME (expr))[0] != '\0') {
        DBUG_ASSERT ((ID_NT_TAG (expr) != NULL), "wrong icm-arg: no tag found");
        (*ret) = ILIBstringCopy (ID_NT_TAG (expr));
    } else {
        (*ret) = ILIBstringCopy ("");
    }

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

    DBUG_ASSERT ((exprs != NULL), "wrong icm-arg: NULL found!");
    DBUG_ASSERT ((NODE_TYPE (exprs) == N_exprs), "wrong icm-arg: N_exprs expected");
    expr = EXPRS_EXPR (exprs);

    DBUG_ASSERT ((NODE_TYPE (expr) == N_id), "wrong icm-arg: N_id expected");
    DBUG_ASSERT ((ID_NT_TAG (expr) == NULL), "wrong icm-arg: tag found");

    /*
     * we may have to use SPNAME here, as the backend does not
     * store the name in the avis (NAME), but directly
     * inside of the id node (SPNAME)
     */
    (*ret) = ILIBstringCopy (ID_NAME_OR_SPNAME (expr));

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

    DBUG_ASSERT ((exprs != NULL), "wrong icm-arg: NULL found!");
    DBUG_ASSERT ((NODE_TYPE (exprs) == N_exprs), "wrong icm-arg: N_exprs expected");
    expr = EXPRS_EXPR (exprs);

    DBUG_ASSERT ((NODE_TYPE (expr) == N_str), "wrong icm-arg: N_str expected");
    (*ret) = ILIBmalloc (strlen (STR_STRING (expr)) + 3);
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

    DBUG_ASSERT ((exprs != NULL), "wrong icm-arg: NULL found!");
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

    DBUG_ASSERT ((exprs != NULL), "wrong icm-arg: NULL found!");
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

    DBUG_ASSERT ((exprs != NULL), "wrong icm-arg: NULL found!");
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

    DBUG_ASSERT ((exprs != NULL), "wrong icm-arg: NULL found!");
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

    DBUG_ASSERT ((exprs != NULL), "wrong icm-arg: NULL found!");
    DBUG_ASSERT ((NODE_TYPE (exprs) == N_exprs), "wrong icm-arg: N_exprs expected");
    expr = EXPRS_EXPR (exprs);

    DBUG_ASSERT ((NODE_TYPE (expr) == N_double), "wrong icm-arg: N_double expected");
    (*ret) = DOUBLE_VAL (expr);

    DBUG_PRINT ("PRINT", ("icm-arg found: %d", (*ret)));

    exprs = EXPRS_NEXT (exprs);

    DBUG_RETURN (exprs);
}

static node *
GetNextAny (char **ret, node *exprs)
{
    int ival;
    char cval;
    bool bval;
    float fval;
    double dval;
    node *expr;

    DBUG_ENTER ("GetNextAny");

    DBUG_ASSERT ((exprs != NULL), "wrong icm-arg: NULL found!");
    DBUG_ASSERT ((NODE_TYPE (exprs) == N_exprs), "wrong icm-arg: N_exprs expected");
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
    case N_str:
        exprs = GetNextString (ret, exprs);
        break;
    case N_num:
        (*ret) = (char *)ILIBmalloc (sizeof (char) * 50);
        exprs = GetNextInt (&ival, exprs);
        sprintf ((*ret), "%d", ival);
        break;
    case N_char:
        (*ret) = (char *)ILIBmalloc (sizeof (char) * 5);
        exprs = GetNextChar (&cval, exprs);
        sprintf ((*ret), "%d", cval);
        break;
    case N_bool:
        (*ret) = (char *)ILIBmalloc (sizeof (char) * 6);
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
    case N_double:
        exprs = GetNextDouble (&dval, exprs);
        (*ret) = CVdouble2String (dval);
        break;
    default:
        DBUG_ASSERT ((0), "illegal icm-arg found!");
    }

    DBUG_RETURN (exprs);
}

static node *
GetNextVarAny (char ***ret, int *ret_len, int cnt, node *exprs)
{
    int i;
    int len = 0;

    DBUG_ENTER ("GetNextVarAny");

    (*ret) = (char **)ILIBmalloc (cnt * sizeof (char *));

    DBUG_ASSERT ((exprs != NULL), "wrong icm-arg: NULL found!");
    DBUG_ASSERT ((NODE_TYPE (exprs) == N_exprs), "wrong icm-arg: N_exprs expected");

    len = 0;
    for (i = 0; i < cnt; i++) {
        exprs = GetNextAny (&((*ret)[i]), exprs);
        len += strlen ((*ret)[i]);
    }

    if (ret_len != NULL) {
        (*ret_len) = len;
    }

    DBUG_RETURN (exprs);
}

static node *
GetNextVarNt (char ***ret, int cnt, node *exprs)
{
    node *expr;
    int i;

    DBUG_ENTER ("GetNextVarNt");

    (*ret) = (char **)ILIBmalloc (cnt * sizeof (char *));

    DBUG_ASSERT ((exprs != NULL), "wrong icm-arg: NULL found!");
    DBUG_ASSERT ((NODE_TYPE (exprs) == N_exprs), "wrong icm-arg: N_exprs expected");
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

  DBUG_ENTER( "GetNextVarId");

  (*ret) = (char **) ILIBmalloc( cnt * sizeof( char*));

  DBUG_ASSERT( (exprs != NULL), "wrong icm-arg: NULL found!");
  DBUG_ASSERT( (NODE_TYPE( exprs) == N_exprs),
               "wrong icm-arg: N_exprs expected");
  expr = EXPRS_EXPR( exprs);

  for (i = 0; i < cnt; i++) {
    exprs = GetNextId( &((*ret)[i]), exprs);
  }

  DBUG_RETURN( exprs);
}
#endif

static node *
GetNextVarInt (int **ret, int cnt, node *exprs)
{
    node *expr;
    int i;

    DBUG_ENTER ("GetNextVarInt");

    (*ret) = (int *)ILIBmalloc (cnt * sizeof (int));

    DBUG_ASSERT ((exprs != NULL), "wrong icm-arg: NULL found!");
    DBUG_ASSERT ((NODE_TYPE (exprs) == N_exprs), "wrong icm-arg: N_exprs expected");
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
#undef ICM_VARANY
#undef ICM_VARNT
#undef ICM_VARID
#undef ICM_VARINT
#undef ICM_END
