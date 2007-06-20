/** <!--********************************************************************-->
 *
 * @defgroup SCS Symbolic Constant Simplification
 *
 *   This module contains the function table entries that
 *   implement symbolic constant simplification.
 *
 *   Specifically, the following replacements are made by
 *   functions here:
 *
 *    add (x+0          ->x,     0+x ->x),
 *    add (x+esdneg(x)  -> 0
 *    add (esdneg(x)+x  -> 0
 *    sub (x-0          ->x,     x-x ->0)
 *    mul (x*0          ->0,     0*x ->0, x*0 -> 0, x*1 ->x, 1*x. ->x),
 *    div (x/0          ->error, 0/x ->0, x/1 ->x),
 *    and (x&1          ->x,     1&x ->x, x&1 -> x, x&0 ->0, 0&x. ->0),
 *    or  (x|1          ->1,     1|x ->1, x|1 -> 1, x|0 ->x, 0|x. ->x)
 *    mod (mod(x,0)     -> error)
 *
 *    eq ((x == y)  -> TRUE  IFF (x == y))
 *    neq((x != y)  -> FALSE IFF (x == y))
 *    ge ((x >= x)  -> TRUE  IFF (x == x))
 *    le ((x <= x)  -> TRUE  IFF (x == x))
 *    not(x)        -> TRUE  iFF (FALSE == x), FALSE IFF (TRUE == x)
 *
 * The functions in this module are called from the generic
 * constant-folding traversal in constant_folding.c, via function table prf_scs[]
 *
 *  @ingroup opt
 *
 *  @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file symbolic_constant_simplification.c
 *
 * Prefix: SCS
 *
 *****************************************************************************/
#include "symbolic_constant_simplification.h"

#include "dbug.h"
#include "tree_basic.h"
#include "node_basic.h"
#include "tree_compound.h"
#include "new_types.h"
#include "type_utils.h"
#include "new_typecheck.h"
#include "globals.h"
#include "DupTree.h"
#include "constants.h"
#include "shape.h"
#include "compare_tree.h"
#include "ctinfo.h"

/* local used helper functions */

/******************************************************************************
 *
 * function:
 *   shape *GetShapeOfExpr(node *expr)
 *
 * description:
 *   try to calculate the shape of the given expression. this can be a
 *   identifier or an array node. returns NULL if no shape can be computed.
 *
 *****************************************************************************/

static shape *
GetShapeOfExpr (node *expr)
{
    ntype *etype;
    shape *shp = NULL;

    DBUG_ENTER ("GetShapeOfExpr");

    DBUG_ASSERT ((expr != NULL), "GetShapeOfExpr called with NULL pointer");

    etype = NTCnewTypeCheck_Expr (expr);

    if (TUshapeKnown (etype)) {
        shp = SHcopyShape (TYgetShape (etype));
    }

    etype = TYfreeType (etype);

    DBUG_RETURN (shp);
}

/** <!--********************************************************************-->
 *
 * @fn node *MakeZero( node *prfarg)
 * Create an array of zeros of the same type and shape as prfarg.
 *
 *****************************************************************************/
node *
MakeZero (node *prfarg)
{
    constant *tmp;
    constant *con;
    node *res;

    DBUG_ENTER ("MakeZero");
    con = COaST2Constant (prfarg);
    tmp = COmakeZero (COgetType (con), GetShapeOfExpr (prfarg));
    res = COconstant2AST (tmp);

    con = COfreeConstant (con);
    tmp = COfreeConstant (tmp);
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *MakeOne( node *prfarg)
 * Create an array of ones of the same type and shape as prfarg.
 *
 *****************************************************************************/
node *
MakeOne (node *prfarg)
{
    constant *tmp;
    constant *con;
    node *res;

    DBUG_ENTER ("MakeOne");
    con = COaST2Constant (prfarg);
    tmp = COmakeOne (COgetType (con), GetShapeOfExpr (prfarg));
    res = COconstant2AST (tmp);

    con = COfreeConstant (con);
    tmp = COfreeConstant (tmp);
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *MakeFalse( node *prfarg)
 * Create an array of FALSE of the same type and shape as prfarg.
 *
 *****************************************************************************/
node *
MakeFalse (node *prfarg)
{
    constant *tmp;
    node *res;

    DBUG_ENTER ("MakeFalse");
    tmp = COmakeFalse (GetShapeOfExpr (prfarg));
    res = COconstant2AST (tmp);

    tmp = COfreeConstant (tmp);
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *MakeTrue( node *prfarg)
 * Create an array of TRUE of the same type and shape as prfarg.
 *
 *****************************************************************************/
node *
MakeTrue (node *prfarg)
{
    constant *tmp;
    node *res;

    DBUG_ENTER ("MakeTrue");
    tmp = COmakeTrue (GetShapeOfExpr (prfarg));
    res = COconstant2AST (tmp);

    tmp = COfreeConstant (tmp);
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn bool MatchVar( node *prfarg, node **avis)
 * Predicate for prfarg (a PRF_ARG) being an N_id. If so,
 * it has the SIDE EFFECT of setting *avis iff it is NULL.
 * Bodo considers this a feature. rbe considers it a taste error.
 *
 *
 *****************************************************************************/
bool
MatchVar (node *prfarg, node **avis)
{
    bool res = FALSE;

    DBUG_ENTER ("MatchVar");

    if (N_id == NODE_TYPE (prfarg)) {
        res = TRUE;
        if (NULL == avis) {
            *avis = ID_AVIS (prfarg);
        } else {
            res = ID_AVIS (prfarg) == *avis; /* Pointer check suffices due to CSE, etc. */
        }
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn bool MatchConstantZero( node *prfarg)
 * Predicate for PRF_ARG being a constant zero of any rank or type.
 * E.g., 0 or  [0,0] or  genarray([2,3], 0)
 *
 *****************************************************************************/
bool
MatchConstantZero (node *prfarg)
{
    constant *argconst;
    bool res = FALSE;

    DBUG_ENTER ("MatchConstantZero");
    argconst = COaST2Constant (prfarg);
    if (NULL != argconst) {
        res = COisZero (argconst, TRUE);
        argconst = COfreeConstant (argconst);
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn bool MatchConstantOne( node *prfarg)
 * Predicate for PRF_ARG being a constant one of any rank or type.
 * E.g., 1 or  [1,1] or  genarray([2,3], 1)
 *
 *****************************************************************************/
bool
MatchConstantOne (node *prfarg)
{
    constant *argconst;
    bool res = FALSE;

    DBUG_ENTER ("MatchConstantOne");
    argconst = COaST2Constant (prfarg);
    if (NULL != argconst) {
        res = COisOne (argconst, TRUE);
        argconst = COfreeConstant (argconst);
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn bool MatchConstantTrue( node *prfarg)
 * Predicate for PRF_ARG being a constant TRUE of any rank or type.
 * E.g., TRUE or  [TRUE,TRUE] or  genarray([2,3], TRUE)
 *
 *****************************************************************************/
bool
MatchConstantTrue (node *prfarg)
{
    constant *argconst;
    bool res = FALSE;

    DBUG_ENTER ("MatchConstantTrue");
    argconst = COaST2Constant (prfarg);
    if (NULL != argconst) {
        res = COisTrue (argconst, TRUE);
        argconst = COfreeConstant (argconst);
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn bool MatchConstantFalse( node *prfarg)
 * Predicate for PRF_ARG being a constant FALSE of any rank or type.
 * E.g., FALSE or  [FALSE,FALSE] or  genarray([2,3], FALSE)
 *
 *****************************************************************************/
bool
MatchConstantFalse (node *prfarg)
{
    constant *argconst;
    bool res = FALSE;

    DBUG_ENTER ("MatchConstantFalse");
    argconst = COaST2Constant (prfarg);
    if (NULL != argconst) {
        res = COisFalse (argconst, TRUE);
        argconst = COfreeConstant (argconst);
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn bool MatchEsdneg( node *prfarg1, node *prfarg2)
 * Predicate for prfarg1 being esdneg(prfarg2) or
 *               prfarg2 being esdneg(prfarg1)
 *
 *****************************************************************************/
bool
MatchEsdneg (node *prfarg1, node *prfarg2)
{
    bool res = FALSE;
    node *ssas1;
    node *ssas2;

    DBUG_ENTER ("MatchEsdneg");
    if ((NODE_TYPE (prfarg1) == N_id) && (NODE_TYPE (prfarg2) == N_id)
        && (TUshapeKnown (AVIS_TYPE (ID_AVIS (prfarg1))))
        && (TUshapeKnown (AVIS_TYPE (ID_AVIS (prfarg2))))) {
        ssas1 = AVIS_SSAASSIGN (ID_AVIS (prfarg1));
        ssas2 = AVIS_SSAASSIGN (ID_AVIS (prfarg2));

        if (((ssas1 != NULL) && (NODE_TYPE (ASSIGN_RHS (ssas1)) == N_prf)
             && (PRF_PRF (ASSIGN_RHS (ssas1)) == F_esd_neg)
             && (ID_AVIS (PRF_ARG1 (ASSIGN_RHS (ssas1))) == ID_AVIS (prfarg2)))
            || ((ssas2 != NULL) && (NODE_TYPE (ASSIGN_RHS (ssas2)) == N_prf)
                && (PRF_PRF (ASSIGN_RHS (ssas2)) == F_esd_neg)
                && (ID_AVIS (PRF_ARG1 (ASSIGN_RHS (ssas2))) == ID_AVIS (prfarg1)))) {
            res = TRUE;
        }
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_add( node *arg_node, info *arg_info)
 * X+0 -> X
 * 0+X -> X
 *
 * x+esdneg(x) -> 0 of shape(x)
 * esdneg(x)+x -> 0 of shape(x)
 *
 *****************************************************************************/
node *
SCSprf_add (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_add");
    if (MatchConstantZero (PRF_ARG1 (arg_node))) { /* 0 + X */
        res = DUPdoDupTree (PRF_ARG2 (arg_node));
    } else if (MatchConstantZero (PRF_ARG2 (arg_node))) { /* X + 0 */
        res = DUPdoDupTree (PRF_ARG1 (arg_node));
    } else if (MatchEsdneg (PRF_ARG1 (arg_node),
                            PRF_ARG2 (arg_node))) { /*    X + esdneg(X) */
                                                    /* or esdneg(X) + X */
        res = MakeZero (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_sub( node *arg_node, info *arg_info)
 * X-0 -> X
 *
 *****************************************************************************/
node *
SCSprf_sub (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *X;

    DBUG_ENTER ("SCSprf_sub");
    if (MatchConstantZero (PRF_ARG2 (arg_node))) { /* X - 0 */
        res = DUPdoDupTree (PRF_ARG1 (arg_node));
    } else if (MatchVar (PRF_ARG1 (arg_node), &X)
               && MatchVar (PRF_ARG2 (arg_node), &X)) { /* X - X */
        res = MakeZero (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_mul( node *arg_node, info *arg_info)
 * X * 1 -> X
 * 1 * X -> X
 * X * 0 -> 0 of shape(X)
 * 0 * X -> 0 of shape(X)
 *
 *****************************************************************************/
node *
SCSprf_mul (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_mul");
    if (MatchConstantOne (PRF_ARG2 (arg_node))) { /* X * 1 */
        res = DUPdoDupTree (PRF_ARG1 (arg_node));

    } else if (MatchConstantOne (PRF_ARG1 (arg_node))) { /* 1 * X */
        res = DUPdoDupTree (PRF_ARG2 (arg_node));

    } else if (MatchConstantZero (PRF_ARG2 (arg_node))) { /* X * 0 */
        res = MakeZero (PRF_ARG1 (arg_node));

    } else if (MatchConstantZero (PRF_ARG1 (arg_node))) { /* 0 * X */
        res = MakeZero (PRF_ARG2 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_div( node *arg_node, info *arg_info)
 * X / 1 -> X
 * 0 / X -> 0 of shape(X)
 * X / 0 -> error
 *
 *****************************************************************************/
node *
SCSprf_div (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_div");
    if (MatchConstantOne (PRF_ARG2 (arg_node))) { /*  X / 1 */
        res = DUPdoDupTree (PRF_ARG1 (arg_node));
    } else if (MatchConstantZero (PRF_ARG1 (arg_node))) { /* 0 / X */
        res = MakeZero (PRF_ARG2 (arg_node));
        CTIwarnLine (NODE_LINE (arg_node), "Expression 0/x replaced by 0");
    } else if (MatchConstantZero (PRF_ARG2 (arg_node))) { /* X / 0 */
        CTIabortLine (NODE_LINE (arg_node), "Division by zero encountered");
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_not( node *arg_node, info *arg_info)
 *
 *
 *****************************************************************************/
node *
SCSprf_not (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_not");
    if (MatchConstantTrue (PRF_ARG1 (arg_node))) { /*  !TRUE */
        res = MakeFalse (PRF_ARG1 (arg_node));
    } else if (MatchConstantFalse (PRF_ARG1 (arg_node))) { /*  !FALSE */
        res = MakeTrue (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_or( node *arg_node, info *arg_info)
 * X, info *arg_info | 0  -> X
 * 0 | X  -> X
 * X | 1  -> 1 of shape(X)
 * 1 | X  -> 1 of shape(X)
 *
 *****************************************************************************/
node *
SCSprf_or (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_or");
    if (MatchConstantZero (PRF_ARG2 (arg_node))) { /*  X | 0 */
        res = DUPdoDupTree (PRF_ARG1 (arg_node));
    } else if (MatchConstantZero (PRF_ARG1 (arg_node))) { /*  0 | X */
        res = DUPdoDupTree (PRF_ARG2 (arg_node));
    } else if (MatchConstantOne (PRF_ARG2 (arg_node))) { /*  X | 1 */
        res = MakeOne (PRF_ARG1 (arg_node));
    } else if (MatchConstantOne (PRF_ARG1 (arg_node))) { /*  1 | X */
        res = MakeOne (PRF_ARG2 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_and( node *arg_node, info *arg_info)
 * X & 1  -> X
 * 1 & X  -> X
 * X & 0  -> 0 of shape(X)
 * 0 & X  -> 0 of shape(X)
 *
 *****************************************************************************/
node *
SCSprf_and (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_and");
    if (MatchConstantOne (PRF_ARG2 (arg_node))) { /*  X & 1 */
        res = DUPdoDupTree (PRF_ARG1 (arg_node));
    } else if (MatchConstantOne (PRF_ARG1 (arg_node))) { /*  1 & X */
        res = DUPdoDupTree (PRF_ARG2 (arg_node));
    } else if (MatchConstantZero (PRF_ARG2 (arg_node))) { /*  X & 0 */
        res = MakeZero (PRF_ARG1 (arg_node));
    } else if (MatchConstantZero (PRF_ARG1 (arg_node))) { /*  0 & X */
        res = MakeZero (PRF_ARG2 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_mod( node *arg_node, info *arg_info)
 * mod(X,0)  -> error
 *
 *****************************************************************************/
node *
SCSprf_mod (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_mod");
    if (MatchConstantZero (PRF_ARG2 (arg_node))) {
        CTIabortLine (NODE_LINE (PRF_ARG2 (arg_node)), "Mod(X,0) encountered.");
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_to_something( node *arg_node, type typ)
 * Identity if arg is already of type typ.
 *
 *****************************************************************************/
node *
SCSprf_to_something (node *arg_node, simpletype typ)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_to_something");
    if (typ == TYgetSimpleType (TYgetScalar (ID_NTYPE (PRF_ARG1 (arg_node))))) {
        res = DUPdoDupTree (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_minmax( node *arg_node, info *arg_info)
 * If (X == Y), then X.
 *
 *****************************************************************************/
node *
SCSprf_minmax (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *X;

    DBUG_ENTER ("SCSprf_minmax");
    if (MatchVar (PRF_ARG1 (arg_node), &X)
        && MatchVar (PRF_ARG2 (arg_node), &X)) { /* min(X,X) or max(X,X) */
        res = DUPdoDupTree (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * The actual function table operations follow
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_toi_S( node *arg_node, info *arg_info)
 * Identity if arg is already an int.
 *
 *****************************************************************************/
node *
SCSprf_toi_S (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCSprf_toi_S");
    DBUG_RETURN (SCSprf_to_something (arg_node, N_num));
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_tof_S( node *arg_node, info *arg_info)
 * Identity if arg is already a float.
 *
 *****************************************************************************/
node *
SCSprf_tof_S (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCSprf_tof_S");
    DBUG_RETURN (SCSprf_to_something (arg_node, N_num));
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_tod_S( node *arg_node, info *arg_info)
 * Identity if arg is already a double.
 *
 *****************************************************************************/
node *
SCSprf_tod_S (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCSprf_tod_S");
    DBUG_RETURN (SCSprf_to_something (arg_node, N_num));
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_add_SxS( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_add_SxS (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("SCSprf_add_SxS");
    DBUG_RETURN (SCSprf_add (arg_node, arg_info));
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_add_SxV( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_add_SxV (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCSprf_add_SxV");
    DBUG_RETURN (SCSprf_add (arg_node, arg_info));
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_add_VxS( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_add_VxS (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCSprf_add_VxS");
    DBUG_RETURN (SCSprf_add (arg_node, arg_info));
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_add_VxV( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_add_VxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_add_VxV");
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_sub_SxS( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_sub_SxS (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCSprf_sub_SxS");
    DBUG_RETURN (SCSprf_sub (arg_node, arg_info));
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_sub_SxV( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_sub_SxV (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCSprf_sub_SxV");
    DBUG_RETURN (SCSprf_sub (arg_node, arg_info));
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_sub_VxS( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_sub_VxS (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCSprf_sub_VxS");
    DBUG_RETURN (SCSprf_sub (arg_node, arg_info));
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_sub_VxV( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_sub_VxV (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *X;

    DBUG_ENTER ("SCSprf_sub_VxV");
    if (MatchVar (PRF_ARG1 (arg_node), &X)
        && MatchVar (PRF_ARG2 (arg_node), &X)) { /* X - X */
        res = MakeZero (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_mul_SxS( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_mul_SxS (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCSprf_mul_SxS");
    DBUG_RETURN (SCSprf_mul (arg_node, arg_info));
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_mul_SxV( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_mul_SxV (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCSprf_mul_SxV");
    DBUG_RETURN (SCSprf_mul (arg_node, arg_info));
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_mul_VxS( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_mul_VxS (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCSprf_mul_VxS");
    DBUG_RETURN (SCSprf_mul (arg_node, arg_info));
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_mul_VxV( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_mul_VxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_mul_VxV");
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_div_SxS( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_div_SxS (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCSprf_div_SxS");
    DBUG_RETURN (SCSprf_div (arg_node, arg_info));
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_div_SxV( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_div_SxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_div_SxV");
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_div_VxS( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_div_VxS (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCSprf_div_VxS");
    DBUG_RETURN (SCSprf_div (arg_node, arg_info));
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_div_VxV( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_div_VxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_div_VxV");
    /* Can't use MatchVar trick here, due to  0/0  problem */
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_not_S( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_not_S (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCSprf_not_S");
    DBUG_RETURN (SCSprf_not (arg_node, arg_info));
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_not_V( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_not_V (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCSprf_not_V");
    DBUG_RETURN (SCSprf_not (arg_node, arg_info));
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_and_SxS( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_and_SxS (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCSprf_and_SxS");
    DBUG_RETURN (SCSprf_and (arg_node, arg_info));
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_and_SxV( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_and_SxV (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCSprf_and_SxV");
    DBUG_RETURN (SCSprf_and (arg_node, arg_info));
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_and_VxS( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_and_VxS (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCSprf_and_VxS");
    DBUG_RETURN (SCSprf_and (arg_node, arg_info));
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_and_VxV( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_and_VxV (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *X;

    DBUG_ENTER ("SCSprf_and_VxV");
    if (MatchVar (PRF_ARG1 (arg_node), &X)
        && MatchVar (PRF_ARG2 (arg_node), &X)) { /* X & X */
        res = DUPdoDupTree (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_or_SxS( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_or_SxS (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCSprf_or_SxS");
    DBUG_RETURN (SCSprf_or (arg_node, arg_info));
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_or_SxV( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_or_SxV (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCSprf_or_SxV");
    DBUG_RETURN (SCSprf_or (arg_node, arg_info));
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_or_VxS( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_or_VxS (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCSprf_or_VxS");
    DBUG_RETURN (SCSprf_or (arg_node, arg_info));
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_or_VxV( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_or_VxV (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *X;

    DBUG_ENTER ("SCSprf_or_VxV");
    if (MatchVar (PRF_ARG1 (arg_node), &X)
        && MatchVar (PRF_ARG2 (arg_node), &X)) { /* X | X */
        res = DUPdoDupTree (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSmod_SxS( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_mod_SxS (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCSprf_mod_SxS");
    DBUG_RETURN (SCSprf_mod (arg_node, arg_info));
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_mod_SxV( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_mod_SxV (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCSprf_mod_SxV");
    DBUG_RETURN (SCSprf_mod (arg_node, arg_info));
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_mod_VxS( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_mod_VxS (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCSprf_mod_VxS");
    DBUG_RETURN (SCSprf_mod (arg_node, arg_info));
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_mod_VxV( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_mod_VxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_mod_VxV");
    /* Could be supported with ISMOP and shape conformability check */
    /* FIXME : what is mod(0,0) etc??? ??? */
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_eq_SxS( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_eq_SxS (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *X;

    DBUG_ENTER ("SCSprf_eq_SxS");
    if (MatchVar (PRF_ARG1 (arg_node), &X)
        && MatchVar (PRF_ARG2 (arg_node), &X)) { /* X == X */
        res = MakeTrue (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_eq_SxV( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_eq_SxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_eq_SxV");
    /* Could be supported with ISMOP */
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_eq_VxS( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_eq_VxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_eq_VxS");
    /* Could be supported with ISMOP */
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_eq_VxV( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_eq_VxV (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *X;

    DBUG_ENTER ("SCSprf_eq_VxV");
    if (MatchVar (PRF_ARG1 (arg_node), &X)
        && MatchVar (PRF_ARG2 (arg_node), &X)) { /* X == X */
        res = MakeTrue (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_neq_SxS( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_neq_SxS (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *X;

    DBUG_ENTER ("SCSprf_neq_SxS");
    if (MatchVar (PRF_ARG1 (arg_node), &X)
        && MatchVar (PRF_ARG2 (arg_node), &X)) { /* X != X */
        res = MakeFalse (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_neq_SxV( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_neq_SxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_neq_SxV");
    /* Could be supported with ISMOP */
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_neq_VxS( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_neq_VxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_neq_VxS");
    /* Could be supported with ISMOP */
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_neq_VxV( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_neq_VxV (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *X;

    DBUG_ENTER ("SCSprf_neq_VxV");
    if (MatchVar (PRF_ARG1 (arg_node), &X)
        && MatchVar (PRF_ARG2 (arg_node), &X)) { /* X != X */
        res = MakeFalse (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_le_SxS( node *arg_node, info *arg_info)
 * If (X == Y), then TRUE.
 *
 *****************************************************************************/
node *
SCSprf_le_SxS (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *X;

    DBUG_ENTER ("SCSprf_le_SxS");
    if (MatchVar (PRF_ARG1 (arg_node), &X)
        && MatchVar (PRF_ARG2 (arg_node), &X)) { /* X <= X */
        res = MakeTrue (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_le_SxV( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_le_SxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_le_SxV");
    /* Could be supported with ISMOP */
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_le_VxS( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_le_VxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_le_VxS");
    /* Could be supported with ISMOP */
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_le_VxV( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_le_VxV (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *X;

    DBUG_ENTER ("SCSprf_le_VxV");
    if (MatchVar (PRF_ARG1 (arg_node), &X)
        && MatchVar (PRF_ARG2 (arg_node), &X)) { /* X <= X */
        res = MakeTrue (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_ge_SxS( node *arg_node, info *arg_info)
 * If (X == Y), then TRUE.
 *
 *****************************************************************************/
node *
SCSprf_ge_SxS (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *X;

    DBUG_ENTER ("SCSprf_ge_SxS");
    if (MatchVar (PRF_ARG1 (arg_node), &X)
        && MatchVar (PRF_ARG2 (arg_node), &X)) { /* X >= X */
        res = MakeTrue (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_ge_SxV( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_ge_SxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_ge_SxV");
    /* Could be supported with ISMOP */
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_ge_VxS( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_ge_VxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_ge_VxS");
    /* Could be supported with ISMOP */
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_ge_VxV( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_ge_VxV (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *X;

    DBUG_ENTER ("SCSprf_ge_VxV");
    if (MatchVar (PRF_ARG1 (arg_node), &X)
        && MatchVar (PRF_ARG2 (arg_node), &X)) { /* X >= X */
        res = MakeTrue (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSmin_SxS( node *arg_node, info *arg_info)
 * If (X == Y), then X.
 *
 *****************************************************************************/
node *
SCSprf_min_SxS (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCSprf_min_SxS");
    DBUG_RETURN (SCSprf_minmax (arg_node, arg_info));
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_min_SxV( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_min_SxV (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCSprf_min_SxV");
    DBUG_RETURN (SCSprf_minmax (arg_node, arg_info));
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_min_VxS( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_min_VxS (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCSprf_min_VxS");
    DBUG_RETURN (SCSprf_minmax (arg_node, arg_info));
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_min_VxV( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_min_VxV (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCSprf_min_VxV");
    DBUG_RETURN (SCSprf_minmax (arg_node, arg_info));
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_max_SxS( node *arg_node, info *arg_info)
 * If (X == Y), then X.
 *
 *****************************************************************************/
node *
SCSprf_max_SxS (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCSprf_max_SxS");
    DBUG_RETURN (SCSprf_minmax (arg_node, arg_info));
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_max_SxV( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_max_SxV (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCSprf_max_SxV");
    DBUG_RETURN (SCSprf_minmax (arg_node, arg_info));
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_max_VxS( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_geq_VxS (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCSprf_max_VxS");
    DBUG_RETURN (SCSprf_minmax (arg_node, arg_info));
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_max_VxV( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_max_VxV (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCSprf_max_VxV");
    DBUG_RETURN (SCSprf_minmax (arg_node, arg_info));
}
