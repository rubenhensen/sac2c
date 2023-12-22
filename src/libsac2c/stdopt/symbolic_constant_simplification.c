/******************************************************************************
 *
 * @defgroup SCS Symbolic Constant Simplification
 *
 * This module contains the function table entries that implement symbolic
 * constant simplification.
 *
 * Specifically, the following replacements are made by functions here:
 *
 *   add (x+0          -> x,     0+x ->x),
 *   add (x+esdneg(x)  -> 0
 *   add (esdneg(x)+x  -> 0
 *   sub (x-0          -> x,     x-x ->0)
 *   mul (x*0          -> 0,     0*x ->0, x*0 -> 0, x*1 ->x, 1*x. ->x),
 *   div (x/0          -> error, x/1 ->x),
 *   and (x&1          -> x,     1&x ->x, x&1 -> x, x&0 ->0, 0&x. ->0),
 *   or  (x|1          -> 1,     1|x ->1, x|1 -> 1, x|0 ->x, 0|x. ->x)
 *
 *   eq  ((x == y)  -> TRUE  IFF (x == y))
 *   neq ((x != y)  -> FALSE IFF (x == y))
 *   ge  ((x >= x)  -> TRUE  IFF (x == x))
 *   le  ((x <= x)  -> TRUE  IFF (x == x))
 *   not (x)        -> TRUE  iFF (FALSE == x), FALSE IFF (TRUE == x)
 *   tob (boolean)  -> Boolean and other coercion identities
 *                     (int->int, char->char, float->float, double->double)
 *
 * Conformability checking CF:
 *
 * In: x1', .., xn' = guard (n, x1, .., xn, p1, .., pm) remove any predicates
 * that are true. If all predicates are removed, replace by just x1, .., xn.
 *
 * In: iv', p1 = _non_neg_val_V_( iv), replace by iv', p1 = iv, TRUE; if we can
 * know that all elements of iv are >= 0.
 *
 * Also, if we discover ANY predicates that are false, signal an error, and
 * abort compilation.
 *
 * For the redesign of AS/AL/DL, the following optimizations are introduced:
 *
 *   neg (neg(x)) => x
 *   neg (x + y)  => neg(x) + neg(y)
 *
 *   rec (rec(x)) => x
 *   rec (x * y)  => rec(x) * rec( y)
 *
 *   neg(x) + x  => zero
 *   x + neg(x)  => zero
 *   rec(x) * x  => one
 *   x * rec(x)  => one
 *
 * @todo Most of the optimizations could be extended to operate on VxV data,
 * when we can detect that the array shapes must match. Perhaps I should go try
 * to pull this stuff out of SAA-land.
 *
 * The functions in this module are called from the generic constant-folding
 * traversal in constant_folding.c, via function table prf_scs[].
 *
 ******************************************************************************/
#include "compare_tree.h"
#include "constants.h"
#include "constant_folding.h"
#include "constant_folding_info.h"
#include "ctinfo.h"
#include "DupTree.h"
#include "flattengenerators.h"
#include "free.h"
#include "globals.h"
#include "ivexpropagation.h"
#include "new_typecheck.h"
#include "new_types.h"
#include "pattern_match.h"
#include "phase.h"
#include "polyhedral_utilities.h"
#include "saa_constant_folding.h"
#include "shape.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "tree_utils.h"
#include "type_utils.h"

#define DBUG_PREFIX "SCS"
#include "debug.h"

#include "symbolic_constant_simplification.h"

/******************************************************************************
 *
 * function: Predicate for determining if an argument is
 *           a selection from a shape vector.
 *           Or, just a guarded shape vector.
 *
 * description: We look for (shape(v)[iv] in various guises.
 *
 * result: True if argument is known to be a selection from shape.
 *         If true, the argument is also known to be non-negative.
 *         Else false.
 *
 *****************************************************************************/
bool
SCSisSelOfShape (node *arg_node)
{
    pattern *pat1;
    pattern *pat2;
    pattern *pat3;
    node *iv = NULL;
    node *x = NULL;
    node *m = NULL;
    bool z;

    DBUG_ENTER ();

    /* z = _sel_VxA_( iv, x); */
    pat1 = PMprf (1, PMAisPrf (F_sel_VxA), 2, PMvar (1, PMAgetNode (&iv), 0),
                  PMvar (1, PMAgetNode (&x), 0));
    /* z = _idx_sel( offset, x); */
    pat2 = PMprf (1, PMAisPrf (F_idx_sel), 2, PMvar (1, PMAgetNode (&iv), 0),
                  PMvar (1, PMAgetNode (&x), 0));
    /* x = _shape_A_( m ); */
    pat3 = PMprf (1, PMAisPrf (F_shape_A), 1, PMvar (1, PMAgetNode (&m), 0));

    z = PMmatchFlatSkipGuards (pat3, arg_node); // Guarded shape vector
    z = (PMmatchFlatSkipGuards (pat1, arg_node) || PMmatchFlatSkipGuards (pat2, arg_node))
        && PMmatchFlatSkipGuards (pat3, x);

    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);
    pat3 = PMfree (pat3);

    DBUG_RETURN (z);
}

/******************************************************************************
 * bool SCSisNonNegative( node *arg_node)
 *
 * function: Predicate for determining if an argument is known to
 *           be non-negative.
 *
 * description: We check for a constant arg_node, and if that
 *              fails, look for a suitable constant AVIS_MIN.
 *
 * @brief: arg_node: An N_id or N_num node
 *
 * result: True if argument is known to be non-negative.
 *         Else false.
 *
 *         NB. False does NOT imply that the argument is negative.
 *
 *****************************************************************************/
bool
SCSisNonNegative (node *arg_node)
{
    pattern *pat;
    constant *con = NULL;
    bool z;

    DBUG_ENTER ();

    z = (N_num == NODE_TYPE (arg_node)) && (NUM_VAL (arg_node) >= 0);

    if ((!z) && N_id == NODE_TYPE (arg_node)) {
        pat = PMconst (1, PMAgetVal (&con));
        z = PMmatchFlatSkipExtrema (pat, arg_node) && COisNonNeg (con, TRUE);
        z = z || SCSisSelOfShape (arg_node);

        if (!z) {
            con = SAACFchaseMinMax (arg_node, SAACFCHASEMIN);
            z = (NULL != con) && COisNonNeg (con, TRUE);
        }

        con = (NULL != con) ? COfreeConstant (con) : con;
        pat = PMfree (pat);
    }

    DBUG_RETURN (z);
}

/******************************************************************************
 * bool SCSisNegative( node *arg_node)
 *
 * function: Predicate for determining if an argument is known to
 *           be negative.
 *
 * description: We check for a constant arg_node, and if that
 *              fails, look for a suitable constant AVIS_MIN.
 *
 * result: True if argument is known to be negative.
 *         Else false.
 *
 *         NB. False does NOT imply that the argument is non-negative.
 *
 *****************************************************************************/
bool
SCSisNegative (node *arg_node)
{
    pattern *pat;
    constant *con = NULL;
    bool z;

    DBUG_ENTER ();

    z = (N_num == NODE_TYPE (arg_node)) && (NUM_VAL (arg_node) < 0);

    if ((!z) && N_id == NODE_TYPE (arg_node)) {
        pat = PMconst (1, PMAgetVal (&con));
        z = PMmatchFlatSkipExtrema (pat, arg_node) && COisNeg (con, TRUE);
        if (!z) {
            // If maximum value is <= 0, then arg_node is negative.
            con = SAACFchaseMinMax (arg_node, SAACFCHASEMAX);
            z = (NULL != con) && (COisNeg (con, TRUE) || COisZero (con, TRUE));
        }

        con = (NULL != con) ? COfreeConstant (con) : con;
        pat = PMfree (pat);
    }

    DBUG_RETURN (z);
}

/******************************************************************************
 * bool SCSisNonPositive( node *arg_node)
 *
 * function: Predicate for determining if an argument is known to
 *           be non-positive.
 *
 * description: We check for a constant arg_node, and if that
 *              fails, look for a suitable constant AVIS_MIN.
 *
 * result: True if argument is known to be <= 0.
 *         Else false.
 *
 *         NB. False does NOT imply that the argument is positive.
 *
 *****************************************************************************/
bool
SCSisNonPositive (node *arg_node)
{
    pattern *pat;
    constant *con = NULL;
    bool z;

    DBUG_ENTER ();

    z = (N_num == NODE_TYPE (arg_node)) && (NUM_VAL (arg_node) <= 0);
    if ((!z) && N_id == NODE_TYPE (arg_node)) {
        pat = PMconst (1, PMAgetVal (&con));
        z = PMmatchFlatSkipExtrema (pat, arg_node) && COisNeg (con, TRUE);
        if (!z) {
            // If maximum value is <= 1, then arg_node is non-positive
            con = SAACFchaseMinMax (arg_node, SAACFCHASEMAX);
            z = (NULL != con)
                && (COisNeg (con, TRUE) || COisZero (con, TRUE) || COisOne (con, TRUE));
        }

        con = (NULL != con) ? COfreeConstant (con) : con;
        pat = PMfree (pat);
    }

    DBUG_RETURN (z);
}

/******************************************************************************
 * bool SCSisPositive( node *arg_node)
 *
 * function: Predicate for determining if an argument is known to
 *           be positive.
 *
 * description: We check for a constant arg_node, and if that
 *              fails, look for a suitable constant AVIS_MIN.
 *
 * result: True if argument is known to be non-negative.
 *         Else false.
 *
 *         NB. False does NOT imply that the argument is negative.
 *
 *****************************************************************************/
bool
SCSisPositive (node *arg_node)
{
    pattern *pat;
    constant *con = NULL;
    bool z;

    DBUG_ENTER ();

    z = (N_num == NODE_TYPE (arg_node)) && (NUM_VAL (arg_node) > 0);
    if ((!z) && N_id == NODE_TYPE (arg_node)) {
        pat = PMconst (1, PMAgetVal (&con));
        z = PMmatchFlatSkipExtrema (pat, arg_node) && COisPos (con, TRUE);
        if (!z) {
            // If minimum value is > 0, then arg_node is positive.
            con = SAACFchaseMinMax (arg_node, SAACFCHASEMIN);
            z = ((NULL != con) && ((!COisNeg (con, TRUE)) && (!COisZero (con, TRUE))));
        }

        con = (NULL != con) ? COfreeConstant (con) : con;
        pat = PMfree (pat);
    }

    DBUG_RETURN (z);
}

/******************************************************************************
 *
 * function: Function to apply current CF function on extrema
 *
 * description: arg1 and arg2 are PRF_ARG1/2 or AVIS_MIN/MAXof same.
 *
 * note: DOINGEXTREMA prevents endless recursion.
 *
 * result: fun() applied to arg1, arg2.
 *
 *****************************************************************************/
node *
SCSrecurseWithExtrema (node *arg_node, info *arg_info, node *arg1, node *arg2,
                       node *(*fun) (node *, info *))
{
    node *res = NULL;
    node *myarg_node;

    DBUG_ENTER ();

    if (!INFO_DOINGEXTREMA (arg_info)) {
        DBUG_PRINT ("Starting recursion for extrema");
        INFO_DOINGEXTREMA (arg_info) = TRUE;
        DBUG_ASSERT (N_id == NODE_TYPE (arg1), "Expected N_id arg1");
        DBUG_ASSERT (N_id == NODE_TYPE (arg2), "Expected N_id arg2");

        myarg_node = DUPdoDupNode (arg_node);
        FREEdoFreeNode (PRF_ARG1 (myarg_node));
        FREEdoFreeNode (PRF_ARG2 (myarg_node));
        PRF_ARG1 (myarg_node) = DUPdoDupNode (arg1);
        PRF_ARG2 (myarg_node) = DUPdoDupNode (arg2);
        res = (*fun) (myarg_node, arg_info);
        FREEdoFreeNode (myarg_node);
        INFO_DOINGEXTREMA (arg_info) = FALSE;
        DBUG_PRINT ("Ending recursion for extrema");
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   simpletype SCSgetBasetypeOfExpr(node *expr)
 *
 * description:
 *   try to get the basetype of the given expression. this can be a
 *   identifier or an array node. returns NULL if no type can be computed.
 *
 *****************************************************************************/
simpletype
SCSgetBasetypeOfExpr (node *expr)
{
    simpletype stype;
    ntype *etype;

    DBUG_ENTER ();
    DBUG_ASSERT (expr != NULL, "Called with NULL pointer");

    etype = NTCnewTypeCheck_Expr (expr);

    stype = TYgetSimpleType (TYgetScalar (etype));

    etype = TYfreeType (etype);

    DBUG_RETURN (stype);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSmakeZero( node *prfarg)
 * Create an array of zeros/FALSE of the same type and shape as prfarg.
 * If prfarg is not AKS, we give up.
 *
 *****************************************************************************/
node *
SCSmakeZero (node *prfarg)
{
    constant *con;
    shape *shp;
    ntype *typ;
    node *res = NULL;

    DBUG_ENTER ();

    typ = NTCnewTypeCheck_Expr (prfarg);
    if (TUshapeKnown (typ)) {
        shp = TYgetShape (typ);
        con = COmakeZero (SCSgetBasetypeOfExpr (prfarg), shp);
        if (NULL != con) {
            res = COconstant2AST (con);
            con = COfreeConstant (con);
        }
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSmakeFalse( node *prfarg)
 * Create an array of FALSE of the same shape as prfarg.
 * If prfarg is not AKS, we give up.
 *
 *****************************************************************************/
node *
SCSmakeFalse (node *prfarg)
{
    constant *con;
    shape *shp;
    ntype *typ;
    node *res = NULL;

    DBUG_ENTER ();
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
 * @fn node *SCSmakeVectorArray( shape *shp, node *scalarval)
 *
 * @brief: Create an N_array, of shape shp elements, containing
 *         scalarval as each item.
 *
 * @return the N_array thus created.
 *
 *****************************************************************************/
node *
SCSmakeVectorArray (shape *shp, node *scalarval)
{
    node *res = NULL;
    node *aelems = NULL;
    ntype *elemtype;
    shape *frameshape;
    int xrho;

    DBUG_ENTER ();

    elemtype
      = TYmakeAKS (TYcopyType (TYgetScalar (ID_NTYPE (scalarval))), SHcreateShape (0));
    frameshape = SHcopyShape (shp);
    xrho = SHgetExtent (shp, 0);

    while (xrho != 0) {
        aelems = TBmakeExprs (DUPdoDupNode (scalarval), aelems);
        xrho--;
    }

    res = TBmakeArray (elemtype, frameshape, aelems);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSmakeTrue( node *prfarg)
 * Create an array of TRUE of the same shape as prfarg.
 * If prfarg is not AKS, we give up.
 *
 *****************************************************************************/
node *
SCSmakeTrue (node *prfarg)
{
    constant *con;
    shape *shp;
    ntype *typ;
    node *res = NULL;

    DBUG_ENTER ();
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
 * @fn bool SCSisConstantZero( node *prfarg)
 * Predicate for PRF_ARG being a constant zero of any rank or type.
 * E.g., 0 or  [0,0] or  genarray([2,3], 0)
 *
 *****************************************************************************/
bool
SCSisConstantZero (node *arg_node)
{
    constant *argconst = NULL;
    pattern *pat;
    bool res = FALSE;

    DBUG_ENTER ();

    pat = PMconst (1, PMAgetVal (&argconst));
    if (PMmatchFlatSkipExtremaAndGuards (pat, arg_node)) {
        res = COisZero (argconst, TRUE);
        argconst = COfreeConstant (argconst);
    }
    pat = PMfree (pat);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn bool SCSisConstantNonZero( node *prfarg)
 * Predicate for PRF_ARG being a constant non-zero of any rank or type.
 * E.g., 0 or  [0,0] or  genarray([2,3], 0)
 *
 *****************************************************************************/
bool
SCSisConstantNonZero (node *arg_node)
{
    constant *argconst = NULL;
    pattern *pat;
    bool res = FALSE;

    DBUG_ENTER ();

    pat = PMconst (1, PMAgetVal (&argconst));
    if (PMmatchFlatSkipExtremaAndGuards (pat, arg_node)) {
        res = !COisZero (argconst, TRUE);
        argconst = COfreeConstant (argconst);
    }
    pat = PMfree (pat);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn bool SCSisConstantOne( node *prfarg)
 * Predicate for PRF_ARG being a constant one of any rank or type.
 * E.g., 1 or  [1,1] or  genarray([2,3], 1)
 *
 *****************************************************************************/
bool
SCSisConstantOne (node *prfarg)
{
    constant *argconst = NULL;
    pattern *pat;
    bool res = FALSE;

    DBUG_ENTER ();

    pat = PMconst (1, PMAgetVal (&argconst));
    if (PMmatchFlatSkipExtremaAndGuards (pat, prfarg)) {
        res = COisOne (argconst, TRUE);
        argconst = COfreeConstant (argconst);
    }
    pat = PMfree (pat);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn bool SCSisConstantTrue( node *prfarg)
 * Predicate for PRF_ARG being an array of constant true.
 * E.g., true or  [true,true] or  genarray([2,3], true)
 *
 *****************************************************************************/
bool
SCSisConstantTrue (node *arg_node)
{
    constant *argconst = NULL;
    pattern *pat;
    bool res = FALSE;

    DBUG_ENTER ();

    pat = PMconst (1, PMAgetVal (&argconst));
    if (PMmatchFlatSkipExtremaAndGuards (pat, arg_node)) {
        res = COisTrue (argconst, TRUE);
        argconst = COfreeConstant (argconst);
    }
    pat = PMfree (pat);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn bool SCSisConstantFlase( node *prfarg)
 * Predicate for PRF_ARG being an array of constant false.
 * E.g., false or  [false,false] or  genarray([2,3], false)
 *
 *****************************************************************************/
bool
SCSisConstantFalse (node *prfarg)
{
    constant *argconst = NULL;
    pattern *pat;
    bool res = FALSE;

    DBUG_ENTER ();

    pat = PMconst (1, PMAgetVal (&argconst));
    if (PMmatchFlatSkipExtremaAndGuards (pat, prfarg)) {
        res = COisFalse (argconst, TRUE);
        argconst = COfreeConstant (argconst);
    }
    pat = PMfree (pat);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn bool isGenwidth1Partition(node *arg, info *arg_info)
 *
 * @brief: This predicate is intended to determine if arg represents
 *         a WL partition with 1 == GENWIDTH.
 *
 *         If so, then arg is, effectively, loop-invariant, and
 *         can be treated, in some contexts, as a constant.
 *         Cf. comments in MatchGenwidth1Partition.
 *
 * @result: true if:
 *
 *       Case 0: GENERATOR_GENWIDTH == 1!
 *         This requires that we be able to find the N_part we are
 *         in, if it exists.
 *
 *       Case 1:
 *         ( AVIS_MIN( arg) == ( AVIS_MAX( arg) -  1)) or
 *         ( AVIS_MIN( arg) == ( AVIS_MAX( arg) + -1)).
 *
 *         Do NOT assume that a FALSE result means that the
 *         1 !+ GENWIDTH: it may just be that we do not have
 *         adequate information to make the decision.
 *
 *       Case 2:
 *         Same, but with constants.
 *
 *****************************************************************************/
static bool
isGenwidth1Partition (node *arg, info *arg_info)
{
    bool res = FALSE;
    pattern *patadd;
    pattern *patsub;
    pattern *patmin;
    pattern *patIsconsum;
    node *amax = NULL;
    constant *con = NULL;
    constant *cone = NULL;
    constant *consum = NULL;
#ifdef FIXME
    node *partn;
#endif // FIXME

    DBUG_ENTER ();

    DBUG_PRINT (" Checking %s for 1==GENWIDTH", AVIS_NAME (ID_AVIS (arg)));

#ifdef FIXME
    this is rubbish
      - we have no guarantee that we will be called from somewhere with the same arg_info
          block as CF !
      // Case 0
      partn
      = INFO_PART (arg_info);
    res
      = (NULL != partn) && SCSisConstantOne (GENERATOR_GENWIDTH (PART_GENERATOR (partn)));
#endif // FIXME

    // Case 1
    if ((!res) && (IVEXPisAvisHasMin (ID_AVIS (arg)))
        && (IVEXPisAvisHasMax (ID_AVIS (arg)))) {
        amax = AVIS_MAX (ID_AVIS (arg));

        patadd = PMprf (1, PMAisPrf (F_add_SxS), 2, PMvar (1, PMAisVar (&amax), 0),
                        PMconst (1, PMAgetVal (&con), 0));

        patsub = PMprf (1, PMAisPrf (F_sub_SxS), 2, PMvar (1, PMAisVar (&amax), 0),
                        PMconst (1, PMAgetVal (&con), 0));
        res = PMmatchFlat (patadd, AVIS_MIN (ID_AVIS (arg))) && (-1 == COconst2Int (con));
        con = (NULL != con) ? COfreeConstant (con) : NULL;
        res
          = res
            || (PMmatchFlat (patsub, AVIS_MIN (ID_AVIS (arg))) && 1 == COconst2Int (con));
        con = (NULL != con) ? COfreeConstant (con) : NULL;

        patadd = PMfree (patadd);
        patsub = PMfree (patsub);

        // Case 2
        if (!res) {
            patmin = PMconst (1, PMAgetVal (&con), 0);
            patIsconsum = PMconst (1, PMAisVal (&consum), 0);
            if (PMmatchFlat (patmin, AVIS_MIN (ID_AVIS (arg)))) {
                cone = COmakeConstantFromInt (1);
                consum = COadd (cone, con, NULL);
                res = PMmatchFlat (patIsconsum, AVIS_MAX (ID_AVIS (arg)));
                cone = COfreeConstant (cone);
                consum = COfreeConstant (consum);
            }
            con = (NULL != con) ? COfreeConstant (con) : NULL;
            patmin = PMfree (patmin);
            patIsconsum = PMfree (patIsconsum);
        }
    }

    DBUG_PRINT ("check of %s produced %s", AVIS_NAME (ID_AVIS (arg)),
                res ? "TRUE" : "FALSE");

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn bool MatchGenwidth1Partition(node *arg1, node *arg2, info *arg_info)
 *
 * @brief: This function is intended to help match variables that have
 *         extrema in WL partitions that have genwidth 1.
 *
 *         This came about as a result of studying the performance
 *         of the wlcondo.sac constant-folding unit test, using
 *         this SIMD APLish version of the code:
 *
 *          mask = (iota(N+1) == 0) | (iota(N+1) == N);
 *          a = tod( mask) * a + tod( !mask) * (shift([1], a) + shift([-1], a));
 *
 * @result: TRUE if  ( AVIS_MIN( arg1) == arg2) && arg1 has 1 == GENWIDTH.
 *
 *****************************************************************************/
static bool
isMatchGenwidth1Partition (node *arg1, node *arg2, info *arg_info)
{
    bool res = FALSE;
    pattern *pat;

    DBUG_ENTER ();

    if ((IVEXPisAvisHasMin (ID_AVIS (arg1)))) {
        pat = PMvar (1, PMAisVar (&arg2), 0);
        res = PMmatchFlat (pat, AVIS_MIN (ID_AVIS (arg1)));
        res = res && isGenwidth1Partition (arg1, arg_info);
        pat = PMfree (pat);
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn bool SCSisMatchPrfrgs( node *arg_node, arg_info)
 *
 * @brief Predicate for PRF_ARG1 matching PRF_ARG2
 *
 *        If this returns TRUE, then the nodes are guaranteed equal.
 *        If this returns FALSE, no guarantees are made. In particular,
 *        do NOT assume that FALSE means not-equal!
 *
 *****************************************************************************/
bool
SCSisMatchPrfargs (node *arg_node, info *arg_info)
{
    pattern *pat1;
    pattern *pat2;
    node *node_ptr = NULL;
    bool res;

    DBUG_ENTER ();

    pat1 = PMany (1, PMAgetNodeOrAvis (&node_ptr), 0);
    pat2 = PMany (1, PMAisNodeOrAvis (&node_ptr), 0);

    res = PMmatchFlatSkipExtremaAndGuards (pat1, PRF_ARG1 (arg_node))
          && PMmatchFlatSkipExtremaAndGuards (pat2, PRF_ARG2 (arg_node));

    res
      = res
        || isMatchGenwidth1Partition (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node), arg_info)
        || isMatchGenwidth1Partition (PRF_ARG2 (arg_node), PRF_ARG1 (arg_node), arg_info);

    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn bool isNotEqual( node *arg1, node *arg2, arg_info)
 *
 * @brief Predicate for arg1 not equal to arg2
 *
 *        If this returns TRUE, then the nodes are guaranteed not-equal.
 *        If this returns FALSE, no guarantees are made. In particular,
 *        do NOT assume that FALSE means equal!
 *
 * @notes Case 1: If maxval( arg1) == arg2, then x<y  --> Not equal
 *        Case 2: If constant( minval( arg1)) >  constant( arg2) --> Not equal
 *        Case 3: If constant( maxval( arg1)) <= constant( arg2) --> Not equal
 *
 *****************************************************************************/
static bool
isNotEqual (node *arg1, node *arg2, info *arg_info)
{
    pattern *pat1;
    pattern *pat2;
    pattern *pat1mincon;
    pattern *pat1maxcon;
    pattern *pat2con;
    node *node_ptr = NULL;
    constant *con1min = NULL;
    constant *con1max = NULL;
    constant *con2 = NULL;
    constant *conrel = NULL;
    node *avis1;
    bool res = FALSE;

    DBUG_ENTER ();

    pat1 = PMany (1, PMAgetNodeOrAvis (&node_ptr), 0);
    pat2 = PMany (1, PMAisNodeOrAvis (&node_ptr), 0);
    pat1mincon = PMconst (1, PMAgetVal (&con1min), 0);
    pat1maxcon = PMconst (1, PMAgetVal (&con1max), 0);
    pat2con = PMconst (1, PMAgetVal (&con2), 0);
    avis1 = ID_AVIS (arg1);

    // Case 1: If maxval( arg1) == arg2, then x<y  --> Not equal
    if (IVEXPisAvisHasMax (avis1)) { // Case 1
        res = PMmatchFlatSkipExtremaAndGuards (pat1, AVIS_MAX (avis1))
              && PMmatchFlatSkipExtremaAndGuards (pat2, arg2);
    }

    if (!res) {
        if (IVEXPisAvisHasMin (avis1)) {
            PMmatchFlatSkipExtremaAndGuards (pat1mincon, AVIS_MIN (avis1));
        }

        if (IVEXPisAvisHasMax (avis1)) {
            PMmatchFlatSkipExtremaAndGuards (pat1maxcon, AVIS_MAX (avis1));
        }
        PMmatchFlatSkipExtremaAndGuards (pat2con, arg2);
    }

    // Case 2: If constant( minval( arg1)) >  constant( arg2) --> Not equal
    if ((!res) && (NULL != con1min) && (NULL != con2)) {
        conrel = COgt (con1min, con2, NULL);
        res = COisTrue (conrel, TRUE);
    }

    // Case 3: If constant( maxval( arg1)) <= constant( arg2) --> Not equal
    if ((!res) && (NULL != con1max) && (NULL != con2)) {
        conrel = COle (con1max, con2, NULL);
        res = COisTrue (conrel, TRUE);
    }

    con1min = (NULL != con1min) ? COfreeConstant (con1min) : NULL;
    con1max = (NULL != con1max) ? COfreeConstant (con1max) : NULL;
    con2 = (NULL != con2) ? COfreeConstant (con2) : NULL;
    conrel = (NULL != conrel) ? COfreeConstant (conrel) : NULL;

    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);
    pat1mincon = PMfree (pat1mincon);
    pat1maxcon = PMfree (pat1maxcon);
    pat2con = PMfree (pat2con);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn bool isNotEqualPrf( node *arg_node, info *arg_info)
 *
 * @brief Predicate for not equal on PRF_ARG1 and PRF_ARG2
 *
 *        See notes for isNotEqual, above.
 *
 *****************************************************************************/
static bool
isNotEqualPrf (node *arg_node, info *arg_info)
{
    bool res;

    DBUG_ENTER ();

    res = isNotEqual (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node), arg_info)
          || isNotEqual (PRF_ARG2 (arg_node), PRF_ARG1 (arg_node), arg_info);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn bool isMatchPrfShapes( node *arg_node)
 * Predicate for shape( PRF_ARG1) matching shape( PRF_ARG2)
 *
 *****************************************************************************/
static bool
isMatchPrfShapes (node *arg_node)
{
    bool res;
    ntype *type1, *type2;

    DBUG_ENTER ();

    type1 = AVIS_TYPE (ID_AVIS (PRF_ARG1 (arg_node)));
    type2 = AVIS_TYPE (ID_AVIS (PRF_ARG2 (arg_node)));
    res = TUshapeKnown (type1) && TUshapeKnown (type2)
          && TUeqShapes (type1, type2);

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

    DBUG_ENTER ();

    pat1 = PMvar (1, PMAgetNode (&arg1p), 0);

    pat2 = PMprf (1, PMAisPrf (F_neg_V), 1, PMvar (1, PMAisVar (&arg1p), 0));

    res = PMmatchFlatSkipExtremaAndGuards (pat1, arg1)
          && PMmatchFlatSkipExtremaAndGuards (pat2, arg2);

    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn bool MatchNegS( node *arg1, node *arg2)
 *
 * Predicate for:
 *
 *     arg2 ==  _neg_S_( arg1)
 *
 * Arguments:  N_id nodes
 *
 *****************************************************************************/
static bool
MatchNegS (node *arg1, node *arg2)
{
    bool res;
    pattern *pat1;
    pattern *pat2;
    node *arg1p = NULL;

    DBUG_ENTER ();

    pat1 = PMvar (1, PMAgetNode (&arg1p), 0);

    pat2 = PMprf (1, PMAisPrf (F_neg_S), 1, PMvar (1, PMAisVar (&arg1p), 0));

    res = PMmatchFlatSkipExtremaAndGuards (pat1, arg1)
          && PMmatchFlatSkipExtremaAndGuards (pat2, arg2);
    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @fn static bool StripTrues (node **args)
 *
 * @brief Takes N_exprs chain and returns same with TRUE predicates removed.
 *
 * If we find a FALSE predicate, we complain, bitterly. The other choice is to
 * have TC complain, and it gives no hint as to what might be wrong.
 *
 * @returns Whether any arguments were stripped.
 *
 ******************************************************************************/
static bool
StripTrues (node **args)
{
    bool changed = FALSE;
    ntype *pred_type;

    DBUG_ENTER ();

    if (*args != NULL) {
        DBUG_ASSERT (NODE_TYPE (*args) == N_exprs,
                     "N_exprs chain expected, got %s",
                     global.mdb_nodetype[NODE_TYPE (*args)]);

        // Strip truthy arguments bottom-up
        changed = StripTrues (&EXPRS_NEXT (*args));

        DBUG_PRINT ("Looking at: %s", ID_NAME (EXPRS_EXPR (*args)));

        // Delete predicate if truthy
        pred_type = ID_NTYPE (EXPRS_EXPR (*args));
        if (TYisAKV (pred_type)) {
            DBUG_ASSERT (COisTrue (TYgetValue (pred_type), TRUE),
                         "guard is statically known to be false");

            DBUG_PRINT ("Stripping: %s", ID_NAME (EXPRS_EXPR (*args)));

            // Pop the predicate off the chain
            *args = FREEdoFreeNode (*args);
            changed = TRUE;
        }
    }

    DBUG_RETURN (changed);
}

/** <!--********************************************************************-->
 *
 * @fn node *SawingTheBoardInTwo( arg_node, arg_info)
 *
 * @brief Case 7: The "sawing the board in two" optimization, where
 *          we observe that if we cut off part of a board, both
 *          resulting parts are shorter than the original board.
 *
 *          This appears in an the stdlib shift() code, where lack of
 *          this optimization causes AWLF to fail.
 *          Cf. the AWLF unit tests.
 *
 *          We have this code for shift right (Case 7a):
 *
 *           count = _max_SxS_( shiftcount, 0);    // count >= 0
 *           shpa = _shape_A_( vec);               // shpa >= 0
 *           nc = shpa - count;
 *           iv = _non_neg_val_V_( nc);            // iv >= 0
 *           iv', p = =_val_le_val_SxS( iv, shpa);
 *
 *          If (count >= 0) && (shpa >= 0), then:
 *            (shpa - count) <= shpa
 *          so we can statically resolve this guard.
 *
 *          Similarly, for shift left, we have (Case 7b):
 *
 *           count = _min_SxS_( shiftcount, -1);    // count < 0
 *           shpa = _shape_A_( vec);                // shpa >= 0
 *           nc = shpa + count;
 *           iv = _non_neg_val_V_( nc);             // iv >= 0
 *           iv', p = =_val_le_val_SxS( iv, shpa);
 *
 *          If (count < 0) && (shpa >= 0), then:
 *            (shpa + count) <= shpa
 *          so we can statically resolve this guard.
 *
 *****************************************************************************/
static node *
SawingTheBoardInTwo (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *shpa;
    node *iv;
    node *count = NULL;
    pattern *patadd1;
    pattern *patadd2;
    pattern *patsub;

    DBUG_ENTER ();

    patadd1 = PMprf (1, PMAisPrf (F_add_SxS), 2, PMvar (1, PMAisVar (&shpa), 0),
                     PMvar (1, PMAgetNode (&count), 0));

    patadd2 = PMprf (1, PMAisPrf (F_add_SxS), 2, PMvar (1, PMAgetNode (&count), 0),
                     PMvar (1, PMAisVar (&shpa), 0));

    patsub = PMprf (1, PMAisPrf (F_sub_SxS), 2, PMvar (1, PMAisVar (&shpa), 0),
                    PMvar (1, PMAgetNode (&count), 0));

    // Case 7a
    iv = PRF_ARG1 (arg_node);
    shpa = PRF_ARG2 (arg_node);

    if ((SCSisNonNegative (shpa)) && (PMmatchFlatSkipGuards (patsub, iv))
        && (SCSisNonNegative (count))) {
        res = TBmakeExprs (DUPdoDupNode (iv), TBmakeExprs (TBmakeBool (TRUE), NULL));
        DBUG_PRINT ("removed guard Case 7a( %s)", AVIS_NAME (ID_AVIS (iv)));
    }

    // Case 7b
    if ((NULL == res) && (SCSisNonNegative (shpa))
        && ((PMmatchFlatSkipGuards (patadd1, iv))
            || (PMmatchFlatSkipGuards (patadd2, iv)))
        && (SCSisNegative (count))) {
        res = TBmakeExprs (DUPdoDupNode (iv), TBmakeExprs (TBmakeBool (TRUE), NULL));
        DBUG_PRINT ("removed guard Case 7b( %s)", AVIS_NAME (ID_AVIS (iv)));
    }

    patadd1 = PMfree (patadd1);
    patadd2 = PMfree (patadd2);
    patsub = PMfree (patsub);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *isVal1IsSumOfVal2(,,,)
 *
 * @brief  Return TRUE if arg1 is one argument to a sum on arg2, and
 *         the sign of the sum's other argument is:
 *           non-negative, if signum.
 *           negative,     if !signum.
 *
 *         E.g., we have:
 *
 *             con'  = non_neg( con);
 *             arg2  = arg2' + con';
 *             z   = _min_SxS_( arg1, arg2);
 *
 *         We can thus show that arg1 <= arg2, with signum == TRUE.
 *
 *         Do NOT assume that a FALSE result here means arg1 > arg2; it
 *         may just mean Do Not Know!
 *
 *         See above SawingTheBoardInTwo for a similar case.
 *
 *  Note:  This function could be extended to non-scalar types,
 *         and to subtraction, using ISMOP.
 *
 *****************************************************************************/
static bool
isVal1IsSumOfVal2 (node *arg1, node *arg2, info *arg_info, bool signum)
{
    bool z;
    pattern *patadd1;
    pattern *patadd2;
    node *v2;

    DBUG_ENTER ();

    patadd1 = PMprf (1, PMAisPrf (F_add_SxS), 2, PMvar (1, PMAisVar (&arg1), 0),
                     PMvar (1, PMAgetNode (&v2), 0));

    patadd2 = PMprf (1, PMAisPrf (F_add_SxS), 2, PMvar (1, PMAgetNode (&v2), 0),
                     PMvar (1, PMAisVar (&arg1), 0));

    z = (SCSisNonNegative (arg1)) && (SCSisNonNegative (arg2));
    z = z
        && (PMmatchFlat (patadd1, arg2) || // prf( arg1, arg1 + arg2);
            PMmatchFlat (patadd2, arg2));  // prf( arg1, arg2 + arg1);
    if (signum) {
        z = z && SCSisNonNegative (v2);
    } else {
        z = z && SCSisNegative (v2);
    }

    patadd1 = PMfree (patadd1);
    patadd2 = PMfree (patadd2);

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn bool matchesDyadicWithLeftArg (node *expr, node *arg1, prf *fun, node **arg2)
 *
 * @brief: checks whether <expr> matches (<arg1> fun arg2)
 *         <expr> and <arg1> are given through the parameters,
 *         fun and arg2 are being matched here and returned through the
 *         parameters. NOTE here, that not necessarily fun itself is returned
 *         but the _SxS_ version of it!
 *
 *****************************************************************************/
static
bool matchesDyadicWithLeftArg (node *expr, node *arg1, prf *fun, node **arg2)
{
    bool res = FALSE;
    pattern *pat;
    DBUG_ENTER ();

    pat = PMprf (1, PMAgetPrf (fun), 2, PMvar (1, PMAisVar (&arg1), 0),
                 PMvar (1, PMAgetNode (arg2), 0));
    res = PMmatchFlatSkipExtrema (pat, expr);
    if (res)
        *fun = TULSgetPrfFamilyName (*fun);
    pat = PMfree (pat);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn bool matchesDyadicWithRightArg (node *expr, node *arg2, prf *fun, node **arg1)
 *
 * @brief: checks whether <expr> matches (arg1 fun <arg2>)
 *         <expr> and <arg2> are given through the parameters,
 *         fun and arg2 are being matched here and returned through the
 *         parameters. NOTE here, that not necessarily fun itself is returned
 *         but the _SxS_ version of it!
 *
 *****************************************************************************/
static
bool matchesDyadicWithRightArg (node *expr, node *arg2, prf *fun, node **arg1)
{
    bool res = FALSE;
    pattern *pat;
    DBUG_ENTER ();

    pat = PMprf (1, PMAgetPrf (fun), 2, PMvar (1, PMAgetNode (arg1), 0),
                 PMvar (1, PMAisVar (&arg2), 0));
    res = PMmatchFlatSkipExtrema (pat, expr);
    if (res)
        *fun = TULSgetPrfFamilyName (*fun);
    pat = PMfree (pat);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn bool matchesDyadicWithGivenArg (node *expr, node *arg,
 *                                     prf *fun, node **other_arg, bool *arg_is_left)
 *
 * @brief: checks whether <expr> matches
 *         (<arg> fun other_arg) or (other_arg fun <arg>) where
 *         <expr> and <arg> are given through the parameters,
 *         fun and other_arg are being matched here and returned through the
 *         parameters. The final return value arg_is_left indicates which
 *         variant matched; if true, it is the former.
 *         NOTE here, that not necessarily fun itself is returned
 *         but the _SxS_ version of it!
 *
 *****************************************************************************/
static
bool matchesDyadicWithGivenArg (node *expr, node *arg, prf *fun, node **other_arg,
                                bool *arg_is_left)
{
    DBUG_ENTER ();
    *arg_is_left = matchesDyadicWithLeftArg (expr, arg, fun, other_arg);
    DBUG_RETURN ( *arg_is_left
                 || matchesDyadicWithRightArg (expr, arg, fun, other_arg));
}

/** <!--********************************************************************-->
 *
 * @fn bool canOptLEOnDyadicFn (node *arg1, node *arg2, bool *result)
 *
 * @brief: checks whether ( arg1 <= arg2 ) can statically be decided.
 *         if so, *result is set accordingly.
 *         Currently, the following cases are supported:
 *           (x min y) <= x            TRUE
 *           (y min x) <= x            TRUE
 *             (x + y) <= x   | y<0    TRUE
 *             (y + x) <= x   | y<0    TRUE
 *             (x - y) <= x   | y>=0   TRUE
 *             (x + y) <= x   | y>0    FALSE
 *             (y + x) <= x   | y>0    FALSE
 *         and
 *           x <= (x max y)            TRUE
 *           x <= (y max x)            TRUE
 *           x <=   (x + y)   | y>=0   TRUE
 *           x <=   (y + x)   | y>=0   TRUE
 *           x <=   (x + y)   | y<0    FALSE
 *           x <=   (y + x)   | y<0    FALSE
 *           x <=   (x - y)   | y>0    FALSE
 *****************************************************************************/
static
bool canOptLEOnDyadicFn (node *arg1, node *arg2, bool *result)
{
    prf fun;
    node *y;
    bool is_left;
    bool can = FALSE;

    DBUG_ENTER ();
    if (matchesDyadicWithGivenArg (arg1, arg2, &fun, &y, &is_left)) {
        /*
         * arg1 == (arg2 fun y) <= arg2    | is_left
         * arg1 == (y fun arg2) <= arg2    | otherwise
         */
        if ( (fun == F_min_SxS)
             || ((fun == F_add_SxS) && SCSisNegative (y))
             || ((fun == F_sub_SxS) && SCSisNonNegative (y) && is_left) ) {
            can = TRUE;
            *result = TRUE;
        } else if ((fun == F_add_SxS) && SCSisPositive (y)) {
            can = TRUE;
            *result = FALSE;
        }
    } else if (matchesDyadicWithGivenArg (arg2, arg1, &fun, &y, &is_left)) {
        /*
         * arg1 <= (arg1 fun y)     | is_left
         * arg1 <= (y fun arg1)     | otherwise
         */
        if ( (fun == F_max_SxS)
             || ((fun == F_add_SxS) && SCSisNonNegative (y)) ) {
            can = TRUE;
            *result = TRUE;
        } else if ( ((fun == F_add_SxS) && SCSisNegative (y))
                    || ((fun == F_sub_SxS) && SCSisPositive (y) && is_left) ) {
            can = TRUE;
            *result = FALSE;
        }
    }
    DBUG_RETURN (can);
}

/** <!--********************************************************************-->
 *
 * @fn bool canOptGEOnDyadicFn (node *arg1, node *arg2, bool *result)
 *
 * @brief: checks whether ( arg1 >= arg2 ) can statically be decided.
 *         if so, *result is set accordingly.
 *         Currently, the following cases are supported:
 *           (x max y) >= x            TRUE
 *           (y max x) >= x            TRUE
 *             (x + y) >= x   | y<0    TRUE
 *             (y + x) >= x   | y<0    TRUE
 *             (x + y) >= x   | y<0    FALSE
 *             (y + x) >= x   | y<0    FALSE
 *             (x - y) >= x   | y>0    FALSE
 *         and
 *           x >= (x min y)            TRUE
 *           x >= (y min x)            TRUE
 *           x >=   (x + y)   | y<0    TRUE
 *           x >=   (y + x)   | y<0    TRUE
 *           x >=   (x - y)   | y>=0   TRUE
 *           x >=   (x + y)   | y>0    FALSE
 *           x >=   (y + x)   | y>0    FALSE
 *****************************************************************************/
bool SCScanOptGEOnDyadicFn (node *arg1, node *arg2, bool *result)
{
    prf fun;
    node *y;
    bool is_left;
    bool can = FALSE;

    DBUG_ENTER ();
    if (matchesDyadicWithGivenArg (arg1, arg2, &fun, &y, &is_left)) {
        /*
         * arg1 == (arg2 fun y) >= arg2    | is_left
         * arg1 == (y fun arg2) >= arg2    | otherwise
         */
        if ( (fun == F_max_SxS)
             || ((fun == F_add_SxS) && SCSisNonNegative (y)) ) {
            can = TRUE;
            *result = TRUE;
        } else if ( ((fun == F_add_SxS) && SCSisNegative (y))
                    || ((fun == F_sub_SxS) && SCSisPositive (y) && is_left) ) {
            can = TRUE;
            *result = FALSE;
        }
    } else if (matchesDyadicWithGivenArg (arg2, arg1, &fun, &y, &is_left)) {
        /*
         * arg1 >= (arg1 fun y)     | is_left
         * arg1 >= (y fun arg1)     | otherwise
         */
        if ( (fun == F_min_SxS)
             || ((fun == F_add_SxS) && SCSisNegative (y))
             || ((fun == F_sub_SxS) && SCSisNonNegative (y) && is_left) ) {
            can = TRUE;
            *result = TRUE;
        } else if ((fun == F_add_SxS) && SCSisPositive (y)) {
            can = TRUE;
            *result = FALSE;
        }
    }
    DBUG_RETURN (can);
}

/** <!--********************************************************************-->
 *
 * @fn bool canOptGTOnDyadicFn (node *arg1, node *arg2, bool *result)
 *
 * @brief: checks whether ( arg1 > arg2 ) can statically be decided.
 *         if so, *result is set accordingly.
 *         Handles the same cases as canOptLEOnDyadicFn.
 *****************************************************************************/
static
bool canOptGTOnDyadicFn (node *arg1, node *arg2, bool *result)
{
    bool can = FALSE;

    DBUG_ENTER ();
    /*
     * Whenever I can decide that  (arg1 <= arg2) holds,
     * I can also decide that      (arg1 >  arg2) will not hold
     * and vice versa!
     */
    can = canOptLEOnDyadicFn (arg1, arg2, result);
    if (can)
        *result = !*result;
    DBUG_RETURN (can);
}

/** <!--********************************************************************-->
 *
 * @fn bool canOptLTOnDyadicFn (node *arg1, node *arg2, bool *result)
 *
 * @brief: checks whether ( arg1 < arg2 ) can statically be decided.
 *         if so, *result is set accordingly.
 *         Handles the same cases as canOptGEOnDyadicFn.
 *****************************************************************************/
static
bool canOptLTOnDyadicFn (node *arg1, node *arg2, bool *result)
{
    bool can = FALSE;

    DBUG_ENTER ();
    /*
     * Whenever I can decide that  (arg1 >= arg2) holds,
     * I can also decide that      (arg1 <  arg2) will not hold
     * and vice versa!
     */
    can = SCScanOptGEOnDyadicFn (arg1, arg2, result);
    if (can)
        *result = !*result;
    DBUG_RETURN (can);
}

/** <!--********************************************************************-->
 *
 * @fn bool SCScanOptMAXOnDyadicFn (node *arg1, node *arg2, bool *result_is_arg1)
 *
 * @brief: checks whether ( arg1 max arg2 ) can statically be decided.
 *         if so, *result_is_arg1 indicates which argument is the result.
 *         Currently, the following cases are supported:
 *           ( (x min y) max x )       x   (!result_is_arg1)
 *           ( (y min x) max x )       x   (!result_is_arg1)
 *         and
 *           ( x max (x min y) )       x   (result_is_arg1)
 *           ( x max (y min x) )       x   (result_is_arg1)
 *****************************************************************************/
bool SCScanOptMAXOnDyadicFn (node *arg1, node *arg2, bool *result_is_arg1)
{
    prf fun;
    node *y;
    bool is_left;
    bool can = FALSE;

    DBUG_ENTER ();
    if (matchesDyadicWithGivenArg (arg1, arg2, &fun, &y, &is_left)) {
        /*
         * max (arg1 == (arg2 fun y), arg2)    | is_left
         * max (arg1 == (y fun arg2), arg2)    | otherwise
         */
        if (fun == F_min_SxS) {
            can = TRUE;
            *result_is_arg1 = FALSE;
        }
    } else if (matchesDyadicWithGivenArg (arg2, arg1, &fun, &y, &is_left)) {
        /*
         * max (arg1, (arg1 fun y))     | is_left
         * max (arg1, (y fun arg1))     | otherwise
         */
        if (fun == F_min_SxS) {
            can = TRUE;
            *result_is_arg1 = TRUE;
        }
    }
    DBUG_RETURN (can);
}

/** <!--********************************************************************-->
 *
 * @fn bool SCScanOptMINOnDyadicFn (node *arg1, node *arg2, bool *result_is_arg1)
 *
 * @brief: checks whether ( arg1 min arg2 ) can statically be decided.
 *         if so, *result_is_arg1 indicates which argument is the result.
 *         Currently, the following cases are supported:
 *           ( (x max y) min x )       x   (!result_is_arg1)
 *           ( (y max x) min x )       x   (!result_is_arg1)
 *         and
 *           ( x min (x max y) )       x   (result_is_arg1)
 *           ( x min (y max x) )       x   (result_is_arg1)
 *****************************************************************************/
bool SCScanOptMINOnDyadicFn (node *arg1, node *arg2, bool *result_is_arg1)
{
    prf fun;
    node *y;
    bool is_left;
    bool can = FALSE;

    DBUG_ENTER ();
    if (matchesDyadicWithGivenArg (arg1, arg2, &fun, &y, &is_left)) {
        /*
         * min (arg1 == (arg2 fun y), arg2)    | is_left
         * min (arg1 == (y fun arg2), arg2)    | otherwise
         */
        if (fun == F_max_SxS) {
            can = TRUE;
            *result_is_arg1 = FALSE;
        }
    } else if (matchesDyadicWithGivenArg (arg2, arg1, &fun, &y, &is_left)) {
        /*
         * min (arg1, (arg1 fun y))     | is_left
         * min (arg1, (y fun arg1))     | otherwise
         */
        if (fun == F_max_SxS) {
            can = TRUE;
            *result_is_arg1 = TRUE;
        }
    }
    DBUG_RETURN (can);
}


/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_add_SxS( node *arg_node, info *arg_info)
 *   Case 1:  X + 0            --> X
 *   Case 2:  0 + X            --> X
 *   Case 3:  _neg_S_( X) + X  --> 0
 *   Case 4:  X + _neg_S_( X)  --> 0
 *
 *****************************************************************************/
node *
SCSprf_add_SxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();
    if (SCSisConstantZero (PRF_ARG1 (arg_node))) {
        /*  Case 2: 0 + X */
        res = DUPdoDupNode (PRF_ARG2 (arg_node));
    } else if (SCSisConstantZero (PRF_ARG2 (arg_node))) {
        /*  Case 1: X + 0 */
        res = DUPdoDupNode (PRF_ARG1 (arg_node));
    } else if (MatchNegS (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node))
               || MatchNegS (PRF_ARG2 (arg_node), PRF_ARG1 (arg_node))) {
        /* Case 4:   X + _neg_S_( X)  */
        /* Case 3:	 _neg_S_( X) + X  */
        res = SCSmakeZero (PRF_ARG1 (arg_node));
        DBUG_PRINT ("SCSprf_add_SxS generated zero vector");
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_add_SxV( node *arg_node, info *arg_info)
 *   Case 1:   0 + X -> X
 *
 *   Case 2:   SCALAR + [0,0,0] --> [SCALAR, SCALAR, SCALAR]
 *
 *****************************************************************************/
node *
SCSprf_add_SxV (node *arg_node, info *arg_info)
{
    node *res = NULL;
    pattern *pat;
    node *arr = NULL;

    DBUG_ENTER ();

    if (SCSisConstantZero (PRF_ARG1 (arg_node))) { /* 0 + X */
        res = DUPdoDupNode (PRF_ARG2 (arg_node));
        DBUG_PRINT ("SCSprf_add_SxV replaced 0 + VEC by VEC");
    } else {
        /* SCALAR + [0,0,..., 0] */
        pat = PMarray (1, PMAgetNode (&arr), 1, PMskip (0));

        if (SCSisConstantZero (PRF_ARG2 (arg_node))
            && PMmatchFlatSkipExtremaAndGuards (pat, PRF_ARG2 (arg_node))) {

            res = SCSmakeVectorArray (ARRAY_FRAMESHAPE (arr), PRF_ARG1 (arg_node));
            DBUG_PRINT ("SCSprf_add_SxV replaced S + [0,0...,0] by [S,S,..S]");
        }

        pat = PMfree (pat);
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

    DBUG_ENTER ();

    if (MatchNegV (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node))
        || MatchNegV (PRF_ARG2 (arg_node), PRF_ARG1 (arg_node))) {
        /* Case 1:	 _neg_V_( X) + X  */
        /* Case 2:   X + _neg_V_( X)  */
        res = SCSmakeZero (PRF_ARG1 (arg_node));
        DBUG_PRINT ("SCSprf_add_VxV generated zero vector");
    } else {
        if (SCSisConstantZero (PRF_ARG2 (arg_node))) { /* X + 0 */
            res = DUPdoDupNode (PRF_ARG1 (arg_node));
        } else {
            if (SCSisConstantZero (PRF_ARG1 (arg_node))) { /* 0 + X */
                res = DUPdoDupNode (PRF_ARG2 (arg_node));
            }
        }
    }

    DBUG_RETURN (res);
}

/* FIXME Implement SIMD constants properly.  */
node *
SCSprf_add_SMxSM (node *arg_node, info *arg_info)
{
    return NULL;
}
node *
SCSprf_sub_SMxSM (node *arg_node, info *arg_info)
{
    return NULL;
}
node *
SCSprf_mul_SMxSM (node *arg_node, info *arg_info)
{
    return NULL;
}
node *
SCSprf_div_SMxSM (node *arg_node, info *arg_info)
{
    return NULL;
}
node *
SCSprf_simd_sel_VxA (node *arg_node, info *arg_info)
{
    return NULL;
}

node *
SCSprf_simd_sel_SxS (node *arg_node, info *arg_info)
{
    return NULL;
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
    pattern *pat;
    node *arr = NULL;

    DBUG_ENTER ();

    if (SCSisConstantZero (PRF_ARG2 (arg_node))) { /* X + 0 */
        res = DUPdoDupNode (PRF_ARG1 (arg_node));
        DBUG_PRINT ("SCSprf_add_VxS replaced VEC + 0 by VEC");
    } else {
        /* [0,0,..., 0] + X */
        pat = PMarray (1, PMAgetNode (&arr), 1, PMskip (0));

        if (SCSisConstantZero (PRF_ARG1 (arg_node))
            && PMmatchFlatSkipExtremaAndGuards (pat, PRF_ARG1 (arg_node))) {

            res = SCSmakeVectorArray (ARRAY_FRAMESHAPE (arr), PRF_ARG2 (arg_node));
            DBUG_PRINT ("SCSprf_add_VxS replaced [0,0...,0] + S by [S,S,...S]");
        }

        pat = PMfree (pat);
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_sub_SxV( node *arg_node, info *arg_info)
 *
 *   replace  S - [0,0...0] by  [S,S,...S]
 *
 *   replace  S - [S,S,...S] by [0,0,...0]
 *
 *****************************************************************************/
node *
SCSprf_sub_SxV (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *arr = NULL;
    pattern *pat;

    DBUG_ENTER ();

    pat = PMarray (1, PMAgetNode (&arr), 1, PMskip (0));

    if (SCSisConstantZero (PRF_ARG2 (arg_node))
        && PMmatchFlatSkipExtremaAndGuards (pat, PRF_ARG2 (arg_node))) {

        res = SCSmakeVectorArray (ARRAY_FRAMESHAPE (arr), PRF_ARG1 (arg_node));
        DBUG_PRINT ("SCSprf_sub_SxV replaced  S - [0,0...,0] by [S,S,...S]");
    }
    pat = PMfree (pat);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_sub( node *arg_node, info *arg_info)
 *
 * SCSprf_sub_SxS, SCSprf_sub_SxV
 *   replace  X - 0  by  X
 *
 *
 *****************************************************************************/
node *
SCSprf_sub (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

    if (SCSisConstantZero (PRF_ARG2 (arg_node))) { /* X - 0 */
        res = DUPdoDupNode (PRF_ARG1 (arg_node));
    } else if (SCSisMatchPrfargs (arg_node, arg_info)) { /* X - X */
        res = SCSmakeZero (PRF_ARG1 (arg_node));
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_sub_VxV( node *arg_node, info *arg_info)
 * X - X -> (shape(X)) reshape(0)
 *
 *
 *****************************************************************************/
node *
SCSprf_sub_VxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

    if (SCSisConstantZero (PRF_ARG2 (arg_node))) { /* X - 0 */
        res = DUPdoDupNode (PRF_ARG1 (arg_node));
    } else if (SCSisMatchPrfargs (arg_node, arg_info)) { /* X - X */
        res = SCSmakeZero (PRF_ARG1 (arg_node));
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
 *
 *****************************************************************************/
node *
SCSprf_mul_SxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();
    if (SCSisConstantOne (PRF_ARG2 (arg_node))) { /* X * 1 */
        res = DUPdoDupNode (PRF_ARG1 (arg_node));

    } else if (SCSisConstantOne (PRF_ARG1 (arg_node))) { /* 1 * X */
        res = DUPdoDupNode (PRF_ARG2 (arg_node));

    } else if (SCSisConstantZero (PRF_ARG2 (arg_node))) { /* X * 0 */
        res = SCSmakeZero (PRF_ARG1 (arg_node));

    } else if (SCSisConstantZero (PRF_ARG1 (arg_node))) { /* 0 * X */
        res = SCSmakeZero (PRF_ARG2 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_mul_SxV( node *arg_node, info *arg_info)
 * 1 * X -> X
 * 0 * X -> 0 of shape(X)
 *
 *****************************************************************************/
node *
SCSprf_mul_SxV (node *arg_node, info *arg_info)
{
    node *res = NULL;
    pattern *pat;
    node *arr = NULL;

    DBUG_ENTER ();

    pat = PMarray (1, PMAgetNode (&arr), 1, PMskip (0));

    /* Scalar constant cases */
    if (SCSisConstantOne (PRF_ARG1 (arg_node))) { /* 1 * X */
        res = DUPdoDupNode (PRF_ARG2 (arg_node));
    } else if (SCSisConstantZero (PRF_ARG1 (arg_node))) { /* 0 * X */
        res = SCSmakeZero (PRF_ARG2 (arg_node));

        /* Vector constant cases */
    } else if (SCSisConstantZero (PRF_ARG2 (arg_node))) { /*  S * [0,0,...0] */
        res = DUPdoDupNode (PRF_ARG2 (arg_node));
        DBUG_PRINT ("SCSprf_mul_SxV replaced  S* [0,0...,0] by [0,0,...0]");

    } else if (SCSisConstantOne (PRF_ARG2 (arg_node))
               && PMmatchFlatSkipExtremaAndGuards (pat, PRF_ARG2 (arg_node))) {
        res = SCSmakeVectorArray (ARRAY_FRAMESHAPE (arr), PRF_ARG1 (arg_node));
        DBUG_PRINT ("SCSprf_mul_SxV replaced S * [1,1,...1] by [S,S,...S]");
    }

    pat = PMfree (pat);

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
    pattern *pat;
    node *arr = NULL;

    DBUG_ENTER ();

    /* Scalar constant cases */
    pat = PMarray (1, PMAgetNode (&arr), 1, PMskip (0));

    if (SCSisConstantOne (PRF_ARG2 (arg_node))) { /* X * 1 */
        res = DUPdoDupNode (PRF_ARG1 (arg_node));
        DBUG_PRINT ("SCSprf_mul_VxS replaced  V * 1 by V");

    } else if (SCSisConstantZero (PRF_ARG2 (arg_node))) { /* X * 0 */
        res = SCSmakeZero (PRF_ARG1 (arg_node));
        DBUG_PRINT ("SCSprf_mul_VxS replaced  V * 0 by [0,0,...0]");

        /* Vector constant cases */
    } else if (SCSisConstantZero (PRF_ARG1 (arg_node))) { /* [0,0,...0] * S */
        res = DUPdoDupNode (PRF_ARG1 (arg_node));
        DBUG_PRINT ("SCSprf_mul_VxS replaced [0,0...,0] * S by [0,0,...0]");

    } else if (SCSisConstantOne (PRF_ARG1 (arg_node))
               && PMmatchFlatSkipExtremaAndGuards (pat, PRF_ARG1 (arg_node))) {
        res = SCSmakeVectorArray (ARRAY_FRAMESHAPE (arr), PRF_ARG2 (arg_node));
        DBUG_PRINT ("SCSprf_mul_VxS replaced [1,1,...1] * S by [S,S,...S]");
    }

    pat = PMfree (pat);

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

    DBUG_ENTER ();

    if (SCSisConstantOne (PRF_ARG2 (arg_node))) { /* X * 1 */
        res = DUPdoDupNode (PRF_ARG1 (arg_node));
    } else if (SCSisConstantOne (PRF_ARG1 (arg_node))) { /* 1 * X */
        res = DUPdoDupNode (PRF_ARG2 (arg_node));
    } else if (SCSisConstantZero (PRF_ARG2 (arg_node))) { /* X * 0 */
        res = SCSmakeZero (PRF_ARG1 (arg_node));
    } else if (SCSisConstantZero (PRF_ARG1 (arg_node))) { /* 0 * X */
        res = SCSmakeZero (PRF_ARG2 (arg_node));
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_div_SxX( node *arg_node, info *arg_info)
 * S / 1         --> S
 * S / 0         --> error
 * S / [1,1...1] --> [S,S,...S]
 *
 * Handles SCSprf_div_SxS, SCSprf_div_SxV_
 *
 * FIXME: We could handle 0 / vec, if we could show that vec has no
 * zero elements.
 *
 *****************************************************************************/
node *
SCSprf_div_SxX (node *arg_node, info *arg_info)
{
    node *res = NULL;
    pattern *pat;
    node *arr = NULL;

    DBUG_ENTER ();

    pat = PMarray (1, PMAgetNode (&arr), 1, PMskip (0));

    if (SCSisConstantZero (PRF_ARG2 (arg_node))) { /* S / 0 */
        CTIabort (NODE_LOCATION (arg_node),
                      "SCSprf_div_SxX: Division by zero encountered");

        /* Scalar extension case:      S / [1,1,...1]  --> [S,S,..,S] */
    } else if (SCSisConstantOne (PRF_ARG2 (arg_node))
               && PMmatchFlatSkipExtremaAndGuards (pat, PRF_ARG2 (arg_node))) {
        res = SCSmakeVectorArray (ARRAY_FRAMESHAPE (arr), PRF_ARG1 (arg_node));
        DBUG_PRINT ("SCSprf_div_SxX replaced S / [1,1,...1] by [S,S,...S]");
    }
    pat = PMfree (pat);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_div_VxV( node *arg_node, info *arg_info)
 *
 *  V / [1,1..,1]        --> V
 *
 *****************************************************************************/
node *
SCSprf_div_VxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

    if (SCSisConstantOne (PRF_ARG2 (arg_node))) { /* V / [1,1...1] */
        res = DUPdoDupNode (PRF_ARG1 (arg_node));
        DBUG_PRINT ("SCSprf_div_VxV replaced V / [1,1,...1] by V");
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_div_XxS( node *arg_node, info *arg_info)
 * X / 1          --> X
 * X / 0          --> error
 *
 * Handles SCSprf_div_SxS, SCSprf_div_VxS
 *
 *****************************************************************************/
node *
SCSprf_div_XxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

    if (SCSisConstantOne (PRF_ARG2 (arg_node))) { /* X / 1 */
        res = DUPdoDupNode (PRF_ARG1 (arg_node));

    } else if (SCSisConstantZero (PRF_ARG2 (arg_node))) { /* X / 0 */
        CTIabort (NODE_LOCATION (arg_node),
                      "SCSprf_div_XxS: Division by zero encountered");
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

    DBUG_ENTER ();
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_or_SxS( node *arg_node, info *arg_info)
 * X     | false -> X
 * false | X     -> X
 * X     | true  -> true of shape(X)
 * true  | X     -> true of shape(X)
 * X     | Z     -> X
 *
 *****************************************************************************/
node *
SCSprf_or_SxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();
    if (SCSisConstantTrue (PRF_ARG2 (arg_node))) { /* X | true */
        res = SCSmakeTrue (PRF_ARG1 (arg_node));

    } else if (SCSisConstantTrue (PRF_ARG1 (arg_node))) { /* true | X */
        res = SCSmakeTrue (PRF_ARG2 (arg_node));

    } else if (SCSisConstantFalse (PRF_ARG2 (arg_node))) { /* X | false */
        res = DUPdoDupNode (PRF_ARG1 (arg_node));

    } else if (SCSisConstantFalse (PRF_ARG1 (arg_node))) { /* false | X */
        res = DUPdoDupNode (PRF_ARG2 (arg_node));

        /* S | S */
    } else if (SCSisMatchPrfargs (arg_node, arg_info)) {
        res = DUPdoDupNode (PRF_ARG1 (arg_node));
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_or_SxV( node *arg_node, info *arg_info)
 * false | X  -> X
 * true  | X  -> true of shape(X)
 *
 *****************************************************************************/
node *
SCSprf_or_SxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();
    if (SCSisConstantTrue (PRF_ARG1 (arg_node))) { /* true | X */
        res = SCSmakeTrue (PRF_ARG2 (arg_node));
    } else if (SCSisConstantFalse (PRF_ARG1 (arg_node))) { /* false | X */
        res = DUPdoDupNode (PRF_ARG2 (arg_node));
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_or_VxS( node *arg_node, info *arg_info)
 * X | false  -> X
 * X | true   -> true of shape(X)
 *
 *****************************************************************************/
node *
SCSprf_or_VxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();
    if (SCSisConstantTrue (PRF_ARG2 (arg_node))) { /* X | true */
        res = SCSmakeTrue (PRF_ARG1 (arg_node));
    } else if (SCSisConstantFalse (PRF_ARG2 (arg_node))) { /* X | false */
        res = DUPdoDupNode (PRF_ARG1 (arg_node));
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_or_VxV( node *arg_node, info *arg_info)
 *    X     | X      --> X
 *    X     | FALSE  --> X
 *    X     | TRUE   --> TRUE
 *    FALSE | X      --> X
 *    TRUE  | X      --> TRUE
 *
 *****************************************************************************/
node *
SCSprf_or_VxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();
    if (SCSisMatchPrfargs (arg_node, arg_info)) { /*  X | X */
        res = DUPdoDupNode (PRF_ARG2 (arg_node));
    } else if (isMatchPrfShapes (arg_node)) {
        res = SCSprf_or_SxS (arg_node, arg_info);
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_and_SxS( node *arg_node, info *arg_info)
 * X     & true  -> X
 * true  & X     -> X
 * X     & false -> false of shape(X)
 * false & X     -> false of shape(X)
 * X     & X     -> X
 *
 *****************************************************************************/
node *
SCSprf_and_SxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();
    if (SCSisConstantTrue (PRF_ARG2 (arg_node))) { /* X & true */
        DBUG_PRINT ("found _and_SxS_ (x, true)");
        res = DUPdoDupNode (PRF_ARG1 (arg_node));

    } else if (SCSisConstantTrue (PRF_ARG1 (arg_node))) { /* true & X */
        DBUG_PRINT ("found _and_SxS_ (true, x)");
        res = DUPdoDupNode (PRF_ARG2 (arg_node));

    } else if (SCSisConstantFalse (PRF_ARG2 (arg_node))) { /* X & false */
        DBUG_PRINT ("found _and_SxS_ (x, false)");
        res = SCSmakeFalse (PRF_ARG1 (arg_node));

    } else if (SCSisConstantFalse (PRF_ARG1 (arg_node))) { /* false & X */
        DBUG_PRINT ("found _and_SxS_ (false, x)");
        res = SCSmakeFalse (PRF_ARG2 (arg_node));

    } else if (SCSisMatchPrfargs (arg_node, arg_info)) { /* X & X */
        DBUG_PRINT ("found _and_SxS_ (x, x)");
        res = DUPdoDupNode (PRF_ARG1 (arg_node));
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_and_SxV( node *arg_node, info *arg_info)
 * true  & X  -> X
 * false & X  -> false of shape(X)
 *
 *****************************************************************************/
node *
SCSprf_and_SxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();
    if (SCSisConstantTrue (PRF_ARG1 (arg_node))) { /* true & X */
        res = DUPdoDupNode (PRF_ARG2 (arg_node));
    } else if (SCSisConstantFalse (PRF_ARG1 (arg_node))) { /* false & X */
        res = SCSmakeFalse (PRF_ARG2 (arg_node));
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_and_VxS( node *arg_node, info *arg_info)
 * X & true  -> X
 * X & false -> false of shape(X)
 *
 *****************************************************************************/
node *
SCSprf_and_VxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();
    if (SCSisConstantTrue (PRF_ARG2 (arg_node))) { /* X & true */
        res = DUPdoDupNode (PRF_ARG1 (arg_node));
    } else if (SCSisConstantFalse (PRF_ARG2 (arg_node))) { /* X & false */
        res = SCSmakeFalse (PRF_ARG1 (arg_node));
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

    DBUG_ENTER ();
    if (SCSisMatchPrfargs (arg_node, arg_info)) { /*  X & X */
        res = DUPdoDupNode (PRF_ARG2 (arg_node));
    } else if (isMatchPrfShapes (arg_node)) {
        res = SCSprf_and_SxS (arg_node, arg_info);
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_mod_SxS( node *arg_node, info *arg_info)
 *
 * @brief: If both args are positive, and arg1<arg2,
 *         then the result is arg1.
 *
 *****************************************************************************/
node *
SCSprf_mod_SxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

    if ((SCSisPositive (PRF_ARG1 (arg_node))) && (SCSisPositive (PRF_ARG2 (arg_node)))) {
        res = SAACFonRelationalsWithExtrema (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node),
                                             NULL, F_lt_SxS);
        if (NULL != res) {
            FREEdoFreeNode (res);
            res = DUPdoDupNode (PRF_ARG1 (arg_node));
        }
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_mod( node *arg_node, info *arg_info)
 *
 * @brief:
 *
 *****************************************************************************/
node *
SCSprf_mod (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

    CTIabort (EMPTY_LOC, "Modulo using vectors is not supported!");

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_aplmod( node *arg_node, info *arg_info)
 *
 * @brief: Extended mod definition to match ISO Standard APL, N8485:
 *
 *  1. If PRF_ARG2 is zero, the result is PRF_ARG1.
 *     This can't be handled by type checker, if PRF_ARG1 is not AKV.
 *
 *****************************************************************************/
node *
SCSprf_aplmod (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();
    if (SCSisConstantZero (PRF_ARG2 (arg_node))) {
        res = DUPdoDupNode (PRF_ARG1 (arg_node));
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_aplmod_SxV( node *arg_node, info *arg_info)
 *
 * @brief: Extend mod definition to that of ISO Standard APL, N8485:
 *
 *  1. If PRF_ARG2 is zero, the result is PRF_ARG1, reshaped to that of PRF_ARG2.
 *     This can't be handled by type checker, if PRF_ARG1 is not AKV.
 *
 *****************************************************************************/
node *
SCSprf_aplmod_SxV (node *arg_node, info *arg_info)
{
    node *res = NULL;
    shape *shp;

    DBUG_ENTER ();
    if (SCSisConstantZero (PRF_ARG2 (arg_node))) {
        shp = TYgetShape (ID_NTYPE (PRF_ARG2 (arg_node)));
        res = SCSmakeVectorArray (shp, PRF_ARG1 (arg_node));
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_tobool_S( node *arg_node, info *arg_info)
 * Delete type coercion tob(boolvec) if arg_node is already of type bool.
 *
 *****************************************************************************/
node *
SCSprf_tobool_S (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();
    if ((N_bool == NODE_TYPE (PRF_ARG1 (arg_node)))
        || ((N_id == NODE_TYPE (PRF_ARG1 (arg_node)))
            && (T_bool
                == TYgetSimpleType (TYgetScalar (ID_NTYPE (PRF_ARG1 (arg_node))))))) {
        res = DUPdoDupNode (PRF_ARG1 (arg_node));
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

    DBUG_ENTER ();
    if ((N_char == NODE_TYPE (PRF_ARG1 (arg_node)))
        || ((N_id == NODE_TYPE (PRF_ARG1 (arg_node)))
            && (T_char
                == TYgetSimpleType (TYgetScalar (ID_NTYPE (PRF_ARG1 (arg_node))))))) {
        res = DUPdoDupNode (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_tob_S( node *arg_node, info *arg_info)
 * Delete type coercion tob(bytevec) if arg_node is already of type byte.
 *
 *****************************************************************************/
node *
SCSprf_tob_S (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();
    if ((N_numbyte == NODE_TYPE (PRF_ARG1 (arg_node)))
        || ((N_id == NODE_TYPE (PRF_ARG1 (arg_node)))
            && (T_byte
                == TYgetSimpleType (TYgetScalar (ID_NTYPE (PRF_ARG1 (arg_node))))))) {
        res = DUPdoDupNode (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_tos_S( node *arg_node, info *arg_info)
 * Delete type coercion tos(shortvec) if arg_node is already of type short.
 *
 *****************************************************************************/
node *
SCSprf_tos_S (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();
    if ((N_numshort == NODE_TYPE (PRF_ARG1 (arg_node)))
        || ((N_id == NODE_TYPE (PRF_ARG1 (arg_node)))
            && (T_short
                == TYgetSimpleType (TYgetScalar (ID_NTYPE (PRF_ARG1 (arg_node))))))) {
        res = DUPdoDupNode (PRF_ARG1 (arg_node));
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

    DBUG_ENTER ();
    if ((N_num == NODE_TYPE (PRF_ARG1 (arg_node)))
        || (N_numint == NODE_TYPE (PRF_ARG1 (arg_node)))
        || ((N_id == NODE_TYPE (PRF_ARG1 (arg_node)))
            && (T_int
                == TYgetSimpleType (TYgetScalar (ID_NTYPE (PRF_ARG1 (arg_node))))))) {
        res = DUPdoDupNode (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_tol_S( node *arg_node, info *arg_info)
 * Delete type coercion tol(longvec) if arg_node is already of type long.
 *
 *****************************************************************************/
node *
SCSprf_tol_S (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();
    if ((N_numlong == NODE_TYPE (PRF_ARG1 (arg_node)))
        || ((N_id == NODE_TYPE (PRF_ARG1 (arg_node)))
            && (T_long
                == TYgetSimpleType (TYgetScalar (ID_NTYPE (PRF_ARG1 (arg_node))))))) {
        res = DUPdoDupNode (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_toll_S( node *arg_node, info *arg_info)
 * Delete type coercion toll(longlongvec) if arg_node is already of type
 * longlong.
 *
 *****************************************************************************/
node *
SCSprf_toll_S (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();
    if ((N_numlonglong == NODE_TYPE (PRF_ARG1 (arg_node)))
        || ((N_id == NODE_TYPE (PRF_ARG1 (arg_node)))
            && (T_longlong
                == TYgetSimpleType (TYgetScalar (ID_NTYPE (PRF_ARG1 (arg_node))))))) {
        res = DUPdoDupNode (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_toub_S( node *arg_node, info *arg_info)
 * Delete type coercion toub(ubytevec) if arg_node is already of type ubyte.
 *
 *****************************************************************************/
node *
SCSprf_toub_S (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();
    if ((N_numubyte == NODE_TYPE (PRF_ARG1 (arg_node)))
        || ((N_id == NODE_TYPE (PRF_ARG1 (arg_node)))
            && (T_ubyte
                == TYgetSimpleType (TYgetScalar (ID_NTYPE (PRF_ARG1 (arg_node))))))) {
        res = DUPdoDupNode (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_tous_S( node *arg_node, info *arg_info)
 * Delete type coercion tous(ushortvec) if arg_node is already of type ushort.
 *
 *****************************************************************************/
node *
SCSprf_tous_S (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();
    if ((N_numushort == NODE_TYPE (PRF_ARG1 (arg_node)))
        || ((N_id == NODE_TYPE (PRF_ARG1 (arg_node)))
            && (T_ushort
                == TYgetSimpleType (TYgetScalar (ID_NTYPE (PRF_ARG1 (arg_node))))))) {
        res = DUPdoDupNode (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_toui_S( node *arg_node, info *arg_info)
 * Delete type coercion toui(uintvec) if arg_node is already of type uint.
 *
 *****************************************************************************/
node *
SCSprf_toui_S (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();
    if ((N_numuint == NODE_TYPE (PRF_ARG1 (arg_node)))
        || ((N_id == NODE_TYPE (PRF_ARG1 (arg_node)))
            && (T_uint
                == TYgetSimpleType (TYgetScalar (ID_NTYPE (PRF_ARG1 (arg_node))))))) {
        res = DUPdoDupNode (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_toul_S( node *arg_node, info *arg_info)
 * Delete type coercion toul(ulongvec) if arg_node is already of type ulong.
 *
 *****************************************************************************/
node *
SCSprf_toul_S (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();
    if ((N_numulong == NODE_TYPE (PRF_ARG1 (arg_node)))
        || ((N_id == NODE_TYPE (PRF_ARG1 (arg_node)))
            && (T_ulong
                == TYgetSimpleType (TYgetScalar (ID_NTYPE (PRF_ARG1 (arg_node))))))) {
        res = DUPdoDupNode (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_toull_S( node *arg_node, info *arg_info)
 * Delete type coercion toull(longlongvec) if arg_node is already of type
 * ulonglong.
 *
 *****************************************************************************/
node *
SCSprf_toull_S (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();
    if ((N_numulonglong == NODE_TYPE (PRF_ARG1 (arg_node)))
        || ((N_id == NODE_TYPE (PRF_ARG1 (arg_node)))
            && (T_ulonglong
                == TYgetSimpleType (TYgetScalar (ID_NTYPE (PRF_ARG1 (arg_node))))))) {
        res = DUPdoDupNode (PRF_ARG1 (arg_node));
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

    DBUG_ENTER ();
    if ((N_float == NODE_TYPE (PRF_ARG1 (arg_node)))
        || ((N_id == NODE_TYPE (PRF_ARG1 (arg_node)))
            && (T_float
                == TYgetSimpleType (TYgetScalar (ID_NTYPE (PRF_ARG1 (arg_node))))))) {
        res = DUPdoDupNode (PRF_ARG1 (arg_node));
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

    DBUG_ENTER ();
    if ((N_double == NODE_TYPE (PRF_ARG1 (arg_node)))
        || ((N_id == NODE_TYPE (PRF_ARG1 (arg_node)))
            && (T_double
                == TYgetSimpleType (TYgetScalar (ID_NTYPE (PRF_ARG1 (arg_node))))))) {
        res = DUPdoDupNode (PRF_ARG1 (arg_node));
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_min_SxS( node *arg_node, info *arg_info)
 *
 * The codes for min_SxS, max_SxS, min_VxV, and max_VxV are
 * basically identical.
 * The SxV and VxS cases could be implemented with ISMOP.
 *
 * Case 1: If (X == Y), then X.
 *
 * Case 2: If max( X, Y), where AVIS_MIN( Y) == X, then Y.
 * Case 3: If max( X, Y), where AVIS_MIN( X) == Y, then X.
 *
 * Case 4: If min( X, Y), where AVIS_MAX( Y) == X, then Y.
 * Case 5: If min( X, Y), where AVIS_MAX( X) == Y, then X.
 *
 * Case 6: check relationals on extrema.
 *
 * Case 7:    min( X, max( X, k))
 *         if X >= k,
 *            min( X, max( X, k)) --> min( X, X) --> X
 *         if X < k,
 *            min( X, max( X, k)) --> min( X, k) --> X
 *
 *         The same holds for max( X, min( N, k)):
 *         if X >= k,
 *            max( X, min( X, k)) --> max( X, k) --> X
 *         if X < k,
 *            max( X, min( X, k)) --> max( X, X) --> X
 *
 *         And ditto for PRFARGs in reversed order.
 *
 *****************************************************************************/
node *
SCSprf_max_SxS (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *res2 = NULL;
    bool z = FALSE;
    ntype *restype;

    DBUG_ENTER ();
    if (SCSisMatchPrfargs (arg_node, arg_info)) { /* max ( X, X) */
        res = DUPdoDupNode (PRF_ARG1 (arg_node));
    }

    // Do TC's job: this is needed for call from IVEXP.
    if (NULL == res) {
        restype = NTCnewTypeCheck_Expr (arg_node);
        // DBUG_ASSERT( !TYisProd( restype), "expected one result type");
        restype = TYgetProductMember (restype, 0);
        if (TYisAKV (restype)) {
            res = CFcreateConstExprsFromType (restype);
        }
        restype = TYfreeType (restype);
    }

    /* Case 2 */
    if ((NULL == res) && (NULL != AVIS_MIN (ID_AVIS (PRF_ARG2 (arg_node))))
        && (ID_AVIS (PRF_ARG1 (arg_node))
            == ID_AVIS (AVIS_MIN (ID_AVIS (PRF_ARG2 (arg_node)))))) {
        res = DUPdoDupNode (PRF_ARG2 (arg_node));
    }

    /* Case 3 */
    if ((NULL == res) && (NULL != AVIS_MIN (ID_AVIS (PRF_ARG1 (arg_node))))
        && (ID_AVIS (PRF_ARG2 (arg_node))
            == ID_AVIS (AVIS_MIN (ID_AVIS (PRF_ARG1 (arg_node)))))) {
        res = DUPdoDupNode (PRF_ARG1 (arg_node));
    }

    if (NULL == res) {
        res2 = SAACFonRelationalsWithExtrema (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node),
                                              NULL, F_ge_SxS);
        if (NULL != res2) {
            if (SCSisConstantOne (res2)) {
                res = DUPdoDupNode (PRF_ARG1 (arg_node));
            } else {
                res = DUPdoDupNode (PRF_ARG2 (arg_node));
            }
            res2 = FREEdoFreeNode (res2);
        }
    }

    if ((NULL == res)
        && ((isVal1IsSumOfVal2 (PRF_ARG1 (arg_node), // max( x, x+nonneg)
                                PRF_ARG2 (arg_node), arg_info, TRUE))
            || (isVal1IsSumOfVal2 (PRF_ARG2 (arg_node), // max( x+neg, x)
                                   PRF_ARG1 (arg_node), arg_info, FALSE)))) {
        res = DUPdoDupNode (PRF_ARG2 (arg_node));
    }

    if ((NULL == res)
        && ((isVal1IsSumOfVal2 (PRF_ARG2 (arg_node), // max( x+nonneg, x)
                                PRF_ARG1 (arg_node), arg_info, TRUE))
            || (isVal1IsSumOfVal2 (PRF_ARG1 (arg_node), // max( x, x+neg)
                                   PRF_ARG2 (arg_node), arg_info, FALSE)))) {
        res = DUPdoDupNode (PRF_ARG1 (arg_node));
    }

    // Case 7
    if ((NULL == res)
        && SCScanOptMAXOnDyadicFn (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node), &z)) {
        if (z) {
            res = DUPdoDupNode (PRF_ARG1 (arg_node));
        } else {
            res = DUPdoDupNode (PRF_ARG2 (arg_node));
        }
    }

    DBUG_RETURN (res);
}

node *
SCSprf_max_VxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();
    res = SCSprf_max_SxS (arg_node, arg_info);
    DBUG_RETURN (res);
}

node *
SCSprf_min_SxS (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *res2 = NULL;
    bool z = FALSE;
    ntype *restype;

    DBUG_ENTER ();
    if (SCSisMatchPrfargs (arg_node, arg_info)) { /* min( X, X) */
        res = DUPdoDupNode (PRF_ARG1 (arg_node));
    }

    // Do TC's job: this is needed for call from IVEXP.
    if (NULL == res) {
        restype = NTCnewTypeCheck_Expr (arg_node);
        // DBUG_ASSERT( !TYisProd( restype), "expected one result type");
        restype = TYgetProductMember (restype, 0);
        if (TYisAKV (restype)) {
            res = CFcreateConstExprsFromType (restype);
        }
        restype = TYfreeType (restype);
    }

    /* Case 4 */
    if ((NULL == res) && (NULL != AVIS_MAX (ID_AVIS (PRF_ARG2 (arg_node))))
        && (ID_AVIS (PRF_ARG1 (arg_node))
            == ID_AVIS (AVIS_MAX (ID_AVIS (PRF_ARG2 (arg_node)))))) {
        res = DUPdoDupNode (PRF_ARG2 (arg_node));
    }

    /* Case 5 */
    if ((NULL == res) && (NULL != AVIS_MAX (ID_AVIS (PRF_ARG1 (arg_node))))
        && (ID_AVIS (PRF_ARG2 (arg_node))
            == ID_AVIS (AVIS_MAX (ID_AVIS (PRF_ARG1 (arg_node)))))) {
        res = DUPdoDupNode (PRF_ARG1 (arg_node));
    }

    // case 6
    if (NULL == res) {
        res2 = SAACFonRelationalsWithExtrema (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node),
                                              NULL, F_le_SxS);
        if (NULL != res2) {
            if (SCSisConstantOne (res2)) {
                res = DUPdoDupNode (PRF_ARG1 (arg_node));
            } else {
                res = DUPdoDupNode (PRF_ARG2 (arg_node));
            }
            res2 = FREEdoFreeNode (res2);
        }
    }

    if ((NULL == res)
        && ((isVal1IsSumOfVal2 (PRF_ARG1 (arg_node), // min( x, x+nonneg)
                                PRF_ARG2 (arg_node), arg_info, TRUE))
            || (isVal1IsSumOfVal2 (PRF_ARG2 (arg_node), // min( x+neg, x)
                                   PRF_ARG1 (arg_node), arg_info, FALSE)))) {
        res = DUPdoDupNode (PRF_ARG1 (arg_node));
    }

    if ((NULL == res)
        && ((isVal1IsSumOfVal2 (PRF_ARG2 (arg_node), // min( x+nonneg, x)
                                PRF_ARG1 (arg_node), arg_info, TRUE))
            || (isVal1IsSumOfVal2 (PRF_ARG1 (arg_node), // min( x, x+neg)
                                   PRF_ARG2 (arg_node), arg_info, FALSE)))) {
        res = DUPdoDupNode (PRF_ARG2 (arg_node));
    }

    // Case 7
    if ((NULL == res)
        && SCScanOptMINOnDyadicFn (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node), &z)) {
        if (z) {
            res = DUPdoDupNode (PRF_ARG1 (arg_node));
        } else {
            res = DUPdoDupNode (PRF_ARG2 (arg_node));
        }
    }

    DBUG_RETURN (res);
}

node *
SCSprf_min_VxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();
    res = SCSprf_min_SxS (arg_node, arg_info);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * The actual function table operations follow
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_lege( node *arg_node, info *arg_info)
 * If (X == Y), then TRUE.
 * Implements SCSprf_ge_SxS, SCSprf_ge_VxV, SCSprf_le_SxS, SCSprf_ge_VxV
 *
 *****************************************************************************/
node *
SCSprf_lege (node *arg_node, info *arg_info)
{
    node *res = NULL;
    bool z = FALSE;

    DBUG_ENTER ();
    if (SCSisMatchPrfargs (arg_node, arg_info)) {
        // (x <prf> x)
        res = SCSmakeTrue (PRF_ARG1 (arg_node));
    } else if ( ((TULSgetPrfFamilyName (PRF_PRF (arg_node)) == F_ge_SxS)
             && SCScanOptGEOnDyadicFn (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node), &z))
             || (TULSgetPrfFamilyName ((PRF_PRF (arg_node)) == F_le_SxS)
                && canOptLEOnDyadicFn (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node), &z)) ) {
        if (z) {
            res = SCSmakeTrue (PRF_ARG1 (arg_node));
        } else {
            res = SCSmakeFalse (PRF_ARG1 (arg_node));
        }
        DBUG_PRINT ("Found TRUE relational minmax 12");
    }

    if (NULL == res) {
        res = SAACFonRelationalsWithExtrema (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node),
                                             NULL, PRF_PRF (arg_node));
    }

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
    bool z = FALSE;

    DBUG_ENTER ();
    if (SCSisMatchPrfargs (arg_node, arg_info)) {
        res = SCSmakeFalse (PRF_ARG1 (arg_node));
    } else if ( ((TULSgetPrfFamilyName (PRF_PRF (arg_node)) == F_lt_SxS)
             && canOptLTOnDyadicFn (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node), &z))
             || (TULSgetPrfFamilyName ((PRF_PRF (arg_node)) == F_gt_SxS)
                && canOptGTOnDyadicFn (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node), &z)) ) {
        if (z) {
            res = SCSmakeTrue (PRF_ARG1 (arg_node));
        } else {
            res = SCSmakeFalse (PRF_ARG1 (arg_node));
        }
        DBUG_PRINT ("Found TRUE relational minmax 12");
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_eq_SxS( node *arg_node, info *arg_info)
 *
 * @brief:
 *
 *****************************************************************************/
node *
SCSprf_eq_SxS (node *arg_node, info *arg_info)
{
    node *res=NULL;

    DBUG_ENTER ();

    if (SCSisMatchPrfargs (arg_node, arg_info)) {
        res = SCSmakeTrue (PRF_ARG1 (arg_node));
    }

    if ((NULL == res) && isNotEqualPrf (arg_node, arg_info)) {
        res = SCSmakeFalse (PRF_ARG1 (arg_node));
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_eq_SxV( node *arg_node, info *arg_info)
 *
 * @brief:
 *
 *****************************************************************************/
node *
SCSprf_eq_SxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

    if ((NULL == res) && isNotEqualPrf (arg_node, arg_info)) {
        res = SCSmakeFalse (PRF_ARG2 (arg_node));
    }

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

    DBUG_ENTER ();

    if ((NULL == res) && isNotEqualPrf (arg_node, arg_info)) {
        res = SCSmakeFalse (PRF_ARG1 (arg_node));
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_eq_VxV( node *arg_node, info *arg_info)
 *
 * @brief:
 *
 *****************************************************************************/
node *
SCSprf_eq_VxV (node *arg_node, info *arg_info)
{
    node *res=NULL;

    DBUG_ENTER ();

    if (SCSisMatchPrfargs (arg_node, arg_info)) {
        res = SCSmakeTrue (PRF_ARG1 (arg_node));
    }

    if ((NULL == res) && isNotEqualPrf (arg_node, arg_info)) {
        res = SCSmakeFalse (PRF_ARG1 (arg_node));
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
    DBUG_ENTER ();

    res = SCSprf_nlege (arg_node, arg_info);

    if ((NULL == res) && isNotEqualPrf (arg_node, arg_info)) {
        res = SCSmakeTrue (PRF_ARG1 (arg_node));
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

    DBUG_ENTER ();

    if (isNotEqualPrf (arg_node, arg_info)) {
        res = SCSmakeTrue (PRF_ARG2 (arg_node));
    }

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
    DBUG_ENTER ();

    if (isNotEqualPrf (arg_node, arg_info)) {
        res = SCSmakeTrue (PRF_ARG1 (arg_node));
    }

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
    DBUG_ENTER ();

    res = SCSprf_nlege (arg_node, arg_info);

    if ((NULL == res) && isNotEqualPrf (arg_node, arg_info)) {
        res = SCSmakeTrue (PRF_ARG1 (arg_node));
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

    DBUG_ENTER ();
    /* Handled by SAACF */
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

    DBUG_ENTER ();
    /* Handled by SAACF */
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

    DBUG_ENTER ();
    /* Handled by SAACF */
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_ge_VxS( node *arg_node, info *arg_info)
 *
 * @brief: Handle relational where one argument is constant
 *         and the other has known extrema.
 *
 *****************************************************************************/
node *
SCSprf_ge_VxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();
    /* Handled by SAACF */
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_sel_VxA( node *arg_node, info *arg_info)
 *
 * @brief:
 *
 *****************************************************************************/
node *
SCSprf_sel_VxA (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @fn node *SCSprf_all_V (node *arg_node, info *arg_info)
 *
 ******************************************************************************/
node *
SCSprf_all_V (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_sel_VxIA( node *arg_node, info *arg_info)
 *
 * @brief:
 *
 *
 *****************************************************************************/
node *
SCSprf_sel_VxIA (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

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

    DBUG_ENTER ();

    pat = PMprf (1, PMAisPrf (F_reshape_VxA), 2, PMvar (1, PMAgetNode (&shp1), 0),
                 PMprf (1, PMAisPrf (F_reshape_VxA), 2, PMvar (1, PMAgetNode (&shp2), 0),
                        PMvar (1, PMAgetNode (&X), 0)));
    if (PMmatchFlatSkipExtremaAndGuards (pat, arg_node)) {
        DBUG_PRINT_TAG ("SCS", "Replacing reshape of reshape");

        res = DUPdoDupNode (arg_node);
        shp2 = FREEdoFreeNode (PRF_ARG2 (res));
        PRF_ARG2 (res) = DUPdoDupNode (X);
    }
    pat = PMfree (pat);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 *  Functions for removing array operation conformability checks
 *
 ******************************************************************************/

/******************************************************************************
 *
 * @fn node *SCSprf_guard (node *arg_node, info *arg_info)
 *
 * @brief x1', .., xn' = guard (x1, .., xn, p1, .., pm)
 * If all boolean predicates pi are true, then guard behaves as the identity
 * function, and the result is x1, .., xn.
 *
 * @returns NULL if no predicates could be stripped, a new N_prf with updated
 * predicates if some but not all predicates could be stripped, or an N_exprs
 * chain of the arguments x1, .., xn if all predicates could be stripped.
 *
 ******************************************************************************/
node *
SCSprf_guard (node *arg_node, info *arg_info)
{
    bool changed;
    size_t num_args, num_rets;
    node *last_arg, *preds, *res;

    DBUG_ENTER ();

    num_rets = PRF_NUMVARIABLERETS (arg_node);
    DBUG_ASSERT (num_rets > 0, "guard has no return values");
    num_args = TCcountExprs (PRF_ARGS (arg_node));
    DBUG_ASSERT (num_args >= num_rets + 1,
                 "guard requires at least %lu arguments, got %lu",
                 num_rets + 1, num_args);

    // Skip x1, .., xn
    last_arg = TCgetNthExprs (num_rets - 1, PRF_ARGS (arg_node));
    preds = EXPRS_NEXT (last_arg);

    // Strip all predicates that we statically know are true
    changed = StripTrues (&preds);
    EXPRS_NEXT (last_arg) = preds;

    if (changed) {
        if (preds == NULL) {
            DBUG_PRINT ("No predicates remain, replacing guard with identity");
            // Duplicate N_exprs chain x1, .., xn
            res = DUPdoDupTree (PRF_ARGS (arg_node));
        } else {
            DBUG_PRINT ("Guard predicates changed, updating predicates chain");
            // Preds have already been updated, duplicate the entire N_prf
            res = DUPdoDupTree (arg_node);
        }
    } else {
        DBUG_PRINT ("All guard predicates remain");
        res = NULL;
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

    DBUG_ENTER ();
    arg1type = ID_NTYPE (PRF_ARG1 (arg_node));
    arg2type = ID_NTYPE (PRF_ARG2 (arg_node));
    if (SCSisMatchPrfargs (arg_node, arg_info) /* same_shape(X, X) */
        || (TUshapeKnown (arg1type) && TUshapeKnown (arg2type)
            && TUeqShapes (arg1type, arg2type))) { /* AKS & shapes match */
        res = TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                           TBmakeExprs (DUPdoDupNode (PRF_ARG2 (arg_node)),
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

    DBUG_ENTER ();

    pat = PMprf (1, PMAisPrf (F_shape_matches_dim_VxA), 2, PMvar (1, PMAgetNode (&iv), 0),
                 PMvar (1, PMAgetNode (&arr), 0));

    if (PMmatchFlatSkipExtrema (pat, arg_node)) {
        ivtype = ID_NTYPE (iv);
        arrtype = ID_NTYPE (arr);
        if (TUshapeKnown (ivtype) && TUdimKnown (arrtype)
            && SHgetExtent (TYgetShape (ivtype), 0) == TYgetDim (arrtype)) {
            res = TBmakeExprs (DUPdoDupNode (iv), TBmakeExprs (TBmakeBool (TRUE), NULL));
            DBUG_PRINT_TAG ("SCS", "SCSprf_shape_matches_dim_VxA removed guard( %s, %s)",
                            AVIS_NAME (ID_AVIS (iv)), AVIS_NAME (ID_AVIS (arr)));
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
 *  Case 1: PRF_ARG1 is constant.
 *
 *  Case 2: PRF_ARG1 has non-negative, constant AVIS_MIN.
 *
 *****************************************************************************/
node *
SCSprf_non_neg_val_V (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

    /* Case 1*/
    if (SCSisNonNegative (PRF_ARG1 (arg_node))) {

        DBUG_PRINT ("Removed non_neg guard on %s",
                    AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))));
        res = TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                           TBmakeExprs (TBmakeBool (TRUE), NULL));
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_non_neg_val_S( node *arg_node, info *arg_info)
 *
 * description: Scalar version.
 *
 *****************************************************************************/
node *
SCSprf_non_neg_val_S (node *arg_node, info *arg_info)
{
    node *res;

    DBUG_ENTER ();

    res = SCSprf_non_neg_val_V (arg_node, arg_info);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_val_lt_shape_VxA( node *arg_node, info *arg_info)
 *
 * description:
 *  This primitive checks that iv is less than the shape of arr in arr[iv]
 *  and, if so, replaces:
 *
 *   iv', pred = prf_val_lt_shape_VxA( iv, arr)
 *
 *  by
 *
 *   iv', pred = iv, TRUE;
 *
 * Case 1: PRF_ARG1 is constant, and matches the shape of AKS/AKV PRF_ARG2.
 *
 * Case 2: AVIS_MAX( ID_AVIS( PRF_ARG1)) is the shape of PRF_ARG2,
 *         and both are N_id nodes.
 *
 * Case 3: Like Case 2, but AVIS_SHAPE of PRF_ARG2 is an N_array.
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
    ntype *arrtype;
    shape *arrshp;
    pattern *pat1;

    DBUG_ENTER ();

    pat1 = PMprf (1, PMAisPrf (F_val_lt_shape_VxA), 2, PMconst (1, PMAgetVal (&ivc)),
                  PMvar (1, PMAgetNode (&arr), 0));

    /* Case 1 */
    iv = PRF_ARG1 (arg_node);
    if (PMmatchFlat (pat1, arg_node)) {
        arrtype = ID_NTYPE (arr);
        if (TUshapeKnown (arrtype)) {
            arrshp = TYgetShape (arrtype);
            arrc = COmakeConstantFromShape (arrshp);
            if ((COgetExtent (ivc, 0) == COgetExtent (arrc, 0))
                && COlt (ivc, arrc, NULL)) {
                res = TBmakeExprs (DUPdoDupNode (iv),
                                   TBmakeExprs (TBmakeBool (TRUE), NULL));
                DBUG_PRINT_TAG ("SCS", "Case 1 removed guard( %s, %s)",
                                AVIS_NAME (ID_AVIS (iv)), AVIS_NAME (ID_AVIS (arr)));
            }
        }
    }
    pat1 = PMfree (pat1);
    arrc = (NULL != arrc) ? COfreeConstant (arrc) : arrc;

    /* Case 2 */
    if ((NULL == res) && (NULL != AVIS_MAX (ID_AVIS (PRF_ARG1 (arg_node))))
        && (NULL != AVIS_SHAPE (ID_AVIS (PRF_ARG2 (arg_node))))
        && (N_id == NODE_TYPE (AVIS_SHAPE (ID_AVIS (PRF_ARG2 (arg_node)))))
        && (ID_AVIS (AVIS_MAX (ID_AVIS (PRF_ARG1 (arg_node))))
            == ID_AVIS (AVIS_SHAPE (ID_AVIS (PRF_ARG2 (arg_node)))))) {
        res = TBmakeExprs (DUPdoDupNode (iv), TBmakeExprs (TBmakeBool (TRUE), NULL));
        DBUG_PRINT_TAG ("SCS", "Case 2 removed guard( %s, %s)", AVIS_NAME (ID_AVIS (iv)),
                        AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node))));
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_val_lt_val_SxS( node *arg_node, info *arg_info)
 *
 * description:
 *  Similar to val_le_val_SxS version.
 *
 *  Case 6: If this is an extended guard (GGS), with PRF_ARG3 not NULL,
 *          and PRF_ARG3 is TRUE, perform the optimization,
 *          and PERHAPS? set AVIS_MAXVAL( res) = PRF_ARG2.
 *
 *****************************************************************************/
node *
SCSprf_val_lt_val_SxS (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *val = NULL;
    node *val2 = NULL;
    node *val3 = NULL;
    node *res2;
    node *b = NULL;
    constant *con1 = NULL;
    constant *con2 = NULL;
    constant *conrel = NULL;
    pattern *pat1;
    pattern *pat3;
    pattern *pat4;
    bool flg = FALSE;

    DBUG_ENTER ();

    pat1 = PMprf (1, PMAisPrf (F_val_lt_val_SxS), 2, PMconst (1, PMAgetVal (&con1)),
                  PMconst (1, PMAgetVal (&con2), 0));

    pat3 = PMprf (1, PMAisPrf (F_val_lt_val_SxS), 2, PMvar (1, PMAgetNode (&val), 0),
                  PMvar (1, PMAgetNode (&val2), 0));

    pat4 = PMprf (1, PMAisPrf (F_val_lt_val_SxS), 2, PMvar (1, PMAgetNode (&val3), 0),
                  PMvar (1, PMAisVar (&val2), 0));

    /* Case 1 */
    if (PMmatchFlat (pat1, arg_node)) {
        conrel = COlt (con1, con2, NULL);
        if ((NULL != conrel) && COisTrue (conrel, TRUE)) {
            res = TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                               TBmakeExprs (TBmakeBool (TRUE), NULL));
            DBUG_PRINT_TAG ("SCS", "removed guard Case 1( %s, %s)",
                            AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                            AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node))));
        }
    }
    con1 = (NULL != con1) ? COfreeConstant (con1) : con1;
    con2 = (NULL != con2) ? COfreeConstant (con2) : con2;
    conrel = (NULL != conrel) ? COfreeConstant (conrel) : conrel;

#ifdef DEADCODE
    /* Case 2 */
    if ((NULL == res)
        && (ID_AVIS (PRF_ARG1 (arg_node)) == ID_AVIS (PRF_ARG2 (arg_node)))) {
        res = TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                           TBmakeExprs (TBmakeBool (TRUE), NULL));
        DBUG_PRINT_TAG ("SCS", "removed guard Case 2( %s, %s)",
                        AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                        AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node))));
    }
#endif // DEADCODE

    /* Case 3a */
    if ((NULL == res) && (NULL != AVIS_MIN (ID_AVIS (PRF_ARG2 (arg_node))))) {
        res2 = SCSrecurseWithExtrema (arg_node, arg_info, PRF_ARG1 (arg_node),
                                      AVIS_MIN (ID_AVIS (PRF_ARG2 (arg_node))),
                                      &SCSprf_val_lt_val_SxS);
        if (NULL != res2) {
            FREEdoFreeNode (res2);
            res = TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                               TBmakeExprs (TBmakeBool (TRUE), NULL));
            DBUG_PRINT_TAG ("SCS", "removed guard Case 3a( %s, %s)",
                            AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                            AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node))));
        }
    }

    /* Case 3b */
    if ((NULL == res) && (NULL != AVIS_MAX (ID_AVIS (PRF_ARG1 (arg_node))))) {
        res2 = SCSrecurseWithExtrema (arg_node, arg_info,
                                      AVIS_MAX (ID_AVIS (PRF_ARG1 (arg_node))),
                                      PRF_ARG2 (arg_node), &SCSprf_val_lt_val_SxS);
        if (NULL != res2) {
            FREEdoFreeNode (res2);
            res = TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                               TBmakeExprs (TBmakeBool (TRUE), NULL));
            DBUG_PRINT_TAG ("SCS", "removed guard Case 3b( %s, %s)",
                            AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                            AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node))));
        }
    }

    /* Case 4:  */
    if ((NULL == res) && (PMmatchFlat (pat3, arg_node)) && (PMmatchFlat (pat4, val))) {
        res = TBmakeExprs (DUPdoDupNode (val3), TBmakeExprs (TBmakeBool (TRUE), NULL));
        DBUG_PRINT_TAG ("SCS", "removed guard Case 4( %s -> %s)",
                        AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                        AVIS_NAME (ID_AVIS (val3)));
    }

    /* Case 5 */
    if (NULL == res) {
        b = SAACFonRelationalsWithExtrema (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node),
                                           arg_info, F_lt_SxS);

        if ((NULL != b) && SCSisConstantOne (b)) {
            res = TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                               TBmakeExprs (TBmakeBool (TRUE), NULL));
            DBUG_PRINT ("removed guard Case 5( %s, %s)",
                        AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                        AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node))));
            b = (NULL != b) ? FREEdoFreeNode (b) : b;
        }
    }

    /* Case 6 */
    b = PRF_EXPRS3 (arg_node);
    if ((NULL == res) && (NULL != b) && SCSisConstantOne (b)) {
        res = TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                           TBmakeExprs (TBmakeBool (TRUE), NULL));
        DBUG_PRINT ("removed guard Case 6( %s, %s)",
                    AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                    AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node))));
    }

    if ((NULL == res)
        && canOptLTOnDyadicFn (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node), &flg)) {
        if (flg) {
            res = TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                               TBmakeExprs (TBmakeBool (TRUE), NULL));
        }
    }

    con1 = (NULL != con1) ? COfreeConstant (con1) : con1;
    con2 = (NULL != con2) ? COfreeConstant (con2) : con2;
    pat1 = PMfree (pat1);
    pat3 = PMfree (pat3);
    pat4 = PMfree (pat4);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_val_le_val_SxS( node *arg_node, info *arg_info)
 *
 * @brief Case 1. If both arguments are constant, compare them
 *                to determine if the guard is true.
 *
 *        Case 2. If both arguments are identical, the guard is true.
 *
 *        Case 3. Compare extrema. The guard is true if:
 *             (x <= minval(y))  || (maxval(x) <= y)
 *
 *        Case 4:
 *         iv', p   =_val_le_val_SxS_( iv, y);
 *         iv'', p' =_val_le_val_SxS_( iv', y);
 *
 *         I know this case looks silly, but it arises in
 *         SCCFprf_modarray12.sac, and is critical for
 *         loop fusion/array contraction performance.
 *         We can remove the second guard, on iv''.
 *
 *       Case 5: Constant AVIS_MINVAL( x) <= constant y.
 *
 *       Case 6: If this is an extended guard (GGS) , with PRF_ARG3 not NULL,
 *          and PRF_ARG3 is TRUE, perform the optimization,
 *          and PERHAPS? set AVIS_MAXVAL( res) = PRF_ARG2.
 *
 *       Case 7: The "sawing the board in two" optimization, where
 *          we observe that if we cut off part of a board, the
 *          resulting parts are both shorter than the original board.
 *          This arose in an attempt to make stdlib shift() AWLF.
 *          Cf. the AWLF unit tests.
 *
 *          We have this code for shift right (Case 7a):
 *
 *           count = _max_SxS_( 0, shiftcount);    // count >= 0
 *           shpa = _shape_A_( vec);               // shpa >= 0
 *           iv = shpa - count;
 *           iv', p = =_val_le_val_SxS( iv, shpa);
 *
 *          If (count >= 0) && (shpa >= 0), then:
 *            (shpa - count) <= shpa
 *
 *          Similarly, for shift left, we have (Case 7b):
 *
 *           count = _min_SxS_( -1, shiftcount);    // count < 0
 *           shpa = _shape_A_( vec);                // shpa >= 0
 *           iv = shpa + count;
 *           iv', p = =_val_le_val_SxS( iv, shpa);
 *
 *          If (count < 0) && (shpa >= 0), then:
 *            (shpa + count) <= shpa
 *
 *
 *****************************************************************************/
node *
SCSprf_val_le_val_SxS (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *val = NULL;
    node *val2 = NULL;
    node *val3 = NULL;
    node *res2;
    node *b;
    constant *con1 = NULL;
    constant *con2 = NULL;
    constant *con3 = NULL;
    constant *conrel = NULL;
    pattern *pat1;
    pattern *pat2;
    pattern *pat3;
    pattern *pat4;
    bool flg = FALSE;
    bool flg2;

    DBUG_ENTER ();

    pat1 = PMprf (1, PMAisPrf (F_val_le_val_SxS), 2, PMconst (1, PMAgetVal (&con1)),
                  PMconst (1, PMAgetVal (&con2), 0));

    pat2 = PMprf (1, PMAisPrf (F_val_le_val_SxS), 2, PMvar (1, PMAgetNode (&val), 0),
                  PMvar (1, PMAisVar (&val), 0));

    pat3 = PMprf (1, PMAisPrf (F_val_le_val_SxS), 2, PMvar (1, PMAgetNode (&val), 0),
                  PMvar (1, PMAgetNode (&val2), 0));

    pat4 = PMprf (1, PMAisPrf (F_val_le_val_SxS), 2, PMvar (1, PMAgetNode (&val3), 0),
                  PMvar (1, PMAisVar (&val2), 0));

    /* Cases 1 and 2 */
    flg2 = PMmatchFlat (pat2, arg_node);
    if (PMmatchFlat (pat1, arg_node)) {
        conrel = COle (con1, con2, NULL);
        flg2 = flg2 || ((NULL != conrel) && COisTrue (conrel, TRUE));
    }
    if (flg2) {
        res = TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                           TBmakeExprs (TBmakeBool (TRUE), NULL));
        DBUG_PRINT ("removed guard Case 1( %s, %s)",
                    AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                    AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node))));
    }
    con1 = (NULL != con1) ? COfreeConstant (con1) : con1;
    con2 = (NULL != con2) ? COfreeConstant (con2) : con2;
    conrel = (NULL != conrel) ? COfreeConstant (conrel) : conrel;

    /* Case 3 */
    if ((NULL == res) && (NULL != AVIS_MIN (ID_AVIS (PRF_ARG2 (arg_node))))) {
        res2 = SCSrecurseWithExtrema (arg_node, arg_info, PRF_ARG1 (arg_node),
                                      AVIS_MIN (ID_AVIS (PRF_ARG2 (arg_node))),
                                      &SCSprf_val_le_val_SxS);
        if (NULL != res2) {
            FREEdoFreeNode (res2);
            res = TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                               TBmakeExprs (TBmakeBool (TRUE), NULL));
            DBUG_PRINT ("removed guard Case 3a( %s, %s)",
                        AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                        AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node))));
        }
    }

    if ((NULL == res) && (NULL != AVIS_MAX (ID_AVIS (PRF_ARG1 (arg_node))))) {
        res2 = SCSrecurseWithExtrema (arg_node, arg_info,
                                      AVIS_MAX (ID_AVIS (PRF_ARG1 (arg_node))),
                                      PRF_ARG2 (arg_node), &SCSprf_val_le_val_SxS);
        if (NULL != res2) {
            FREEdoFreeNode (res2);
            res = TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                               TBmakeExprs (TBmakeBool (TRUE), NULL));
        }
    }

    /* Case 4:  */
    if ((NULL == res) && (PMmatchFlat (pat3, arg_node)) && (PMmatchFlat (pat4, val))) {
        res = TBmakeExprs (DUPdoDupNode (val3), TBmakeExprs (TBmakeBool (TRUE), NULL));
        DBUG_PRINT ("removed guard Case 4( %s -> %s)",
                    AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                    AVIS_NAME (ID_AVIS (val3)));
    }

    /* Case 5 */
    if (NULL == res) {
        b = SAACFonRelationalsWithExtrema (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node),
                                           arg_info, F_le_SxS);

        if ((NULL != b) && SCSisConstantOne (b)) {
            res = TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                               TBmakeExprs (TBmakeBool (TRUE), NULL));
            DBUG_PRINT ("removed guard Case 5( %s, %s)",
                        AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                        AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node))));
            b = (NULL != b) ? FREEdoFreeNode (b) : b;
        }
    }

    /* Case 6 */
    if ((NULL == res) && (NULL != PRF_EXPRS3 (arg_node))) {
        con3 = COaST2Constant (PRF_ARG3 (arg_node));
        if ((NULL != con3) && COisTrue (con3, TRUE)) {
            res = TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                               TBmakeExprs (TBmakeBool (TRUE), NULL));
            DBUG_PRINT ("removed guard Case 6( %s, %s)",
                        AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                        AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node))));
        }
        con3 = (NULL != con1) ? COfreeConstant (con3) : con3;
    }
    con1 = (NULL != con1) ? COfreeConstant (con1) : con1;
    con2 = (NULL != con2) ? COfreeConstant (con2) : con2;

    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);
    pat3 = PMfree (pat3);
    pat4 = PMfree (pat4);

    if (NULL == res) {
        res = SawingTheBoardInTwo (arg_node, arg_info);
    }

    if ((NULL == res)
        && canOptLEOnDyadicFn (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node), &flg)) {
        if (flg) {
            res = TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                               TBmakeExprs (TBmakeBool (TRUE), NULL));
        }
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_val_le_val_VxV( node *arg_node, info *arg_info)
 *
 * @brief Case 1. If both arguments are constant, compare them, and
 *                determine if the guard is true.
 *        Case 2. If both arguments are identical, the guard is true.
 *        Case 3. Compare extrema:
 *                  (x <= minval(y))  || (maxval(x) <= y)
 *        Case 4:
 *         iv', p   =_val_lt_val_SxS_( iv, y);
 *         iv'', p' =_val_lt_val_SxS_( iv', y);
 *
 *       Case 5: Constant AVIS_MINVAL( y) <= constant y.
 *               Or, AVIS_MINVAL(... AVIS_MINVAL( y)) <= constant y.
 *
 *****************************************************************************/
node *
SCSprf_val_le_val_VxV (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *val1 = NULL;
    node *val2 = NULL;
    node *val3 = NULL;
    node *res2;
    node *b;
    constant *con1 = NULL;
    constant *con2 = NULL;
    pattern *pat1;
    pattern *pat2;
    pattern *pat3;
    pattern *pat4;
    bool flg = FALSE;

    DBUG_ENTER ();

    pat1 = PMprf (1, PMAisPrf (F_val_le_val_VxV), 2, PMconst (1, PMAgetVal (&con1)),
                  PMconst (1, PMAgetVal (&con2), 0));

    pat2 = PMprf (1, PMAisPrf (F_val_le_val_VxV), 2, PMvar (1, PMAgetNode (&val1), 0),
                  PMvar (1, PMAisVar (&val1), 0));

    pat3 = PMprf (1, PMAisPrf (F_val_le_val_VxV), 2, PMvar (1, PMAgetNode (&val1), 0),
                  PMvar (1, PMAgetNode (&val2), 0));

    pat4 = PMprf (1, PMAisPrf (F_val_le_val_VxV), 2, PMvar (1, PMAgetNode (&val3), 0),
                  PMvar (1, PMAisVar (&val2), 0));

    /* Cases 1 and 2 */
    if ((PMmatchFlat (pat2, arg_node))
        || (PMmatchFlat (pat1, arg_node)
            && (COgetExtent (con1, 0) == COgetExtent (con2, 0))
            && COle (con1, con2, NULL))) {
        res = TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                           TBmakeExprs (TBmakeBool (TRUE), NULL));
        DBUG_PRINT_TAG ("SCS", "removed guard Case 1( %s, %s)",
                        AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                        AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node))));
    }
    con1 = (NULL != con1) ? COfreeConstant (con1) : con1;
    con2 = (NULL != con2) ? COfreeConstant (con2) : con2;

#ifdef DEADCODE
    /* Case 2 */
    if ((NULL == res)
        && (ID_AVIS (PRF_ARG1 (arg_node)) == ID_AVIS (PRF_ARG2 (arg_node)))) {
        res = TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                           TBmakeExprs (TBmakeBool (TRUE), NULL));
        DBUG_PRINT_TAG ("SCS", "removed guard Case 2( %s, %s)",
                        AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                        AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node))));
    }
#endif // DEADCODE

    /* Case 3a */
    if ((NULL == res) && (NULL != AVIS_MIN (ID_AVIS (PRF_ARG2 (arg_node))))) {
        res2 = SCSrecurseWithExtrema (arg_node, arg_info, PRF_ARG1 (arg_node),
                                      AVIS_MIN (ID_AVIS (PRF_ARG2 (arg_node))),
                                      &SCSprf_val_le_val_VxV);
        if (NULL != res2) {
            FREEdoFreeNode (res2);
            res = TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                               TBmakeExprs (TBmakeBool (TRUE), NULL));
            DBUG_PRINT_TAG ("SCS", "removed guard Case 3a( %s, %s)",
                            AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                            AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node))));
        }
    }

    /* Case 3b */
    if ((NULL == res) && (NULL != AVIS_MAX (ID_AVIS (PRF_ARG1 (arg_node))))) {
        res2 = SCSrecurseWithExtrema (arg_node, arg_info,
                                      AVIS_MAX (ID_AVIS (PRF_ARG1 (arg_node))),
                                      PRF_ARG2 (arg_node), &SCSprf_val_le_val_VxV);
        if (NULL != res2) {
            FREEdoFreeNode (res2);
            res = TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                               TBmakeExprs (TBmakeBool (TRUE), NULL));
            DBUG_PRINT_TAG ("SCS", "removed guard Case 3b( %s, %s)",
                            AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                            AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node))));
        }
    }

    /* Case 4:  */
    if ((NULL == res) && (PMmatchFlat (pat3, arg_node)) && (PMmatchFlat (pat4, val1))) {
        res = TBmakeExprs (DUPdoDupNode (val3), TBmakeExprs (TBmakeBool (TRUE), NULL));
        DBUG_PRINT_TAG ("SCS", "removed guard Case 4( %s -> %s)",
                        AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                        AVIS_NAME (ID_AVIS (val3)));
    }

    /* Case 5 */
    if (NULL == res) {
        b = SAACFonRelationalsWithExtrema (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node),
                                           arg_info, F_le_SxS);
        if ((NULL != b) && (SCSisConstantOne (b))) {
            res = TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                               TBmakeExprs (TBmakeBool (TRUE), NULL));
            DBUG_PRINT ("removed guard Case 5( %s, %s)",
                        AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                        AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node))));
            b = (NULL != b) ? FREEdoFreeNode (b) : b;
        }
    }

    if ((NULL == res)
        && canOptLEOnDyadicFn (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node), &flg)) {
        if (flg) {
            res = TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                               TBmakeExprs (TBmakeBool (TRUE), NULL));
        }
    }

    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);
    pat3 = PMfree (pat3);
    pat4 = PMfree (pat4);
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
 *   Two cases: Case 1: AVIS_SHAPE is an N_id
 *              Case 2: AVIS_SHAPE is an N_array
 *
 *****************************************************************************/
node *
SCSprf_prod_matches_prod_shape_VxA (node *arg_node, info *arg_info)
{
    node *res = NULL;
    pattern *pat1;
    pattern *pat2;
    node *shp;
    node *arr = NULL;

    DBUG_ENTER ();

    pat1 = PMprf (1, PMAisPrf (F_prod_matches_prod_shape_VxA), 2,
                  PMvar (1, PMAisVar (&shp), 0), PMskip (0));
    pat2 = PMarray (1, PMAgetNode (&arr), 1, PMskip (0));

    shp = AVIS_SHAPE (ID_AVIS (PRF_ARG2 (arg_node)));
    if (NULL != shp) {
        if ((N_id == NODE_TYPE (shp)) && /* Case 1 */
            (PMmatchFlat (pat1, arg_node))) {
            res = TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                               TBmakeExprs (TBmakeBool (TRUE), NULL));
            DBUG_PRINT ("Case 1 Result is %ss",
                        AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))));
        } else {
            if ((N_array == NODE_TYPE (shp)) && /* Case 2 */
                (PMmatchFlat (pat2, PRF_ARG1 (arg_node)))
                && (CMPT_EQ == CMPTdoCompareTree (shp, arr))) {
                res = TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                                   TBmakeExprs (TBmakeBool (TRUE), NULL));
                DBUG_PRINT ("Case 2 Result is %ss",
                            AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))));
            }
        }
    }
    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);

    DBUG_RETURN (res);
}

/** <!--*************************************************************-->
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

    DBUG_ENTER ();

#ifdef BUG524
/* This code should never have gone live. It absolutely trashes
 * the performance of MANY benchmarks, by about a factor of 3-4X.
 *
 */
#endif //  BUG524

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_idx_shape_sel( node *arg_node, info *arg_info)
 *
 * description:
 *
 *****************************************************************************/
node *
SCSprf_idx_shape_sel (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

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

    DBUG_ENTER ();

    pat1 = PMprf (1, PMAisPrf (F_reciproc_S), 1, PMvar (1, PMAgetNode (&arg1), 0));
    pat2 = PMprf (1, PMAisPrf (F_reciproc_S), 1, PMvar (1, PMAgetNode (&arg1p), 0));
    pat3 = PMprf (1, PMAisPrf (F_mul_SxS), 2, PMvar (1, PMAgetNode (&arg1p), 0),
                  PMvar (1, PMAgetNode (&arg2p), 0));

    if (PMmatchFlatSkipExtremaAndGuards (pat1, arg_node)) {
        if (PMmatchFlatSkipExtremaAndGuards (pat2, arg1)) {
            /* Case 1 */
            res = DUPdoDupNode (arg1p);
            DBUG_PRINT ("SCSprf_reciproc_S Case 1 replacing %s by %s",
                        AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                        AVIS_NAME (ID_AVIS (arg1p)));
        } else {
            if (PMmatchFlatSkipExtremaAndGuards (pat3, arg1)) {
                /* Case 2 */
                p1
                  = FLATGexpression2Avis (TCmakePrf1 (F_reciproc_S, DUPdoDupNode (arg1p)),
                                          &INFO_VARDECS (arg_info),
                                          &INFO_PREASSIGN (arg_info), NULL);
                // TYcopyType(( AVIS_TYPE( ID_AVIS( arg1p)))));
                p2
                  = FLATGexpression2Avis (TCmakePrf1 (F_reciproc_S, DUPdoDupNode (arg2p)),
                                          &INFO_VARDECS (arg_info),
                                          &INFO_PREASSIGN (arg_info), NULL);
                // TYcopyType(( AVIS_TYPE( ID_AVIS( arg1p)))));
                res = TCmakePrf2 (F_mul_SxS, TBmakeId (p1), TBmakeId (p2));
                DBUG_PRINT ("SCSprf_reciproc_S Case 2 replacing %s by %s",
                            AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                            AVIS_NAME (ID_AVIS (arg1p)));
            }
        }
    }
    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);
    pat3 = PMfree (pat3);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_reciproc_V( node *arg_node, info *arg_info)
 *
 * description:
 *
 *   Case 1: Replace _reciproc_V( _reciproc_V( V1)) by
 *                   V1
 *   Case 2: Replace _reciproc_V( V1 * V2)   by
 *                   _reciproc_V_( V1)  * _reciproc_V_( V2)
 *
 *****************************************************************************/
node *
SCSprf_reciproc_V (node *arg_node, info *arg_info)
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

    DBUG_ENTER ();

    pat1 = PMprf (1, PMAisPrf (F_reciproc_V), 1, PMvar (1, PMAgetNode (&arg1), 0));
    pat2 = PMprf (1, PMAisPrf (F_reciproc_V), 1, PMvar (1, PMAgetNode (&arg1p), 0));
    pat3 = PMprf (1, PMAisPrf (F_mul_VxV), 2, PMvar (1, PMAgetNode (&arg1p), 0),
                  PMvar (1, PMAgetNode (&arg2p), 0));

    if (PMmatchFlatSkipExtremaAndGuards (pat1, arg_node)) {
        if (PMmatchFlatSkipExtremaAndGuards (pat2, arg1)) {
            /* Case 1 */
            res = DUPdoDupNode (arg1p);
            DBUG_PRINT ("SCSprf_reciproc_V Case 1 replacing %s by %s",
                        AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                        AVIS_NAME (ID_AVIS (arg1p)));
        } else {
            if (PMmatchFlatSkipExtremaAndGuards (pat3, arg1)) {
                /* Case 2 */
                p1
                  = FLATGexpression2Avis (TCmakePrf1 (F_reciproc_V, DUPdoDupNode (arg1p)),
                                          &INFO_VARDECS (arg_info),
                                          &INFO_PREASSIGN (arg_info), NULL);
                // TYcopyType(( AVIS_TYPE( ID_AVIS( arg1p)))));
                p2
                  = FLATGexpression2Avis (TCmakePrf1 (F_reciproc_V, DUPdoDupNode (arg2p)),
                                          &INFO_VARDECS (arg_info),
                                          &INFO_PREASSIGN (arg_info), NULL);
                // TYcopyType(( AVIS_TYPE( ID_AVIS( arg1p)))));
                res = TCmakePrf2 (F_mul_VxV, TBmakeId (p1), TBmakeId (p2));
                DBUG_PRINT ("SCSprf_reciproc_V Case 2 replacing %s by %s",
                            AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                            AVIS_NAME (ID_AVIS (arg1p)));
            }
        }
    }

    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);
    pat3 = PMfree (pat3);

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

    DBUG_ENTER ();

    pat1 = PMprf (1, PMAisPrf (F_neg_S), 1, PMvar (1, PMAgetNode (&arg1), 0));
    pat2 = PMprf (1, PMAisPrf (F_neg_S), 1, PMvar (1, PMAgetNode (&arg1), 0));
    pat3 = PMprf (1, PMAisPrf (F_add_SxS), 2, PMvar (1, PMAgetNode (&arg1p), 0),
                  PMvar (1, PMAgetNode (&arg2p), 0));

    if (PMmatchFlatSkipExtremaAndGuards (pat1, arg_node)) {
        if (PMmatchFlatSkipExtremaAndGuards (pat2, arg1)) {
            /* Case 1 */
            res = DUPdoDupNode (arg1);
            DBUG_PRINT ("SCSprf_neg_S Case 1 replacing %s by %s",
                        AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                        AVIS_NAME (ID_AVIS (arg1)));
        } else {
            if (PMmatchFlatSkipExtremaAndGuards (pat3, arg1)) {
                /* Case 2 */
                p1 = FLATGexpression2Avis (TCmakePrf1 (F_neg_S, DUPdoDupNode (arg1p)),
                                           &INFO_VARDECS (arg_info),
                                           &INFO_PREASSIGN (arg_info), NULL);
                // TYcopyType(( AVIS_TYPE( ID_AVIS( arg1p)))));
                p2 = FLATGexpression2Avis (TCmakePrf1 (F_neg_S, DUPdoDupNode (arg2p)),
                                           &INFO_VARDECS (arg_info),
                                           &INFO_PREASSIGN (arg_info), NULL);
                // TYcopyType(( AVIS_TYPE( ID_AVIS( arg2p)))));
                res = TCmakePrf2 (F_add_SxS, TBmakeId (p1), TBmakeId (p2));
                DBUG_PRINT ("SCSprf_neg_S Case 2 replacing %s by %s",
                            AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                            AVIS_NAME (ID_AVIS (arg1p)));
            }
        }
    }
    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);
    pat3 = PMfree (pat3);

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

    DBUG_ENTER ();

    pat1 = PMprf (1, PMAisPrf (F_neg_V), 1, PMvar (1, PMAgetNode (&arg1), 0));
    pat2 = PMprf (1, PMAisPrf (F_neg_V), 1, PMvar (1, PMAgetNode (&arg1), 0));
    pat3 = PMprf (1, PMAisPrf (F_add_VxV), 2, PMvar (1, PMAgetNode (&arg1p), 0),
                  PMvar (1, PMAgetNode (&arg2p), 0));

    if (PMmatchFlatSkipExtremaAndGuards (pat1, arg_node)) {
        if (PMmatchFlatSkipExtremaAndGuards (pat2, arg1)) {
            /* Case 1*/

            res = DUPdoDupNode (arg1);
            DBUG_PRINT ("SCSprf_neg_V case 1 replacing %s by %s",
                        AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                        AVIS_NAME (ID_AVIS (arg1)));
        } else {
            if (PMmatchFlatSkipExtremaAndGuards (pat3, arg1)) {
                /* Case 2 */
                p1 = FLATGexpression2Avis (TCmakePrf1 (F_neg_V, DUPdoDupNode (arg1p)),
                                           &INFO_VARDECS (arg_info),
                                           &INFO_PREASSIGN (arg_info), NULL);
                // TYcopyType(( AVIS_TYPE( ID_AVIS( arg1p)))));
                p2 = FLATGexpression2Avis (TCmakePrf1 (F_neg_V, DUPdoDupNode (arg2p)),
                                           &INFO_VARDECS (arg_info),
                                           &INFO_PREASSIGN (arg_info), NULL);
                // TYcopyType(( AVIS_TYPE( ID_AVIS( arg2p)))));
                res = TCmakePrf2 (F_add_VxV, TBmakeId (p1), TBmakeId (p2));
                DBUG_PRINT ("SCSprf_neg_V Case 2 replacing %s by %s",
                            AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                            AVIS_NAME (ID_AVIS (arg1p)));
            }
        }
    }

    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);
    pat3 = PMfree (pat3);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_noteminval( node *arg_node, info *arg_info)
 *
 * description: If this function is placing an extremum on a node
 *              that already has the same extremum, scrap the
 *              function invocation.
 *
 *****************************************************************************/
node *
SCSprf_noteminval (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *arg1p;
    node *arg2p;
    pattern *pat;

    DBUG_ENTER ();

    pat = PMprf (1, PMAisPrf (F_noteminval), 2, PMvar (1, PMAgetNode (&arg1p), 0),
                 PMvar (1, PMAgetNode (&arg2p), 0));
    if ((PMmatchFlat (pat, arg_node)) && (NULL != AVIS_MIN (ID_AVIS (arg1p)))
        && (ID_AVIS (AVIS_MIN (ID_AVIS (arg1p))) == ID_AVIS (arg2p))) {
        res = DUPdoDupNode (arg1p);
        DBUG_PRINT ("Deleting nugatory F_noteminval for %s",
                    AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))));
    }

    pat = PMfree (pat);
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_notemaxval( node *arg_node, info *arg_info)
 *
 * description: If this function is placing an extremum on a node
 *              that already has the same extremum, scrap the
 *              function invocation.
 *
 *****************************************************************************/
node *
SCSprf_notemaxval (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *arg1p;
    node *arg2p;
    pattern *pat;

    DBUG_ENTER ();

    pat = PMprf (1, PMAisPrf (F_notemaxval), 2, PMvar (1, PMAgetNode (&arg1p), 0),
                 PMvar (1, PMAgetNode (&arg2p), 0));
    if ((PMmatchFlat (pat, arg_node)) && (NULL != AVIS_MAX (ID_AVIS (arg1p)))
        && (ID_AVIS (AVIS_MAX (ID_AVIS (arg1p))) == ID_AVIS (arg2p))) {
        res = DUPdoDupNode (arg1p);
        DBUG_PRINT ("Deleting nugatory F_notemaxval for %s",
                    AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))));
    }

    pat = PMfree (pat);
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_abs_S( node *arg_node, info *arg_info)
 *
 * description: If PRF_ARG1 has a constant minval >= 0,
 *              result is PRF_ARG1.
 *              If PRF_ARG1 has a constant (denormalized)  maxval < 0,
 *              result is  ( -PRF_ARG1).
 *
 *****************************************************************************/
node *
SCSprf_abs_S (node *arg_node, info *arg_info)
{
    node *res = NULL;
    constant *con1 = NULL;
    constant *con2;
    constant *denorm;
    pattern *pat;
    node *minmax;

    DBUG_ENTER ();

    pat = PMconst (1, PMAgetVal (&con1));
    minmax = AVIS_MIN (ID_AVIS (PRF_ARG1 (arg_node)));
    if ((NULL != minmax) && (PMmatchFlat (pat, minmax)) && (COisNonNeg (con1, TRUE))) {
        res = DUPdoDupNode (PRF_ARG1 (arg_node));
        DBUG_PRINT ("_abs_S_( %s) AVIS_MIN >= 0, so prf removed",
                    AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))));
    }
    con1 = (NULL != con1) ? COfreeConstant (con1) : NULL;

    minmax = AVIS_MAX (ID_AVIS (PRF_ARG1 (arg_node)));
    if ((NULL == res) && (NULL != minmax) && (PMmatchFlat (pat, minmax))) {
        denorm = COmakeConstantFromInt (1);
        con2 = COsub (con1, denorm, NULL);
        if (!COisNonNeg (con2, TRUE)) {
            res = TBmakeId (
              FLATGexpression2Avis (TCmakePrf1 (F_neg_S,
                                                DUPdoDupNode (PRF_ARG1 (arg_node))),
                                    &INFO_VARDECS (arg_info), &INFO_PREASSIGN (arg_info),
                                    NULL));
            // TYcopyType(( AVIS_TYPE( ID_AVIS( PRF_ARG1( arg_node)))))));
            DBUG_PRINT ("_abs_S_( %s) AVIS_MAX <0, so replaced by _neg_S_()",
                        AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))));
        }
        con2 = COfreeConstant (con2);
        denorm = COfreeConstant (denorm);
    }
    con1 = (NULL != con1) ? COfreeConstant (con1) : NULL;
    pat = PMfree (pat);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCSprf_abs_V( node *arg_node, info *arg_info)
 *
 * description: If PRF_ARG1 has all ( constant minval >= 0),
 *                result is PRF_ARG1.
 *              If PRF_ARG1 has all constant (denormalized)  maxval < 0,
 *              result is  ( -PRF_ARG1).
 *
 *****************************************************************************/
node *
SCSprf_abs_V (node *arg_node, info *arg_info)
{
    node *res = NULL;
    constant *con1 = NULL;
    constant *con2;
    constant *denorm;
    pattern *pat;
    node *minmax;

    DBUG_ENTER ();

    pat = PMconst (1, PMAgetVal (&con1));
    minmax = AVIS_MIN (ID_AVIS (PRF_ARG1 (arg_node)));
    if ((NULL != minmax) && (PMmatchFlat (pat, minmax)) && (COisNonNeg (con1, TRUE))) {
        res = DUPdoDupNode (PRF_ARG1 (arg_node));
    }
    con1 = (NULL != con1) ? COfreeConstant (con1) : NULL;

    minmax = AVIS_MAX (ID_AVIS (PRF_ARG1 (arg_node)));
    if ((NULL == res) && (NULL != minmax) && (PMmatchFlat (pat, minmax))) {
        denorm = COmakeConstantFromInt (1);
        con2 = COsub (con1, denorm, NULL);
        if (!COisNonNeg (con2, TRUE)) {
            res = TBmakeId (
              FLATGexpression2Avis (TCmakePrf1 (F_neg_V,
                                                DUPdoDupNode (PRF_ARG1 (arg_node))),
                                    &INFO_VARDECS (arg_info), &INFO_PREASSIGN (arg_info),
                                    NULL));
            // TYcopyType(( AVIS_TYPE( ID_AVIS( PRF_ARG1( arg_node)))))));
        }
        con2 = COfreeConstant (con2);
        denorm = COfreeConstant (denorm);
    }
    con1 = (NULL != con1) ? COfreeConstant (con1) : NULL;
    pat = PMfree (pat);

    DBUG_RETURN (res);
}

#undef DBUG_PREFIX
