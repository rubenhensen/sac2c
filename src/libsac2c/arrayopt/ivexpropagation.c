/*
 * $Id: ivexpropagation.c 15815 2008-10-24 18:04:47Z rbe $
 */

/** <!--********************************************************************-->
 *
 * @defgroup ivexp Index Vector Extrema Propagation Traversal
 *
 * @brief:
 *
 *  This code propagates extrema (AVIS_MINVAL and AVIS_MAXVAL) through
 *  assigns and, where we are able to do so, primitive functions.
 *
 *  Details:
 *    Assigns: RHS extrema are copied to LHS.
 *
 *    F_attachextrema: PRF_ARG1's extrema are copied to result.
 *
 *    Primitives:
 *
 *      Several cases:
 *
 *      1. If the primitive result has known extrema, we do nothing.
 *
 *      2. If it has one known extrema, I am confused.
 *
 *      3. If the primitive result has unknown extrema, but
 *         the function has an _attachextrema attached to it,
 *         as below, we propagate minv' and maxv into
 *         the extrema of the primitive function's result,
 *         and replace ivextrema in the original primitive
 *         by iv'.
 *
 *      4.  If the primitive result has unknown extrema,
 *          but the primitive arguments have known extrema,
 *          we generate new code to clone the primitive so that
 *          it operates on the extrema. E.g., if we have (this
 *          from sac/apex/iotan/iotan.sac):
 *
 *             iv'' =  _sub_SxS_( 49999999, iv');
 *
 *          with minv = AVIS_MINVAL( iv') and
 *               maxv = AVIS_MAXVAL( iv')
 *
 *          We turn this into:
 *
 *             minv' = _sub_SxS_( 49999999, minv);
 *             maxv' = _sub_SxS_( 49999999, maxv);
 *             ivextrema = _attachextrema( iv', minv', maxv');
 *             iv'' = _sub_SxS_( 49999999, ivextrema);
 *
 *          After enough trips through the optimization cycle,
 *          this code should turn into case 4.
 *
 *****************************************************************************
 *
 * @ingroup ivexp
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file ivexpropagation.c
 *
 * Prefix: IVEXP
 *
 *****************************************************************************/
#include "ivexpropagation.h"

/*
 * Other includes go here
 */
#include "dbug.h"
#include "traverse.h"
#include "free.h"
#include "tree_basic.h"
#include "memory.h"
#include "new_types.h"
#include "type_utils.h"
#include "constants.h"
#include "tree_compound.h"
#include "pattern_match.h"
#include "ivextrema.h"
#include "symb_wlfi.h"
#include "DupTree.h"
#include "check.h"
#include "phase.h"
#include "shape.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    node *minval;
    node *maxval;
    node *preassigns;
    node *vardecs;
    node *curwith;
};

/**
 * INFO macros
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_MINVAL(n) ((n)->minval)
#define INFO_MAXVAL(n) ((n)->maxval)
#define INFO_PREASSIGNS(n) ((n)->preassigns)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_CURWITH(n) ((n)->curwith)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_MINVAL (result) = NULL;
    INFO_MAXVAL (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_CURWITH (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}
/** <!--********************************************************************-->
 * @}  <!-- INFO structure -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *IVEXPdoIndexVectorExtremaProp( node *arg_node)
 *
 * @brief: Perform index vector extrema propagation on a module.
 *
 *****************************************************************************/
node *
IVEXPdoIndexVectorExtremaPropModule (node *arg_node)
{
    info *arg_info;
    DBUG_ENTER ("IVEXPdoIndexVectorExtremaPropModule");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_module),
                 "IVEXPdoIndexVectorExtremaPropModule expected N_module");

    arg_info = MakeInfo ();

    DBUG_PRINT ("IVEXP", ("Starting index vector extrema propagation traversal."));

    TRAVpush (TR_ivexp);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    DBUG_PRINT ("IVEXP", ("Index vector extrema propagation complete."));

    arg_info = FreeInfo (arg_info);
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/******************************************************************************
 *
 * function: node *makeNarray( node *extrema, info *arg_info, int arrxrho)
 *
 * description: Create an N_array from an N_exprs chain at extrema,
 *              of frame shape arrxrho. Also creates an assign and
 *              vardec for same.
 *
 * @params:  extrema: an N_exprs chain.
 *           arg_info: Your basic arg_info node.
 *           arrxrho: The frame shape for the N_array.
 * @result:
 *           The N_avis for the shiny new N_array.
 *
 *****************************************************************************/
static node *
makeNarray (node *extrema, info *arg_info, int arrxrho)
{
    node *resavis;
    node *narr;
    node *zavis;

    DBUG_ENTER ("makeNarray");

    narr = TBmakeArray (TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)),
                        SHcreateShape (1, arrxrho), extrema);
    ARRAY_NOEXTREMAWANTED (narr) = TRUE;

    resavis = TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeSimpleType (T_int),
                                                    SHcreateShape (1, arrxrho)));
    if (isSAAMode ()) {
        AVIS_DIM (resavis) = TBmakeNum (1);
        AVIS_SHAPE (resavis) = TCmakeIntVector (TBmakeExprs (TBmakeNum (arrxrho), NULL));
        ;
    }

    INFO_VARDECS (arg_info) = TBmakeVardec (resavis, INFO_VARDECS (arg_info));

    zavis = SWLFIflattenExpression (narr, &INFO_VARDECS (arg_info),
                                    &INFO_PREASSIGNS (arg_info), resavis);
    DBUG_RETURN (zavis);
}

/******************************************************************************
 *
 * function:
 *   bool isAvisHasExtrema( node *avis)
 *
 * description: Predicate for determining if an N_avis node has known extrema.
 *
 * @params  arg_node: an N_avis node.
 * @result: True if the node has both extrema present.
 *
 ******************************************************************************/
static bool
isAvisHasExtrema (node *avis)
{
    bool z;

    DBUG_ENTER ("isAvisHasExtrema");
    z = (NULL != AVIS_MINVAL (avis)) || (NULL != AVIS_MAXVAL (avis));

    DBUG_RETURN (z);
}

/******************************************************************************
 *
 * function:
 * static
 *   bool isArgExtremaAttach( node *arg_node)
 *
 * description: Predicate for determining if N_id arg_node has
 *              an F_attachextrema at its SSAASSIGN.LHS
 *
 * @params  arg_node: an N_id node.
 * @result: True if we have already issued an F_attachextrema for N_id.
 *
 ******************************************************************************/
static bool
isArgExtremaAttached (node *arg_node)
{
    node *assgn;
    bool z;

    DBUG_ENTER ("isArgExtremaAttached");

    assgn = AVIS_SSAASSIGN (ID_AVIS (arg_node));

    z = (NULL != assgn) && (N_prf == NODE_TYPE (LET_EXPR (ASSIGN_INSTR (assgn))))
        && (F_attachextrema == PRF_PRF (LET_EXPR (ASSIGN_INSTR (assgn))));

    DBUG_RETURN (z);
}

/******************************************************************************
 *
 * function:
 *   bool isPrfArgHasKnownExtrema( node *arg_node, info *arg_info)
 *
 * description: Predicate for determining if PRF_ARG in arg_node has
 *              F_attachextrema attached, with known extrema,
 *              attached to the F_attachextrema as
 *              PRF_ARG2 and PRF_ARG3, or if the F_attachextrema
 *              result is a constant, in which case it acts
 *              in the same role.
 *
 * @params  arg_node: an N_id node.
 * @result: True if the N_id derives directly from an F_attachextrema,
 *          and the F_attachextrema primitive result has both extrema known.
 *
 *          Side effect is to set INFO_MINVAL and INFO_MAXVAL.
 *
 ******************************************************************************/
static bool
isPrfArgHasKnownExtrema (node *arg_node, info *arg_info)
{
    node *ids;
    bool z = FALSE;

    DBUG_ENTER ("isPrfArgHasKnownExtrema");
    if ((N_id == NODE_TYPE (arg_node))) {
        if (isArgExtremaAttached (arg_node)) {
            ids = LET_IDS (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (arg_node))));
            if ((NULL != AVIS_MINVAL (IDS_AVIS (ids)))
                && (NULL != AVIS_MAXVAL (IDS_AVIS (ids)))) {
                z = TRUE;
                INFO_MINVAL (arg_info) = AVIS_MINVAL (IDS_AVIS (ids));
                INFO_MAXVAL (arg_info) = AVIS_MAXVAL (IDS_AVIS (ids));
            }
            if (COisConstant (ids)) {
                z = TRUE;
                INFO_MINVAL (arg_info) = IDS_AVIS (ids);
                INFO_MAXVAL (arg_info) = IDS_AVIS (ids);
            }
        }
    }

    DBUG_RETURN (z);
}

/******************************************************************************
 *
 * function:
 *   node *PropagateNarray node *arg_node, info *arg_info)
 *
 * description:
 *        Propagate N_array extrema to LHS.
 *
 * @params  arg_node: an N_let node pointing to an N_array node.
 *
 * @result: Consider an N_array:
 *
 *               V = [ I, J, K];
 *
 *          If all N_array elements have non_null extrema,
 *          build LHS extrema values as follows:
 *
 *            minv = AVIS_MINVAL( I), AVIS_MINVAL( J), AVIS_MINVAL( K)
 *            maxv = AVIS_MAXVAL( I), AVIS_MAXVAL( J), AVIS_MAXVAL( K)
 *            I' = _attachextrema( I, minv, maxv);
 *            NB. minv and maxv are N_exprs chains, actually, in
 *            NB. the guard.
 *            V = [ I', J, K];   NB. With minv and maxv as extrema of V.
 *
 *          The problem we have here is being able to look at the V''
 *          N_array and show that we have already generated an
 *          _attachextrema_ for it. Chaining any N_array
 *          element through that guard should suffice.
 *
 ******************************************************************************/
static node *
PropagateNarray (node *arg_node, info *arg_info)
{
    node *v;
    node *lhsavis;
    node *minv = NULL;
    node *maxv = NULL;
    node *exprs;
    node *aelemI;
    node *Iprime;
    node *newexprs;
    bool allex;
    int arrxrho;

    DBUG_ENTER ("PropagateNarray");

    v = LET_EXPR (arg_node);

    /* Check that we have extrema for all array elements.
     * We allow N_id elements only. All others are rejected.
     * We may have to come back and extend this to support N_num nodes...
     */
    allex = TRUE;
    exprs = ARRAY_AELEMS (v);
    DBUG_ASSERT (NULL != exprs, ("PropagateNarray got empty array joke"));
    allex = (NULL != exprs); /* No funny biz with [:int] */
    while (allex && (exprs != NULL)) {
        if (N_id == NODE_TYPE (EXPRS_EXPR (exprs))) {
            allex = allex & isAvisHasExtrema (ID_AVIS (EXPRS_EXPR (exprs)));
        } else {
            allex = FALSE;
        }
        exprs = EXPRS_NEXT (exprs);
    }
    if (allex) {
        /* Build exprs chains of minima and maxima. */
        exprs = ARRAY_AELEMS (v);
        while (exprs != NULL) {
            minv
              = TBmakeExprs (TBmakeId (AVIS_MINVAL (ID_AVIS (EXPRS_EXPR (exprs)))), minv);
            maxv
              = TBmakeExprs (TBmakeId (AVIS_MAXVAL (ID_AVIS (EXPRS_EXPR (exprs)))), maxv);
            exprs = EXPRS_NEXT (exprs);
        }

        /* At this point, we have two N_exprs chains of extrema.
         * Make these N_array nodes and give them names, then attach
         * them, via dataflow, to the N_let LHS.
         */
        arrxrho = SHgetUnrLen (ARRAY_FRAMESHAPE (v));
        lhsavis = IDS_AVIS (LET_IDS (arg_node));
        minv = makeNarray (minv, arg_info, arrxrho);
        maxv = makeNarray (maxv, arg_info, arrxrho);

        /*     I' = _attachextrema( I, minv, maxv);
         *      V = [I', J, K];      NB. Decorated with minv, maxv.
         */
        aelemI = DUPdoDupNode (EXPRS_EXPR (ARRAY_AELEMS (v)));
        DBUG_ASSERT (N_id == NODE_TYPE (aelemI),
                     ("PropagateNarray expected N_id in N_array"));

        Iprime = TBmakeId (IVEXIattachExtrema (TBmakeId (minv), TBmakeId (maxv), aelemI,
                                               &INFO_VARDECS (arg_info),
                                               &INFO_PREASSIGNS (arg_info)));

        /*
         *       V = [ I', J, K]; NB. With extrema on V.
         *
         */

        newexprs = DUPdoDupTree (ARRAY_AELEMS (v));
        FREEdoFreeNode (EXPRS_EXPR (newexprs));
        EXPRS_EXPR (newexprs) = Iprime;

        ARRAY_AELEMS (LET_EXPR (arg_node))
          = FREEdoFreeTree (ARRAY_AELEMS (LET_EXPR (arg_node)));
        ARRAY_AELEMS (LET_EXPR (arg_node)) = newexprs;

        AVIS_MINVAL (lhsavis) = minv;
        AVIS_MAXVAL (lhsavis) = maxv;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   bool isResultHasExtrema( node *arg_node, info *arg_info)
 *
 * description: Predicate for determining if LHS of N_let node
 *              has known extrema.
 *
 * @params  arg_node: an N_let node.
 * @result: True if the LHS has known extrema.
 *
 ******************************************************************************/
static bool
isResultHasExtrema (node *arg_node, info *arg_info)
{
    bool z;

    DBUG_ENTER ("isResultHasExtrema");
    z = isAvisHasExtrema (IDS_AVIS (LET_IDS (arg_node)));

    DBUG_RETURN (z);
}

/******************************************************************************
 *
 * function:
 * static
 * node *ExtractedNarrayExtrema( node *arg_node, info *arg_info)
 *
 * description:
 *    The N_let arg_node RHS is an N_array.
 *    If element N_array[0] derives immediately from
 *    an F_attachextrema, AND its PRF_ARG1 and PRF_ARG2 have different
 *    ranks, then the extrema on the F_attachextrema belong to the
 *    N_array LHS, and NOT to N_array[0].
 *
 *    A cleaner approach might be to give this case its own N_prf:
 *    F_attachnarrayextrema.
 *
 * @params  arg_node: an N_let node.
 * @result:  Pointer to N_prf of the F_attachextrema, if it
 *           is of righ one. Else NULL.
 *
 ******************************************************************************/
static node *
ExtractedNarrayExtrema (node *arg_node, info *arg_info)
{
    node *prf;
    node *el0;
    node *z = NULL;

    DBUG_ENTER ("ExtractedNarrayExtrema");

    el0 = EXPRS_EXPR (ARRAY_AELEMS (LET_EXPR (arg_node)));
    if (N_id == NODE_TYPE (el0)) {
        el0 = AVIS_SSAASSIGN (ID_AVIS (el0));
        if ((NULL != el0) && /* missing SSAASSIGN in N_ap parameter today FIXME */
            (N_let == NODE_TYPE (ASSIGN_INSTR (el0)))) {
            prf = LET_EXPR (ASSIGN_INSTR (el0));
            if ((N_prf == NODE_TYPE (prf)) && (F_attachextrema == PRF_PRF (prf))
                && ((TUisScalar (AVIS_TYPE (ID_AVIS (PRF_ARG1 (prf)))))
                    != (TUisScalar (AVIS_TYPE (ID_AVIS (PRF_ARG2 (prf))))))) {
                z = prf;
            }
        }
    }
    DBUG_RETURN (z);
}

/******************************************************************************
 *
 * function:
 *   bool PrfExtractExtrema( node *arg_node, info *arg_info)
 *
 * description: Extracts extrema from within a primitive.
 *              We know that the LET_LHS does not have extrema.
 *
 * @params  arg_node: an N_prf node.
 * @result: True if we have found extrema, and stored
 *          their values in INFO_MINVAL and INFO_MAXVAL.
 *          Caller will populate LHS avis appropriately.
 *
 ******************************************************************************/
static bool
PrfExtractExtrema (node *arg_node, info *arg_info)
{
    node *avis;
    bool z = FALSE;
    bool arg1c;
    bool arg2c;

    DBUG_ENTER ("PrfExtractExtrema");

    INFO_MINVAL (arg_info) = NULL;
    INFO_MAXVAL (arg_info) = NULL;

    switch (PRF_PRF (arg_node)) {

    case F_attachintersect:
        /* The extrema of PRF_ARG1 become the extrema of the LHS */
        avis = ID_AVIS (PRF_ARG1 (arg_node));
        INFO_MINVAL (arg_info) = AVIS_MINVAL (avis);
        INFO_MAXVAL (arg_info) = AVIS_MAXVAL (avis);
        z = (NULL != INFO_MINVAL (arg_info)) && (NULL != INFO_MAXVAL (arg_info));
        break;

    case F_attachextrema:
        /* We have to be careful here: It might be better to give
         * N_array extrema calc its own guard. However,
         * for now, we'll try the cheat: If this is an N_array
         * guard, the ranks of PRF_ARG1 and PRG_ARG2 will
         * be different. For run of the mill stuff, they'll be the same.
         */
        if ((TUisScalar (AVIS_TYPE (ID_AVIS (PRF_ARG1 (arg_node)))))
            == (TUisScalar (AVIS_TYPE (ID_AVIS (PRF_ARG2 (arg_node)))))) {
            INFO_MINVAL (arg_info) = ID_AVIS (PRF_ARG2 (arg_node));
            INFO_MAXVAL (arg_info) = ID_AVIS (PRF_ARG3 (arg_node));
            z = (NULL != INFO_MINVAL (arg_info)) && (NULL != INFO_MAXVAL (arg_info));
        }
        break;

    /* These cases are all dyadic functions */
    case F_add_SxS:
    case F_add_SxV:
    case F_add_VxS:
    case F_add_VxV:

    case F_and_SxS:
    case F_and_SxV:
    case F_and_VxS:
    case F_and_VxV:

    case F_or_SxS:
    case F_or_SxV:
    case F_or_VxS:
    case F_or_VxV:

    case F_mul_SxS:
    case F_mul_SxV:
    case F_mul_VxS:
    case F_mul_VxV:

    case F_min_SxS:
    case F_min_SxV:
    case F_min_VxS:
    case F_min_VxV:

    case F_max_SxS:
    case F_max_SxV:
    case F_max_VxS:
    case F_max_VxV:

    case F_sub_SxS:
    case F_sub_SxV:
    case F_sub_VxS:
    case F_sub_VxV:

        /* This is dumb. Should use prf_info.mac table. */

        /*  The result is known not to have extrema.
         * If one argument is constant, and the other has extrema,
         * and no extrema-computing code exists,
         * we will introduce code to propagate the extrema.
         *
         */
        arg1c = COisConstant (PRF_ARG1 (arg_node));
        arg2c = COisConstant (PRF_ARG2 (arg_node));

        if (arg1c != arg2c) {

            if (arg1c && (PRF_EXTREMAATTACHED (arg_node))
                && (NULL != AVIS_MINVAL (ID_AVIS (PRF_ARG2 (arg_node))))
                && (isPrfArgHasKnownExtrema (PRF_ARG2 (arg_node), arg_info))) {
                z = TRUE;
            }

            if (arg2c && (PRF_EXTREMAATTACHED (arg_node))
                && (NULL != AVIS_MINVAL (ID_AVIS (PRF_ARG1 (arg_node))))
                && (isPrfArgHasKnownExtrema (PRF_ARG1 (arg_node), arg_info))) {
                z = TRUE;
            }

            DBUG_PRINT ("IVEXP", ("Introduced extrema code for N_prf"));
        }
        break;

    /* These cases are ISMOP */
    case F_tob_S:
    case F_toi_S:
    case F_toc_S:
    case F_tof_S:
    case F_tod_S:

    case F_not_S:
    case F_not_V:

    case F_neg_S:
    case F_neg_V:

    case F_abs_S:
    case F_abs_V:

#ifdef FIXME
    case F_non_neg_val:
        /* We should insert some _max_ code here if we
         * know the AVIS_MINVAL of PRF_ARG1. However, if
         * we don't know it, we can still make some progress here.
         */
        avis = ID_AVIS (PRF_ARG1 (arg_node));
        if (NULL == AVIS_MINVAL (avis)) {
            INFO_MINVAL (arg_info) = ID_AVIS (makemeaZERO (avis));
            z = TRUE;
            break;

#endif // FIXME

            DBUG_PRINT ("IVEXP", ("ISMOP extrema code for N_prf"));
            break;

        default:
            break;
        }

        DBUG_RETURN (z);
    }
    /*
     * For other cases, such as F_non_negval, we merely
     * simplify the expression in the absence of extrema
     * info.
     *
     */

    /** <!--********************************************************************-->
     *
     * static
     * node *IntroducePrfExtremaCalc(node *arg_node, info *arg_info)
     *
     * description:
     *      For some primitives, where CF and friends are able to perform
     *      expression simplification, we insert code to (attempt to)
     *      compute the extrema for this function application.
     *
     *      If this insertion is already done, this function is an identity.
     *
     * Arguments:
     *      arg_node: N_let for the N_prf.
     *
     * Results:
     *      The updated arg_node, with iv' -> iv''.
     *
     *      INFO_VARDECS are appended to, with entries for:
     *         minv, maxv, and iv''.
     *
     *      INFO_PREASSIGNS are appended to, with the assigns for
     *        minv, maxv, and iv''.
     *
     * Restrictions:
     *      A subset of N_prfs only are supported.
     *      The N_prf extrema must have a chance of being computed by the optimizer.
     *      We require that, for dyadic prfs, that one argument be
     *      constant and the other be non-constant, with both extrema present.
     *
     *      It may be possible to relax some of these restrictions with
     *      ISMOP or better insight. Right now, we're just trying to
     *      make it work.
     *
     * Example:
     *
     *      The N_prf is:
     *          offset = 42;
     *          lhs = _sub_SxS_( offset, iv');
     *
     *      We know that iv' has extrema attached, so we transform this
     *      into:
     *
     *          offset = 42;
     *          minv = _sub_SxS__( offset, AVIS_MINVAL( iv'));
     *          maxv = _sub_SxS__( offset, AVIS_MAXVAL( iv'));
     *          iv'' = _attachextrema( iv', minv, maxv);
     *          lhs = _sub_SxS_( offset, iv'');
     *
     ******************************************************************************/
    static node *IntroducePrfExtremaCalc (node * arg_node, info * arg_info)
    {
        node *rhs;
        node *z;
        node *minv;
        node *maxv;
        node *minarg1;
        node *minarg2;
        node *maxarg1;
        node *maxarg2;
        node *lhsavis;
        node *nca = NULL;
        bool arg1c;
        bool arg2c;
        bool docalc = FALSE;

        DBUG_ENTER ("IntroducePrfExtremaCalc");

        rhs = LET_EXPR (arg_node);

        /* First, we decide if we should insert the extrema calculation code. */
        if ((N_prf == NODE_TYPE (rhs)) && (!PRF_NOEXTREMAWANTED (rhs))
            && (!PRF_EXTREMAATTACHED (rhs))) {
            switch
                PRF_PRF (rhs)
                { /* sheep vs. goats - ignore oddball N_prfs.*/
                default:
                    break;

                    /* These cases are all dyadic functions */
                case F_sub_SxS:
                case F_sub_SxV:
                case F_sub_VxS:
                case F_sub_VxV:

                case F_add_SxS:
                case F_add_SxV:
                case F_add_VxS:
                case F_add_VxV:

                case F_and_SxS:
                case F_and_SxV:
                case F_and_VxS:
                case F_and_VxV:

                case F_or_SxS:
                case F_or_SxV:
                case F_or_VxS:
                case F_or_VxV:

                case F_mul_SxS:
                case F_mul_SxV:
                case F_mul_VxS:
                case F_mul_VxV:

                case F_min_SxS:
                case F_min_SxV:
                case F_min_VxS:
                case F_min_VxV:

                case F_max_SxS:
                case F_max_SxV:
                case F_max_VxS:
                case F_max_VxV:

                    arg1c = COisConstant (PRF_ARG1 (rhs));
                    arg2c = COisConstant (PRF_ARG2 (rhs));
                    if (arg1c != arg2c) { /* One constant, one non-constant */

                        nca = arg1c ? PRF_ARG2 (rhs) : PRF_ARG1 (rhs);
                        docalc = isAvisHasExtrema (ID_AVIS (nca));
                    }
                    break;
                }
        }

        /* Now, we either do it or not. */
        if (docalc) {
            DBUG_PRINT ("IVEXP", ("Introducing N_prf extrema calc for %s",
                                  AVIS_NAME (IDS_AVIS (LET_IDS (arg_node)))));

            PRF_EXTREMAATTACHED (rhs) = TRUE;

            lhsavis = IDS_AVIS (LET_IDS (arg_node));

            minarg1 = arg1c ? DUPdoDupTree (PRF_ARG1 (rhs))
                            : TBmakeId (AVIS_MINVAL (ID_AVIS (PRF_ARG1 (rhs))));
            minarg2 = arg2c ? DUPdoDupTree (PRF_ARG2 (rhs))
                            : TBmakeId (AVIS_MINVAL (ID_AVIS (PRF_ARG2 (rhs))));

            minv = TCmakePrf2 (PRF_PRF (rhs), minarg1, minarg2);
            PRF_NOEXTREMAWANTED (minv) = TRUE;

            minv = SWLFIflattenExpression (minv, &INFO_VARDECS (arg_info),
                                           &INFO_PREASSIGNS (arg_info), lhsavis);

            maxarg1 = arg1c ? DUPdoDupTree (PRF_ARG1 (rhs))
                            : TBmakeId (AVIS_MAXVAL (ID_AVIS (PRF_ARG1 (rhs))));

            maxarg2 = arg2c ? DUPdoDupTree (PRF_ARG2 (rhs))
                            : TBmakeId (AVIS_MAXVAL (ID_AVIS (PRF_ARG2 (rhs))));

            maxv = TCmakePrf2 (PRF_PRF (rhs), maxarg1, maxarg2);
            PRF_NOEXTREMAWANTED (maxv) = TRUE;

            maxv = SWLFIflattenExpression (maxv, &INFO_VARDECS (arg_info),
                                           &INFO_PREASSIGNS (arg_info), lhsavis);

            switch
                PRF_PRF (rhs)
                {

                /* These cases are all dyadic functions */
                case F_add_SxS:
                case F_add_SxV:
                case F_add_VxS:
                case F_add_VxV:

                case F_and_SxS:
                case F_and_SxV:
                case F_and_VxS:
                case F_and_VxV:

                case F_or_SxS:
                case F_or_SxV:
                case F_or_VxS:
                case F_or_VxV:

                case F_mul_SxS:
                case F_mul_SxV:
                case F_mul_VxS:
                case F_mul_VxV:

                case F_min_SxS:
                case F_min_SxV:
                case F_min_VxS:
                case F_min_VxV:

                case F_max_SxS:
                case F_max_SxV:
                case F_max_VxS:
                case F_max_VxV:

                    z = TBmakeId (IVEXIattachExtrema (TBmakeId (minv), TBmakeId (maxv),
                                                      DUPdoDupNode (nca),
                                                      &INFO_VARDECS (arg_info),
                                                      &INFO_PREASSIGNS (arg_info)));
                    break;

                case F_sub_SxS:
                case F_sub_SxV:
                case F_sub_VxS:
                case F_sub_VxV:
                    z = TBmakeId (IVEXIattachExtrema (TBmakeId (maxv), TBmakeId (minv),
                                                      DUPdoDupNode (nca),
                                                      &INFO_VARDECS (arg_info),
                                                      &INFO_PREASSIGNS (arg_info)));
                    break;

                default:
                    z = NULL; /* Keep gcc happy */
                    DBUG_ASSERT (FALSE, "IVEXP missed a case.");
                    break;
                }

            DBUG_PRINT ("IVEXP", ("attached Extrema for prf."));
            if (arg1c) {
                FREEdoFreeNode (PRF_ARG2 (LET_EXPR (arg_node)));
                PRF_ARG2 (LET_EXPR (arg_node)) = z;
            } else {
                FREEdoFreeNode (PRF_ARG1 (LET_EXPR (arg_node)));
                PRF_ARG1 (LET_EXPR (arg_node)) = z;
            }
        }

        DBUG_RETURN (arg_node);
    }

    /** <!--********************************************************************-->
     *
     * @name Traversal functions
     * @{
     *
     *****************************************************************************/

    /******************************************************************************
     *
     * function:
     *   node *IVEXPassign( node *arg_node, info *arg_info)
     *
     * description:
     *
     *   Place preassigns in their proper locations.
     *
     *   This is slightly tricky, because we start with:
     *
     *    arg_node-> instr -> next
     *
     *   and want to end up with:
     *
     *    arg_node -> preassigns -> next
     *
     ******************************************************************************/
    node *IVEXPassign (node * arg_node, info * arg_info)
    {
        node *new_arg_node;

        DBUG_ENTER ("IVEXPassign");

        new_arg_node = arg_node;

        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

        if (NULL != INFO_PREASSIGNS (arg_info)) {
            new_arg_node = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
            INFO_PREASSIGNS (arg_info) = NULL;
        }

        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

        DBUG_RETURN (new_arg_node);
    }

    /******************************************************************************
     *
     * function:
     *   node *IVEXPlet( node *arg_node, info *arg_info)
     *
     * description:
     *
     *    0. If result (LHS) already has extrema, do nothing.
     *    1. Propagate iv extrema from RHS to LHS. This
     *       is a bit subtle, or at least crude. See comments
     *       at PropagateNarray for the ast structure when
     *       we can perform propagation.
     *
     *    2. If extrema exist for one argument, and other argument is
     *       constant, introduce code to compute extrema for this LHS.
     *
     ******************************************************************************/
    node *IVEXPlet (node * arg_node, info * arg_info)
    {
        node *rhs;
        node *lhsavis;
        node *rhsavis;
        node *extr;

        DBUG_ENTER ("IVEXPlet");

        lhsavis = IDS_AVIS (LET_IDS (arg_node));

#ifdef VERBOSE
        DBUG_PRINT ("IVEXP", ("Found let for %s", AVIS_NAME (lhsavis)));
#endif // VERBOSE

        rhs = LET_EXPR (arg_node);

        /* If we know the answer already, or not inside WL, do nothing */
        if ((!isResultHasExtrema (arg_node, arg_info))) {

            switch (NODE_TYPE (rhs)) {
            case N_id:
                rhsavis = ID_AVIS (rhs);
                if ((NULL != AVIS_MINVAL (rhsavis)) || (NULL != AVIS_MAXVAL (rhsavis))) {
                    DBUG_PRINT ("IVEXP", ("IVEXP N_id: propagating extrema from %s to %s",
                                          AVIS_NAME (rhsavis), AVIS_NAME (lhsavis)));
                    AVIS_MINVAL (lhsavis) = AVIS_MINVAL (rhsavis);
                    AVIS_MAXVAL (lhsavis) = AVIS_MAXVAL (rhsavis);
                }
                break;

            case N_prf:
                if (PrfExtractExtrema (rhs, arg_info)) {
                    DBUG_PRINT ("IVEXP", ("IVEXP N_prf: propagating extrema to lhs %s",
                                          AVIS_NAME (lhsavis)));
                    AVIS_MINVAL (lhsavis) = INFO_MINVAL (arg_info);
                    AVIS_MAXVAL (lhsavis) = INFO_MAXVAL (arg_info);
                    DBUG_ASSERT (N_avis == NODE_TYPE (AVIS_MINVAL (lhsavis)),
                                 ("PrfExtractExtrema returned non-avis minval"));
                    DBUG_ASSERT (N_avis == NODE_TYPE (AVIS_MAXVAL (lhsavis)),
                                 ("PrfExtractExtrema returned non-avis maxval"));
                } else {
                    /* Could not extract extrema from RHS.
                     * We may introduce extrema-computation code here.
                     * But, only within a WL.
                     */
                    if (NULL != INFO_CURWITH (arg_info)) {
                        arg_node = IntroducePrfExtremaCalc (arg_node, arg_info);
                    }
                }
                break;

            /* Constant RHS */
            case N_bool:
            case N_char:
            case N_num:
            case N_float:
            case N_double:
                /*
                 * We no longer generate/propagate extrema into constants.
                 *
                 * The idea now (2009-05-21) is this:
                 *
                 *   - We introduce extrema only into WL IVs.
                 *   - N_let: We propagate extrema, via assignment, into other avis nodes.
                 *   - N_prf: if one argument of a dyadic scalar function has
                 *            extrema, and the other argument is constant, we
                 *            will insert code to compute the adjusted extrema
                 *            and propagate it into the result of the primitive.
                 *
                 *            Effectively, this gives us the same linear transform
                 *            coverage as similar schemes: (k * IV) + n.
                 *
                 */

#define CONSTANTS
#ifdef CONSTANTS
                /* This causes cvp crashes, and is probably not so good, anyway. */
                /* but let's try it, now that extrema are exact */
                if (TYisAKV (AVIS_TYPE (lhsavis))) {
                    AVIS_MINVAL (lhsavis) = lhsavis;
                    AVIS_MAXVAL (lhsavis) = lhsavis;
                }
#endif // CONSTANTS
                break;
            /* We are unable to help these poor souls */
            case N_ap:

                break;

            case N_with:
                /* We have to descend into the depths here */
                rhs = TRAVdo (rhs, arg_info);
                LET_EXPR (arg_node) = rhs;

                break;

            case N_array:
                /* If the LHS value is constant, this is easy */
                lhsavis = IDS_AVIS (LET_IDS (arg_node));
                if (TYisAKV (AVIS_TYPE (lhsavis))) {
                    AVIS_MINVAL (lhsavis) = lhsavis;
                    AVIS_MAXVAL (lhsavis) = lhsavis;
                } else {
                    extr = ExtractedNarrayExtrema (arg_node, arg_info);
                    if (NULL != extr) {
                        AVIS_MINVAL (lhsavis) = ID_AVIS (PRF_ARG2 (extr));
                        AVIS_MAXVAL (lhsavis) = ID_AVIS (PRF_ARG3 (extr));
                    } else {
                        arg_node = PropagateNarray (arg_node, arg_info);
                    }
                }

                break;

            default:
                DBUG_PRINT ("IVEXP", ("IVEXP ISMOP: please fix this RHS for LHS: %s",
                                      AVIS_NAME (lhsavis)));
                break;
            }
        }

        DBUG_RETURN (arg_node);
    }

    /******************************************************************************
     *
     * function:
     *   node *IVEXPwith( node *arg_node, info *arg_info)
     *
     * description: Into the depths
     *  We have to save current WL pointer, because we don't want
     *  IVEXPlet running off to attempt extrema introduction outside
     *  of a WL.
     *
     ******************************************************************************/
    node *IVEXPwith (node * arg_node, info * arg_info)
    {
        node *oldwith;

        DBUG_ENTER ("IVEXPwith");

        oldwith = INFO_CURWITH (arg_info);
        INFO_CURWITH (arg_info) = arg_node;

        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

        INFO_CURWITH (arg_info) = oldwith;

        DBUG_RETURN (arg_node);
    }

    /******************************************************************************
     *
     * function:
     *   node *IVEXPpart( node *arg_node, info *arg_info)
     *
     * description: Into the depths
     *
     ******************************************************************************/
    node *IVEXPpart (node * arg_node, info * arg_info)
    {

        DBUG_ENTER ("IVEXPpart");

        PART_CODE (arg_node) = TRAVdo (PART_CODE (arg_node), arg_info);
        PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

        DBUG_RETURN (arg_node);
    }

    /******************************************************************************
     *
     * function:
     *   node *IVEXPcond( node *arg_node, info *arg_info)
     *
     * description: Into the depths
     *
     ******************************************************************************/
    node *IVEXPcond (node * arg_node, info * arg_info)
    {

        DBUG_ENTER ("IVEXPcond");

        COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);

        DBUG_RETURN (arg_node);
    }

    /******************************************************************************
     *
     * function:
     *   node *IVEXPfuncond( node *arg_node, info *arg_info)
     *
     * description: Into the depths
     *
     ******************************************************************************/
    node *IVEXPfuncond (node * arg_node, info * arg_info)
    {

        DBUG_ENTER ("IVEXPfuncond");

        COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);
        COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
        COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

        DBUG_RETURN (arg_node);
    }

    /******************************************************************************
     *
     * function:
     *   node *IVEXPwhile( node *arg_node, info *arg_info)
     *
     * description: Into the depths
     *
     ******************************************************************************/
    node *IVEXPwhile (node * arg_node, info * arg_info)
    {

        DBUG_ENTER ("IVEXPwhile");

        WHILE_COND (arg_node) = TRAVdo (WHILE_COND (arg_node), arg_info);
        WHILE_BODY (arg_node) = TRAVdo (WHILE_BODY (arg_node), arg_info);

        DBUG_RETURN (arg_node);
    }

    /******************************************************************************
     *
     * function:
     *   node *IVEXPfundef( node *arg_node, info *arg_info)
     *
     * description:
     *
     *  Insert new vardecs into fundef node.
     *
     ******************************************************************************/
    node *IVEXPfundef (node * arg_node, info * arg_info)
    {

        DBUG_ENTER ("IVEXPfundef");
        DBUG_PRINT ("IVEXP", ("IVEXP in %s %s begins",
                              (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                              FUNDEF_NAME (arg_node)));

        DBUG_ASSERT (INFO_VARDECS (arg_info) == NULL,
                     ("IVEXPfundef INFO_VARDECS not NULL"));

        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        INFO_FUNDEF (arg_info) = NULL;

        /* If new vardecs were made, append them to the current set */
        if (INFO_VARDECS (arg_info) != NULL) {
            BLOCK_VARDEC (FUNDEF_BODY (arg_node))
              = TCappendVardec (INFO_VARDECS (arg_info),
                                BLOCK_VARDEC (FUNDEF_BODY (arg_node)));
            INFO_VARDECS (arg_info) = NULL;
        }

        DBUG_PRINT ("IVEXP", ("IVEXP in %s %s ends",
                              (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                              FUNDEF_NAME (arg_node)));

        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

        DBUG_RETURN (arg_node);
    }
