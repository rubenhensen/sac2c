/** <!--********************************************************************-->
 *
 * @defgroup awlf Algebraic With-Loop Folding
 *
 * @terminology:
 *        ProducerWL: The WL that will no longer
 *        exist after this phase completes. In the example
 *        below, A is the PWL.
 *
 *        ConsumerWL: the WL that will absorb the block(s) from
 *        the PWL. In the example below, B is the consumerWL.
 *
 * @brief Algebraic With-Loop Folding
 *        This performs WLF on AKD arrays, not foldable by WLF.
 *
 *        The features of AWLF are:
 *
 *            - Ability to fold arrays whose shapes are not
 *              known statically. Specifically, if the index
 *              set of the consumerWL is known to be identical to,
 *              or a subset of, the index set of the PWL,
 *              folding will occur.
 *
 *           - the PWL must have an SSAASSIGN.
 *
 *           - the PWL must have an empty code block (so that the cost
 *             must have of duplicating it is zero), or it must have
 *             a NEEDCOUNT of 1.
 *             There must be no other references to the PWL.
 *             If CVP and friends have done their job, this should
 *             not be a severe restriction.
 *
 *             This code includes a little trick to convert a
 *             modarray( PWL) into a genarray( shape( PWL)).
 *             This happens after any folding is performed.
 *             Hence, there is a kludge to
 *             allow a NEEDCOUNT of 2 if there is a modarray consumerWL
 *             present.
 *
 *           - the PWL must have a DEPDEPTH value of 1??
 *             I think the idea here is to ensure that we do not
 *             pull a PWL inside a loop, which could cause
 *             increased computation.
 *
 *           - The consumerWL must refer to the PWL via
 *              _sel_VxA_(idx, PWL)
 *             and idx must be the consumerWL's WITHID, or a
 *             linear function thereof.
 *
 *           - The PWL operator is a genarray or modarray
 *             (NO folds, please).
 *
 *           - The WL is a single-operator WL. (These should have been
 *             eliminated by phase 10, I think.)
 *
 *           - The index set of the consumerWL's partition matches
 *             the generator of the PWL, or is a subset
 *             of it. Note that this implies that
 *             the WL-bounds are the same length, which
 *             may not be the case. Consider this example:
 *
 *               x = with([0] <= iv < [n]) ... : [1,2,3,4]; NB. x is int[.,.]
 *               z = with([0,0], <= iv < shape(x)) :  x[iv];
 *
 *             The bounds of x are int[1], while the bounds of z are int[2].
 *             It is important that AWLFI not generate a partition
 *             intersection expression such as:
 *
 *              _awlf = _max_VxV_( bound1(x), bound1(z));
 *
 *             because TUP will get very confused about the length
 *             error. As I did...
 *
 * An example for Algebraic With-Loop Folding is given below:
 *
 * <pre>
 *  1  A = with( iv)             NB. Foldee WL
 *  2          ( lb <= iv < ub) {
 *  3        <block_a>
 *  4      } : val;
 *  5      genarray( shp);
 *  6
 *  7  ...
 *  8
 *  9  B = with( jv)               NB. Folder WL
 * 10          ( lb <= jv < ub) {
 * 11        <block_b1>
 * 12        ael = sel( jv, A);
 * 13        <block_b2>
 * 14      } : -
 * 15      genarray( shp);
 * </pre>
 *
 *   is transformed into
 *
 * <pre>
 *  1  A = with( iv)
 *  2          ( lb <= iv < ub) {
 *  3        <block_a>
 *  4      } : val;
 *  5      genarray( shp);
 *  6
 *  7  ...
 *  8
 *  9  B = with( jv)
 * 10          ( lb <= jv < ub) {
 * 11        <block_b1>
 * 12        <block_a>, with references to iv renamed to jv.
 * 13        ael = new id of val;
 * 14        <block_b2>
 * 15      } : -
 * 16      genarray( shp);
 * </pre>
 *
 * Then lines 1-5 are removed by DCR(Dead Code Removal).
 *
 * NB. This code supports "naked-consumer WLF", which is where we
 *     have code such as:
 *
 *            pwl = with(...);
 *            iv = 23;
 *            nakedconsumer = pwl[ iv];
 *
 *     We need to know the value of iv, or at very least, its extrema,
 *     so that we can deduce which partition of pwl to use for folding.
 *     This restriction could be lifted by generating code to select from
 *     each of the partitions, with the code chosen at run time (or
 *     eliminated by optimizations earlier) based on the value of iv
 *     and each of the pwl partition bounds.
 *
 *  TODO:
 *   1. At present, AWLF does not occur unless ALL references
 *      to the PWL are in the consumerWL, or unless the
 *      PWL has an empty code block. Here is a possible extension
 *      to allow small computations to be folded into several
 *      consumerWL:
 *
 *      Introduce a cost function into WLNC. The idea here is
 *      to provide a crude measure of the cost of computing
 *      a single WL result element. We start by giving
 *      each primitive a cost:
 *        F_xxx_SxS_:  1
 *        F_xxx_SxV_:  infinity
 *        F_xxx_VxS_:  infinity
 *        F_xxx_VxV_:  infinity
 *        N_ap:        infinity
 *        N_with:      infinity
 *        etc.
 *
 *      The wl_needcount code will sum the cost of the code
 *      in each WL. Hmm. The cost must be associated with the
 *      WL-partition.
 *
 *      If the PWL is otherwise ripe for folding, we allow
 *      the fold to occur if the cost is less than some threshold,
 *      even if the references to the PWL occur in several
 *      consumerWL.
 *
 *   2. Permit WLF through reshape operations.
 *      The idea here is that, if the consumerWL and PWL
 *      operating in wlidx mode, then
 *      the folding can be done, because neither WL cares about
 *      the shape of the PWL result, just that they have
 *      identical element counts.
 *
 *      A bit more thought here might give a nice way to
 *      extend this to the case where only one WL is operating
 *      in wlidx mode.
 *
 * @ingroup opt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file algebraic_wlf.c
 *
 * Prefix: AWLF
 *
 *****************************************************************************/
#include "algebraic_wlf.h"
#include "algebraic_wlfi.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"
#include "print.h"

#define DBUG_PREFIX "AWLF"
#include "debug.h"

#include "traverse.h"
#include "str.h"
#include "memory.h"
#include "inferneedcounters.h"
#include "compare_tree.h"
#include "DupTree.h"
#include "free.h"
#include "LookUpTable.h"
#include "globals.h"
#include "wl_cost_check.h"
#include "wl_needcount.h"
#include "pattern_match.h"
#include "constants.h"
#include "shape.h"
#include "new_types.h"
#include "phase.h"
#include "check.h"
#include "wls.h"
#include "cubeslicer.h"
#include "ivexcleanup.h"
#include "deadcoderemoval.h"
#include "indexvectorutils.h"
#include "type_utils.h"
#include "flattengenerators.h"
#include "with_loop_utilities.h"
#include "set_withloop_depth.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    node *cwlpart;
    /* This is the current partition in the consumerWL. */
    node *let; /* Current N_let */
    node *cwlids;
    /* current consumerWL N_ids */
    node *pwlid;
    /* current producerWL N_id */
    int defdepth;
    /* This is the current nesting level of WLs */
    node *producerpart;
    lut_t *lut;
    /* This is the WITH_ID renaming lut */
    node *vardecs;
    node *preassigns;
    node *idxbound1;
    node *idxbound2;
    intersect_type_t intersecttype;
    node *assign;
};

/**
 * Macro definitions for INFO structure
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_CWLPART(n) ((n)->cwlpart)
#define INFO_LET(n) ((n)->let)
#define INFO_CWLIDS(n) ((n)->cwlids)
#define INFO_PWLID(n) ((n)->pwlid)
#define INFO_DEFDEPTH(n) ((n)->defdepth)
#define INFO_PRODUCERPART(n) ((n)->producerpart)
#define INFO_LUT(n) ((n)->lut)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_PREASSIGNS(n) ((n)->preassigns)
#define INFO_IDXBOUND1(n) ((n)->idxbound1)
#define INFO_IDXBOUND2(n) ((n)->idxbound2)
#define INFO_INTERSECTTYPE(n) ((n)->intersecttype)
#define INFO_ASSIGN(n) ((n)->assign)

static info *
MakeInfo (node *fundef)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = fundef;
    INFO_CWLPART (result) = NULL;
    INFO_LET (result) = NULL;
    INFO_CWLIDS (result) = NULL;
    INFO_PWLID (result) = NULL;
    INFO_DEFDEPTH (result) = 0;
    INFO_PRODUCERPART (result) = NULL;
    INFO_LUT (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_IDXBOUND1 (result) = NULL;
    INFO_IDXBOUND2 (result) = NULL;
    INFO_INTERSECTTYPE (result) = INTERSECT_unknown;
    INFO_ASSIGN (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

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
 * @fn node *AWLFdoAlgebraicWithLoopFolding( node *arg_node)
 *
 * @brief global entry point of Algebraic With-Loop folding
 *
 * @param N_fundef apply AWLF.
 *
 * @return optimized N_fundef
 *
 *****************************************************************************/
node *
AWLFdoAlgebraicWithLoopFolding (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo (NULL);
    INFO_LUT (arg_info) = LUTgenerateLut ();

    TRAVpush (TR_awlf);
    arg_node = TRAVopt (arg_node, arg_info);
    TRAVpop ();

    INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));
    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *BypassNoteintersect( node *arg_node)
 *
 * @brief Bypass the  F_noteintersect that might precede the arg_node.
 *        "might", because we may have SimpleComposition happening.
 *        Actually, we just bypass that node and let DCR do the dirty work.
 *
 * @param N_assign for _idx_sel() or _sel_VxA_()
 * @result Rebuilt arg_node.
 *
 *****************************************************************************/
static node *
BypassNoteintersect (node *arg_node)
{
    node *z = NULL;
    pattern *pat;
    node *expr;
    node *exprni;

    DBUG_ENTER ();

    expr = LET_EXPR (ASSIGN_STMT (arg_node));

    exprni = AWLFIfindNoteintersect (PRF_ARG1 (expr));
    if (NULL != exprni) {
        DBUG_PRINT ("Insertion cycle was %d, folding cycle is %d",
                    PRF_NOTEINTERSECTINSERTIONCYCLE (exprni), global.cycle_counter);
    }

    pat = PMprf (1, PMAisPrf (F_noteintersect), 2, PMvar (1, PMAgetNode (&z), 0),
                 PMskip (0));
    if (PMmatchFlat (pat, PRF_ARG1 (expr))) {
        PRF_ARG1 (expr) = FREEdoFreeNode (PRF_ARG1 (expr));
        PRF_ARG1 (expr) = DUPdoDupNode (z);
    }

    pat = PMfree (pat);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn bool checkAWLFoldable( node *arg_node, info *arg_info,
 * node *cwlp, int level)
 *
 * @brief check if _sel_VxA_(idx, PWL), appearing in
 *        consumer WL partition cwlp, is foldable into cwlp.
 *        Most checks have already been made by AWLFI.
 *
 *        We have to be careful, because (See Bug #652) the
 *        PWL may have changed underfoot, due to CWLE
 *        and the like. Specifically, the copy-WL that was
 *        the PWL may not exist any more, or it may
 *        have been replaced by a multi-op WL, which would
 *        be equally unpleasant.
 *
 * @param _sel_VxA_( idx, PWL)
 * @param cwlp: The partition into which we would like to
 *        fold this sel(). Will become NULL, if this is a naked consumer.
 * @param level
 * @result If the PWL is foldable into the consumerWL, return the
 *         N_part of the PWL that should be used for the fold; else NULL.
 *
 *****************************************************************************/
static node *
checkAWLFoldable (node *arg_node, info *arg_info, node *cwlp, int level)
{
    node *producerWLavis;
    node *PWL;
    node *pwlp = NULL;

    DBUG_ENTER ();

    PWL = AWLFIfindWlId (PRF_ARG2 (arg_node)); /* Now the N_id */
    if (NULL != PWL) {
        producerWLavis = ID_AVIS (PWL);
        PWL = AWLFIfindWL (PWL); /* Now the N_with */

        /* Allow naked _sel_() of WL */
        // TEMP DEBUG logd.sac by killing naked CWL folding
        if (NULL != cwlp) {
            if (AWLFIisNakedWL (level, AVIS_DEFDEPTH (producerWLavis))) {
                cwlp = NULL; /* cwlp was a lie anyway, in this case */
            }
            if (((AVIS_DEFDEPTH (producerWLavis) + 1) >= level) && (NULL != cwlp)
                && (WLUTisSingleOpWl (PWL))) {
                DBUG_PRINT ("PWL %s: AVIS_NEEDCOUNT=%d, AVIS_WL_NEEDCOUNT=%d",
                            AVIS_NAME (producerWLavis), AVIS_NEEDCOUNT (producerWLavis),
                            AVIS_WL_NEEDCOUNT (producerWLavis));
                /* Search for pwlp that intersects cwlp */
                INFO_INTERSECTTYPE (arg_info)
                  = CUBSLfindMatchingPart (arg_node, cwlp, PWL, NULL,
                                           &INFO_PRODUCERPART (arg_info));

                switch (INFO_INTERSECTTYPE (arg_info)) {

                default:
                    DBUG_PRINT ("Should not get here.");
                    DBUG_UNREACHABLE ("We are confused");
                    break;

                case INTERSECT_unknown:
                case INTERSECT_sliceneeded:
                case INTERSECT_null:
                    DBUG_PRINT ("Cube can not be intersected exactly");
                    pwlp = NULL;
                    break;

                case INTERSECT_exact:
                    DBUG_PRINT ("Cube can be folded");
                    pwlp = INFO_PRODUCERPART (arg_info);
                    break;
                }

                /* Allow fold if needcounts match OR if pwlp
                 * has empty code block. This is a crude cost function:
                 * We should allow "cheap" PWL partitions to fold.
                 * E.g., toi(iota(N)).
                 */
                if ((NULL != pwlp) &&
                    // I am disabling this check. We'll see who complains about duplicate
                    // work (when PWL gets copied into several CWLs).
                    // I think a better fix is to write a cost function for each PWL
                    // partition, and allow AWLF if the cost is "low enough".
                    //
                    // Oops: we need the cost function: ipbb.sac ended up folding the
                    // _aplmod_() reshape function into the inner loop of the matrix
                    // product. That was a bad idea, as it turns out, as it's quite
                    // expensive.
                    //
                    ((AVIS_NEEDCOUNT (producerWLavis)
                      != AVIS_WL_NEEDCOUNT (producerWLavis)))
                    && (!WLUTisEmptyPartitionCodeBlock (pwlp))) {
                    DBUG_PRINT (
                      "Can't intersect PWL %s: AVIS_NEEDCOUNT=%d, AVIS_WL_NEEDCOUNT=%d",
                      AVIS_NAME (producerWLavis), AVIS_NEEDCOUNT (producerWLavis),
                      AVIS_WL_NEEDCOUNT (producerWLavis));
                    pwlp = NULL;
                }
            } else {
                DBUG_PRINT ("PWL %s will never fold. AVIS_DEFDEPTH: %d, level: %d",
                            AVIS_NAME (producerWLavis), AVIS_DEFDEPTH (producerWLavis),
                            level);
            }

            if (NULL != pwlp) {
                DBUG_PRINT ("PWL %s will be folded.", AVIS_NAME (producerWLavis));
            } else {
                DBUG_PRINT ("PWL %s can not be folded, at present.",
                            AVIS_NAME (producerWLavis));
            }
        }

    } else {
        // TEMP DEBUG logd.sac by killing naked consumer WLF
        pwlp = NULL;
    }

    DBUG_RETURN (pwlp);
}

/** <!--********************************************************************-->
 *
 * @fn static node *populateLut( node *arg_node, info *arg_info, shape *shp)
 *
 * @brief Generate a clone name for a WITHID.
 *        Populate one element of a look up table with
 *        said name and its original, which we will use
 *        to do renames in the copied WL code block.
 *        See caller for description. Basically,
 *        we have a PWL with generator of this form:
 *
 *    PWL = with {
 *         ( . <= iv=[i,j] <= .) : _sel_VxA_( iv, AAA);
 *
 *        We want to perform renames in the PWL code block as follows:
 *
 *        iv --> iv'
 *        i  --> i'
 *        j  --> j'
 *
 * @param  arg_node: one N_avis node of the PWL generator (e.g., iv),
 *                   to serve as iv for above assigns.
 * @param  arg_info: your basic arg_info.
 * @param  shp:      the shape descriptor of the new LHS.
 *
 * @result  New N_avis node, e.g, iv'.
 *          Side effect: mapping iv -> iv' entry is now in LUT.
 *                       New vardec for iv'.
 *
 *****************************************************************************/
static node *
populateLut (node *arg_node, info *arg_info, shape *shp)
{
    node *navis;

    DBUG_ENTER ();

    /* Generate a new LHS name for WITHID_VEC/IDS */
    navis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (arg_node)),
                        TYmakeAKS (TYcopyType (TYgetScalar (AVIS_TYPE (arg_node))), shp));

    INFO_VARDECS (arg_info) = TBmakeVardec (navis, INFO_VARDECS (arg_info));
    LUTinsertIntoLutP (INFO_LUT (arg_info), arg_node, navis);

    DBUG_PRINT ("Inserted WITHID_VEC into lut: oldname: %s, newname %s",
                AVIS_NAME (arg_node), AVIS_NAME (navis));

    DBUG_RETURN (navis);
}

/** <!--********************************************************************-->
 *
 * @ fn static node *makeIdxAssigns( node *arg_node, node *pwlpart)
 *
 * @brief for a PWL partition, with generator:
 *        (. <= iv=[i,j] < .)
 *        and a consumer _sel_VxA_( idx, PWL),
 *
 *        generate an N_assigns chain of this form:
 *
 *        iv = idx;
 *        k0 = [0];
 *        i  = _sel_VxA_( k0, idx);
 *        k1 = [1];
 *        j  = _sel_VxA_( k1, idx);
 *
 *        Then, iv, i, j will all be SSA-renamed by the caller.
 *
 * @param arg_node: An N_assign node.
 *
 * @result  an N_assign chain as above.
 *
 *****************************************************************************/
static node *
makeIdxAssigns (node *arg_node, info *arg_info, node *pwlpart)
{
    node *z = NULL;
    node *ids;
    node *narray;
    node *idxavis;
    node *navis;
    node *nass;
    node *lhsids;
    node *lhsavis;
    node *sel;
    node *args;
    int k;

    DBUG_ENTER ();

    ids = WITHID_IDS (PART_WITHID (pwlpart));
    args = LET_EXPR (ASSIGN_STMT (arg_node));
    idxavis = IVUToffset2Vect (args, &INFO_VARDECS (arg_info),
                               &INFO_PREASSIGNS (arg_info), pwlpart, NULL);
    DBUG_ASSERT (NULL != idxavis, "Could not rebuild iv for _sel_VxA_(iv, PWL)");

    k = 0;

    while (NULL != ids) {
        /* Build k0 = [k]; */
        /* First, the k */
        narray = TCmakeIntVector (TBmakeExprs (TBmakeNum (k), NULL));
        navis = TBmakeAvis (TRAVtmpVar (),
                            TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (1, 1)));

        nass = TBmakeAssign (TBmakeLet (TBmakeIds (navis, NULL), narray), NULL);
        AVIS_SSAASSIGN (navis) = nass;
        z = TCappendAssign (nass, z);
        INFO_VARDECS (arg_info) = TBmakeVardec (navis, INFO_VARDECS (arg_info));

        lhsavis = populateLut (IDS_AVIS (ids), arg_info, SHcreateShape (0));
        DBUG_PRINT ("created %s = _sel_VxA_(%d, %s)", AVIS_NAME (lhsavis), k,
                    AVIS_NAME (idxavis));

        sel = TBmakeAssign (TBmakeLet (TBmakeIds (lhsavis, NULL),
                                       TCmakePrf2 (F_sel_VxA, TBmakeId (navis),
                                                   TBmakeId (idxavis))),
                            NULL);
        z = TCappendAssign (z, sel);
        AVIS_SSAASSIGN (lhsavis) = sel;
        ids = IDS_NEXT (ids);
        k++;
    }

    /* Now generate iv = idx; */
    lhsids = WITHID_VEC (PART_WITHID (pwlpart));
    lhsavis = populateLut (IDS_AVIS (lhsids), arg_info, SHcreateShape (1, k));
    z = TBmakeAssign (TBmakeLet (TBmakeIds (lhsavis, NULL), TBmakeId (idxavis)), z);
    AVIS_SSAASSIGN (lhsavis) = z;
    DBUG_PRINT ("makeIdxAssigns created %s = %s)", AVIS_NAME (lhsavis),
                AVIS_NAME (idxavis));
    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn static void CopyWithPragma(...)
 *
 * @brief If the producer-WL has a WITH_PRAGMA, and the consumer-WL does not,
 *        copy the pwl pragma to the cwl, unless cwl is a naked-cosumer,
 *        in which case, do nothing.
 *
 *        We introduced this for ~/sac/demos/applications/numerical/misc/matmul.sac.
 *        When modified to do sum(matmul()), the pragma on the matmul
 *        was lost. This is an attempt to propagate it into the cwl.
 *
 * @param pwl: producer N_with
 * @param cwl: consumer N_with
 *
 * @result  none. Side effect on cwl N_with
 *
 *****************************************************************************/
static void
CopyWithPragma (node *pwl, node *cwl)
{

    DBUG_ENTER ();

    if ((NULL != pwl) && (NULL != cwl) && (N_with == NODE_TYPE (cwl))
        && (NULL == WITH_PRAGMA (cwl)) && (NULL != WITH_PRAGMA (pwl))) {
        WITH_PRAGMA (cwl) = DUPdoDupNode (WITH_PRAGMA (pwl));
    }

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn node *AWLFperformFold( ... )
 *
 * @brief
 *   In
 *    PWL = with...  elb = _sel_VxA_(iv=[i,j], AAA) ...
 *    consumerWL = with...  elc = _sel_VxA_(idx, PWL) ...
 *
 *   Replace, in the consumerWL:
 *     elc = _sel_VxA_( idx, PWL)
 *   by
 *     iv = idx;
 *     i = _sel_VxA_([0], idx);
 *     j = _sel_VxA_([1], idx);
 *
 *     {code block from PWL, with SSA renames}
 *
 *     tmp = PWL)
 *     elc = tmp;
 *
 * @param arg_node: N_assign for the sel()
 * #param PWL: N_part node of PWL.
 *
 *****************************************************************************/
node *
AWLFperformFold (node *arg_node, node *producerWLPart, info *arg_info)
{
    node *oldblock;
    node *newblock;
    node *newsel;
    node *idxassigns;
    node *cellexpr;
    node *pwl;

    DBUG_ENTER ();

    DBUG_PRINT ("Replacing code block in CWL=%s",
                AVIS_NAME (IDS_AVIS (LET_IDS (ASSIGN_STMT (INFO_ASSIGN (arg_info))))));

    arg_node = BypassNoteintersect (arg_node);

    pwl = AWLFIfindWlId (PRF_ARG2 (LET_EXPR (ASSIGN_STMT (arg_node))));
    if (NULL != pwl) {
        pwl = AWLFIfindWL (pwl); /* Now the N_with */
    }

    CopyWithPragma (pwl, LET_EXPR (ASSIGN_STMT (arg_node)));
    /* Generate iv=[i,j] assigns, then do renames. */
    idxassigns = makeIdxAssigns (arg_node, arg_info, producerWLPart);

    cellexpr = ID_AVIS (EXPRS_EXPR (CODE_CEXPRS (PART_CODE (producerWLPart))));

    producerWLPart = IVEXCdoIndexVectorExtremaCleanupPartition (producerWLPart, arg_info);
    oldblock = BLOCK_ASSIGNS (CODE_CBLOCK (PART_CODE (producerWLPart)));

    if (NULL != oldblock) {
        /* Remove extrema from old block and recompute everything */
        newblock
          = DUPdoDupTreeLutSsa (oldblock, INFO_LUT (arg_info), INFO_FUNDEF (arg_info));
    } else {
        /* If PWL code block is empty, don't duplicate code block.
         * If the cell is non-scalar, we still need a _sel_() to pick
         * the proper cell element.
         */
        newblock = NULL;
    }

    cellexpr = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), cellexpr);
    newsel = TBmakeId (cellexpr);

    LUTremoveContentLut (INFO_LUT (arg_info));

    /**
     * replace the code
     */
    FREEdoFreeNode (LET_EXPR (ASSIGN_STMT (arg_node)));
    LET_EXPR (ASSIGN_STMT (arg_node)) = newsel;
    if (NULL != newblock) {
        arg_node = TCappendAssign (newblock, arg_node);
    }

    arg_node = TCappendAssign (idxassigns, arg_node);
    global.optcounters.awlf_expr += 1;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn static bool isSimpleComposition()
 *
 * @brief Predicate to determine if consumer-WL cwl is a simple
 *        composition on the producer-WL pwl. I.e., we have something
 *        like:
 *                 z = sum( iota( N));
 *
 *        where iota() comprises pwl, and sum() comprises cwl.
 *
 * @param arg_node: the _sel_VxA() or _idx_sel_() op in the cwl
 * @param pwl: The producer-WL N_id
 * @param cwlids The consumer-WL N_ids
 * @param defdepth: the nesting depth of the cwl.
 * @param cwlpart: the current cwl partition
 *
 * @result: TRUE if the pair of WLs form a simple composition.
 *
 * NB. The rules for a composition, over and above the rules
 *     checked in checkAWLFoldable (AWLF) are:
 *
 *      - cwl and pwl each are 1 partition.
 *      - pwl is a modarray or genarray
 *      - both WLs are single-op WLs
 *      - indexing is in ravel order. I.e., no transpose, rotate, etc.
 *
 * Rationale:
 *     If this is a composition, such as we see in take( [10], iota( N)),
 *     we can blindly AWLF. The rationale for this is that we may
 *     not be able to determine if 10<=N, but if it is, then all is
 *     well. If not, then we will find out the hard way at execution
 *     time, and if you compile with -check c, you will find out why.
 *
 * Extensions: Multi-partition compositions, in which we can
 *             easily show that no array slicing is needed,
 *             can be performed, too.
 *             E.g.,
 *
 *****************************************************************************/
static bool
isSimpleComposition (node *arg_node, node *pwlid, node *cwlids, int defdepth,
                     node *cwlpart)
{
    bool z = FALSE;

    DBUG_ENTER ();

    node *pwlwith;
    node *cwlwith;

    pwlwith = AWLFIfindWL (pwlid);
    if ((global.optimize.doscwlf) && (NULL != pwlwith) && (NULL != cwlids)) {
        cwlwith = LET_EXPR (ASSIGN_STMT (AVIS_SSAASSIGN (IDS_AVIS (cwlids))));
        z = (1 == TCcountParts (WITH_PART (pwlwith)))
            && (1 == TCcountParts (WITH_PART (cwlwith)));
        z = z && WLUTisSingleOpWl (cwlwith);
        z = z && AWLFIcheckProducerWLFoldable (pwlid);
        z = z && AWLFIcheckBothFoldable (pwlid, cwlids, defdepth);

        // Next line implies that CWL[iv] = f( PWL[iv]);
        z = z && (NULL != IVUTfindIvWithid (PRF_ARG1 (arg_node), cwlpart));
    }
#ifdef DEADCODE
    node *noteint;
    node *proj1;
    node *proj2;
    noteint = AWLFIfindNoteintersect (PRF_ARG1 (arg_node));
    if (NULL != noteint) {
        proj1 = TCgetNthExprsExpr (WLPROJECTION1 (0), PRF_ARGS (noteint));
        proj2 = TCgetNthExprsExpr (WLPROJECTION2 (0), PRF_ARGS (noteint));
        z = z && (AWLFIisHasInverseProjection (proj1))
            && (AWLFIisHasInverseProjection (proj2));
    }
#endif // DEADCODE
       // z = FALSE;  // VERY strange WLs for AWLF UT bodomatmulbug2AKD.sac and
       // many others. Fewer WLs, supposedly correct answers,
       // but code clearly could never work properly.
       // E.g., matmul with two non-nested WLs. So, disabled for now.
       // Still broken as of Build #18295.
    if (z) {
        DBUG_PRINT ("Simple composition %s( %s) detected", AVIS_NAME (IDS_AVIS (cwlids)),
                    AVIS_NAME (ID_AVIS (pwlid)));
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *AWLFfundef(node *arg_node, info *arg_info)
 *
 * @brief applies AWLF to a given fundef.
 *
 *****************************************************************************/
node *
AWLFfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node) != NULL) {

        DBUG_PRINT ("Algebraic With-Loop folding in %s %s begins",
                    (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                    FUNDEF_NAME (arg_node));

        INFO_FUNDEF (arg_info) = arg_node;

        /* cubeslicer may leave around old WL. That will confuse
         * WLNC, which is driven by vardecs, NOT by function body traversal.
         */
        arg_node = DCRdoDeadCodeRemoval (arg_node);

        arg_node = INFNCdoInferNeedCountersOneFundef (arg_node, TR_awlfi);
        arg_node = WLNCdoWLNeedCount (arg_node);
        arg_node = WLCCdoWLCostCheck (arg_node);
        arg_node = SWLDdoSetWithloopDepth (arg_node);

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        /* If new vardecs were made, append them to the current set */
        if (INFO_VARDECS (arg_info) != NULL) {
            BLOCK_VARDECS (FUNDEF_BODY (arg_node))
              = TCappendVardec (INFO_VARDECS (arg_info),
                                BLOCK_VARDECS (FUNDEF_BODY (arg_node)));
            INFO_VARDECS (arg_info) = NULL;
        }

        FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

        DBUG_PRINT ("Algebraic With-Loop folding in %s %s ends",
                    (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                    FUNDEF_NAME (arg_node));
    }

    INFO_FUNDEF (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node AWLFassign( node *arg_node, info *arg_info)
 *
 * @brief For a foldable WL, arg_node is x = _sel_VxA_(iv, PWL).
 *
 *****************************************************************************/
node *
AWLFassign (node *arg_node, info *arg_info)
{
    node *foldableProducerPart;
    node *oldassign;

    DBUG_ENTER ();

    /* top-down traversal */
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    oldassign = INFO_ASSIGN (arg_info);
    INFO_ASSIGN (arg_info) = arg_node;

#ifdef VERBOSE
    DBUG_PRINT ("Traversing N_assign for %s",
                AVIS_NAME (IDS_AVIS (LET_IDS (ASSIGN_STMT (INFO_ASSIGN (arg_info))))));
#endif // VERBOSE

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    foldableProducerPart = INFO_PRODUCERPART (arg_info);
    INFO_PRODUCERPART (arg_info) = NULL;
    DBUG_ASSERT (NULL == INFO_PREASSIGNS (arg_info), "INFO_PREASSIGNS not NULL");

    /*
     * Append the new cloned block
     */
    if (NULL != foldableProducerPart) {
        arg_node = AWLFperformFold (arg_node, foldableProducerPart, arg_info);
    }

    if (NULL != INFO_PREASSIGNS (arg_info)) {
        arg_node = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
        INFO_PREASSIGNS (arg_info) = NULL;
    }

    INFO_ASSIGN (arg_info) = oldassign;
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *AWLFwith( node *arg_node, info *arg_info)
 *
 * @brief applies AWLF to a with-loop in a top-down manner.
 *
 *****************************************************************************/
node *
AWLFwith (node *arg_node, info *arg_info)
{
    node *genop;
    node *consumerop;
    node *producershape;
    info *old_info;

    DBUG_ENTER ();

    DBUG_PRINT ("Examining N_with for %s",
                AVIS_NAME (IDS_AVIS (LET_IDS (ASSIGN_STMT (INFO_ASSIGN (arg_info))))));
    old_info = arg_info;
    arg_info = MakeInfo (INFO_FUNDEF (arg_info));
    INFO_LUT (arg_info) = INFO_LUT (old_info);
    INFO_LET (arg_info) = INFO_LET (old_info);
    INFO_DEFDEPTH (arg_info) = INFO_DEFDEPTH (old_info) + 1;
    INFO_VARDECS (arg_info) = INFO_VARDECS (old_info);
    INFO_PREASSIGNS (arg_info) = INFO_PREASSIGNS (old_info);

    INFO_CWLIDS (arg_info) = LET_IDS (INFO_LET (arg_info));

    WITH_REFERENCED_CONSUMERWL (arg_node) = NULL;
    WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);

    /* Try to replace modarray(PWL) by genarray(shape(PWL)).
     * This has no effect on the PWL itself, but is needed to
     * eliminate the reference to PWL, so it can be removed by DCR.
     *
     * If the PWL has been folded, its result will have
     * AVIS_FOLDED set. Since there are, by the definition of
     * folding, no other references to the PWL, we can
     * blindly replace the modarray by the genarray.
     */
    consumerop = WITH_WITHOP (arg_node);
    //#define BREAKSUPLHS
    //#ifdef BREAKSDUPLHS
    /* This code is inadequate. The modarray may not have
     * partitions that cover the MODARRAY_ARRAY. SSAWLF somehow
     * manages to handle this, but until I figure out how it does
     * it, let's leave this code disabled. It breaks the awlf/duplhs.sac
     * unit test, by giving wrong answers!
     */
    if ((N_modarray == NODE_TYPE (consumerop))) {
        DBUG_PRINT ("consumerop %s has AVIS_NEEDCOUNT=%d",
                    AVIS_NAME (ID_AVIS (MODARRAY_ARRAY (consumerop))),
                    AVIS_NEEDCOUNT (ID_AVIS (MODARRAY_ARRAY (consumerop))));
    }

    if ((N_modarray == NODE_TYPE (consumerop))
        && (NULL != AVIS_SHAPE (ID_AVIS (MODARRAY_ARRAY (consumerop))))
        && (1 == AVIS_NEEDCOUNT (ID_AVIS (MODARRAY_ARRAY (consumerop))))) {
        producershape = AVIS_SHAPE (ID_AVIS (MODARRAY_ARRAY (consumerop)));
        genop = TBmakeGenarray (DUPdoDupTree (producershape), NULL);
        GENARRAY_NEXT (genop) = MODARRAY_NEXT (consumerop);
        consumerop = FREEdoFreeNode (consumerop);
        WITH_WITHOP (arg_node) = genop;
        DBUG_PRINT ("Replacing modarray by genarray");
    }
    //#endif // BREAKSDUPLHS

    INFO_VARDECS (old_info) = INFO_VARDECS (arg_info);
    INFO_PREASSIGNS (old_info) = INFO_PREASSIGNS (arg_info);
    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *AWLFcode( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
AWLFcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Traversing N_code");
    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *AWLFpart( node *arg_node, info *arg_info)
 *
 * @brief Traverse each partition of a WL.
 *
 *****************************************************************************/
node *
AWLFpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Traversing N_part");
    INFO_CWLPART (arg_info) = arg_node;
    PART_CODE (arg_node) = TRAVdo (PART_CODE (arg_node), arg_info);
    INFO_CWLPART (arg_info) = NULL;

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *AWLFprf( node *arg_node, info *arg_info)
 *
 * @brief
 *   Examine all _sel_VxA_( idx, PWL)  primitives to see if
 *   the _sel_VxA_ is contained inside a WL, and that idx has
 *   intersect information attached to it.
 *   If so, PWL may be a candidate for folding into this WL.
 *
 *   If idx does not have an SSAASSIGN, it means idx is a WITHID.
 *   If so, we will visit here again, after extrema have been
 *   attached to idx, and idx renamed.
 *
 *****************************************************************************/
node *
AWLFprf (node *arg_node, info *arg_info)
{
    node *pwl;
    node *pwlwith;

    DBUG_ENTER ();

#ifdef VERBOSE
    DBUG_PRINT ("Traversing N_prf");
#endif // VERBOSE
    if (((PRF_PRF (arg_node) == F_sel_VxA) || (PRF_PRF (arg_node) == F_idx_sel))
        && (AWLFIisHasNoteintersect (arg_node))) {
        INFO_PRODUCERPART (arg_info)
          = checkAWLFoldable (arg_node, arg_info, INFO_CWLPART (arg_info),
                              INFO_DEFDEPTH (arg_info));
        pwl = PRF_ARG2 (arg_node);
        pwlwith = AWLFIfindWL (pwl); /* Now the N_with */
        if ((NULL == INFO_PRODUCERPART (arg_info) && (NULL != INFO_CWLIDS (arg_info))
             && (NULL != INFO_CWLPART (arg_info)) && (NULL != pwlwith)
             && (isSimpleComposition (arg_node, pwl, INFO_CWLIDS (arg_info),
                                      INFO_DEFDEPTH (arg_info),
                                      INFO_CWLPART (arg_info))))) {
            //  || (FALSE && PRF_ISFOLDNOW( arg_node))))) {  // DISABLED. wrong answers
            // AWLF UT realrelax.sac and overly enthusiastic AWLF on other ones.
            INFO_PRODUCERPART (arg_info) = WITH_PART (pwlwith);
            PRF_ISFOLDNOW (arg_node) = FALSE;
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AWLFcond(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse conditional parts in the given order.
 *
 ******************************************************************************/
node *
AWLFcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Traversing N_cond");
    COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);
    COND_THENASSIGNS (arg_node) = TRAVopt (COND_THENASSIGNS (arg_node), arg_info);
    COND_ELSEASSIGNS (arg_node) = TRAVopt (COND_ELSEASSIGNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AWLFfuncond( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/
node *
AWLFfuncond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Traversing N_funcond");
    FUNCOND_IF (arg_node) = TRAVopt (FUNCOND_IF (arg_node), arg_info);
    FUNCOND_THEN (arg_node) = TRAVopt (FUNCOND_THEN (arg_node), arg_info);
    FUNCOND_ELSE (arg_node) = TRAVopt (FUNCOND_ELSE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AWLFwhile( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/
node *
AWLFwhile (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Traversing N_while");
    WHILE_COND (arg_node) = TRAVopt (WHILE_COND (arg_node), arg_info);
    WHILE_BODY (arg_node) = TRAVopt (WHILE_BODY (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AWLFlet( node *arg_node, info *arg_info)
 *
 * description:
 *   Tuck away N_let for debugging info, then
 *   descend into let.
 *
 ******************************************************************************/
node *
AWLFlet (node *arg_node, info *arg_info)
{
    node *oldlet;

    DBUG_ENTER ();

    DBUG_PRINT ("Traversing N_let for LHS %s", AVIS_NAME (IDS_AVIS (LET_IDS (arg_node))));
    oldlet = INFO_LET (arg_info);
    INFO_LET (arg_info) = arg_node;

    LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);

    INFO_LET (arg_info) = oldlet;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Algebraic with loop folding -->
 *****************************************************************************/

#undef DBUG_PREFIX
