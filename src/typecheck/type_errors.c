/*
 * $Log$
 * Revision 1.17  2004/11/24 18:14:46  sbs
 * compiles
 *
 * Revision 1.16  2004/11/24 17:42:48  sbs
 * not yet
 *
 * Revision 1.15  2004/11/23 20:53:52  sbs
 * SacDevCamp 04 done
 *
 * Revision 1.14  2004/10/26 10:46:59  sbs
 * type_info now holds the module name as well.
 *
 * Revision 1.13  2003/12/02 09:53:02  sbs
 * TEAssureNonNegativeValues added.
 *
 * Revision 1.12  2003/09/10 09:42:13  sbs
 * TEAssureAbsValFitsShape added.
 *
 * Revision 1.11  2003/09/09 14:56:11  sbs
 * extended type error reporting added
 *
 * Revision 1.10  2003/04/14 10:50:29  sbs
 * the dimensionality of the result of _Reshape_ is determined by its first
 * arguments shape component rather than its dimensionality (which always
 * should be 1 8-).
 *
 * Revision 1.9  2003/04/11 17:59:01  sbs
 * TEAssureProdValMatchesProdShape added.
 *
 * Revision 1.8  2003/04/09 15:35:34  sbs
 * TEAssureNumS and TEAssureNumA added.
 *
 * Revision 1.7  2003/04/07 14:32:39  sbs
 * type assertions extended for AKV types.
 * signature of TEMakeInfo extended
 * TEAssureValMatchesShape and TEGetCFFun added.
 *
 * Revision 1.6  2003/03/19 10:34:10  sbs
 * TEAssureVect added.
 *
 * Revision 1.5  2002/09/04 12:59:46  sbs
 * TEArrayElem2Obj and TEAssureSameScalarType added.
 *
 * Revision 1.4  2002/09/03 14:41:45  sbs
 * DupTree machanism for duplicating condi funs established
 *
 * Revision 1.3  2002/08/07 09:51:07  sbs
 * TEAssureIntS added.
 *
 * Revision 1.2  2002/08/06 08:26:49  sbs
 * some vars initialized to please gcc for the product version.
 *
 * Revision 1.1  2002/08/05 16:58:39  sbs
 * Initial revision
 *
 *
 */

#include "type_errors.h"
#include "dbug.h"
#include "Error.h"
#include "internal_lib.h"
#include "shape.h"
#include "constants.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "new_types.h"
#include "new_typecheck.h"

struct TE_INFO {
    int line;             /* line where the application is situated */
    char *kind_str;       /* kind of function we are dealing with */
    char *mod_str;        /* optional module name */
    const char *name_str; /* name of the function */
    node *wrapper;        /* for udfs, this pointer points to the wrapper function */
    node *assign;         /* for udfs, this pointer points to the assign node of the ap */
    const void *cffun;   /* for prfs, this pointer points to the CF function of the prf */
    struct TE_INFO *chn; /* for udfs, this pointer points to the info of the caller */
};

#define TI_LINE(n) (n->line)
#define TI_KIND(n) (n->kind_str)
#define TI_MOD(n) (n->mod_str)
#define TI_NAME(n) (n->name_str)
#define TI_FUNDEF(n) (n->wrapper)
#define TI_ASSIGN(n) (n->assign)
#define TI_CFFUN(n) (n->cffun)
#define TI_CHN(n) (n->chn)

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

/******************************************************************************
 ***
 ***          functions for handling te_info structures:
 ***          ------------------------------------------
 ***
 ******************************************************************************/

te_info *
TEmakeInfo (int linenum, char *kind_str, char *mod_str, const char *name_str,
            node *wrapper, node *assign, const void *cffun, te_info *parent)
{
    te_info *res;

    DBUG_ENTER ("TEmakeInfo");

    res = (te_info *)ILIBmalloc (sizeof (te_info));
    TI_LINE (res) = linenum;
    TI_KIND (res) = kind_str;
    TI_MOD (res) = mod_str;
    TI_NAME (res) = name_str;
    TI_FUNDEF (res) = wrapper;
    TI_ASSIGN (res) = assign;
    TI_CFFUN (res) = cffun;
    TI_CHN (res) = parent;

    DBUG_RETURN (res);
}

int
TEgetLine (te_info *info)
{
    DBUG_ENTER ("TEgetLine");
    DBUG_RETURN (TI_LINE (info));
}

char *
TEgetKindStr (te_info *info)
{
    DBUG_ENTER ("TEgetKindStr");
    DBUG_RETURN (TI_KIND (info));
}

char *
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

const void *
TEgetCFFun (te_info *info)
{
    DBUG_ENTER ("TEgetCFFun");
    DBUG_RETURN (TI_CFFUN (info));
}

te_info *
TEgetParent (te_info *info)
{
    DBUG_ENTER ("TEgetParent");
    DBUG_RETURN (TI_CHN (info));
}

void
TEextendedAbort ()
{
    node *assign;
    ntype *args;

    DBUG_ENTER ("TEextendedAbort");
    DBUG_PRINT ("NTC_INFOCHN", ("act_info_chn is %p", global.act_info_chn));
    if (global.act_info_chn != NULL) {
        CONT_ERROR ((""));
        CONT_ERROR (("TYPE ERROR TRACE:"));
        while (global.act_info_chn != NULL) {
            assign = TI_ASSIGN (global.act_info_chn);
            if (!FUNDEF_ISLACFUN (TI_FUNDEF (global.act_info_chn))) {
                args = NTCnewTypeCheck_Expr (AP_ARGS (ASSIGN_RHS (assign)));
                CONT_ERROR (
                  ("-- %s(?): %d: %s:%s%s", global.filename,
                   TI_LINE (global.act_info_chn),
                   ((TI_MOD (global.act_info_chn) != NULL) ? TI_MOD (global.act_info_chn)
                                                           : "--"),
                   TI_NAME (global.act_info_chn), TYtype2String (args, FALSE, 0)));
            }
            global.act_info_chn = TI_CHN (global.act_info_chn);
        }
    }
    ABORT_ON_ERROR;
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
        ERROR (global.linenum, ("%s should be a scalar; type found: %s", obj,
                                TYtype2String (type, FALSE, 0)));
        TEextendedAbort ();
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
        ERROR (global.linenum, ("%s should be a vector; type found: %s", obj,
                                TYtype2String (type, FALSE, 0)));
        TEextendedAbort ();
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
        ERROR (global.linenum, ("%s should be of type int; type found: %s", obj,
                                TYtype2String (type, FALSE, 0)));
        TEextendedAbort ();
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
        ERROR (global.linenum, ("%s should be of type bool; type found: %s", obj,
                                TYtype2String (type, FALSE, 0)));
        TEextendedAbort ();
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
        ERROR (global.linenum, ("element type of %s should be boolean; type found: %s",
                                obj, TYtype2String (type, FALSE, 0)));
        TEextendedAbort ();
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
        ERROR (global.linenum,
               ("%s should be of type int / float / double; type found: %s", obj,
                TYtype2String (type, FALSE, 0)));
        TEextendedAbort ();
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
        ERROR (global.linenum, ("element type of %s should be numeric; type found: %s",
                                obj, TYtype2String (type, FALSE, 0)));
        TEextendedAbort ();
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

    if (!TYisSimple (TYgetScalar (type))) {
        ERROR (global.linenum, ("%s should be a built-in type; type found: %s", obj,
                                TYtype2String (type, FALSE, 0)));
        TEextendedAbort ();
    }
    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    void TEassureIntVect( char *obj, ntype *type)
 *
 * description:
 *
 *
 ******************************************************************************/

void
TEassureIntVect (char *obj, ntype *type)
{
    DBUG_ENTER ("AssureIntVect");

    if (!MatchIntA (type) || !MatchVect (type)) {
        ERROR (global.linenum, ("%s should be an integer vector; type found: %s", obj,
                                TYtype2String (type, FALSE, 0)));
        TEextendedAbort ();
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
                ERROR (global.linenum,
                       ("%s should not contain negative values; type found: %s", obj,
                        TYtype2String (type, FALSE, 0)));
                TEextendedAbort ();
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
        ERROR (global.linenum, ("shape of %s should match dimensionality of %s;"
                                " types found: %s  and  %s",
                                obj1, obj2, TYtype2String (type1, FALSE, 0),
                                TYtype2String (type2, FALSE, 0)));
        TEextendedAbort ();
    }

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void TEassureValMatchesShape( char *obj1, ntype *type1,
 *                                   char *obj2, ntype *type2)
 *
 *   @brief  makes shure, that if type1 is AKV and type2 is AKS, type1
 *           constitutes a legal index into type2.
 *           It is assumed, that the shape of type1 matches the dim of type2!!
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
        dim = TYgetDim (type2);
        dv = (int *)COgetDataVec (TYgetValue (type1));
        for (i = 0; i < dim; i++) {
            if ((dv[i] < 0) || (dv[i] >= SHgetExtent (TYgetShape (type2), i))) {
                ERROR (global.linenum, ("%s should be legal index into %s;"
                                        " types found: %s  and  %s",
                                        obj1, obj2, TYtype2String (type1, FALSE, 0),
                                        TYtype2String (type2, FALSE, 0)));
                TEextendedAbort ();
            }
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
                ERROR (global.linenum, ("%s should not exceed the shape of %s;"
                                        " types found: %s  and  %s",
                                        obj1, obj2, TYtype2String (type1, FALSE, 0),
                                        TYtype2String (type2, FALSE, 0)));
                TEextendedAbort ();
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
            ERROR (global.linenum, ("%s should be legal shape for the data vector of %s;"
                                    " types found: %s  and  %s",
                                    obj1, obj2, TYtype2String (type1, FALSE, 0),
                                    TYtype2String (type2, FALSE, 0)));
            TEextendedAbort ();
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
        ERROR (global.linenum, ("element types of %s and %s should be identical;"
                                " types found: %s  and  %s",
                                obj1, obj2, TYtype2String (type1, FALSE, 0),
                                TYtype2String (type2, FALSE, 0)));
        TEextendedAbort ();
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
        ERROR (global.linenum, ("element types of %s and %s should be identical;"
                                " types found: %s  and  %s",
                                obj1, obj2, TYtype2String (type1, FALSE, 0),
                                TYtype2String (type2, FALSE, 0)));
        TEextendedAbort ();
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
        ERROR (global.linenum, ("%s and %s should have identical shapes;"
                                " types found: %s  and  %s",
                                obj1, obj2, TYtype2String (type1, FALSE, 0),
                                TYtype2String (type2, FALSE, 0)));
        TEextendedAbort ();
    }

    DBUG_RETURN (res);
}
