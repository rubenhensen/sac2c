/*
 * $Log$
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

#include "dbug.h"
#include "type_errors.h"
#include "new_typecheck.h"

struct TE_INFO {
    int line;            /* line where the application is situated */
    char *kind_str;      /* kind of function we are dealing with */
    char *name_str;      /* name of the function */
    node *wrapper;       /* for udfs, this pointer points to the wrapper function */
    node *assign;        /* for udfs, this pointer points to the assign node of the ap */
    void *cffun;         /* for prfs, this pointer points to the CF function of the prf */
    struct TE_INFO *chn; /* for udfs, this pointer points to the info of the caller */
};

#define TI_LINE(n) (n->line)
#define TI_KIND(n) (n->kind_str)
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

    switch (TYGetConstr (type)) {
    case TC_akv:
    case TC_aks:
    case TC_akd:
        res = (TYGetDim (type) == 0);
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

    switch (TYGetConstr (type)) {
    case TC_akv:
    case TC_aks:
    case TC_akd:
        res = (TYGetDim (type) == 1);
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

    res = ((TYGetConstr (TYGetScalar (type)) == TC_simple)
           && (TYGetSimpleType (TYGetScalar (type)) == T_int));

    DBUG_RETURN (res);
}

static bool
MatchBoolA (ntype *type)
{
    bool res;

    DBUG_ENTER ("MatchBoolA");

    res = ((TYGetConstr (TYGetScalar (type)) == TC_simple)
           && (TYGetSimpleType (TYGetScalar (type)) == T_bool));

    DBUG_RETURN (res);
}

static bool
MatchNumA (ntype *type)
{
    bool res;

    DBUG_ENTER ("MatchNumA");

    res = ((TYGetConstr (TYGetScalar (type)) == TC_simple)
           && ((TYGetSimpleType (TYGetScalar (type)) == T_int)
               || (TYGetSimpleType (TYGetScalar (type)) == T_float)
               || (TYGetSimpleType (TYGetScalar (type)) == T_double)));

    DBUG_RETURN (res);
}

/******************************************************************************
 ***
 ***          functions for handling te_info structures:
 ***          ------------------------------------------
 ***
 ******************************************************************************/

te_info *
TEMakeInfo (int linenum, char *kind_str, char *name_str, node *wrapper, node *assign,
            void *cffun, te_info *parent)
{
    te_info *res;

    DBUG_ENTER ("TEMakeInfo");

    res = (te_info *)Malloc (sizeof (te_info));
    TI_LINE (res) = linenum;
    TI_KIND (res) = kind_str;
    TI_NAME (res) = name_str;
    TI_FUNDEF (res) = wrapper;
    TI_ASSIGN (res) = assign;
    TI_CFFUN (res) = cffun;
    TI_CHN (res) = parent;

    DBUG_RETURN (res);
}

int
TEGetLine (te_info *info)
{
    DBUG_ENTER ("TEGetLine");
    DBUG_RETURN (TI_LINE (info));
}

char *
TEGetKindStr (te_info *info)
{
    DBUG_ENTER ("TEGetKindStr");
    DBUG_RETURN (TI_KIND (info));
}

char *
TEGetNameStr (te_info *info)
{
    DBUG_ENTER ("TEGetNameStr");
    DBUG_RETURN (TI_NAME (info));
}

node *
TEGetWrapper (te_info *info)
{
    DBUG_ENTER ("TEGetWrapper");
    DBUG_RETURN (TI_FUNDEF (info));
}

node *
TEGetAssign (te_info *info)
{
    DBUG_ENTER ("TEGetAssign");
    DBUG_RETURN (TI_ASSIGN (info));
}

void *
TEGetCFFun (te_info *info)
{
    DBUG_ENTER ("TEGetCFFun");
    DBUG_RETURN (TI_CFFUN (info));
}

te_info *
TEGetParent (te_info *info)
{
    DBUG_ENTER ("TEGetParent");
    DBUG_RETURN (TI_CHN (info));
}

void
TEExtendedAbort ()
{
    node *assign;
    ntype *args;

    DBUG_ENTER ("TEExtendedAbort");
    DBUG_PRINT ("NTC_INFOCHN", ("act_info_chn is %p", act_info_chn));
    if (act_info_chn != NULL) {
        CONT_ERROR ((""));
        CONT_ERROR (("TYPE ERROR TRACE:"));
        while (act_info_chn != NULL) {
            assign = TI_ASSIGN (act_info_chn);
            if (!FUNDEF_IS_LACFUN (TI_FUNDEF (act_info_chn))) {
                args = NewTypeCheck_Expr (AP_ARGS (ASSIGN_RHS (assign)));
                CONT_ERROR (("-- %s(?): %d: %s%s", filename, TI_LINE (act_info_chn),
                             TI_NAME (act_info_chn), TYType2String (args, FALSE, 0)));
            }
            act_info_chn = TI_CHN (act_info_chn);
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
 *    char *TEPrfArg2Obj( char *prf_str, int pos )
 *
 * description:
 *
 *
 ******************************************************************************/

char *
TEPrfArg2Obj (char *prf_str, int pos)
{
    static char buffer[64];
    char *tmp = &buffer[0];

    DBUG_ENTER ("TEPrfArg2Obj");

    tmp += sprintf (tmp, "argument #%d of \"%s\"", pos, prf_str);

    DBUG_RETURN (&buffer[0]);
}

/******************************************************************************
 *
 * function:
 *    char *TEArg2Obj( int pos )
 *
 * description:
 *
 *
 ******************************************************************************/

char *
TEArg2Obj (int pos)
{
    static char buffer[64];
    char *tmp = &buffer[0];

    DBUG_ENTER ("TEArg2Obj");

    tmp += sprintf (tmp, "argument #%d", pos);

    DBUG_RETURN (&buffer[0]);
}

/******************************************************************************
 *
 * function:
 *    char *TEArrayElem2Obj( int pos )
 *
 * description:
 *
 *
 ******************************************************************************/

char *
TEArrayElem2Obj (int pos)
{
    static char buffer[64];
    char *tmp = &buffer[0];

    DBUG_ENTER ("TEArrayElem2Obj");

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
 *    void TEAssureScalar( char *obj, ntype *type)
 *
 * description:
 *
 *
 ******************************************************************************/

void
TEAssureScalar (char *obj, ntype *type)
{
    DBUG_ENTER ("TEAssureScalar");

    if (!MatchScalar (type)) {
        ERROR (linenum, ("%s should be a scalar; type found: %s", obj,
                         TYType2String (type, FALSE, 0)));
        TEExtendedAbort ();
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    void TEAssureVect( char *obj, ntype *type)
 *
 * description:
 *
 *
 ******************************************************************************/

void
TEAssureVect (char *obj, ntype *type)
{
    DBUG_ENTER ("TEAssureVect");

    if (!MatchVect (type)) {
        ERROR (linenum, ("%s should be a vector; type found: %s", obj,
                         TYType2String (type, FALSE, 0)));
        TEExtendedAbort ();
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    void TEAssureIntS( char *obj, ntype *type)
 *
 * description:
 *
 *
 ******************************************************************************/

void
TEAssureIntS (char *obj, ntype *type)
{
    DBUG_ENTER ("TEAssureIntS");

    if (!MatchScalar (type) || !MatchIntA (type)) {
        ERROR (linenum, ("%s should be of type int; type found: %s", obj,
                         TYType2String (type, FALSE, 0)));
        TEExtendedAbort ();
    }
    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    void TEAssureBoolS( char *obj, ntype *type)
 *
 * description:
 *
 *
 ******************************************************************************/

void
TEAssureBoolS (char *obj, ntype *type)
{
    DBUG_ENTER ("TEAssureBoolS");

    if (!MatchScalar (type) || !MatchBoolA (type)) {
        ERROR (linenum, ("%s should be of type bool; type found: %s", obj,
                         TYType2String (type, FALSE, 0)));
        TEExtendedAbort ();
    }
    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    void TEAssureBoolA( char *obj, ntype *type)
 *
 * description:
 *
 *
 ******************************************************************************/

void
TEAssureBoolA (char *obj, ntype *type)
{
    DBUG_ENTER ("TEAssureBoolA");

    if (!MatchBoolA (type)) {
        ERROR (linenum, ("element type of %s should be boolean; type found: %s", obj,
                         TYType2String (type, FALSE, 0)));
        TEExtendedAbort ();
    }
    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    void TEAssureNumS( char *obj, ntype *type)
 *
 * description:
 *
 *
 ******************************************************************************/

void
TEAssureNumS (char *obj, ntype *type)
{
    DBUG_ENTER ("TEAssureNumS");

    if (!MatchScalar (type) || !MatchNumA (type)) {
        ERROR (linenum, ("%s should be of type int / float / double; type found: %s", obj,
                         TYType2String (type, FALSE, 0)));
        TEExtendedAbort ();
    }
    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    void TEAssureNumA( char *obj, ntype *type)
 *
 * description:
 *
 *
 ******************************************************************************/

void
TEAssureNumA (char *obj, ntype *type)
{
    DBUG_ENTER ("TEAssureNumA");

    if (!MatchNumA (type)) {
        ERROR (linenum, ("element type of %s should be numeric; type found: %s", obj,
                         TYType2String (type, FALSE, 0)));
        TEExtendedAbort ();
    }
    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    void TEAssureSimpleType( char *obj, ntype *type)
 *
 * description:
 *
 *
 ******************************************************************************/

void
TEAssureSimpleType (char *obj, ntype *type)
{
    DBUG_ENTER ("TEAssureSimpleType");

    if (!TYIsSimple (TYGetScalar (type))) {
        ERROR (linenum, ("%s should be a built-in type; type found: %s", obj,
                         TYType2String (type, FALSE, 0)));
        TEExtendedAbort ();
    }
    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    void TEAssureIntVect( char *obj, ntype *type)
 *
 * description:
 *
 *
 ******************************************************************************/

void
TEAssureIntVect (char *obj, ntype *type)
{
    DBUG_ENTER ("AssureIntVect");

    if (!MatchIntA (type) || !MatchVect (type)) {
        ERROR (linenum, ("%s should be an integer vector; type found: %s", obj,
                         TYType2String (type, FALSE, 0)));
        TEExtendedAbort ();
    }
    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    void TEAssureShpMatchesDim( char *obj1, ntype *type1,
 *                                char *obj2, ntype *type2)
 *
 * description:
 *
 *
 ******************************************************************************/

void
TEAssureShpMatchesDim (char *obj1, ntype *type1, char *obj2, ntype *type2)
{
    DBUG_ENTER ("TEAssureShpMatchesDim");

    if (((TYGetConstr (type1) == TC_aks) || (TYGetConstr (type1) == TC_akv))
        && ((TYGetConstr (type2) == TC_akv) || (TYGetConstr (type2) == TC_aks)
            || (TYGetConstr (type2) == TC_akd))
        && (SHGetExtent (TYGetShape (type1), 0) != TYGetDim (type2))) {
        ERROR (linenum, ("shape of %s should match dimensionality of %s;"
                         " types found: %s  and  %s",
                         obj1, obj2, TYType2String (type1, FALSE, 0),
                         TYType2String (type2, FALSE, 0)));
        TEExtendedAbort ();
    }

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void TEAssureValMatchesShape( char *obj1, ntype *type1,
 *                                   char *obj2, ntype *type2)
 *
 *   @brief  makes shure, that if type1 is AKV and type2 is AKS, type1
 *           constitutes a legal index into type2.
 *
 ******************************************************************************/

void
TEAssureValMatchesShape (char *obj1, ntype *type1, char *obj2, ntype *type2)
{
    int i, dim;
    int *dv;

    DBUG_ENTER ("TEAssureValMatchesShape");

    if ((TYGetConstr (type1) == TC_akv)
        && ((TYGetConstr (type2) == TC_aks) || (TYGetConstr (type2) == TC_akv))) {
        dim = TYGetDim (type2);
        dv = (int *)COGetDataVec (TYGetValue (type1));
        for (i = 0; i < dim; i++) {
            if ((dv[i] < 0) || (dv[i] >= SHGetExtent (TYGetShape (type2), i))) {
                ERROR (linenum, ("%s should be legal index into %s;"
                                 " types found: %s  and  %s",
                                 obj1, obj2, TYType2String (type1, FALSE, 0),
                                 TYType2String (type2, FALSE, 0)));
                TEExtendedAbort ();
            }
        }
    }

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void TEAssureProdValMatchesProdShape( char *obj1, ntype *type1,
 *                                           char *obj2, ntype *type2)
 *
 *   @brief  makes shure, that if type1 is AKV and type2 is AKS, type1
 *           constitutes a legal shape for the elements of type2, i.e,
 *           prod( values(type1)) == prod( shape(type2)).
 *
 ******************************************************************************/

void
TEAssureProdValMatchesProdShape (char *obj1, ntype *type1, char *obj2, ntype *type2)
{
    int i, dim, prod;
    int *dv;

    DBUG_ENTER ("TEAssureProdValMatchesProdShape");

    if ((TYGetConstr (type1) == TC_akv)
        && ((TYGetConstr (type2) == TC_aks) || (TYGetConstr (type2) == TC_akv))) {
        dim = SHGetExtent (TYGetShape (type1), 0);
        dv = (int *)COGetDataVec (TYGetValue (type1));
        prod = 1;
        for (i = 0; i < dim; i++) {
            prod *= dv[i];
        }
        if (prod != SHGetUnrLen (TYGetShape (type2))) {
            ERROR (linenum, ("%s should be legal shape for the data vector of %s;"
                             " types found: %s  and  %s",
                             obj1, obj2, TYType2String (type1, FALSE, 0),
                             TYType2String (type2, FALSE, 0)));
            TEExtendedAbort ();
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    void TEAssureSameSimpleType( char *obj1, ntype *type1,
 *                                 char *obj2, ntype *type2)
 *
 * description:
 *    expects two array types with  a simple type as scalar type!!!
 *    This property may be checked by applying   TEAssureSimpleType( )
 *    to the given argument types prior to calling this function.
 *
 ******************************************************************************/

void
TEAssureSameSimpleType (char *obj1, ntype *type1, char *obj2, ntype *type2)
{
    DBUG_ENTER ("TEAssureSameSimpleType");

    if (TYGetSimpleType (TYGetScalar (type1)) != TYGetSimpleType (TYGetScalar (type2))) {
        ERROR (linenum, ("element types of %s and %s should be identical;"
                         " types found: %s  and  %s",
                         obj1, obj2, TYType2String (type1, FALSE, 0),
                         TYType2String (type2, FALSE, 0)));
        TEExtendedAbort ();
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    void TEAssureSameScalarType( char *obj1, ntype *type1,
 *                                 char *obj2, ntype *type2)
 *
 * description:
 *
 ******************************************************************************/

void
TEAssureSameScalarType (char *obj1, ntype *type1, char *obj2, ntype *type2)
{
    DBUG_ENTER ("TEAssureSameScalarType");

    if (!TYEqTypes (TYGetScalar (type1), TYGetScalar (type2))) {
        ERROR (linenum, ("element types of %s and %s should be identical;"
                         " types found: %s  and  %s",
                         obj1, obj2, TYType2String (type1, FALSE, 0),
                         TYType2String (type2, FALSE, 0)));
        TEExtendedAbort ();
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    ntype *TEAssureSameShape( char *obj1, ntype *type1,
 *                              char *obj2, ntype *type2)
 *
 * description:
 *
 *
 ******************************************************************************/

ntype *
TEAssureSameShape (char *obj1, ntype *type1, char *obj2, ntype *type2)
{
    ntype *res = NULL;
    ntype *pend1 = NULL, *pend2 = NULL;
    ntype *type1_org = NULL, *type2_org = NULL;

    DBUG_ENTER ("TEAssureSameShape");

    /**
     * lift AKVs to AKSs!
     * pend<i> indicates ntype structures that have to be deleted!
     * type<i>_org is needed for better error messages!
     */
    if (TYGetConstr (type1) == TC_akv) {
        pend1 = TYMakeAKS (TYCopyType (TYGetScalar (type1)),
                           SHCopyShape (TYGetShape (type1)));
        type1_org = type1;
        type1 = pend1;
    }
    if (TYGetConstr (type2) == TC_akv) {
        pend2 = TYMakeAKS (TYCopyType (TYGetScalar (type2)),
                           SHCopyShape (TYGetShape (type2)));
        type2_org = type2;
        type2 = pend2;
    }

    /**
     *
     * Now, the AKV free switch:
     */
    switch (TYGetConstr (type1)) {
    case TC_aks:
        switch (TYGetConstr (type2)) {
        case TC_aks:
            if (SHCompareShapes (TYGetShape (type2), TYGetShape (type1))) {
                res = TYCopyType (type2);
            }
            break;
        case TC_akd:
            if (TYGetDim (type2) == TYGetDim (type1)) {
                res = TYCopyType (type1);
            }
            break;
        case TC_audgz:
            if (TYGetDim (type1) > 0) {
                res = TYCopyType (type1);
            }
            break;
        case TC_aud:
            res = TYCopyType (type1);
            break;
        default:
            DBUG_ASSERT (FALSE, "AssureSameShape applied to non-array type");
        }
        break;

    case TC_akd:
        switch (TYGetConstr (type2)) {
        case TC_aks:
        case TC_akd:
            if (TYGetDim (type2) == TYGetDim (type1)) {
                res = TYCopyType (type2);
            }
            break;
        case TC_audgz:
            if (TYGetDim (type1) > 0) {
                res = TYCopyType (type1);
            }
            break;
        case TC_aud:
            res = TYCopyType (type1);
            break;
        default:
            DBUG_ASSERT (FALSE, "AssureSameShape applied to non-array type");
        }
        break;

    case TC_audgz:
        switch (TYGetConstr (type2)) {
        case TC_aks:
        case TC_akd:
            if (TYGetDim (type2) > 0) {
                res = TYCopyType (type2);
            }
            break;
        case TC_audgz:
        case TC_aud:
            res = TYCopyType (type1);
            break;
        default:
            DBUG_ASSERT (FALSE, "AssureSameShape applied to non-array type");
        }
        break;

    case TC_aud:
        res = TYCopyType (type2);
        break;

    default:
        DBUG_ASSERT (FALSE, "AssureSameShape applied to non-array type");
    }

    if (pend1 != NULL) {
        type1 = type1_org;
        pend1 = TYFreeType (pend1);
    }
    if (pend2 != NULL) {
        type2 = type2_org;
        pend2 = TYFreeType (pend2);
    }

    if (res == NULL) {
        ERROR (linenum, ("%s and %s should have identical shapes;"
                         " types found: %s  and  %s",
                         obj1, obj2, TYType2String (type1, FALSE, 0),
                         TYType2String (type2, FALSE, 0)));
        TEExtendedAbort ();
    }

    DBUG_RETURN (res);
}
