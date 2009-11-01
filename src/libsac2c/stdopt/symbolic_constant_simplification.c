
/* $id$ */

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
 *    _idx_sel(iv, _shape_A_(X)) ->  _idx_shape_sel(iv, X)
 *
 *    _idx_shape_sel(iv, WLmodarrayresult) ->   idx_shape_sel(iv,
 *MODARRAY_ARRAY(WITH_OP(WLmodarray)))
 *     FIXME: We should be able to do the same sort of thing with WLgenarray, when the
 *cell shape is scalar.
 *
 *  Conformability checking CF:
 *
 *    In _afterguard(x, p1, p2, p3...), remove any predicates that are true.
 *    If all predicates are removed, replace
 *     _afterguard(x) by x
 *
 *    In:  iv', p1 = _non_neg_val_V_( iv), replace by
 *         iv', p1 = iv, TRUE;
 *         if we can know that all elements of iv are >= 0.
 *
 *  For the redesign of AS/AL/DL, the following optimizations
 *  are introduced:
 *
 *     neg( neg( x)) => x
 *     neg( x+y) => neg(x) + neg(y)
 *
 *     rec( rec( x)) => x
 *     rec( x*y) => rec(x) * rec( y)
 *
 *     neg(x) + x  => zero
 *     x + neg(x)  => zero
 *     rec(x) * x  => one
 *     x * rec(x)  => one
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
#include "traverse.h"
#include "new_types.h"
#include "type_utils.h"
#include "new_typecheck.h"
#include "free.h"
#include "globals.h"
#include "DupTree.h"
#include "constants.h"
#include "shape.h"
#include "compare_tree.h"
#include "ctinfo.h"
#include "pattern_match.h"
#include "constant_folding_info.h"
#include "phase.h"
#include "algebraic_wlfi.h"

/*******************************************************************************
 *  some static initialisation needed for pattern sharing across all functions
 */
static bool initialised = FALSE;
static node *node_ptr;
static pattern *prf_id_args_pat;

void
SCSinitSymbolicConstantSimplification ()
{
    if (!initialised) {
        prf_id_args_pat = PMprf (0, 2, PMvar (1, PMAgetNode (&node_ptr), 0),
                                 PMvar (1, PMAisVar (&node_ptr), 0));
        initialised = TRUE;
    }
}

void
SCSfinalizeSymbolicConstantSimplification ()
{
    if (initialised) {
        prf_id_args_pat = PMfree (prf_id_args_pat);
        initialised = FALSE;
    }
}

/* local used helper functions */

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
static node *
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
static node *
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
static node *
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
static bool
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
static bool
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
 * @fn bool MatchNegV( node *arg1, node *arg2)
 *
 * Predicate for:
 *
 *     arg1 +  _neg_V_( arg1)
 *
 * Arguments:  PRF_ARG1 and PRF_ARG2 for +
 *
 *****************************************************************************/
static bool
MatchNegV (node *arg1, node *arg2)
{
    bool res;
    pattern *pat1;
    pattern *pat2;
    node *arg1p = NULL;

    DBUG_ENTER ("MatchNegV");

    pat1 = PMvar (1, PMAgetNode (&arg1p), 0);

    pat2 = PMprf (1, PMAisPrf (F_neg_V), 1, PMvar (1, PMAisVar (&arg1p), 0));

    res = PMmatchFlatSkipExtrema (pat1, arg1) && PMmatchFlatSkipExtrema (pat2, arg2);

    PMfree (pat1);
    PMfree (pat2);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn bool MatchNegS( node *arg1, node *arg2)
 *
 * Predicate for:
 *
 *     arg1 +  _neg_S_( arg1)
 *
 * Arguments:  PRF_ARG1 and PRF_ARG2 for +
 *
 *****************************************************************************/
static bool
MatchNegS (node *arg1, node *arg2)
{
    bool res;
    pattern *pat1;
    pattern *pat2;
    node *arg1p = NULL;

    DBUG_ENTER ("MatchNegS");

    pat1 = PMvar (1, PMAgetNode (&arg1p), 0);

    pat2 = PMprf (1, PMAisPrf (F_neg_S), 1, PMvar (1, PMAisVar (&arg1p), 0));

    res = PMmatchFlatSkipExtrema (pat1, arg1) && PMmatchFlatSkipExtrema (pat2, arg2);
    PMfree (pat1);
    PMfree (pat2);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn static node *StripTrues( node *args)
 * Takes N_exprs chain and returns same with TRUE predicates removed.
 *****************************************************************************/

static node *
StripTrues (node *args)
{
    ntype *predtyp;

    if (args != NULL) {
        DBUG_ASSERT (N_exprs == NODE_TYPE (args), "StripTrues expected exprs chain");
        EXPRS_NEXT (args) = StripTrues (EXPRS_NEXT (args));
        /* Delete predicate if true */
        predtyp = ID_NTYPE (EXPRS_EXPR (args));
        if (TYisAKV (predtyp) && (COisTrue (TYgetValue (predtyp), TRUE))) {
            args = FREEdoFreeNode (args); /* Pops one off the chain */
        }
    }
    return (args);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_add_SxS( node *arg_node, info *arg_info)
 *   Case 1:  X + 0            --> X
 *   Case 2:  0 + X            --> X
 *   Case 3:  _neg_S_( X) + X  --> 0
 *   Case 4:  X + _neg_S_( X)  --> 0
 *
 *
 *****************************************************************************/
node *
SCSprf_add_SxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_add_SxS");
    if (MatchConstantZero (PRF_ARG1 (arg_node))) {
        /*  Case 2: 0 + X */
        res = DUPdoDupTree (PRF_ARG2 (arg_node));
    } else if (MatchConstantZero (PRF_ARG2 (arg_node))) {
        /*  Case 1: X + 0 */
        res = DUPdoDupTree (PRF_ARG1 (arg_node));
    } else if (MatchNegS (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node))
               || MatchNegS (PRF_ARG2 (arg_node), PRF_ARG1 (arg_node))) {
        /* Case 4:   X + _neg_S_( X)  */
        /* Case 3:	 _neg_S_( X) + X  */
        res = MakeZero (PRF_ARG1 (arg_node));
        DBUG_PRINT ("CF", ("SCSprf_add_SxS generated zero vector"));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_add_SxV( node *arg_node, info *arg_info)
 *   Case 1:   0 + X -> X
 *
 *   Case 2:   X + [0,0,0] --> [X,X,X]  -- not supported yet
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
 * @fn node *SCSprf_add_VxV( node *arg_node, info *arg_info)
 *   Case 1:  _neg_V_( X) + X  --> 0
 *   Case 2:  X + _neg_V_( X)  --> 0
 *
 *****************************************************************************/
node *
SCSprf_add_VxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_add_VxV");

    if (MatchNegV (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node))
        || MatchNegV (PRF_ARG2 (arg_node), PRF_ARG1 (arg_node))) {
        /* Case 1:	 _neg_V_( X) + X  */
        /* Case 2:   X + _neg_V_( X)  */
        res = MakeZero (PRF_ARG1 (arg_node));
        DBUG_PRINT ("CF", ("SCSprf_add_VxV generated zero vector"));
    } else {
        if (MatchConstantZero (PRF_ARG2 (arg_node))) { /* X + 0 */
            res = DUPdoDupTree (PRF_ARG1 (arg_node));
        } else {
            if (MatchConstantZero (PRF_ARG1 (arg_node))) { /* 0 + X */
                res = DUPdoDupTree (PRF_ARG2 (arg_node));
            }
        }
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_add_VxS( node *arg_node, info *arg_info)
 *   Case 1:  X + 0 -> X
 *
 *   Case 2: [0,0,0] + X  --> [X,X,X]  -- not supported yet
 *
 *****************************************************************************/
node *
SCSprf_add_VxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_add_VxS");

    if (MatchConstantZero (PRF_ARG2 (arg_node))) { /* X + 0 */
        res = DUPdoDupTree (PRF_ARG1 (arg_node));
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_sub( node *arg_node, info *arg_info)
 * X - 0 -> X
 * SCSprf_sub_SxS, SCSprf_sub_SxV
 *
 *****************************************************************************/
node *
SCSprf_sub (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_sub");
    if (MatchConstantZero (PRF_ARG2 (arg_node))) { /* X - 0 */
        res = DUPdoDupTree (PRF_ARG1 (arg_node));
    } else if (PMmatchFlatSkipExtrema (prf_id_args_pat, arg_node)) { /* X - X */
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

    DBUG_ENTER ("SCSprf_sub_VxV");
    /* Can't do X - 0 unless we know argument shapes match */
    if (PMmatchFlatSkipExtrema (prf_id_args_pat, arg_node)) { /* X - X */
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
 * @fn node *SCSprf_mul_VxV( node *arg_node, info *arg_info)
 * X * 1 -> X
 * 1 * X -> X
 * X * 0 -> vector of zeros
 * 0 * X -> vector of zeros
 *****************************************************************************/
node *
SCSprf_mul_VxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_mul_VxV");

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

    DBUG_ENTER ("SCSprf_or_SxS");
    if (MatchConstantOne (PRF_ARG2 (arg_node))) { /* X | 1 */
        res = MakeTrue (PRF_ARG1 (arg_node));

    } else if (MatchConstantOne (PRF_ARG1 (arg_node))) { /* 1 | X */
        res = MakeTrue (PRF_ARG2 (arg_node));

    } else if (MatchConstantZero (PRF_ARG2 (arg_node))) { /* X | 0 */
        res = DUPdoDupTree (PRF_ARG1 (arg_node));

    } else if (MatchConstantZero (PRF_ARG1 (arg_node))) { /* 0 | X */
        res = DUPdoDupTree (PRF_ARG2 (arg_node));

        /* S | S */
    } else if (PMmatchFlatSkipExtrema (prf_id_args_pat, arg_node)) {
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

    DBUG_ENTER ("SCSprf_or_VxV");
    if (PMmatchFlatSkipExtrema (prf_id_args_pat, arg_node)) { /*  X | X */
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

    DBUG_ENTER ("SCSprf_and_SxS");
    if (MatchConstantOne (PRF_ARG2 (arg_node))) { /* X & 1 */
        res = DUPdoDupTree (PRF_ARG1 (arg_node));

    } else if (MatchConstantOne (PRF_ARG1 (arg_node))) { /* 1 & X */
        res = DUPdoDupTree (PRF_ARG2 (arg_node));

    } else if (MatchConstantZero (PRF_ARG2 (arg_node))) { /* X & 0 */
        res = MakeFalse (PRF_ARG1 (arg_node));

    } else if (MatchConstantZero (PRF_ARG1 (arg_node))) { /* 0 & X */
        res = MakeFalse (PRF_ARG2 (arg_node));

    } else if (PMmatchFlatSkipExtrema (prf_id_args_pat, arg_node)) { /* X & X */
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

    DBUG_ENTER ("SCSprf_and_VxV");
    if (PMmatchFlatSkipExtrema (prf_id_args_pat, arg_node)) { /*  X & X */
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
    if ((N_bool == NODE_TYPE (PRF_ARG1 (arg_node)))
        || ((N_id == NODE_TYPE (PRF_ARG1 (arg_node)))
            && (T_bool
                == TYgetSimpleType (TYgetScalar (ID_NTYPE (PRF_ARG1 (arg_node))))))) {
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
    if ((N_char == NODE_TYPE (PRF_ARG1 (arg_node)))
        || ((N_id == NODE_TYPE (PRF_ARG1 (arg_node)))
            && (T_char
                == TYgetSimpleType (TYgetScalar (ID_NTYPE (PRF_ARG1 (arg_node))))))) {
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
    if ((N_num == NODE_TYPE (PRF_ARG1 (arg_node)))
        || ((N_id == NODE_TYPE (PRF_ARG1 (arg_node)))
            && (T_int
                == TYgetSimpleType (TYgetScalar (ID_NTYPE (PRF_ARG1 (arg_node))))))) {
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
    if ((N_float == NODE_TYPE (PRF_ARG1 (arg_node)))
        || ((N_id == NODE_TYPE (PRF_ARG1 (arg_node)))
            && (T_float
                == TYgetSimpleType (TYgetScalar (ID_NTYPE (PRF_ARG1 (arg_node))))))) {
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
    if ((N_double == NODE_TYPE (PRF_ARG1 (arg_node)))
        || ((N_id == NODE_TYPE (PRF_ARG1 (arg_node)))
            && (T_double
                == TYgetSimpleType (TYgetScalar (ID_NTYPE (PRF_ARG1 (arg_node))))))) {
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

    DBUG_ENTER ("SCSprf_minmax");
    if (PMmatchFlatSkipExtrema (prf_id_args_pat, arg_node)) {
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

    DBUG_ENTER ("SCSprf_nlege");
    if (PMmatchFlatSkipExtrema (prf_id_args_pat, arg_node)) {
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

    DBUG_ENTER ("SCSprf_lege");
    if (PMmatchFlatSkipExtrema (prf_id_args_pat, arg_node)) {
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
 * @fn node *SCSprf_sel_VxA( node *arg_node, info *arg_info)
 *
 * @brief: Replace _sel_VxA_([scalarconstant], _shape_A_(arr)) by
 *                 _idx_shape_sel_SxA_(scalarconstant, arr)
 *
 *         NB. The above requires a scalar constant, not just a scalar,
 *             because idx_shape_sel avoids generating the
 *             shape vector by having fairly restricted rules on its domain.
 *
 *****************************************************************************/
node *
SCSprf_sel_VxA (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *arg2 = NULL;
    node *scalar = NULL;
    pattern *pat1;

    DBUG_ENTER ("SCSprf_sel_VxA");

    DBUG_ASSERT (N_id == NODE_TYPE (PRF_ARG1 (arg_node)),
                 "SCSprf_sel_VxA expected N_id as PRF_ARG1");
    pat1 = PMprf (1, PMAisPrf (F_sel_VxA), 2,
                  PMarray (0, 1, PMint (1, PMAgetNode (&scalar), 0)),
                  PMprf (1, PMAisPrf (F_shape_A), 1, PMvar (1, PMAgetNode (&arg2), 0)));

    if (PMmatchFlatSkipExtrema (pat1, arg_node)) {
        DBUG_PRINT ("SCS", ("Replacing idx_sel by idx_shape_sel"));
        res = TCmakePrf2 (F_idx_shape_sel, DUPdoDupNode (scalar), DUPdoDupNode (arg2));
    }
    pat1 = PMfree (pat1);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_reshape( node *arg_node, info *arg_info)
 *
 * @brief: Replace _reshape_VxA_( shp1, _reshape_VxA_( shp2, X))
 *         by
 *                 _reshape_VxA_( shp1, X);
 *
 *****************************************************************************/
node *
SCSprf_reshape (node *arg_node, info *arg_info)
{
    node *res = NULL;
    pattern *pat;
    node *shp1;
    node *shp2;
    node *X;

    DBUG_ENTER ("SCSprf_reshape");

    pat = PMprf (1, PMAisPrf (F_reshape_VxA), 2, PMvar (1, PMAgetNode (&shp1), 0),
                 PMprf (1, PMAisPrf (F_reshape_VxA), 2, PMvar (1, PMAgetNode (&shp2), 0),
                        PMvar (1, PMAgetNode (&X), 0)));
    if (PMmatchFlatSkipExtrema (pat, arg_node)) {
        DBUG_PRINT ("SCS", ("Replacing reshape of reshape"));

        res = DUPdoDupTree (arg_node);
        shp2 = FREEdoFreeNode (PRF_ARG2 (res));
        PRF_ARG2 (res) = DUPdoDupNode (X);
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 *  Functions for removing array operation conformability checks
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_guard( node *arg_node, info *arg_info)
 *
 * FIXME: I think TC should handle all of these cases.
 *****************************************************************************/
node *
SCSprf_guard (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_guard");
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_afterguard( node *arg_node, info *arg_info)
 *
 * In _afterguard(x, p1, p2, p3...), remove any predicates that are true.
 * If all predicates are removed, replace
 *     _afterguard(x) by x
 *
 *****************************************************************************/
node *
SCSprf_afterguard (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *arg2up = NULL;

    DBUG_ENTER ("SCSprf_afterguard");
    res = DUPdoDupTree (arg_node); /* Copy N_prf node and operate on the copy */
    arg2up = EXPRS_NEXT (PRF_ARGS (res));
    DBUG_ASSERT (NULL != arg2up, "Some joker caught us off guard with no guard");
    arg2up = StripTrues (arg2up);
    EXPRS_NEXT (PRF_ARGS (res)) = arg2up;

    /* If no predicates remain, the afterguard becomes an identity */
    if (NULL == arg2up) {
        res = FREEdoFreeTree (res);
        res = DUPdoDupTree (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_same_shape_AxA( node *arg_node, info *arg_info)
 *
 * If arguments B and C are same shape, replace:
 *   b', c', pred = prf_same_shape_AxA_(B,C)
 * by:
 *   b', c', pred = B,C,TRUE;
 *
 * CFassign will turn this into:
 *   b' = b;
 *   c' = c;
 *   pred = TRUE;
 *
 * There are 3 ways to detect matching shapes:
 *  1. Both arrays are AKS.
 *  2. B and C are the same array.
 *  3. SAA tells us so. But SAA is broken today.
 *
 *
 *****************************************************************************/
/*
FIXME: saa
*/

node *
SCSprf_same_shape_AxA (node *arg_node, info *arg_info)
{
    node *res = NULL;
    ntype *arg1type;
    ntype *arg2type;

    DBUG_ENTER ("SCSprf_same_shape_AxA");
    arg1type = ID_NTYPE (PRF_ARG1 (arg_node));
    arg2type = ID_NTYPE (PRF_ARG2 (arg_node));
    if (PMmatchFlatSkipExtrema (prf_id_args_pat, arg_node) /* same_shape(X, X) */
        || (TUshapeKnown (arg1type) && TUshapeKnown (arg2type)
            && TUeqShapes (arg1type, arg2type))) { /* AKS & shapes match */
        res = TBmakeExprs (DUPdoDupTree (PRF_ARG1 (arg_node)),
                           TBmakeExprs (DUPdoDupTree (PRF_ARG2 (arg_node)),
                                        TBmakeExprs (TBmakeBool (TRUE), NULL)));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_shape_matches_dim_VxA( node *arg_node, info *arg_info)
 *
 * description:
 *  This primitive is check that shape(iv)== dim(arr) in arr[iv].
 *  If so, this code replaces:
 *
 *   iv', pred = _shape_matches_dim_VxA_( iv, arr)
 *  by
 *   iv', pred = iv, TRUE;
 *  CFassign will turn this into:
 *   iv' = iv;
 *   pred = TRUE;
 *
 *
 *****************************************************************************/
node *
SCSprf_shape_matches_dim_VxA (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *iv = NULL;
    node *arr = NULL;
    ntype *ivtype;
    ntype *arrtype;
    pattern *pat;

    DBUG_ENTER ("SCSprf_shape_matches_dim_VxA");

    pat = PMprf (1, PMAisPrf (F_shape_matches_dim_VxA), 2, PMvar (1, PMAgetNode (&iv), 0),
                 PMvar (1, PMAgetNode (&arr), 0));

    if (PMmatchFlatSkipExtrema (pat, arg_node)) {
        ivtype = ID_NTYPE (iv);
        arrtype = ID_NTYPE (arr);
        if (TUshapeKnown (ivtype) && TUdimKnown (arrtype)
            && SHgetExtent (TYgetShape (ivtype), 0) == TYgetDim (arrtype)) {
            res = TBmakeExprs (DUPdoDupTree (iv), TBmakeExprs (TBmakeBool (TRUE), NULL));
            DBUG_PRINT ("SCS", ("SCSprf_shape_matches_dim_VxA removed guard( %s, %s)",
                                AVIS_NAME (ID_AVIS (iv)), AVIS_NAME (ID_AVIS (arr))));
        }
    }
    pat = PMfree (pat);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_non_neg_val_V( node *arg_node, info *arg_info)
 *
 * description:
 *  This primitive is a check that iv is all non-negative.
 *  If so, this code replaces:
 *
 *      iv', pred = prf_non_neg_val_V( iv)
 *
 *  by
 *
 *      p = TRUE;
 *      iv', pred = iv, p;
 *
 *  CFassign will turn this into:
 *
 *      iv' = iv;
 *      pred = p;
 *
 *****************************************************************************/
node *
SCSprf_non_neg_val_V (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *tmpivavis;
    constant *arg1c = NULL;
    constant *scalarp;
    pattern *pat;

    DBUG_ENTER ("SCSprf_non_neg_val_V");

    pat = PMprf (1, PMAisPrf (F_non_neg_val_V), 1, PMconst (1, PMAgetVal (&arg1c)));

    if (PMmatchFlatSkipExtrema (pat, arg_node) && COisNonNeg (arg1c, TRUE)) {
        DBUG_PRINT ("CF", ("SCSprf_non_neg_val removed guard"));

        /* Generate   p = TRUE; */
        scalarp = COmakeTrue (SHmakeShape (0));
        tmpivavis
          = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node)))),
                        TYmakeAKV (TYmakeSimpleType (T_bool), scalarp));

        if (isSAAMode ()) {
            AVIS_DIM (tmpivavis) = TBmakeNum (0);
            AVIS_SHAPE (tmpivavis) = TCmakeIntVector (TBmakeExprs (TBmakeNum (0), NULL));
        }
        INFO_VARDECS (arg_info) = TBmakeVardec (tmpivavis, INFO_VARDECS (arg_info));
        INFO_PREASSIGN (arg_info) = TBmakeAssign (TBmakeLet (TBmakeIds (tmpivavis, NULL),
                                                             COconstant2AST (scalarp)),
                                                  INFO_PREASSIGN (arg_info));

        AVIS_SSAASSIGN (tmpivavis) = INFO_PREASSIGN (arg_info);
        DBUG_PRINT ("CF",
                    ("SCSprf_non_neg_val_V(%s) created TRUE pred %s",
                     AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))), AVIS_NAME (tmpivavis)));
        res = TBmakeExprs (DUPdoDupTree (PRF_ARG1 (arg_node)),
                           TBmakeExprs (TBmakeId (tmpivavis), NULL));
        arg1c = (NULL != arg1c) ? COfreeConstant (arg1c) : arg1c;
    }
    pat = PMfree (pat);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_val_lt_shape_VxA( node *arg_node, info *arg_info)
 *
 * description:
 *  This primitive is check that iv is less than the shape of arr in arr[iv].
 *  If so, this code replaces:
 *
 *   iv', pred = prf_val_lt_shape_VxA( iv, arr)
 *
 *  by
 *
 *   iv', pred = iv, TRUE;
 *
 *  CFassign will turn this into:
 *
 *   iv' = iv;
 *   pred = TRUE;
 *
 * Note: We do not check that: shape(vi) == dim( arr).
 *       This should have been done by an earlier guard.
 *
 *****************************************************************************/
node *
SCSprf_val_lt_shape_VxA (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *iv;
    constant *ivc = NULL;
    constant *arrc = NULL;
    node *arr = NULL;
    ntype *ivtype;
    ntype *arrtype;
    shape *arrshp;
    pattern *pat;

    DBUG_ENTER ("SCSprf_val_lt_shape_VxA");

    pat = PMprf (1, PMAisPrf (F_val_lt_shape_VxA), 2, PMconst (1, PMAgetVal (&ivc)),
                 PMvar (1, PMAgetNode (&arr), 0));

    if (PMmatchFlatSkipExtrema (pat, arg_node)) {
        iv = PRF_ARG1 (arg_node);
        ivtype = ID_NTYPE (iv);
        arrtype = ID_NTYPE (arr);
        if (TUdimKnown (arrtype)) {
            arrshp = TYgetShape (arrtype);
            arrc = COmakeConstantFromShape (arrshp);
            if ((COgetExtent (ivc, 0) == COgetExtent (arrc, 0)) && COlt (ivc, arrc)) {
                res = TBmakeExprs (DUPdoDupTree (iv),
                                   TBmakeExprs (TBmakeBool (TRUE), NULL));
                DBUG_PRINT ("SCS", ("SCSprf_val_lt_shape_VxA removed guard( %s, %s)",
                                    AVIS_NAME (ID_AVIS (iv)), AVIS_NAME (ID_AVIS (arr))));
            }
        }
    }
    pat = PMfree (pat);
    arrc = (NULL != arrc) ? COfreeConstant (arrc) : arrc;

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_val_le_val_VxV( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCSprf_val_le_val_VxV (node *arg_node, info *arg_info)
{
    node *res = NULL;
    constant *con1 = NULL;
    constant *con2 = NULL;
    pattern *pat;

    DBUG_ENTER ("SCSprf_val_le_val_VxV");

    pat = PMprf (1, PMAisPrf (F_val_le_val_VxV), 2, PMconst (1, PMAgetVal (&con1)),
                 PMconst (1, PMAgetVal (&con2), 0));

    if (PMmatchFlatSkipExtrema (pat, arg_node)
        && (COgetExtent (con1, 0) == COgetExtent (con2, 0)) && COle (con1, con2)) {
        res = TBmakeExprs (DUPdoDupTree (PRF_ARG1 (arg_node)),
                           TBmakeExprs (TBmakeBool (TRUE), NULL));
        DBUG_PRINT ("SCS", ("SCSprf_val_le_val_VxV removed guard( %s, %s)",
                            AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                            AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node)))));
    }
    pat = PMfree (pat);
    con1 = (NULL != con1) ? COfreeConstant (con1) : con1;
    con2 = (NULL != con2) ? COfreeConstant (con2) : con2;

    DBUG_RETURN (res);
}
/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_prod_matches_prod_shape_VxA( node *arg_node, info *arg_info)
 *
 * description:
 *  This primitive checks that prod(shp) == prod(shape(C)) in:
 *    shp', pred = prod_matches_prod_shape_VxA_(SHP, C);
 *  If so, this code replaces it by:
 *   shp', pred = SHP, TRUE;
 *  CFassign will turn this into:
 *   shp' = shp;
 *   pred = TRUE;
 *
 *****************************************************************************/
node *
SCSprf_prod_matches_prod_shape_VxA (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_prod_matches_prod_shape_VxA");
    DBUG_RETURN (res);
}

/**<!--*************************************************************-->
 *
 * @fn node *SCSprf_shape(node *arg_node, info *arg_info)
 *
 * @brief: performs symbolic constant-folding on shape primitive
 *
 * @param arg_node, arg_info
 *
 * @result new arg_node if shape() operation could be replaced
 *         by a set of shape_sel operations, else NULL
 *
 ********************************************************************/

node *
SCSprf_shape (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SCSprf_shape");

#ifdef BUG524

    /* This code should never have gone live. It absolutely trashes
     * the performance of MANY benchmarks, by about a factor of 3-4X.
     *
     */

    node *avis;
    node *resid = NULL;
    node *resavis;
    node *resassign;
    int arg1dim;
    int i;

    DBUG_ASSERT (N_id == NODE_TYPE (PRF_ARG1 (arg_node)),
                 "SCSprf_shape_ expected N_id node");
    /* If AKD, replace the shape() operation by a list of idx_shape_sel() ops */
    if (TUdimKnown (ID_NTYPE (PRF_ARG1 (arg_node)))) {
        arg1dim = TYgetDim (ID_NTYPE (PRF_ARG1 (arg_node)));
        i = arg1dim;

        for (i = arg1dim - 1; i >= 0; i--) {
            avis = TBmakeAvis (TRAVtmpVarName (ID_NAME (PRF_ARG1 (arg_node))),
                               TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));

            INFO_VARDECS (arg_info) = TBmakeVardec (avis, INFO_VARDECS (arg_info));

            INFO_PREASSIGN (arg_info)
              = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL),
                                         TCmakePrf2 (F_idx_shape_sel, TBmakeNum (i),
                                                     DUPdoDupNode (PRF_ARG1 (arg_node)))),
                              INFO_PREASSIGN (arg_info));
            AVIS_SSAASSIGN (avis) = INFO_PREASSIGN (arg_info);

            if (isSAAMode ()) {
                AVIS_DIM (avis) = TBmakeNum (0); /* each shape element is an int. */
                AVIS_SHAPE (avis) = TCmakeIntVector (NULL);
            }

            res = TBmakeExprs (TBmakeId (avis), res);
        }
        /* At this point, we have an N_exprs chain of idx_shape_sel ops.
         * Make this an N_array and give it a name.
         */
        res = TCmakeIntVector (res);
        resavis = TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeSimpleType (T_int),
                                                        SHcreateShape (1, arg1dim)));
        if (isSAAMode ()) {
            AVIS_DIM (resavis) = TBmakeNum (1);
            AVIS_SHAPE (resavis)
              = TCmakeIntVector (TBmakeExprs (TBmakeNum (arg1dim), NULL));
            ;
        }

        INFO_VARDECS (arg_info) = TBmakeVardec (resavis, INFO_VARDECS (arg_info));

        resid = TBmakeId (resavis);
        resassign = TBmakeAssign (TBmakeLet (TBmakeIds (resavis, NULL), res), NULL);
        INFO_PREASSIGN (arg_info) = TCappendAssign (INFO_PREASSIGN (arg_info), resassign);

        AVIS_SSAASSIGN (resavis) = resassign;
    }
#endif //  BUG524

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_idx_shape_sel( node *arg_node, info *arg_info)
 *
 * description: This optimization replaces an idx_shape_sel operation
 *              on an array, arr, by the same operation on its shape
 *              primogenitor. See tree/pattern_match.c for definition.
 *
 *****************************************************************************/
node *
SCSprf_idx_shape_sel (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *idprimo = NULL;

    DBUG_ENTER ("SCSprf_idx_shape_sel");

    idprimo = PMOshapePrimogenitor (PRF_ARG2 (arg_node));
    if ((ID_AVIS (idprimo) != ID_AVIS (PRF_ARG2 (arg_node)))) {
        res = DUPdoDupTree (arg_node);
        DBUG_PRINT ("CF", ("SCSprf_idx_shape_sel replacing %s by %s",
                           AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node))),
                           AVIS_NAME (ID_AVIS (idprimo))));
        PRF_ARG2 (res) = DUPdoDupTree (idprimo);
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_reciproc_S( node *arg_node, info *arg_info)
 *
 * description:
 *   Case 1: Replace _reciproc_S( _reciproc_S( SC)) by
 *                   SC
 *   Case 2: Replace _reciproc_S( S1 * S2)   by
 *                   _reciproc_S_( S1)  * _reciproc_S_( S2)
 *
 *****************************************************************************/
node *
SCSprf_reciproc_S (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *arg1 = NULL;
    node *arg1p = NULL;
    node *arg2p = NULL;
    node *p1;
    node *p2;
    pattern *pat1;
    pattern *pat2;
    pattern *pat3;

    DBUG_ENTER ("SCSprf_reciproc_S");

    pat1 = PMprf (1, PMAisPrf (F_reciproc_S), 1, PMvar (1, PMAgetNode (&arg1), 0));
    pat2 = PMprf (1, PMAisPrf (F_reciproc_S), 1, PMvar (1, PMAgetNode (&arg1p), 0));
    pat3 = PMprf (1, PMAisPrf (F_mul_SxS), 2, PMvar (1, PMAgetNode (&arg1p), 0),
                  PMvar (1, PMAgetNode (&arg2p), 0));

    if (PMmatchFlatSkipExtrema (pat1, arg_node)) {
        if (PMmatchFlatSkipExtrema (pat2, arg1)) {
            /* Case 1 */
            res = DUPdoDupTree (arg1p);
            DBUG_PRINT ("CF", ("reciproc_S Case 1 replacing %s by %s",
                               AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                               AVIS_NAME (ID_AVIS (arg1p))));
        } else {
            /* FIXME - needs run-time code */
            if (FALSE && PMmatchFlatSkipExtrema (pat3, arg1)) {
                /* Case 2 */
                p1 = AWLFIflattenExpression (TCmakePrf1 (F_reciproc_S,
                                                         DUPdoDupNode (arg1p)),
                                             &INFO_VARDECS (arg_info),
                                             &INFO_PREASSIGN (arg_info), ID_AVIS (arg1p));
                p2 = AWLFIflattenExpression (TCmakePrf1 (F_reciproc_S,
                                                         DUPdoDupNode (arg2p)),
                                             &INFO_VARDECS (arg_info),
                                             &INFO_PREASSIGN (arg_info), ID_AVIS (arg1p));
                res = TCmakePrf2 (F_mul_SxS, TBmakeId (p1), TBmakeId (p2));
                DBUG_PRINT ("CF", ("SCSprf_reciproc_S Case 2 replacing %s by %s",
                                   AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                                   AVIS_NAME (ID_AVIS (arg1p))));
            }
        }
    }
    PMfree (pat1);
    PMfree (pat2);
    PMfree (pat3);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_reciproc_V( node *arg_node, info *arg_info)
 *
 * description:
 *  Replace _reciproc_V( _reciproc_V( VEC)) by VEC
 *
 *****************************************************************************/
node *
SCSprf_reciproc_V (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *arg1 = NULL;
    node *arg1p = NULL;
    pattern *pat1;
    pattern *pat2;

    DBUG_ENTER ("SCSprf_reciproc_V");

    pat1 = PMprf (1, PMAisPrf (F_reciproc_V), 1, PMvar (1, PMAgetNode (&arg1), 0));
    pat2 = PMprf (1, PMAisPrf (F_reciproc_V), 1, PMvar (1, PMAgetNode (&arg1p), 0));

    if (PMmatchFlatSkipExtrema (pat1, arg_node) && PMmatchFlatSkipExtrema (pat2, arg1)) {

        res = DUPdoDupTree (arg1p);
        DBUG_PRINT ("CF", ("SCSprf_reciproc_V replacing %s by %s",
                           AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                           AVIS_NAME (ID_AVIS (arg1p))));
    }
    PMfree (pat1);
    PMfree (pat2);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_neg_S( node *arg_node, info *arg_info)
 *
 * description:
 *  Case 1: Replace _neg_S_( _neg_S( SC)) by SC
 *  Case 2: Replace _neg_S_( x + y)       by _neg_S_( x) + _neg_S_( y)
 *
 *****************************************************************************/
node *
SCSprf_neg_S (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *arg1 = NULL;
    node *arg1p = NULL;
    node *arg2p = NULL;
    node *p1;
    node *p2;
    pattern *pat1;
    pattern *pat2;
    pattern *pat3;

    DBUG_ENTER ("SCSprf_neg_S");

    pat1 = PMprf (1, PMAisPrf (F_neg_S), 1, PMvar (1, PMAgetNode (&arg1), 0));
    pat2 = PMprf (1, PMAisPrf (F_neg_S), 1, PMvar (1, PMAgetNode (&arg1), 0));
    pat3 = PMprf (1, PMAisPrf (F_add_SxS), 2, PMvar (1, PMAgetNode (&arg1p), 0),
                  PMvar (1, PMAgetNode (&arg2p), 0));

    if (PMmatchFlatSkipExtrema (pat1, arg_node)) {
        if (PMmatchFlatSkipExtrema (pat2, arg1)) {
            /* Case 1 */
            res = DUPdoDupTree (arg1);
            DBUG_PRINT ("CF", ("SCSprf_neg_S Case 1 replacing %s by %s",
                               AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                               AVIS_NAME (ID_AVIS (arg1))));
        } else {
            if (PMmatchFlatSkipExtrema (pat3, arg1)) {
                /* Case 2 */
                p1 = AWLFIflattenExpression (TCmakePrf1 (F_neg_S, DUPdoDupNode (arg1p)),
                                             &INFO_VARDECS (arg_info),
                                             &INFO_PREASSIGN (arg_info), ID_AVIS (arg1p));
                p2 = AWLFIflattenExpression (TCmakePrf1 (F_neg_S, DUPdoDupNode (arg2p)),
                                             &INFO_VARDECS (arg_info),
                                             &INFO_PREASSIGN (arg_info), ID_AVIS (arg1p));
                res = TCmakePrf2 (F_add_SxS, TBmakeId (p1), TBmakeId (p2));
                DBUG_PRINT ("CF", ("SCSprf_neg_S Case 2 replacing %s by %s",
                                   AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                                   AVIS_NAME (ID_AVIS (arg1p))));
            }
        }
    }
    PMfree (pat1);
    PMfree (pat2);
    PMfree (pat3);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_neg_V( node *arg_node, info *arg_info)
 *
 * description:
 *  Case 1: Replace _neg_V_( _neg_V( V1)) by V1
 *  Case 2: Replace _neg_V_( V1 + V2)     by _neg_V_( V1) + _neg_V_( V2)
 *
 *****************************************************************************/
node *
SCSprf_neg_V (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *arg1 = NULL;
    node *arg1p = NULL;
    node *arg2p = NULL;
    node *p1;
    node *p2;
    pattern *pat1;
    pattern *pat2;
    pattern *pat3;

    DBUG_ENTER ("SCSprf_neg_V");

    pat1 = PMprf (1, PMAisPrf (F_neg_V), 1, PMvar (1, PMAgetNode (&arg1), 0));
    pat2 = PMprf (1, PMAisPrf (F_neg_V), 1, PMvar (1, PMAgetNode (&arg1), 0));
    pat3 = PMprf (1, PMAisPrf (F_add_VxV), 2, PMvar (1, PMAgetNode (&arg1p), 0),
                  PMvar (1, PMAgetNode (&arg2p), 0));

    if (PMmatchFlatSkipExtrema (pat1, arg_node)) {
        if (PMmatchFlatSkipExtrema (pat2, arg1)) {
            /* Case 1*/

            res = DUPdoDupTree (arg1);
            DBUG_PRINT ("CF", ("SCSprf_neg_V case 1 replacing %s by %s",
                               AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                               AVIS_NAME (ID_AVIS (arg1))));
        } else {
            if (PMmatchFlatSkipExtrema (pat3, arg1)) {
                /* Case 2 */
                p1 = AWLFIflattenExpression (TCmakePrf1 (F_neg_V, DUPdoDupNode (arg1p)),
                                             &INFO_VARDECS (arg_info),
                                             &INFO_PREASSIGN (arg_info), ID_AVIS (arg1p));
                p2 = AWLFIflattenExpression (TCmakePrf1 (F_neg_V, DUPdoDupNode (arg2p)),
                                             &INFO_VARDECS (arg_info),
                                             &INFO_PREASSIGN (arg_info), ID_AVIS (arg1p));
                res = TCmakePrf2 (F_add_VxV, TBmakeId (p1), TBmakeId (p2));
                DBUG_PRINT ("CF", ("SCSprf_neg_V Case 2 replacing %s by %s",
                                   AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                                   AVIS_NAME (ID_AVIS (arg1p))));
            }
        }
    }

    PMfree (pat1);
    PMfree (pat2);
    PMfree (pat3);

    DBUG_RETURN (res);
}
