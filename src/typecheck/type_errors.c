/*
 * $Log$
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

struct TE_INFO {
    int line;       /* line where the application is situated */
    char *kind_str; /* kind of function we are dealing with */
    char *name_str; /* name of the function */
    node *wrapper;  /* for udfs, this pointer points to the wrapper function */
    node *assign;   /* for udfs, this pointer points to the assign node of the ap */
};

#define TI_LINE(n) (n->line)
#define TI_KIND(n) (n->kind_str)
#define TI_NAME(n) (n->name_str)
#define TI_FUNDEF(n) (n->wrapper)
#define TI_ASSIGN(n) (n->assign)

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

/******************************************************************************
 ***
 ***          functions for handling te_info structures:
 ***          ------------------------------------------
 ***
 ******************************************************************************/

te_info *
TEMakeInfo (int linenum, char *kind_str, char *name_str, node *wrapper, node *assign)
{
    te_info *res;

    DBUG_ENTER ("TEMakeInfo");

    res = (te_info *)Malloc (sizeof (te_info));
    TI_LINE (res) = linenum;
    TI_KIND (res) = kind_str;
    TI_NAME (res) = name_str;
    TI_FUNDEF (res) = wrapper;
    TI_ASSIGN (res) = assign;

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
        ABORT (linenum, ("%s should be a scalar; type found: %s", obj,
                         TYType2String (type, FALSE, 0)));
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
        ABORT (linenum, ("%s should be of type int; type found: %s", obj,
                         TYType2String (type, FALSE, 0)));
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
        ABORT (linenum, ("%s should be of type bool; type found: %s", obj,
                         TYType2String (type, FALSE, 0)));
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
        ABORT (linenum, ("element type of %s should be boolean; type found: %s", obj,
                         TYType2String (type, FALSE, 0)));
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
        ABORT (linenum, ("%s should be a built-in type; type found: %s", obj,
                         TYType2String (type, FALSE, 0)));
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
        ABORT (linenum, ("%s should be an integer vector; type found: %s", obj,
                         TYType2String (type, FALSE, 0)));
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

    if ((TYGetConstr (type1) == TC_aks)
        && ((TYGetConstr (type2) == TC_aks) || (TYGetConstr (type2) == TC_akd))
        && (SHGetExtent (TYGetShape (type1), 0) != TYGetDim (type2))) {
        ABORT (linenum, ("%s should be legal index into %s;"
                         " types found: %s  and  %s",
                         obj1, obj2, TYType2String (type1, FALSE, 0),
                         TYType2String (type2, FALSE, 0)));
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
        ABORT (linenum, ("element types of %s and %s should be identical;"
                         " types found: %s  and  %s",
                         obj1, obj2, TYType2String (type1, FALSE, 0),
                         TYType2String (type2, FALSE, 0)));
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

    DBUG_ENTER ("TEAssureSameShape");

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

    if (res == NULL) {
        ABORT (linenum, ("%s and %s should have identical shapes;"
                         " types found: %s  and  %s",
                         obj1, obj2, TYType2String (type1, FALSE, 0),
                         TYType2String (type2, FALSE, 0)));
    }

    DBUG_RETURN (res);
}
