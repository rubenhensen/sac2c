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
 *    div (x/0          ->error, x/1 ->x),
 *    and (x&1          ->x,     1&x ->x, x&1 -> x, x&0 ->0, 0&x. ->0),
 *    or  (x|1          ->1,     1|x ->1, x|1 -> 1, x|0 ->x, 0|x. ->x)
 *    mod (mod(x,0)     -> error)
 *
 *    eq ((x == y)  -> TRUE  IFF (x == y))
 *    neq((x != y)  -> FALSE IFF (x == y))
 *    ge ((x >= x)  -> TRUE  IFF (x == x))
 *    le ((x <= x)  -> TRUE  IFF (x == x))
 *    not(x)        -> TRUE  iFF (FALSE == x), FALSE IFF (TRUE == x)
 *    tob(boolean)  -> Boolean and other coercion identities (int->int, char->char,
 *                     float->float, double->double
 *
 * TODO: Most of the optimizations could be extended to operate
 *       on VxV data, when we can detect that the array shapes
 *       must match. Perhaps I should go try to pull this stuff
 *       out of SAA-land.
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
#include "pattern_match.h"

/* local used helper functions */

#define MATCH_PRF_ARG1_PRF_ARG2(arg_node, glob)                                          \
    PM (PMvar (&glob, PMvar (&glob, PRF_ARGS (arg_node))))

/******************************************************************************
 *
 * function:
 *   simpletype GetBasetypeOfExpr(node *expr)
 *
 * description:
 *   try to get the basetype of the given expression. this can be a
 *   identifier or an array node. returns NULL if no type can be computed.
 *
 *****************************************************************************/
static simpletype
GetBasetypeOfExpr (node *expr)
{
    simpletype stype;
    ntype *etype;

    DBUG_ENTER ("GetBasetypeOfExpr");
    DBUG_ASSERT ((expr != NULL), "GetBasetypeOfExpr called with NULL pointer");

    etype = NTCnewTypeCheck_Expr (expr);

    stype = TYgetSimpleType (TYgetScalar (etype));

    etype = TYfreeType (etype);

    DBUG_RETURN (stype);
}

/** <!--********************************************************************-->
 *
 * @fn node *MakeZero( node *prfarg)
 * Create an array of zeros/FALSE of the same type and shape as prfarg.
 * If prfarg is not AKS, we give up.
 *
 *****************************************************************************/
node *
MakeZero (node *prfarg)
{
    constant *con;
    shape *shp;
    ntype *typ;
    node *res = NULL;

    DBUG_ENTER ("MakeZero");
    typ = NTCnewTypeCheck_Expr (prfarg);
    if (TUshapeKnown (typ)) {
        shp = TYgetShape (typ);
        con = COmakeZero (GetBasetypeOfExpr (prfarg), shp);
        if (NULL != con) {
            res = COconstant2AST (con);
            con = COfreeConstant (con);
        }
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *MakeFalse( node *prfarg)
 * Create an array of FALSE of the same shape as prfarg.
 * If prfarg is not AKS, we give up.
 *
 *****************************************************************************/
node *
MakeFalse (node *prfarg)
{
    constant *con;
    shape *shp;
    ntype *typ;
    node *res = NULL;

    DBUG_ENTER ("MakeFalse");
    typ = NTCnewTypeCheck_Expr (prfarg);
    if (TUshapeKnown (typ)) {
        shp = TYgetShape (typ);
        con = COmakeFalse (shp);
        if (NULL != con) {
            res = COconstant2AST (con);
            con = COfreeConstant (con);
        }
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *MakeTrue( node *prfarg)
 * Create an array of TRUE of the same shape as prfarg.
 * If prfarg is not AKS, we give up.
 *
 *****************************************************************************/
node *
MakeTrue (node *prfarg)
{
    constant *con;
    shape *shp;
    ntype *typ;
    node *res = NULL;

    DBUG_ENTER ("MakeTrue");
    typ = NTCnewTypeCheck_Expr (prfarg);
    if (TUshapeKnown (typ)) {
        shp = TYgetShape (typ);
        con = COmakeTrue (shp);
        if (NULL != con) {
            res = COconstant2AST (con);
            con = COfreeConstant (con);
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

#if 0
/* attempt to debug esdneg code that never manages to find an esdneg  */

    if ( ssas1 != NULL) {
           b1 =  NODE_TYPE( ASSIGN_RHS( ssas1)) == N_prf ;
}
    if (b1) {
	   b1 =  PRF_PRF( ASSIGN_RHS( ssas1)) == F_esd_neg;
}
 if (b1){
	   b1 = ID_AVIS( PRF_ARG1( ASSIGN_RHS( ssas1))) == ID_AVIS( prfarg2);
	}

#endif

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
 * @fn node *SCSprf_add_SxS( node *arg_node, info *arg_info)
 * X + 0 -> X
 * 0 + X -> X
 *
 * x+esdneg(x) -> 0
 * esdneg(x)+x -> 0
 *
 * ISMOP could handle _add_VxV_ if we performed AKS length-error check
 *
 *****************************************************************************/
node *
SCSprf_add_SxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_add_SxS");
    if (MatchConstantZero (PRF_ARG1 (arg_node))) { /* 0 + X */
        res = DUPdoDupTree (PRF_ARG2 (arg_node));
    } else if (MatchConstantZero (PRF_ARG2 (arg_node))) { /* X + 0 */
        res = DUPdoDupTree (PRF_ARG1 (arg_node));
    } else if (MatchEsdneg (PRF_ARG1 (arg_node),
                            PRF_ARG2 (arg_node))) { /*    X + esdneg(X) */
                                                    /* or esdneg(X) + X */
        /* CF is not invoked, at present, during the time when esdneg exists,
         * so don't expect the last IF clause to do anything of interest
         */
        res = MakeZero (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_add_SxV( node *arg_node, info *arg_info)
 * 0 + X -> X
 *
 *****************************************************************************/
node *
SCSprf_add_SxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_add_SxV");
    if (MatchConstantZero (PRF_ARG1 (arg_node))) { /* 0 + X */
        res = DUPdoDupTree (PRF_ARG2 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_add_VxS( node *arg_node, info *arg_info)
 * X + 0 -> X
 *
 *****************************************************************************/
node *
SCSprf_add_VxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_add");
    if (MatchConstantZero (PRF_ARG2 (arg_node))) { /* X + 0 */
        res = DUPdoDupTree (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_sub( node *arg_node, info *arg_info)
 * X - 0 -> X
 * SCSprf_sub_SxS, SCSprf_sub_VxS
 *
 *****************************************************************************/
node *
SCSprf_sub (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *X = NULL;

    DBUG_ENTER ("SCSprf_sub");
    if (MatchConstantZero (PRF_ARG2 (arg_node))) { /* X - 0 */
        res = DUPdoDupTree (PRF_ARG1 (arg_node));
    } else if (MATCH_PRF_ARG1_PRF_ARG2 (arg_node, X)) { /* X - X */
        res = MakeZero (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_sub_VxV( node *arg_node, info *arg_info)
 * X - X -> (shape(X)) reshape(0)
 *
 * ISMOP: Could handle SCSprf_sub_VxV if we could do length error check
 *
 *****************************************************************************/
node *
SCSprf_sub_VxV (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *X = NULL;

    DBUG_ENTER ("SCSprf_sub_VxV");
    /* Can't do X - 0 unless we know argument shapes match */
    if (MATCH_PRF_ARG1_PRF_ARG2 (arg_node, X)) { /* X - X */
        res = MakeZero (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_mul_SxS( node *arg_node, info *arg_info)
 * X * 1 -> X
 * 1 * X -> X
 * X * 0 -> 0 of shape(X)
 * 0 * X -> 0 of shape(X)
 * can't handle VxV because we need length error check
 *
 *****************************************************************************/
node *
SCSprf_mul_SxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_mul_SxS");
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
 * @fn node *SCSprf_mul_SxV( node *arg_node, info *arg_info)
 * 1 * X -> X
 * 0 * X -> 0 of shape(X)
 *****************************************************************************/
node *
SCSprf_mul_SxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_mul_SxV");
    if (MatchConstantOne (PRF_ARG1 (arg_node))) { /* 1 * X */
        res = DUPdoDupTree (PRF_ARG2 (arg_node));
    } else if (MatchConstantZero (PRF_ARG1 (arg_node))) { /* 0 * X */
        res = MakeZero (PRF_ARG2 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_mul_VxS( node *arg_node, info *arg_info)
 * X * 1 -> X
 * X * 0 -> 0 of shape(X)
 * can't handle VxV because we need length error check
 *****************************************************************************/
node *
SCSprf_mul_VxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_mul_VxS");
    if (MatchConstantOne (PRF_ARG2 (arg_node))) { /* X * 1 */
        res = DUPdoDupTree (PRF_ARG1 (arg_node));
    } else if (MatchConstantZero (PRF_ARG2 (arg_node))) { /* X * 0 */
        res = MakeZero (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_div( node *arg_node, info *arg_info)
 * X / 1 -> X
 * X / 0 -> error
 * Handles SCSprf_div_SxS, SCSprf_div_VxS
 *
 *****************************************************************************/
node *
SCSprf_div (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_div");
    if (MatchConstantOne (PRF_ARG2 (arg_node))) { /*  X / 1 */
        res = DUPdoDupTree (PRF_ARG1 (arg_node));

#if 0
disabled because this could let 0/0 creep through!
  } else if ( MatchConstantZero( PRF_ARG1( arg_node))) {  /* 0 / X */
    res = MakeZero( PRF_ARG2( arg_node));
    CTIwarnLine( NODE_LINE( arg_node), "Expression 0/x replaced by 0");
#endif
    } else if (MatchConstantZero (PRF_ARG2 (arg_node))) { /* X / 0 */
        CTIabortLine (NODE_LINE (arg_node), "Division by zero encountered");
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_not( node *arg_node, info *arg_info)
 *
 * This stub left in place so buddy can implement !!X using spiffy PM code.
 *****************************************************************************/
node *
SCSprf_not (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_not");
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_or_SxS( node *arg_node, info *arg_info)
 * X | 0  -> X
 * 0 | X  -> X
 * X | 1  -> 1 of shape(X)
 * 1 | X  -> 1 of shape(X)
 * X | Z  -> X
 *
 *****************************************************************************/
node *
SCSprf_or_SxS (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *X = NULL;

    DBUG_ENTER ("SCSprf_or_SxS");
    if (MatchConstantOne (PRF_ARG2 (arg_node))) { /* X | 1 */
        res = MakeTrue (PRF_ARG1 (arg_node));

    } else if (MatchConstantOne (PRF_ARG1 (arg_node))) { /* 1 | X */
        res = MakeTrue (PRF_ARG2 (arg_node));

    } else if (MatchConstantZero (PRF_ARG2 (arg_node))) { /* X | 0 */
        res = DUPdoDupTree (PRF_ARG1 (arg_node));

    } else if (MatchConstantZero (PRF_ARG1 (arg_node))) { /* 0 | X */
        res = DUPdoDupTree (PRF_ARG2 (arg_node));

    } else if (MATCH_PRF_ARG1_PRF_ARG2 (arg_node, X)) { /* S | S */
        res = DUPdoDupTree (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_or_SxV( node *arg_node, info *arg_info)
 * 0 | X  -> X
 * 1 | X  -> 1 of shape(X)
 *
 *****************************************************************************/
node *
SCSprf_or_SxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_or_SxV");
    if (MatchConstantOne (PRF_ARG1 (arg_node))) { /* 1 | X */
        res = MakeTrue (PRF_ARG2 (arg_node));
    } else if (MatchConstantZero (PRF_ARG1 (arg_node))) { /* 0 | X */
        res = DUPdoDupTree (PRF_ARG2 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_or_VxS( node *arg_node, info *arg_info)
 * X | 0  -> X
 * X | 1  -> 1 of shape(X)
 *
 *****************************************************************************/
node *
SCSprf_or_VxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_or_VxS");
    if (MatchConstantOne (PRF_ARG2 (arg_node))) { /* X | 1 */
        res = MakeTrue (PRF_ARG1 (arg_node));
    } else if (MatchConstantZero (PRF_ARG2 (arg_node))) { /* X | 0 */
        res = DUPdoDupTree (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_or_VxV( node *arg_node, info *arg_info)
 * X | X  -> X
 *
 *****************************************************************************/
node *
SCSprf_or_VxV (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *X = NULL;

    DBUG_ENTER ("SCSprf_or_VxV");
    if (MATCH_PRF_ARG1_PRF_ARG2 (arg_node, X)) { /*  X | X */
        res = DUPdoDupTree (PRF_ARG2 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_and_SxS( node *arg_node, info *arg_info)
 * X & 1  -> X
 * 1 & X  -> X
 * X & 0  -> 0 of shape(X)
 * 0 & X  -> 0 of shape(X)
 * X & X  -> X
 *
 *****************************************************************************/
node *
SCSprf_and_SxS (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *X = NULL;

    DBUG_ENTER ("SCSprf_and_SxS");
    if (MatchConstantOne (PRF_ARG2 (arg_node))) { /* X & 1 */
        res = DUPdoDupTree (PRF_ARG1 (arg_node));

    } else if (MatchConstantOne (PRF_ARG1 (arg_node))) { /* 1 & X */
        res = DUPdoDupTree (PRF_ARG2 (arg_node));

    } else if (MatchConstantZero (PRF_ARG2 (arg_node))) { /* X & 0 */
        res = MakeFalse (PRF_ARG1 (arg_node));

    } else if (MatchConstantZero (PRF_ARG1 (arg_node))) { /* 0 & X */
        res = MakeFalse (PRF_ARG2 (arg_node));

    } else if (MATCH_PRF_ARG1_PRF_ARG2 (arg_node, X)) { /* X & X */
        res = DUPdoDupTree (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_and_SxV( node *arg_node, info *arg_info)
 * 1 & X  -> X
 * 0 & X  -> 0 of shape(X)
 *
 *****************************************************************************/
node *
SCSprf_and_SxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_and_SxV");
    if (MatchConstantOne (PRF_ARG1 (arg_node))) { /* 1 & X */
        res = DUPdoDupTree (PRF_ARG2 (arg_node));
    } else if (MatchConstantZero (PRF_ARG1 (arg_node))) { /* 0 & X */
        res = MakeFalse (PRF_ARG2 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_and_VxS( node *arg_node, info *arg_info)
 * X & 1  -> X
 * X & 0  -> 0 of shape(X)
 *
 *****************************************************************************/
node *
SCSprf_and_VxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_and_VxS");
    if (MatchConstantOne (PRF_ARG2 (arg_node))) { /* X & 1 */
        res = DUPdoDupTree (PRF_ARG1 (arg_node));
    } else if (MatchConstantZero (PRF_ARG2 (arg_node))) { /* X & 0 */
        res = MakeFalse (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_and_VxV( node *arg_node, info *arg_info)
 * X & X  -> X
 *
 *****************************************************************************/
node *
SCSprf_and_VxV (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *X = NULL;

    DBUG_ENTER ("SCSprf_and_VxV");
    if (MATCH_PRF_ARG1_PRF_ARG2 (arg_node, X)) { /*  X & X */
        res = DUPdoDupTree (PRF_ARG2 (arg_node));
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
        CTIabortLine (NODE_LINE (PRF_ARG2 (arg_node)), "mod(X,0) encountered.");
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_tob_S( node *arg_node, info *arg_info)
 * Delete type coercion tob(boolvec) if arg_node is already of type bool.
 *
 *****************************************************************************/
node *
SCSprf_tob_S (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_tob_S");
    if (T_bool == TYgetSimpleType (TYgetScalar (ID_NTYPE (PRF_ARG1 (arg_node))))) {
        res = DUPdoDupTree (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_toc_S( node *arg_node, info *arg_info)
 * Delete type coercion toc(charvec) if arg_node is already of type char.
 *
 *****************************************************************************/
node *
SCSprf_toc_S (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_toc_S");
    if (T_char == TYgetSimpleType (TYgetScalar (ID_NTYPE (PRF_ARG1 (arg_node))))) {
        res = DUPdoDupTree (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_toi_S( node *arg_node, info *arg_info)
 * Delete type coercion toi(intvec) if arg_node is already of type int.
 *
 *****************************************************************************/
node *
SCSprf_toi_S (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_toi_S");
    if (T_int == TYgetSimpleType (TYgetScalar (ID_NTYPE (PRF_ARG1 (arg_node))))) {
        res = DUPdoDupTree (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_tof_S( node *arg_node, info *arg_info)
 * Delete type coercion tof(intvec) if arg_node is already of type T_float.
 *
 *****************************************************************************/
node *
SCSprf_tof_S (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_tof_S");
    if (T_float == TYgetSimpleType (TYgetScalar (ID_NTYPE (PRF_ARG1 (arg_node))))) {
        res = DUPdoDupTree (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_tod_S( node *arg_node, info *arg_info)
 * Delete type coercion tod(intvec) if arg_node is already of type double.
 *
 *****************************************************************************/
node *
SCSprf_tod_S (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_tod_S");
    if (T_double == TYgetSimpleType (TYgetScalar (ID_NTYPE (PRF_ARG1 (arg_node))))) {
        res = DUPdoDupTree (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_minmax( node *arg_node, info *arg_info)
 * If (X == Y), then X.
 * This implements SCSprf_min_SxS, SCSprf_min_VxV
 * This implements SCSprf_max_SxS, SCSprf_max_VxV

 *****************************************************************************/
node *
SCSprf_minmax (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *X = NULL;

    DBUG_ENTER ("SCSprf_minmax");
    if (MATCH_PRF_ARG1_PRF_ARG2 (arg_node, X)) {
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
 * @fn node *SCSprf_eq_SxV( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_eq_SxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_eq_SxV");
    /* Could be supported with ISMOP */
    /* I.e., attempt to convert V to constant, then check to see if all(S == ConstantV) */
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
 * @fn node *SCSprf_nlege( node *arg_node, info *arg_info)
 * Replace X != X by FALSE.
 * Replace X <  X by FALSE.
 * Replace X >  X by FALSE.
 * Implements SCSprf_neq_SxS, SCSprf_neq_VxV, SCSprf_lt_SxS, SCSprf_lt_VxV,
 *            SCSprf_gt_SxS, SCSprf_gt_VxV
 *
 *****************************************************************************/
node *
SCSprf_nlege (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *X = NULL;

    DBUG_ENTER ("SCSprf_nlege");
    if (MATCH_PRF_ARG1_PRF_ARG2 (arg_node, X)) {
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
 * @fn node *SCSprf_lege( node *arg_node, info *arg_info)
 * If (X == Y), then TRUE.
 * Implements SCSprf_ge_SxS, SCSprf_ge_VxV, SCSprf_le_SxS, SCSprf_ge_VxV,
 *            SCSprf_eq_SxS, SCSprf_eq_VxV
 *
 *****************************************************************************/
node *
SCSprf_lege (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *X = NULL;

    DBUG_ENTER ("SCSprf_lege");
    if (MATCH_PRF_ARG1_PRF_ARG2 (arg_node, X)) {
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