/*
 * $Id$
 *
 */

#include <stdarg.h>

#include "type_errors.h"
#include "dbug.h"
#include "ctinfo.h"
#include "str.h"
#include "globals.h"
#include "shape.h"
#include "constants.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "new_types.h"
#include "new_typecheck.h"
#include "private_heap.h"

static char *kind_str[] = {"udf", "prf", "cond", "funcond", "wl", "with", "fold_fun"};
static heap *tinfo_heap = NULL;

struct TE_INFO_UDF {
    const char *mod_str; /* optional module name */
    node *wrapper;       /* pointer to the wrapper function */
    node *assign;        /* pointer to the assign node of the ap */
    struct TE_INFO *chn; /* pointer to the info of the caller */
};

struct TE_INFO_PRF {
    prf prf_no;   /* pointer to the CF function of the prf */
    int num_rets; /* number of return values (depends on #args!) */
};

struct TE_INFO {
    int line;             /* line where the application is situated */
    te_kind_t kind;       /* kind of function we are dealing with */
    const char *name_str; /* name of the function */
    union {
        struct TE_INFO_UDF udf;
        struct TE_INFO_PRF prf;
    } info;
};

#define TI_LINE(n) (n->line)
#define TI_KIND(n) (n->kind)
#define TI_MOD(n) (n->info.udf.mod_str)
#define TI_NAME(n) (n->name_str)
#define TI_FUNDEF(n) (n->info.udf.wrapper)
#define TI_ASSIGN(n) (n->info.udf.assign)
#define TI_CHN(n) (n->info.udf.chn)
#define TI_PRF(n) (n->info.prf.prf_no)
#define TI_NUM_RETS(n) (n->info.prf.num_rets)

#define TI_KIND_STR(n) (kind_str[TI_KIND (n)])

/**
 *
 * Static global variables
 *
 */

static const void *prf_co_funtab[] = {
#define PRFco_fun(co_fun) (void *)co_fun
#include "prf_info.mac"
};

/******************************************************************************
 ***
 ***          local helper functions
 ***          ----------------------
 ***
 ******************************************************************************/

static bool
MatchScalar (ntype *type)
{
    bool res;

    DBUG_ENTER ("MatchScalar");

    switch (TYgetConstr (type)) {
    case TC_akv:
    case TC_aks:
    case TC_akd:
        res = (TYgetDim (type) == 0);
        break;
    case TC_audgz:
    case TC_aud:
        res = TRUE;
        break;
    default:
        DBUG_ASSERT (FALSE, "MatchScalar applied to non-array type");
        res = FALSE; /* just to please gcc 8-) */
    }

    DBUG_RETURN (res);
}

static bool
MatchVect (ntype *type)
{
    bool res = FALSE;
    DBUG_ENTER ("MatchVect");

    switch (TYgetConstr (type)) {
    case TC_akv:
    case TC_aks:
    case TC_akd:
        res = (TYgetDim (type) == 1);
        break;
    case TC_audgz:
    case TC_aud:
        res = TRUE;
        break;
    default:
        DBUG_ASSERT (FALSE, "MatchVect applied to non-array type");
    }

    DBUG_RETURN (res);
}

static bool
MatchVectLengthOne (ntype *type)
{
    bool res = FALSE;
    DBUG_ENTER ("MatchVect");

    switch (TYgetConstr (type)) {
    case TC_akv:
    case TC_aks:
        res = (TYgetDim (type) == 1) && (SHgetExtent (TYgetShape (type), 0) == 1);
        break;
    case TC_akd:
        res = (TYgetDim (type) == 1);
        break;
    case TC_audgz:
    case TC_aud:
        res = TRUE;
        break;
    default:
        DBUG_ASSERT (FALSE, "MatchVect applied to non-array type");
    }

    DBUG_RETURN (res);
}

static bool
MatchIntA (ntype *type)
{
    bool res;

    DBUG_ENTER ("MatchIntA");

    res = ((TYgetConstr (TYgetScalar (type)) == TC_simple)
           && (TYgetSimpleType (TYgetScalar (type)) == T_int));

    DBUG_RETURN (res);
}

static bool
MatchBoolA (ntype *type)
{
    bool res;

    DBUG_ENTER ("MatchBoolA");

    res = ((TYgetConstr (TYgetScalar (type)) == TC_simple)
           && (TYgetSimpleType (TYgetScalar (type)) == T_bool));

    DBUG_RETURN (res);
}

static bool
MatchNumA (ntype *type)
{
    bool res;

    DBUG_ENTER ("MatchNumA");

    res = ((TYgetConstr (TYgetScalar (type)) == TC_simple)
           && ((TYgetSimpleType (TYgetScalar (type)) == T_int)
               || (TYgetSimpleType (TYgetScalar (type)) == T_float)
               || (TYgetSimpleType (TYgetScalar (type)) == T_double)));

    DBUG_RETURN (res);
}

static bool
MatchSimpleA (ntype *type)
{
    bool res;

    DBUG_ENTER ("MatchSimpleA");

    res = TYisSimple (TYgetScalar (type));

    DBUG_RETURN (res);
}

/******************************************************************************
 ***
 ***          functions for handling te_info structures:
 ***          ------------------------------------------
 ***
 ******************************************************************************/

te_info *
TEmakeInfo (int linenum, te_kind_t kind, const char *name_str)
{
    te_info *res;

    DBUG_ENTER ("TEmakeInfo");

    if (tinfo_heap == NULL) {
        tinfo_heap = PHPcreateHeap (sizeof (te_info), 1000);
    }

    res = (te_info *)PHPmalloc (tinfo_heap);

    TI_LINE (res) = linenum;
    TI_KIND (res) = kind;
    TI_NAME (res) = name_str;

    DBUG_RETURN (res);
}

te_info *
TEmakeInfoUdf (int linenum, te_kind_t kind, const char *mod_str, const char *name_str,
               node *wrapper, node *assign, te_info *parent)
{
    te_info *res;

    DBUG_ENTER ("TEmakeInfo");

    res = TEmakeInfo (linenum, kind, name_str);
    TI_MOD (res) = mod_str;
    TI_FUNDEF (res) = wrapper;
    TI_ASSIGN (res) = assign;
    TI_CHN (res) = parent;

    DBUG_RETURN (res);
}

te_info *
TEmakeInfoPrf (int linenum, te_kind_t kind, const char *name_str, prf prf_no,
               int num_rets)
{
    te_info *res;

    DBUG_ENTER ("TEmakeInfo");

    res = TEmakeInfo (linenum, kind, name_str);
    TI_PRF (res) = prf_no;
    TI_NUM_RETS (res) = num_rets;

    DBUG_RETURN (res);
}

void
TEfreeAllTypeErrorInfos ()
{
    DBUG_ENTER ("TEfreeAllTypeErrorInfos");

    tinfo_heap = PHPfreeHeap (tinfo_heap);

    DBUG_VOID_RETURN;
}

int
TEgetLine (te_info *info)
{
    DBUG_ENTER ("TEgetLine");
    DBUG_RETURN (TI_LINE (info));
}

te_kind_t
TEgetKind (te_info *info)
{
    DBUG_ENTER ("TEgetKind");
    DBUG_RETURN (TI_KIND (info));
}
char *
TEgetKindStr (te_info *info)
{
    DBUG_ENTER ("TEgetKindStr");
    DBUG_RETURN (TI_KIND_STR (info));
}

const char *
TEgetModStr (te_info *info)
{
    DBUG_ENTER ("TEgetModStr");
    DBUG_RETURN (TI_MOD (info));
}

const char *
TEgetNameStr (te_info *info)
{
    DBUG_ENTER ("TEgetNameStr");
    DBUG_RETURN (TI_NAME (info));
}

node *
TEgetWrapper (te_info *info)
{
    DBUG_ENTER ("TEgetWrapper");
    DBUG_RETURN (TI_FUNDEF (info));
}

node *
TEgetAssign (te_info *info)
{
    DBUG_ENTER ("TEgetAssign");
    DBUG_RETURN (TI_ASSIGN (info));
}

prf
TEgetPrf (te_info *info)
{
    DBUG_ENTER ("TEgetPrf");
    DBUG_RETURN (TI_PRF (info));
}

const void *
TEgetCFFun (te_info *info)
{
    DBUG_ENTER ("TEgetCFFun");
    DBUG_RETURN (prf_co_funtab[TI_PRF (info)]);
}

te_info *
TEgetParent (te_info *info)
{
    DBUG_ENTER ("TEgetParent");
    DBUG_RETURN (TI_CHN (info));
}

int
TEgetNumRets (te_info *info)
{
    int num_res;
    node *wrapper;

    DBUG_ENTER ("TEgetNumRets");

    switch (TI_KIND (info)) {
    case TE_udf:
        wrapper = TEgetWrapper (info);
        num_res = TCcountRets (FUNDEF_RETS (wrapper));
        break;
    case TE_prf:
        num_res = TI_NUM_RETS (info);
        break;
    case TE_cond:
        num_res = 0;
        break;
    case TE_funcond:
        num_res = 1;
        break;
    case TE_generator:
        num_res = 1;
        break;
    case TE_with:
        num_res = 1;
        break;
    case TE_foldf:
        num_res = 1;
        break;
    default:
        DBUG_ASSERT (FALSE, "illegal TI_KIND in info!");
        num_res = 0; /* just to please gcc 8-) */
        break;
    }

    DBUG_RETURN (num_res);
}

static char *errors = NULL;

/** <!--********************************************************************-->
 *
 * @fn void TEhandleError( int line, const char *format, ...)
 *
 *   @brief  collect the error messages
 *
 *   @param line  line number
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void
TEhandleError (int line, const char *format, ...)
{
    va_list arg_p;

    DBUG_ENTER ("TEhandleError");

    va_start (arg_p, format);

    /**
     * CTIerrorLineVA( line, format, arg_p);
     * TEextendedAbort( );
     */

    if (errors == NULL) {
        errors = CTIgetErrorMessageVA (line, format, arg_p);
    } else {
        errors = STRcatn (3, errors, "@", CTIgetErrorMessageVA (line, format, arg_p));
    }

    va_end (arg_p);

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn char *TEfetchErrors( )
 *
 *   @brief  retrieve the collected errors as string and reset errors.
 *
 *   @return NULL in case there were no errors, a proper String otherwise.
 *
 ******************************************************************************/

char *
TEfetchErrors ()
{
    char *res;
    DBUG_ENTER ("TEfetchErrors");

    res = errors;
    errors = NULL;

    DBUG_RETURN (res);
}

void
TEextendedAbort ()
{
    node *assign;
    ntype *args;

    DBUG_ENTER ("TEextendedAbort");
    DBUG_PRINT ("NTC_INFOCHN", ("act_info_chn is %p", global.act_info_chn));
    if (global.act_info_chn != NULL) {
        CTIerrorContinued ("\nTYPE ERROR TRACE:");
        while (global.act_info_chn != NULL) {
            assign = TI_ASSIGN (global.act_info_chn);
            if (!FUNDEF_ISLACFUN (TI_FUNDEF (global.act_info_chn))) {
                /**
                 * The assigment either is a direct application of a UDF
                 * -or- an indirect application due to a fold-WL.
                 */
                if (NODE_TYPE (ASSIGN_RHS (assign)) == N_with) {
                    CTIerrorContinued (
                      "-- %s(?): %d: %s:%s (while checking fold with loop)",
                      global.filename, TI_LINE (global.act_info_chn),
                      ((TI_MOD (global.act_info_chn) != NULL)
                         ? TI_MOD (global.act_info_chn)
                         : "--"),
                      TI_NAME (global.act_info_chn));
                } else {
                    args = NTCnewTypeCheck_Expr (AP_ARGS (ASSIGN_RHS (assign)));
                    CTIerrorContinued ("-- %s(?): %d: %s:%s%s", global.filename,
                                       TI_LINE (global.act_info_chn),
                                       ((TI_MOD (global.act_info_chn) != NULL)
                                          ? TI_MOD (global.act_info_chn)
                                          : "--"),
                                       TI_NAME (global.act_info_chn),
                                       TYtype2String (args, FALSE, 0));
                }
            }
            global.act_info_chn = TI_CHN (global.act_info_chn);
        }
    }

    CTIabortOnError ();

    DBUG_VOID_RETURN;
}

/******************************************************************************
 ***
 ***          functions for creating static strings:
 ***          --------------------------------------
 ***
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *    char *TEprfArg2Obj( const char *prf_str, int pos )
 *
 * description:
 *
 *
 ******************************************************************************/

char *
TEprfArg2Obj (const char *prf_str, int pos)
{
    static char buffer[64];
    char *tmp = &buffer[0];

    DBUG_ENTER ("TEprfArg2Obj");

    tmp += sprintf (tmp, "argument #%d of \"%s\"", pos, prf_str);

    DBUG_RETURN (&buffer[0]);
}

/******************************************************************************
 *
 * function:
 *    char *TEarg2Obj( int pos )
 *
 * description:
 *
 *
 ******************************************************************************/

char *
TEarg2Obj (int pos)
{
    static char buffer[64];
    char *tmp = &buffer[0];

    DBUG_ENTER ("TEarg2Obj");

    tmp += sprintf (tmp, "argument #%d", pos);

    DBUG_RETURN (&buffer[0]);
}

/******************************************************************************
 *
 * function:
 *    char *TEanotherArg2Obj( int pos )
 *
 * description:
 *
 *
 ******************************************************************************/

char *
TEanotherArg2Obj (int pos)
{
    static char buffer[64];
    char *tmp = &buffer[0];

    DBUG_ENTER ("TEanotherArg2Obj");

    tmp += sprintf (tmp, "argument #%d", pos);

    DBUG_RETURN (&buffer[0]);
}

/******************************************************************************
 *
 * function:
 *    char *TEarrayElem2Obj( int pos )
 *
 * description:
 *
 *
 ******************************************************************************/

char *
TEarrayElem2Obj (int pos)
{
    static char buffer[64];
    char *tmp = &buffer[0];

    DBUG_ENTER ("TEarrayElem2Obj");

    tmp += sprintf (tmp, "array element #%d", pos);

    DBUG_RETURN (&buffer[0]);
}

/******************************************************************************
 ***
 ***          assertion functions:
 ***          --------------------
 ***
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *    void TEassureScalar( char *obj, ntype *type)
 *
 * description:
 *
 *
 ******************************************************************************/

void
TEassureScalar (char *obj, ntype *type)
{
    DBUG_ENTER ("TEassureScalar");

    if (!MatchScalar (type)) {
        TEhandleError (global.linenum, "%s should be a scalar; type found: %s", obj,
                       TYtype2String (type, FALSE, 0));
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    void TEassureVect( char *obj, ntype *type)
 *
 * description:
 *
 *
 ******************************************************************************/

void
TEassureVect (char *obj, ntype *type)
{
    DBUG_ENTER ("TEassureVect");

    if (!MatchVect (type)) {
        TEhandleError (global.linenum, "%s should be a vector; type found: %s", obj,
                       TYtype2String (type, FALSE, 0));
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    void TEassureIntS( char *obj, ntype *type)
 *
 * description:
 *
 *
 ******************************************************************************/

void
TEassureIntS (char *obj, ntype *type)
{
    DBUG_ENTER ("TEassureIntS");

    if (!MatchScalar (type) || !MatchIntA (type)) {
        TEhandleError (global.linenum, "%s should be of type int; type found: %s", obj,
                       TYtype2String (type, FALSE, 0));
    }
    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    void TEassureIntV( char *obj, ntype *type)
 *
 * description:
 *
 *
 ******************************************************************************/

void
TEassureIntV (char *obj, ntype *type)
{
    DBUG_ENTER ("AssureIntV");

    if (!MatchIntA (type) || !MatchVect (type)) {
        TEhandleError (global.linenum, "%s should be an integer vector; type found: %s",
                       obj, TYtype2String (type, FALSE, 0));
    }
    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    void TEassureBoolS( char *obj, ntype *type)
 *
 * description:
 *
 *
 ******************************************************************************/

void
TEassureBoolS (char *obj, ntype *type)
{
    DBUG_ENTER ("TEassureBoolS");

    if (!MatchScalar (type) || !MatchBoolA (type)) {
        TEhandleError (global.linenum, "%s should be of type bool; type found: %s", obj,
                       TYtype2String (type, FALSE, 0));
    }
    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    void TEassureBoolV( char *obj, ntype *type)
 *
 * description:
 *
 *
 ******************************************************************************/

void
TEassureBoolV (char *obj, ntype *type)
{
    DBUG_ENTER ("TEassureBoolV");

    if (!MatchVect (type) || !MatchBoolA (type)) {
        TEhandleError (global.linenum, "%s should be a boolean vector; type found: %s",
                       obj, TYtype2String (type, FALSE, 0));
    }
    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    void TEassureBoolA( char *obj, ntype *type)
 *
 * description:
 *
 *
 ******************************************************************************/

void
TEassureBoolA (char *obj, ntype *type)
{
    DBUG_ENTER ("TEassureBoolA");

    if (!MatchBoolA (type)) {
        TEhandleError (global.linenum,
                       "Element type of %s should be boolean; type found: %s", obj,
                       TYtype2String (type, FALSE, 0));
    }
    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    void TEassureNumS( char *obj, ntype *type)
 *
 * description:
 *
 *
 ******************************************************************************/

void
TEassureNumS (char *obj, ntype *type)
{
    DBUG_ENTER ("TEassureNumS");

    if (!MatchScalar (type) || !MatchNumA (type)) {
        TEhandleError (global.linenum,
                       "%s should be of type int / float / double; type found: %s", obj,
                       TYtype2String (type, FALSE, 0));
    }
    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    void TEassureNumV( char *obj, ntype *type)
 *
 * description:
 *
 *
 ******************************************************************************/

void
TEassureNumV (char *obj, ntype *type)
{
    DBUG_ENTER ("TEassureNumV");

    if (!MatchVect (type) || !MatchNumA (type)) {
        TEhandleError (global.linenum,
                       "%s should be a vector of type int / float / double; type found: "
                       "%s",
                       obj, TYtype2String (type, FALSE, 0));
    }
    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    void TEassureNumA( char *obj, ntype *type)
 *
 * description:
 *
 *
 ******************************************************************************/

void
TEassureNumA (char *obj, ntype *type)
{
    DBUG_ENTER ("TEassureNumA");

    if (!MatchNumA (type)) {
        TEhandleError (global.linenum,
                       "Element type of %s should be numeric; type found: %s", obj,
                       TYtype2String (type, FALSE, 0));
    }
    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    void TEassureSimpleType( char *obj, ntype *type)
 *
 * description:
 *
 *
 ******************************************************************************/

void
TEassureSimpleType (char *obj, ntype *type)
{
    DBUG_ENTER ("TEassureSimpleType");

    if (!MatchSimpleA (type)) {
        TEhandleError (global.linenum, "%s should be a built-in type; type found: %s",
                       obj, TYtype2String (type, FALSE, 0));
    }
    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    void TEassureSimpleS( char *obj, ntype *type)
 *
 * description:
 *
 *
 ******************************************************************************/

void
TEassureSimpleS (char *obj, ntype *type)
{
    DBUG_ENTER ("TEassureSimpleS");

    if (!MatchSimpleA (type) || !MatchScalar (type)) {
        TEhandleError (global.linenum,
                       "%s should be a scalar of a built-in type; type found: %s", obj,
                       TYtype2String (type, FALSE, 0));
    }
    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    void TEassureSimpleV( char *obj, ntype *type)
 *
 * description:
 *
 *
 ******************************************************************************/

void
TEassureSimpleV (char *obj, ntype *type)
{
    DBUG_ENTER ("TEassureSimpleV");

    if (!MatchSimpleA (type) || !MatchVect (type)) {
        TEhandleError (global.linenum,
                       "%s should be a vector of a built-in type; type found: %s", obj,
                       TYtype2String (type, FALSE, 0));
    }
    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    void TEassureIntVectLengthOne( char *obj, ntype *type)
 *
 * description:
 *
 *
 ******************************************************************************/

void
TEassureIntVectLengthOne (char *obj, ntype *type)
{
    DBUG_ENTER ("AssureIntVectLengthOne");

    if (!MatchIntA (type) || !MatchVectLengthOne (type)) {
        TEhandleError (global.linenum, "%s should be an integer vector; type found: %s",
                       obj, TYtype2String (type, FALSE, 0));
    }
    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    void TEassureNonNegativeValues( char *obj, ntype *type)
 *
 * description:
 *
 *
 ******************************************************************************/

void
TEassureNonNegativeValues (char *obj, ntype *type)
{
    int i, dim;
    int *dv;

    DBUG_ENTER ("TEassureNonNegativeValues");

    if (TYgetConstr (type) == TC_akv) {
        dim = SHgetExtent (TYgetShape (type), 0);
        dv = (int *)COgetDataVec (TYgetValue (type));

        for (i = 0; i < dim; i++) {
            if (dv[i] < 0) {
                TEhandleError (global.linenum,
                               "%s should not contain negative values; type found: %s",
                               obj, TYtype2String (type, FALSE, 0));
            }
        }
    }
    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    void TEassureShpMatchesDim( char *obj1, ntype *type1,
 *                                char *obj2, ntype *type2)
 *
 * description:
 *
 *
 ******************************************************************************/

void
TEassureShpMatchesDim (char *obj1, ntype *type1, char *obj2, ntype *type2)
{
    DBUG_ENTER ("TEassureShpMatchesDim");

    if (((TYgetConstr (type1) == TC_aks) || (TYgetConstr (type1) == TC_akv))
        && ((TYgetConstr (type2) == TC_akv) || (TYgetConstr (type2) == TC_aks)
            || (TYgetConstr (type2) == TC_akd))
        && (SHgetExtent (TYgetShape (type1), 0) != TYgetDim (type2))) {
        TEhandleError (global.linenum,
                       "Shape of %s should match dimensionality of %s;"
                       " types found: %s  and  %s",
                       obj1, obj2, TYtype2String (type1, FALSE, 0),
                       TYtype2String (type2, FALSE, 0));
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    void TEassureShpPlusDimMatchesDim( char *obj1, ntype *type1,
 *                                       char *obj2, ntype *type2,
 *                                       char *obj3, ntype *type3)
 *
 * description:
 *
 *
 ******************************************************************************/

void
TEassureShpPlusDimMatchesDim (char *obj1, ntype *type1, char *obj2, ntype *type2,
                              char *obj3, ntype *type3)
{
    DBUG_ENTER ("TEassureShpPlusDimMatchesDim");

    if (((TYgetConstr (type1) == TC_aks) || (TYgetConstr (type1) == TC_akv))
        && ((TYgetConstr (type2) == TC_akv) || (TYgetConstr (type2) == TC_aks)
            || (TYgetConstr (type2) == TC_akd))
        && ((TYgetConstr (type3) == TC_akv) || (TYgetConstr (type3) == TC_aks)
            || (TYgetConstr (type3) == TC_akd))
        && (SHgetExtent (TYgetShape (type1), 0) + TYgetDim (type2) != TYgetDim (type3))) {
        TEhandleError (global.linenum,
                       "Shape of %s + dimensionality of %s "
                       "should match dimensionality of %s;"
                       " types found: %s ,  %s ,  and  %s",
                       obj1, obj2, obj3, TYtype2String (type1, FALSE, 0),
                       TYtype2String (type2, FALSE, 0), TYtype2String (type3, FALSE, 0));
    }

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void TEassureShpIsPostfixOfShp( char *obj1, ntype *type1,
 *                                     char *obj2, ntype *type2)
 *
 *   @brief  makes shure, that if type1 and type2 are AKS, shape( type1)
 *           constitutes a postfix of shape( type2).
 *           It is assumed, that dim( type1) <= dim( type2).
 *
 ******************************************************************************/

void
TEassureShpIsPostfixOfShp (char *obj1, ntype *type1, char *obj2, ntype *type2)
{
    int i, offset;

    DBUG_ENTER ("TEassureShpIsPostfixOfShp");

    if (((TYgetConstr (type1) == TC_aks) || (TYgetConstr (type1) == TC_akv))
        && ((TYgetConstr (type2) == TC_aks) || (TYgetConstr (type2) == TC_akv))) {
        offset = TYgetDim (type2) - TYgetDim (type1);
        for (i = 0; i < TYgetDim (type1); i++) {
            if (SHgetExtent (TYgetShape (type1), i)
                != SHgetExtent (TYgetShape (type2), i + offset)) {
                TEhandleError (global.linenum,
                               "the shape of %s should be a postfix of the shape of %s;"
                               " types found: %s  and  %s",
                               obj1, obj2, TYtype2String (type1, FALSE, 0),
                               TYtype2String (type2, FALSE, 0));

                i = TYgetDim (type1); /* skip the remainder */
            }
        }
    }

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void TEassureValMatchesDim( char *obj1, ntype *type1,
 *                                 char *obj2, ntype *type2)
 *
 *   @brief  makes shure, that if type1 is AKV and type2 is AKS, type1
 *           constitutes a legal index into shape( type2).
 *           It is assumed, that type1 is a vector or a scalar!!
 *
 ******************************************************************************/

void
TEassureValMatchesDim (char *obj1, ntype *type1, char *obj2, ntype *type2)
{
    int *dv;

    DBUG_ENTER ("TEassureValMatchesDim");

    if ((TYgetConstr (type1) == TC_akv)
        && ((TYgetConstr (type2) == TC_akd) || (TYgetConstr (type2) == TC_aks)
            || (TYgetConstr (type2) == TC_akv))) {
        dv = (int *)COgetDataVec (TYgetValue (type1));
        if ((dv[0] < 0) || (dv[0] >= TYgetDim (type2))) {
            TEhandleError (global.linenum,
                           "%s should be legal index into shape( %s);"
                           " types found: %s  and  %s",
                           obj1, obj2, TYtype2String (type1, FALSE, 0),
                           TYtype2String (type2, FALSE, 0));
        }
    }

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void TEassureValMatchesShape( char *obj1, ntype *type1,
 *                                   char *obj2, ntype *type2)
 *
 *   @brief  makes sure, that if type1 is AKV and type2 is AKS, type1
 *           constitutes a legal index into type2.
 *           It is assumed, that the shape of type1 <= the dim of type2!!
 *           NB: if type1 is scalar and type2 is a vector, that's ok too!!
 *
 ******************************************************************************/

void
TEassureValMatchesShape (char *obj1, ntype *type1, char *obj2, ntype *type2)
{
    int i, dim;
    int *dv;

    DBUG_ENTER ("TEassureValMatchesShape");

    if ((TYgetConstr (type1) == TC_akv)
        && ((TYgetConstr (type2) == TC_aks) || (TYgetConstr (type2) == TC_akv))) {
        dim = SHgetExtent (COgetShape (TYgetValue (type1)), 0);
        dv = (int *)COgetDataVec (TYgetValue (type1));
        for (i = 0; i < dim; i++) {
            if ((dv[i] < 0) || (dv[i] >= SHgetExtent (TYgetShape (type2), i))) {
                TEhandleError (global.linenum,
                               "%s should be legal index into %s;"
                               " types found: %s  and  %s",
                               obj1, obj2, TYtype2String (type1, FALSE, 0),
                               TYtype2String (type2, FALSE, 0));
            }
        }
    }

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void TEassureValLeVal( char *obj1, ntype *type1,
 *                                   char *obj2, ntype *type2)
 *
 *   @brief  makes sure, that if type1 is AKV and type2 is AKV, the value
 *           of type1 is <= the value of type2.
 *           It is assumed that shape type1 <= shape type2!
 *           NB: if type1 is scalar and type2 is a vector, that's ok too!!
 *
 ******************************************************************************/

void
TEassureValLeVal (char *obj1, ntype *type1, char *obj2, ntype *type2)
{
    int i, dim1, dim2;
    int *dv1, *dv2;

    DBUG_ENTER ("TEassureValLeVal");

    if ((TYgetConstr (type1) == TC_akv) && (TYgetConstr (type2) == TC_akv)) {
        dim1 = SHgetExtent (COgetShape (TYgetValue (type1)), 0);
        dim2 = SHgetExtent (COgetShape (TYgetValue (type2)), 0);
        dv1 = (int *)COgetDataVec (TYgetValue (type1));
        dv2 = (int *)COgetDataVec (TYgetValue (type2));
        for (i = 0; i < dim1; i++) {
            if ((dv1[i] < 0) || (dv1[i] > dv2[i])) {
                TEhandleError (global.linenum,
                               "%s should be less equal than %s;"
                               " types found: %s  and  %s",
                               obj1, obj2, TYtype2String (type1, FALSE, 0),
                               TYtype2String (type2, FALSE, 0));
            }
        }
    }

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void TEassureValNonZero( char *obj1, ntype *type1)
 *
 *   @brief  makes shure, that if type1 is AKV, the value is neither
 *           zero itself nor an array that contains a zero.
 *           It is assumed, that the element type is a built-in numerical one.
 *
 ******************************************************************************/

void
TEassureValNonZero (char *obj1, ntype *type1)
{
    DBUG_ENTER ("TEassureValNonZero");

    if (TYgetConstr (type1) == TC_akv) {
        if (COisZero (TYgetValue (type1), FALSE)) {
            TEhandleError (global.linenum,
                           "%s must not contain a zero;"
                           " type found: %s",
                           obj1, TYtype2String (type1, FALSE, 0));
        }
    }

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void TEassureIdxMatchesShape( char *obj1, ntype *type1,
 *                                   char *obj2, ntype *type2)
 *
 *   @brief  makes shure, that if type1 is AKV and type2 is AKS, type1
 *           constitutes a legal offset into type2.
 *           Type1 must be scalar!
 *
 ******************************************************************************/

void
TEassureIdxMatchesShape (char *obj1, ntype *type1, char *obj2, ntype *type2)
{
    int *dv;

    DBUG_ENTER ("TEassureValMatchesShape");

    if ((TYgetConstr (type1) == TC_akv)
        && ((TYgetConstr (type2) == TC_aks) || (TYgetConstr (type2) == TC_akv))) {
        dv = (int *)COgetDataVec (TYgetValue (type1));
        if ((dv[0] < 0) || (dv[0] >= SHgetUnrLen (TYgetShape (type2)))) {
            TEhandleError (global.linenum,
                           "%s should be legal offset index into %s;"
                           " types found: %s  and  %s",
                           obj1, obj2, TYtype2String (type1, FALSE, 0),
                           TYtype2String (type2, FALSE, 0));
        }
    }

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void TEassureAbsValFitsShape( char *obj1, ntype *type1,
 *                                   char *obj2, ntype *type2)
 *
 *   @brief  makes shure, that if type1 is AKV and type2 is AKS, type1
 *           constitutes a legal take/drop size into type2.
 *           It is assumed, that the shape of type1 matches the dim of type2!!
 *           NB: if type1 is scalar and type2 is a vector, that's ok too!!
 *
 ******************************************************************************/

void
TEassureAbsValFitsShape (char *obj1, ntype *type1, char *obj2, ntype *type2)
{
    int i, dim;
    int *dv;

    DBUG_ENTER ("TEassureAbsValFitsShape");

    if ((TYgetConstr (type1) == TC_akv)
        && ((TYgetConstr (type2) == TC_aks) || (TYgetConstr (type2) == TC_akv))) {
        dim = TYgetDim (type2);
        dv = (int *)COgetDataVec (TYgetValue (type1));
        for (i = 0; i < dim; i++) {
            if (abs (dv[i]) > SHgetExtent (TYgetShape (type2), i)) {
                TEhandleError (global.linenum,
                               "%s should not exceed the shape of %s;"
                               " types found: %s  and  %s",
                               obj1, obj2, TYtype2String (type1, FALSE, 0),
                               TYtype2String (type2, FALSE, 0));
            }
        }
    }

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void TEassureProdValMatchesProdShape( char *obj1, ntype *type1,
 *                                           char *obj2, ntype *type2)
 *
 *   @brief  makes shure, that if type1 is AKV and type2 is AKS, type1
 *           constitutes a legal shape for the elements of type2, i.e,
 *           prod( values(type1)) == prod( shape(type2)).
 *
 ******************************************************************************/

void
TEassureProdValMatchesProdShape (char *obj1, ntype *type1, char *obj2, ntype *type2)
{
    int i, dim, prod;
    int *dv;

    DBUG_ENTER ("TEassureProdValMatchesProdShape");

    if ((TYgetConstr (type1) == TC_akv)
        && ((TYgetConstr (type2) == TC_aks) || (TYgetConstr (type2) == TC_akv))) {
        dim = SHgetExtent (TYgetShape (type1), 0);
        dv = (int *)COgetDataVec (TYgetValue (type1));
        prod = 1;
        for (i = 0; i < dim; i++) {
            prod *= dv[i];
        }
        if (prod != SHgetUnrLen (TYgetShape (type2))) {
            TEhandleError (global.linenum,
                           "%s should be legal shape for the data vector of %s;"
                           " types found: %s  and  %s",
                           obj1, obj2, TYtype2String (type1, FALSE, 0),
                           TYtype2String (type2, FALSE, 0));
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    void TEassureSameSimpleType( char *obj1, ntype *type1,
 *                                 char *obj2, ntype *type2)
 *
 * description:
 *    expects two array types with  a simple type as scalar type!!!
 *    This property may be checked by applying   TEassureSimpleType( )
 *    to the given argument types prior to calling this function.
 *
 ******************************************************************************/

void
TEassureSameSimpleType (char *obj1, ntype *type1, char *obj2, ntype *type2)
{
    DBUG_ENTER ("TEassureSameSimpleType");

    if (TYgetSimpleType (TYgetScalar (type1)) != TYgetSimpleType (TYgetScalar (type2))) {
        TEhandleError (global.linenum,
                       "Element types of %s and %s should be identical;"
                       " types found: %s  and  %s",
                       obj1, obj2, TYtype2String (type1, FALSE, 0),
                       TYtype2String (type2, FALSE, 0));
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    void TEassureSameScalarType( char *obj1, ntype *type1,
 *                                 char *obj2, ntype *type2)
 *
 * description:
 *
 ******************************************************************************/

void
TEassureSameScalarType (char *obj1, ntype *type1, char *obj2, ntype *type2)
{
    DBUG_ENTER ("TEassureSameScalarType");

    if (!TYeqTypes (TYgetScalar (type1), TYgetScalar (type2))) {
        TEhandleError (global.linenum,
                       "Element types of %s and %s should be identical;"
                       " types found: %s  and  %s",
                       obj1, obj2, TYtype2String (type1, FALSE, 0),
                       TYtype2String (type2, FALSE, 0));
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    ntype *TEassureSameShape( char *obj1, ntype *type1,
 *                              char *obj2, ntype *type2)
 *
 * description:
 *
 *
 ******************************************************************************/

ntype *
TEassureSameShape (char *obj1, ntype *type1, char *obj2, ntype *type2)
{
    ntype *res = NULL;
    ntype *pend1 = NULL, *pend2 = NULL;
    ntype *type1_org = NULL, *type2_org = NULL;

    DBUG_ENTER ("TEassureSameShape");

    /**
     * lift AKVs to AKSs!
     * pend<i> indicates ntype structures that have to be deleted!
     * type<i>_org is needed for better error messages!
     */
    if (TYgetConstr (type1) == TC_akv) {
        pend1 = TYmakeAKS (TYcopyType (TYgetScalar (type1)),
                           SHcopyShape (TYgetShape (type1)));
        type1_org = type1;
        type1 = pend1;
    }
    if (TYgetConstr (type2) == TC_akv) {
        pend2 = TYmakeAKS (TYcopyType (TYgetScalar (type2)),
                           SHcopyShape (TYgetShape (type2)));
        type2_org = type2;
        type2 = pend2;
    }

    /**
     *
     * Now, the AKV free switch:
     */
    switch (TYgetConstr (type1)) {
    case TC_aks:
        switch (TYgetConstr (type2)) {
        case TC_aks:
            if (SHcompareShapes (TYgetShape (type2), TYgetShape (type1))) {
                res = TYcopyType (type2);
            }
            break;
        case TC_akd:
            if (TYgetDim (type2) == TYgetDim (type1)) {
                res = TYcopyType (type1);
            }
            break;
        case TC_audgz:
            if (TYgetDim (type1) > 0) {
                res = TYcopyType (type1);
            }
            break;
        case TC_aud:
            res = TYcopyType (type1);
            break;
        default:
            DBUG_ASSERT (FALSE, "AssureSameShape applied to non-array type");
        }
        break;

    case TC_akd:
        switch (TYgetConstr (type2)) {
        case TC_aks:
        case TC_akd:
            if (TYgetDim (type2) == TYgetDim (type1)) {
                res = TYcopyType (type2);
            }
            break;
        case TC_audgz:
            if (TYgetDim (type1) > 0) {
                res = TYcopyType (type1);
            }
            break;
        case TC_aud:
            res = TYcopyType (type1);
            break;
        default:
            DBUG_ASSERT (FALSE, "AssureSameShape applied to non-array type");
        }
        break;

    case TC_audgz:
        switch (TYgetConstr (type2)) {
        case TC_aks:
        case TC_akd:
            if (TYgetDim (type2) > 0) {
                res = TYcopyType (type2);
            }
            break;
        case TC_audgz:
        case TC_aud:
            res = TYcopyType (type1);
            break;
        default:
            DBUG_ASSERT (FALSE, "AssureSameShape applied to non-array type");
        }
        break;

    case TC_aud:
        res = TYcopyType (type2);
        break;

    default:
        DBUG_ASSERT (FALSE, "AssureSameShape applied to non-array type");
    }

    if (pend1 != NULL) {
        type1 = type1_org;
        pend1 = TYfreeType (pend1);
    }
    if (pend2 != NULL) {
        type2 = type2_org;
        pend2 = TYfreeType (pend2);
    }

    if (res == NULL) {
        TEhandleError (global.linenum,
                       "%s and %s should have identical shapes;"
                       " types found: %s  and  %s",
                       obj1, obj2, TYtype2String (type1, FALSE, 0),
                       TYtype2String (type2, FALSE, 0));
        /**
         * In case of a non-aborting error handling, we need some usefull value
         * for res. To minimize error propagation, we choose the least informative
         * possible value, i.e, the AUD version:
         */
        res = TYmakeAUD (TYcopyType (TYgetScalar (type1)));
    }

    DBUG_RETURN (res);
}

/*******************************************************************************
 * functions for computing the number of return values from the number of
 * argument types (for prfs needed)
 */

int
TEone (int num_args)
{
    DBUG_ENTER ("TEone");
    DBUG_RETURN (1);
}

int
TEtwo (int num_args)
{
    DBUG_ENTER ("TEtwo");
    DBUG_RETURN (2);
}

int
TEthree (int num_args)
{
    DBUG_ENTER ("TEtwo");
    DBUG_RETURN (3);
}

int
TEnMinusOne (int num_args)
{
    DBUG_ENTER ("TEtwo");
    DBUG_RETURN (num_args - 1);
}
