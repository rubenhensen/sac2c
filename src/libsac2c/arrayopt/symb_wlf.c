/*
 * $Id$
 */

// #define DAOEN

/** <!--********************************************************************-->
 *
 * @defgroup swlf Extended With-Loop Folding
 *
 * @terminology:
 *        Foldee-WL, or foldee: The WL that will no longer
 *        exist after this phase completes. In the example
 *        below, A is the foldeeWL.
 *
 *        Folder-WL: the WL that will absorb the block(s) from
 *        the foldeeWL. In the example below, B is the folderWL.
 *
 * @brief Extended With-Loop Folding
 *        This performs WLF on some arrays that are not foldable
 *        by WLF.
 *
 *        The features of EWLF are:
 *
 *            - Ability to fold arrays whose shapes are not
 *              known statically. Specifically, if the index
 *              set of the folderWL is known to be identical to,
 *              or a subset of, the index set of the foldeeWL,
 *              folding will occur.
 *
 *           - the foldeeWL must have an SSAASSIGN.
 *
 *           - the foldeeWL must have a NEEDCOUNT of 1. I.e.,
 *             there must be no other references to the foldeeWL.
 *             If CVP and friends have done their job, this should
 *             not be a severe restriction.
 *             This code includes a little trick to convert a modarray(foldee)
 *             into a genarray(shape(foldee)), but this happens
 *             after any folding. Hence, there is a kludge to
 *             allow a NEEDCOUNT of 2 if there is a modarray folderWL
 *             present.
 *
 *           - the foldeeWL must have a DEPDEPTH value of 1??
 *             Not sure what this means yet...
 *
 *           - The folderWL must refer to the foldee WL via
 *              _sel_VxA_(idx, foldee)
 *             and idx must be the folderWL's WITHID, or a
 *             linear function thereof.
 *
 *           - The foldeeWL operator is a genarray or modarray
 *             (NO folds, please).
 *
 *           - The WL is a single-operator WL. (These should have been
 *             eliminated by phase 10, I think.)
 *
 *           - The index set of the folderWL's partition matches
 *             the generator of the foldeeWL, or is a subset
 *             of it. Note that this implies that
 *             the WL-bounds are the same length, which
 *             may not be the case. Consider this example:
 *
 *               x = with([0] <= iv < [n]) ... : [1,2,3,4]; NB. x is int[.,.]
 *               z = with([0,0], <= iv < shape(x)) :  x[iv];
 *
 *             The bounds of x are int[1], while the bounds of z are int[2].
 *             It is important that EWLFI not generate a partition
 *             intersection expression such as:
 *
 *              _swlfi_789 = _max_VxV_( bound1(x), bound1(z));
 *
 *             because TUP will get very confused about the length
 *             error. As I did...
 *
 * An example for Symbolic With-Loop Folding is given below:
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
 *  TODO:
 *   1. At present, SWLF does not occur unless ALL references
 *      to the foldeeWL are in the folderWL. Here is an extension
 *      to allow small computations to be folded into several
 *      folderWLs:
 *
 *      Introduce cost function into WLNC. The idea here is
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
 *      in each WL. Hmm. This looks like the cost should
 *      reside in the WL-partition.
 *
 *      If the foldeeWL is otherwise ripe for folding, we allow
 *      the fold to occur if the cost is less than some threshold,
 *      even if the references to the foldeeWL occur in several
 *      folderWLs.
 *
 *   2. Permit WLF through reshape operations.
 *      The idea here is that, if the folderWL and foldeeWL
 *      operating in wlidx mode, then
 *      the folding can be done, because neither WL cares about
 *      the shape of the foldeeWL result, just that they have
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
 * @file symb_wlf.c
 *
 * Prefix: SWLF
 *
 *****************************************************************************/
#include "symb_wlf.h"
#include "symb_wlfi.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"
#include "print.h"
#include "dbug.h"
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

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    node *part;
    /* This is the current partition in the folderWL. */
    node *wl;
    /* This is the current folderWL. */
    int level;
    /* This is the current nesting level of WLs */
    node *swlfoldablefoldeepart;
    lut_t *lut;
    /* This is the WITH_ID renaming lut */
    node *vardecs;
    node *preassigns;
    node *intersectb1;
    node *intersectb2;
    node *idxbound1;
    node *idxbound2;
    bool onefundef;
};

/**
 * Macro definitions for INFO structure
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_PART(n) ((n)->part)
#define INFO_WL(n) ((n)->wl)
#define INFO_LEVEL(n) ((n)->level)
#define INFO_SWLFOLDABLEFOLDEEPART(n) ((n)->swlfoldablefoldeepart)
#define INFO_LUT(n) ((n)->lut)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_PREASSIGNS(n) ((n)->preassigns)
#define INFO_INTERSECTB1(n) ((n)->intersectb1)
#define INFO_INTERSECTB2(n) ((n)->intersectb2)
#define INFO_IDXBOUND1(n) ((n)->idxbound1)
#define INFO_IDXBOUND2(n) ((n)->idxbound2)
#define INFO_ONEFUNDEF(n) ((n)->onefundef)

static info *
MakeInfo (node *fundef)
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = fundef;
    INFO_PART (result) = NULL;
    INFO_WL (result) = NULL;
    INFO_LEVEL (result) = 0;
    INFO_SWLFOLDABLEFOLDEEPART (result) = NULL;
    INFO_LUT (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_INTERSECTB1 (result) = NULL;
    INFO_INTERSECTB2 (result) = NULL;
    INFO_IDXBOUND1 (result) = NULL;
    INFO_IDXBOUND2 (result) = NULL;
    INFO_ONEFUNDEF (result) = FALSE;

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
 * @fn node *SWLFdoSymbolicWithLoopFoldingOneFunction( node *fundef)
 *
 * @brief global entry point of symbolic With-Loop folding
 *
 * @param N_fundef apply SWLF.
 *
 * @return optimized N_fundef
 *
 *****************************************************************************/
node *
SWLFdoSymbolicWithLoopFoldingOneFunction (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("SWLFdoSymbolicWithLoopFoldingOneFunction");

    arg_info = MakeInfo (NULL);
    INFO_LUT (arg_info) = LUTgenerateLut ();

    TRAVpush (TR_swlf);
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
 * @fn bool isPrfArg1AttachIntersect( node *arg_node)
 *
 * @brief Predicate to check if arg1 of this N_prf has
 *        an F_attachintersect guard attached to it.
 *
 * @param arg_node: an N_prf
 *
 * @return Boolean TRUE if PRF_ARG2 is an F_attachintersect.
 *
 *****************************************************************************/
bool
isPrfArg1AttachIntersect (node *arg_node)
{
    node *arg1;
    node *assgn;
    bool z = FALSE;

    DBUG_ENTER ("isPrfArg1AttachIntersect");

    arg1 = PRF_ARG1 (arg_node);
    DBUG_ASSERT (N_id == NODE_TYPE (arg1),
                 "isPrfArg1AttachIntersect expected N_id as PRF_ARG1");
    assgn = AVIS_SSAASSIGN (ID_AVIS (arg1));
    if ((NULL != assgn) && (N_prf == NODE_TYPE (LET_EXPR (ASSIGN_INSTR (assgn))))
        && (F_attachintersect == PRF_PRF (LET_EXPR (ASSIGN_INSTR (assgn))))) {
        z = TRUE;
    }
    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn bool isPrfArg1AttachExtrema( node *arg_node)
 *
 * @brief Predicate to check if arg1 of this N_prf is an F_attachextrema op
 *  OR if arg1 is an F_attachintersect, and its PRF_ARG1 is an F_attachextrema.
 *  This is VERY brittle code, and we should find a more robust way
 *  to do this. FIXME.
 *  Like a PM of some sort...
 *
 * @param arg_node: an N_prf
 *
 * @return Boolean TRUE if PRF_ARG2 is an F_attachextrema.
 *
 *****************************************************************************/
bool
isPrfArg1AttachExtrema (node *arg_node)
{
    node *arg1;
    node *assgn;
    node *assgn2;
    node *prf;
    node *prf2;
    bool z = FALSE;

    DBUG_ENTER ("isPrfArg1AttachExtrema");

    arg1 = PRF_ARG1 (arg_node);
    DBUG_ASSERT (N_id == NODE_TYPE (arg1),
                 "isPrfArg1AttachExtrema expected N_id as PRF_ARG1");
    assgn = AVIS_SSAASSIGN (ID_AVIS (arg1));
    if ((NULL != assgn) && (N_prf == NODE_TYPE (LET_EXPR (ASSIGN_INSTR (assgn))))) {

        prf = LET_EXPR (ASSIGN_INSTR (assgn));
        if ((F_attachextrema == PRF_PRF (prf))) {
            z = TRUE;
        } else {
            assgn2 = AVIS_SSAASSIGN (ID_AVIS (PRF_ARG1 (prf)));
            if ((NULL != assgn)
                && (N_prf == NODE_TYPE (LET_EXPR (ASSIGN_INSTR (assgn2))))) {
                prf2 = LET_EXPR (ASSIGN_INSTR (assgn2));
                if ((F_attachintersect == PRF_PRF (LET_EXPR (ASSIGN_INSTR (assgn2))))) {
                    z = TRUE;
                }
            }
        }
    }
    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @name Static helper functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn bool matchGeneratorField( node *fa, node *fb, node *foldeewl)
 *
 * @brief Attempt to match corresponding N_id/N_array fields of
 *        two generators (BOUND, WIDTH, STEP),
 *        such as GENERATOR_BOUND1( wla) and GENERATOR_BOUND1( wlb).
 *        Fields "match" if they meet any of the following criteria:
 *         - both fields are NULL.
 *         - both fields refer to the same name.
 *         - both fields have a common ancestor in their SSAASSIGN chains.
 *         - fa can be shown to be the shape vector for foldewl.
 *
 * @param - fa is a GENERATOR_BOUND2 or similar generator field for
 *          the folder WL.
 *          fb is a GENERATOR_BOUND2 or similar generator field for
 *          the foldee WL.
 *
 * @return Boolean TRUE if both fields are the same or can
 *          be shown to represent the same shape vector.
 *
 *****************************************************************************/
static bool
matchGeneratorField (node *fa, node *fb)
{
    node *fav = NULL;
    node *fbv = NULL;
    constant *fafs = NULL;
    constant *fbfs = NULL;
    constant *fac;
    constant *fbc;
    bool z = FALSE;
    ;

    DBUG_ENTER ("matchGeneratorField");

    if (fa == fb) { /* SAA should do it this way most of the time */
        z = TRUE;
    }

    if ((!z) && (NULL != fa) && (NULL != fb)
        && (PMO (PMOarray (&fafs, &fav, fa)) && (PMO (PMOarray (&fbfs, &fbv, fb))))) {
        z = fav == fbv;
    }

    /* If one field is local and the other is a function argument,
     * we can have both AKV, but they will not be merged, at
     * last in saacyc today. If that ever gets fixed, we can
     * remove this check. This is a workaround for a performance
     * problem in sac/apex/buildv/buildv.sac
     */
    if ((!z) && (NULL != fa) && (NULL != fb) && TYisAKV (AVIS_TYPE (fa))
        && TYisAKV (AVIS_TYPE (fb))) {
        fac = COaST2Constant (fa);
        fbc = COaST2Constant (fa);
        z = COcompareConstants (fac, fbc);
        fac = COfreeConstant (fac);
        fbc = COfreeConstant (fbc);
    }

    fafs = (NULL != fafs) ? COfreeConstant (fafs) : fafs;
    fbfs = (NULL != fbfs) ? COfreeConstant (fbfs) : fbfs;

    if ((NULL != fa) && (NULL != fb)) {
        if (z) {
            DBUG_PRINT ("SWLF", ("matchGeneratorField %s and %s matched", AVIS_NAME (fa),
                                 AVIS_NAME (fb)));
        } else {
            DBUG_PRINT ("SWLF", ("matchGeneratorField %s and %s did not match",
                                 AVIS_NAME (fa), AVIS_NAME (fb)));
        }
    }
    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node * ExtractNthWLIntersection...
 *
 * @brief Extract the Nth WL bounds for the intersection of the
 *        foldee partition with partition partno of the foldee WL.
 *
 * @params partno: the partition number in the foldee WL.
 *         boundnum: 0 for BOUND1, 1 for BOUND2.
 *         idx: the index vector for the _sel_VxA( idx, foldee).
 *         The intersection calculations hang off the F_attachextrema
 *         that is the parent of idx, after the idx and its idxmax
 *         entries.
 *
 * @result: The lower/upper bounds of the WL intersection.
 *
 *****************************************************************************/
static node *
ExtractNthWLIntersection (int partno, int boundnum, node *idx)
{
    node *bnd;
    node *dfg;
    node *val = NULL;

    DBUG_ENTER ("ExtractNthWLIntersection");

    dfg = AVIS_SSAASSIGN (ID_AVIS (idx));
    dfg = LET_EXPR (ASSIGN_INSTR (dfg));
    DBUG_ASSERT ((F_attachintersect == PRF_PRF (dfg)),
                 ("FindMatchingPart wanted F_attachintersect as idx parent"));
    /* expressions are bound1, bound2 for each partition. */
    bnd = TCgetNthExprsExpr (((2 * partno) + boundnum + 2), PRF_ARGS (dfg));

    if (!(PMO (PMOlastVarGuards (&val, bnd)))) {
        DBUG_ASSERT (FALSE, ("ExtractNthWLIntersection could not find var!"));
    }

    DBUG_RETURN (val);
}

/** <!--********************************************************************-->
 *
 * @fn static
 * void MarkPartitionSliceNeeded( node *folderpart,
 *                                node *intersectb1, node *intersectb2,
 *                                node *idxbound1, idxbound2,
 *                                info *arg_info);
 *
 * @brief Possibly mark current folderWL partition as requiring slicing.
 *
 *        The partition requires slicing if:
 *          - intersects and bounds are N_array nodes.
 *          - the intersect is not NULL.
 *          - the intersects and bounds do not match.
 *            [Since we only get called when this is the case,
 *             they are guaranteed not to match.]
 *
 * @params: folderpart: folderWL partition that may need slicing
 *          intersectb1: lower bound of intersection between folderWL
 *                       sel operation index set and foldeeWL partition
 *                       bounds.
 *          intersectb2: upper bound of same.
 *          idxbound1:   lower bound of folderWL sel operation index set.
 *          idxboundi2:  lower bound of same.
 *          arg_info:    your basic arg_info node.
 *
 * @result: None.
 *
 * @note: It may happen that more than one partition in the folderWL
 *        requires slicing.
 *
 *****************************************************************************/
static void
MarkPartitionSliceNeeded (node *folderpart, node *intersectb1, node *intersectb2,
                          node *idxbound1, node *idxbound2, info *arg_info)
{
    DBUG_ENTER ("MarkPartitionSliceNeeded");
    if ((N_array == NODE_TYPE (intersectb1)) && (N_array == NODE_TYPE (intersectb2))
        && (N_array == NODE_TYPE (idxbound1)) && (N_array == NODE_TYPE (idxbound2))) {

        DBUG_PRINT ("SWLF", ("FIXME: check for NULL intersection"));
        INFO_INTERSECTB1 (arg_info) = intersectb1;
        INFO_INTERSECTB2 (arg_info) = intersectb2;
        INFO_IDXBOUND1 (arg_info) = idxbound1;
        INFO_IDXBOUND2 (arg_info) = idxbound2;
    }

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 *
 * @fn node * FindMatchingPart(...
 *
 * @brief check if a WL has a legal foldee partition.
 *        The requirements for folding are:
 *           - The WL foldeeWL operator is a genarray or modarray.
 *
 *           - The WL is a single-operator WL.
 *
 *           - The current folderWL's sel(idx, foldeeWL) index set matches
 *             the index set of some partition of the foldeeWL,
 *             or is a subset of that partition.
 *             We have already computed the intersection of those
 *             index sets, and they hang off the F_attachintersect node.
 *             The Nth foldeeWL partition can be folded if the
 *             Nth intersection set matches the index set of the
 *             folderWL partition. We don't even look
 *             at the foldeeWL any more, except it comes along for
 *             the ride as an easy way to pick the correct
 *             foldeeWL partition when we do find a match.
 *
 * @params *arg_node: the N_prf of the sel(idx, foldeeWL).
 *         *folderpart: the folderWL partition containing arg_node.
 *         *foldeeWL: the N_with of the foldeeWL.
 *
 * @result: The address of the matching foldee partition, if any.
 *          NULL if none is found.
 *
 *****************************************************************************/
static node *
FindMatchingPart (node *arg_node, info *arg_info, node *folderpart, node *foldeeWL)
{
    node *folderpg;
    node *gee;
    node *partee;
    node *intersectb1;
    node *intersectb2;
    node *idx;
    node *idxbound1;
    node *idxbound2;
    node *idxassign;
    node *idxparent;
    bool matched = FALSE;
    int partno = 0;

    DBUG_ENTER ("FindMatchingPart");
    DBUG_ASSERT (N_prf == NODE_TYPE (arg_node),
                 ("FindMatchingPart expected N_prf arg_node"));
    DBUG_ASSERT (N_with == NODE_TYPE (foldeeWL),
                 ("FindMatchingPart expected N_with foldeeWL"));
    DBUG_ASSERT (N_part == NODE_TYPE (folderpart),
                 ("FindMatchingPart expected N_part folderpart"));

    idx = PRF_ARG1 (arg_node); /* idx of _sel_VxA_( idx, foldeeWL) */
    idxassign = AVIS_SSAASSIGN (ID_AVIS (idx));
    DBUG_ASSERT (NULL != idxassign, ("FindMatchingPart found NULL SSAASSIGN"));
    idxparent = LET_EXPR (ASSIGN_INSTR (idxassign));
    DBUG_ASSERT (F_attachintersect == PRF_PRF (idxparent),
                 ("FindMatchingPart expected F_attachintersect as idx parent"));

    /* I know this looks weird. We have to turn idx's AVIS_MAXVAL into
     * generator-bounds form. So, we add 1 to it, and make it
     * PRF_ARG2 of the attachintersect. If you have a cleaner
     * idea, I'm all for it!
     */
    idxbound1 = AVIS_MINVAL (ID_AVIS (PRF_ARG1 (idxparent)));
    idxbound2 = ID_AVIS (PRF_ARG2 (idxparent));

    folderpg = PART_GENERATOR (folderpart);
    partee = WITH_PART (foldeeWL);

    while ((!matched) && partee != NULL) {
        gee = PART_GENERATOR (partee);
        intersectb1 = ID_AVIS (ExtractNthWLIntersection (partno, 0, idx));
        intersectb2 = ID_AVIS (ExtractNthWLIntersection (partno, 1, idx));
        DBUG_PRINT ("SWLF", ("Attempting to match partition #%d BOUND1 %s and %s", partno,
                             AVIS_NAME (idxbound1), AVIS_NAME (intersectb1)));
        DBUG_PRINT ("SWLF", ("Attempting to match partition #%d BOUND2 %s and %s", partno,
                             AVIS_NAME (idxbound2), AVIS_NAME (intersectb2)));
        if (
          /* Find and match Referents for generators, skipping default partitions */
          ((N_generator == NODE_TYPE (folderpg)) && (N_generator == NODE_TYPE (gee)))
          && (matchGeneratorField (idxbound1, intersectb1))
          && (matchGeneratorField (idxbound2, intersectb2)) &&

          (matchGeneratorField (GENERATOR_STEP (folderpg), GENERATOR_STEP (gee)))
          && (matchGeneratorField (GENERATOR_WIDTH (folderpg), GENERATOR_WIDTH (gee)))) {
            matched = TRUE;
            DBUG_PRINT ("SWLF", ("FindMatchingPart referents all match"));
        } else {
            partee = PART_NEXT (partee);
            partno++;
        }
    }

    if (matched) {
        DBUG_PRINT ("SWLF", ("FindMatchingPart matches"));
    } else {
        partee = NULL;
        DBUG_PRINT ("SWLF", ("FindMatchingPart does not match"));
    }

    DBUG_RETURN (partee);
}

#ifdef BROKE
/** <!--********************************************************************-->
 *
 *
 * @fn node * FindMatchingPart(...
 *
 * @brief check if a WL has a legal foldee partition.
 *        The requirements for folding are:
 *           - The WL foldeeWL operator is a genarray or modarray.
 *
 *           - The WL is a single-operator WL.
 *
 *           - The current folderWL's sel(idx, foldeeWL) index set matches
 *             the index set of some partition of the foldeeWL,
 *             or is a subset of that partition.
 *             We have already computed the intersection of those
 *             index sets, and they hang off the F_attachintersect node.
 *             The Nth foldeeWL partition can be folded if the
 *             Nth intersection set matches the index set of the
 *             folderWL partition. We don't even look
 *             at the foldeeWL any more, except it comes along for
 *             the ride as an easy way to pick the correct
 *             foldeeWL partition when we do find a match.
 *
 * @params *arg_node: the N_prf of the sel(idx, foldeeWL).
 *         *folderpart: the folderWL partition containing arg_node.
 *         *foldeeWL: the N_with of the foldeeWL.
 *
 * @result: The address of the matching foldee partition, if any.
 *          NULL if none is found.
 *
 *****************************************************************************/
static node *
FindMatchingPart (node *arg_node, info *arg_info, node *folderpart, node *foldeeWL)
{
    node *folderpg;
    node *gee;
    node *partee;
    node *intersectb1;
    node *intersectb2;
    node *idx;
    node *idxbound1;
    node *idxbound2;
    node *idxassign;
    node *idxparent;
    bool matched = FALSE;
    int partno = 0;

    DBUG_ENTER ("FindMatchingPart");
    DBUG_ASSERT (N_prf == NODE_TYPE (arg_node),
                 ("FindMatchingPart expected N_prf arg_node"));
    DBUG_ASSERT (N_with == NODE_TYPE (foldeeWL),
                 ("FindMatchingPart expected N_with foldeeWL"));
    DBUG_ASSERT (N_part == NODE_TYPE (folderpart),
                 ("FindMatchingPart expected N_part folderpart"));

    idx = PRF_ARG1 (arg_node); /* idx of _sel_VxA_( idx, foldeeWL) */
    idxassign = AVIS_SSAASSIGN (ID_AVIS (idx));
    DBUG_ASSERT (NULL != idxassign, ("FindMatchingPart found NULL SSAASSIGN"));
    idxparent = LET_EXPR (ASSIGN_INSTR (idxassign));
    DBUG_ASSERT (F_attachintersect == PRF_PRF (idxparent),
                 ("FindMatchingPart expected F_attachintersect as idx parent"));
    idxbound1 = AVIS_MINVAL (ID_AVIS (PRF_ARG1 (idxparent)));
    idxbound2 = AVIS_MAXVAL (ID_AVIS (PRF_ARG2 (idxparent)));

    folderpg = PART_GENERATOR (folderpart);
    partee = WITH_PART (foldeeWL);

    while ((!matched) && partee != NULL) {
        gee = PART_GENERATOR (partee);
        intersectb1 = ID_AVIS (ExtractNthWLIntersection (partno, 0, idx));
        intersectb2 = ID_AVIS (ExtractNthWLIntersection (partno, 1, idx));
        if (
          /* Find and match Referents for generators, skipping default partitions */
          ((N_generator == NODE_TYPE (folderpg)) && (N_generator == NODE_TYPE (gee)))
          && (matchGeneratorField (GENERATOR_STEP (folderpg), GENERATOR_STEP (gee)))
          && (matchGeneratorField (GENERATOR_WIDTH (folderpg), GENERATOR_WIDTH (gee)))) {
            DBUG_PRINT ("SWLF", ("FindMatchingPart STEP/WIDTH match"));
            if ((idxbound1 == intersectb1) && (idxbound2 == intersectb2)) {
                DBUG_PRINT ("SWLF", ("FindMatchingPart referents all match"));
                matched = TRUE;
            } else {
                DBUG_PRINT ("SWLF", ("FindMatchingPart intersects do not match"));
                MarkPartitionSliceNeeded (folderpart, intersectb1, intersectb2, idxbound1,
                                          idxbound2, arg_info);
            }
        }

        if (!matched) {
            partee = PART_NEXT (partee);
            partno++;
        }
    }

    if (matched) {
        DBUG_PRINT ("SWLF", ("FindMatchingPart matches"));
    } else {
        partee = NULL;
        DBUG_PRINT ("SWLF", ("FindMatchingPart does not match"));
    }

    DBUG_RETURN (partee);
}
#endif // BROKE

/** <!--********************************************************************-->
 *
 * @fn bool checkSWLFoldable( node *arg_node, info *arg_info,
 * node * folderpart, int level)
 *
 * @brief check if _sel_VxA_(idx, foldee), appearing in folder WL
 *        partition, part, is foldable into the folder WL.
 *        Most checks have already been made by SWLFI.
 *        Here, we check that the generators match,
 *        and that the only references to the foldeeWL result are
 *        in the folderWL.
 *
 * @param _sel_VxA_( idx, foldee)
 * @param folderpart: The partition into which we would like to fold this sel().
 * @param level
 * @result If the foldee is foldable into the folder, return the
 *         N_part of the foldee that should be used for the fold; else NULL.
 *
 *****************************************************************************/
static node *
checkSWLFoldable (node *arg_node, info *arg_info, node *folderpart, int level)
{
    node *foldeeid;
    node *foldeeavis;
    node *foldeeassign;
    node *foldeewl;
    node *foldeepart = NULL;

    DBUG_ENTER ("checkSWLFoldable");

    foldeeid = PRF_ARG2 (arg_node);
    foldeeavis = ID_AVIS (foldeeid);
    if ((NULL != AVIS_SSAASSIGN (foldeeavis))
        && (AVIS_DEFDEPTH (foldeeavis) + 1 == level)) {
        foldeeassign = ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (foldeeid)));
        if (NODE_TYPE (LET_EXPR (foldeeassign)) == N_with) {
            foldeewl = LET_EXPR (foldeeassign);
            DBUG_PRINT ("SWLF", ("WL %s: AVIS_NEEDCOUNT=%d; AVIS_WL_NEEDCOUNT=%d",
                                 AVIS_NAME (foldeeavis), AVIS_NEEDCOUNT (foldeeavis),
                                 AVIS_WL_NEEDCOUNT (foldeeavis)));

            if (AVIS_NEEDCOUNT (foldeeavis) == AVIS_WL_NEEDCOUNT (foldeeavis)) {
                foldeepart = FindMatchingPart (arg_node, arg_info, folderpart, foldeewl);
            }
        }
    } else {
        DBUG_PRINT ("SWLF", ("WL %s will never fold. AVIS_DEFDEPTH: %d, lavel: %d",
                             AVIS_NAME (foldeeavis), AVIS_DEFDEPTH (foldeeavis), level));
    }

    if (NULL != foldeepart) {
        AVIS_ISWLFOLDED (foldeeavis) = TRUE;
        DBUG_PRINT ("SWLF", ("WL %s will be folded.", AVIS_NAME (foldeeavis)));
    } else {
        DBUG_PRINT ("SWLF", ("WLs %s will not be folded.", AVIS_NAME (foldeeavis)));
    }

    DBUG_RETURN (foldeepart);
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
 *        we have a foldeeWL with generator of this form:
 *
 *    foldeeWL = with {
 *         ( . <= iv=[i,j] <= .) : _sel_VxA_( iv, AAA);
 *
 *        We want to perform renames in the foldeeWL code block as follows:
 *
 *        iv --> iv'
 *        i  --> i'
 *        j  --> j'
 *
 * @param: arg_node: one N_avis node of the foldeeWL generator (e.g., iv),
 *                   to serve as iv for above assigns.
 *         arg_info: your basic arg_info.
 *         shp:      the shape descriptor of the new LHS.
 *
 * @result: New N_avis node, e.g, iv'.
 *          Side effect: mapping iv -> iv' entry is now in LUT.
 *                       New vardec for iv'.
 *
 *****************************************************************************/
static node *
populateLut (node *arg_node, info *arg_info, shape *shp)
{
    node *navis;

    DBUG_ENTER ("populateLut");

    /* Generate a new LHS name for WITHID_VEC/IDS */
    navis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (arg_node)),
                        TYmakeAKS (TYcopyType (TYgetScalar (AVIS_TYPE (arg_node))), shp));

    if (isSAAMode ()) {
        AVIS_DIM (navis) = DUPdoDupTree (AVIS_DIM (arg_node));
        AVIS_SHAPE (navis) = DUPdoDupTree (AVIS_SHAPE (arg_node));
    }
    INFO_VARDECS (arg_info) = TBmakeVardec (navis, INFO_VARDECS (arg_info));
    LUTinsertIntoLutP (INFO_LUT (arg_info), arg_node, navis);

    DBUG_PRINT ("SWLF", ("Inserted WITHID_VEC into lut: oldname: %s, newname %s",
                         AVIS_NAME (arg_node), AVIS_NAME (navis)));

    DBUG_RETURN (navis);
}

/** <!--********************************************************************-->
 *
 * @ fn static node *makeIdxAssigns( node *arg_node, node *foldeePart)
 *
 * @brief for a foldee partition, with generator:
 *        (. <= iv=[i,j] < .)
 *        and a folder _sel_VxA_( idx, foldeeWL),
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
 * @result: an N_assign chain as above.
 *
 *****************************************************************************/
static node *
makeIdxAssigns (node *arg_node, info *arg_info, node *foldeePart)
{
    node *z = NULL;
    node *ids;
    node *narray;
    node *idxid;
    node *navis;
    node *nass;
    node *lhsids;
    node *lhsavis;
    node *sel;
    int k;

    DBUG_ENTER ("makeIdxAssigns");

    ids = WITHID_IDS (PART_WITHID (foldeePart));
    idxid = PRF_ARG1 (LET_EXPR (ASSIGN_INSTR (arg_node)));
    k = 0;

    while (NULL != ids) {
        /* Build k0 = [k]; */
        /* First, the k */
        narray = TCmakeIntVector (TBmakeExprs (TBmakeNum (k), NULL));
        navis = TBmakeAvis (TRAVtmpVar (),
                            TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (1, 1)));
        if (isSAAMode ()) {
            AVIS_DIM (navis) = TBmakeNum (1);
            AVIS_SHAPE (navis) = TCmakeIntVector (TBmakeExprs (TBmakeNum (1), NULL));
        }

        nass = TBmakeAssign (TBmakeLet (TBmakeIds (navis, NULL), narray), NULL);
        AVIS_SSAASSIGN (navis) = nass;
        z = TCappendAssign (nass, z);
        INFO_VARDECS (arg_info) = TBmakeVardec (navis, INFO_VARDECS (arg_info));

        lhsavis = populateLut (IDS_AVIS (ids), arg_info, SHcreateShape (0));
        DBUG_PRINT ("SWLF", ("makeIdxAssigns created %s = _sel_VxA_(%d, %s)",
                             AVIS_NAME (lhsavis), k, AVIS_NAME (ID_AVIS (idxid))));

        sel = TBmakeAssign (TBmakeLet (TBmakeIds (lhsavis, NULL),
                                       TCmakePrf2 (F_sel_VxA, TBmakeId (navis),
                                                   DUPdoDupNode (idxid))),
                            NULL);
        z = TCappendAssign (z, sel);
        AVIS_SSAASSIGN (lhsavis) = sel;

        if (isSAAMode ()) {
            AVIS_DIM (lhsavis) = TBmakeNum (0);
            AVIS_SHAPE (lhsavis) = TCmakeIntVector (NULL);
        }

        ids = IDS_NEXT (ids);
        k++;
    }

    /* Now generate iv = idx; */
    lhsids = WITHID_VEC (PART_WITHID (foldeePart));
    lhsavis = populateLut (IDS_AVIS (lhsids), arg_info, SHcreateShape (1, k));
    z = TBmakeAssign (TBmakeLet (TBmakeIds (lhsavis, NULL), DUPdoDupNode (idxid)), z);
    AVIS_SSAASSIGN (lhsavis) = z;
    DBUG_PRINT ("SWLF", ("makeIdxAssigns created %s = %s)", AVIS_NAME (lhsavis),
                         AVIS_NAME (ID_AVIS (idxid))));

    if (isSAAMode ()) {
        AVIS_DIM (lhsavis) = TBmakeNum (1);
        AVIS_SHAPE (lhsavis) = TCmakeIntVector (TBmakeExprs (TBmakeNum (k), NULL));
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *doSWLFreplace( ... )
 *
 * @brief
 *   In
 *    foldeeWL = with...  elb = _sel_VxA_(iv=[i,j], AAA) ...
 *    folderWL = with...  elc = _sel_VxA_(idx, foldeeWL) ...
 *
 *   Replace, in the folderWL:
 *     elc = _sel_VxA_( idx, foldeeWL)
 *   by
 *     iv = idx;
 *     i = _sel_VxA_([0], idx);
 *     j = _sel_VxA_([1], idx);
 *
 *     {code block from foldeeWL, with SSA renames}
 *
 *     tmp = foldeeWLresultelement)
 *     elc = tmp;
 *
 * @params
 *    arg_node: N_assign for the sel()
 *    fundef: N_fundef node, so we can insert new avis node for
 *            temp assign being made here.
 *    foldee: N_part node of foldee.
 *    folder: N_part node of folder.
 *****************************************************************************/
static node *
doSWLFreplace (node *arg_node, node *fundef, node *foldee, node *folder, info *arg_info)
{
    node *oldblock;
    node *newblock;
    node *newavis;
    node *idxassigns;
    node *expravis;

    DBUG_ENTER ("doSWLFreplace");

    oldblock = BLOCK_INSTR (CODE_CBLOCK (PART_CODE (foldee)));

    /* Generate iv=[i,j] assigns, then do renames. */
    idxassigns = makeIdxAssigns (arg_node, arg_info, foldee);

    /* If foldeeWL is empty, don't do any code substitutions.
     * Just replace sel(iv, foldeeWL) by iv.
     */
    newblock
      = (N_empty == NODE_TYPE (oldblock))
          ? NULL
          : DUPdoDupTreeLutSsa (oldblock, INFO_LUT (arg_info), INFO_FUNDEF (arg_info));

    expravis = ID_AVIS (EXPRS_EXPR (CODE_CEXPRS (PART_CODE (foldee))));
    newavis = LUTsearchInLutPp (INFO_LUT (arg_info), expravis);

    LUTremoveContentLut (INFO_LUT (arg_info));

    /**
     * replace the code
     */
    FREEdoFreeNode (LET_EXPR (ASSIGN_INSTR (arg_node)));
    LET_EXPR (ASSIGN_INSTR (arg_node)) = TBmakeId (newavis);
    if (NULL != newblock) {
        arg_node = TCappendAssign (newblock, arg_node);
    }

    arg_node = TCappendAssign (idxassigns, arg_node);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn static node *AppendPart(node *partz, node *newpart)
 *
 * @params: partz: an N_part, the current chain of new partitions
 *          newpart: an N_part, a new partition to be added to the chain.
 *
 * @brief: append newpart to partz.

 *****************************************************************************/
static node *
AppendPart (node *partz, node *newpart)
{
    DBUG_ENTER ("AppendPart");

    if (NULL == partz) {
        partz = newpart;
    } else {
        PART_NEXT (partz) = newpart;
    }

    DBUG_RETURN (partz);
}

/** <!--********************************************************************-->
 *
 * @fn static node *PartitionSlicer(...)
 *
 * @params partn: an N_part of the folderWL.
 *         idx: an N_array, representing the intersect of the
 *              folderWL index set and a foldeeWL partition.
 *         idx must be the same shape as the partn generators.
 *         d: the axis which we are going to slice. e.g., for matrix,
 *            d = 0 --> slice rows
 *            d = 1 --> slice columns
 *            etc.
 *
 * @result: 1-3 N_part nodes, depending on the value of idx.
 *
 * @brief Slice a WL partition into 1-3 partitions.
 *
 * We have a WL partition, partn, and an intersection index set, idx,
 * for the partition that is smaller than the partition. We wish
 * to slice partn into sub-partitions, in order that AWLF
 * can operate on the sub-partition(s).
 *
 * idx is known to lie totally within partn, as it arises from
 * the WL index set intersection bounds.
 *
 * In the simplest situation, there are three possible
 * cases of intersect. The rectangle represents partn;
 * the xxxx's represent the array covered by idx.
 *
 *   alpha        beta        gamma
 *  __________   __________  _________
 *  |xxxxxxxxx| | partA   | | partA   |
 *  |xxpartIxx| |         | |         |
 *  |         | |xxxxxxxxx| |         |
 *  |         | |xxpartIxx| |         |
 *  | partC   | |         | |xxxxxxxxx|
 *  |         | | partC   | |xxxxxxxxx|
 *  |_________| |_________| |xxpartIxx|
 *
 *  For cases alpha and gamma, we split partn into two parts.
 *  For case beta, we split it into three parts. In each case,
 *  one of the partitions is guaranteed to match idx.
 *
 * Because the intersection is multi-dimensional, we perform
 * the splitting on one axis at a time. Hence, we may end up
 * with each axis generating 1-3 new partitions for each
 * partition it gets as input.
 *
 * Here are the cases for splitting along axis 1 (columns) in a rank-2 array;
 * an x denotes partI:
 *
 * alpha   alpha  alpha
 *
 * x..     .x.    ..x
 * ...     ...    ...
 * ...     ...    ...
 *
 *
 * beta    beta   beta
 * ...     ...    ...
 * x..     .x.    ..x
 * ...     ...    ...
 *
 * gamma  gamma   gamma
 * ...    ...     ...
 * ...    ...     ...
 * x..    .x.     ..x
 *
 *
 *
 * Note: This code bears some resemblance to that of CutSlices.
 *       This is simpler, because we know more
 *       about the index set intersection.
 *
 * Note: Re the question of when to perform partition slicing.
 *       I'm not sure, but let's start here:
 *       We want to avoid a situation in which we slice
 *       a partition before we know that slicing is required.
 *       These are the requirements:
 *
 *        - The folderWL index set is an N_array.
 *        - The intersect of the folderWL index set with
 *          the foldeeWL partition bounds is:
 *            . non-empty  (or we are looking at a total mismatch)
 *            . not an exact match (because it could fold as is).
 *
 *****************************************************************************/
static node *
PartitionSlicer (node *partn, node *lb, node *ub, int d, info *arg_info)
{
    node *partz = NULL;
    node *newpart = NULL;
    node *lbpart;
    node *ubpart;
    node *step;
    node *width;
    node *withid;
    node *newlb;
    node *newub;
    node *genn;
    node *coden;
    node *ilb;
    node *iub;
    node *plb;
    node *pub;

    DBUG_ENTER ("PartitionSlicer");

    DBUG_ASSERT (N_part == NODE_TYPE (partn), "Partition Slicer expected N_part partn");
    DBUG_ASSERT (N_array == NODE_TYPE (lb), "Partition Slicer expected N_array lb");
    DBUG_ASSERT (N_array == NODE_TYPE (ub), "Partition Slicer expected N_array ub");
    lbpart = GENERATOR_BOUND1 (PART_GENERATOR (partn));
    ubpart = GENERATOR_BOUND2 (PART_GENERATOR (partn));
    step = GENERATOR_STEP (PART_GENERATOR (partn));
    width = GENERATOR_WIDTH (PART_GENERATOR (partn));
    coden = PART_CODE (partn);
    withid = PART_WITHID (partn);

    ilb = TCgetNthExprs (d, lb);
    iub = TCgetNthExprs (d, ub);
    plb = TCgetNthExprs (d, lbpart);
    pub = TCgetNthExprs (d, ubpart);

    /* Cases beta, gamma need partA */
    if (ilb != plb) { /*           this compare may have to be fancier */
        newlb = DUPdoDupTree (lbpart);
        newlb = WLSflattenBound (newlb, &INFO_VARDECS (arg_info),
                                 &INFO_PREASSIGNS (arg_info));

        newub = DUPdoDupTree (ubpart);
        /* newub[d] = iub */
        EXPRS_EXPR (newub) = FREEdoFreeTree (EXPRS_EXPR (newub));
        EXPRS_EXPR (newub) = DUPdoDupTree (EXPRS_EXPR (iub));
        newub = WLSflattenBound (newub, &INFO_VARDECS (arg_info),
                                 &INFO_PREASSIGNS (arg_info));

        genn = TBmakeGenerator (F_wl_le, F_wl_lt, newlb, newub, DUPdoDupTree (step),
                                DUPdoDupTree (width));
        newpart = TBmakePart (coden, DUPdoDupTree (withid), genn);
        CODE_INC_USED (coden);
        partz = AppendPart (partz, newpart);
    }

    /* All cases need partI */
    newlb = DUPdoDupTree (lb);
    newlb
      = WLSflattenBound (newlb, &INFO_VARDECS (arg_info), &INFO_PREASSIGNS (arg_info));
    newub = DUPdoDupTree (ub);
    newub
      = WLSflattenBound (newub, &INFO_VARDECS (arg_info), &INFO_PREASSIGNS (arg_info));

    genn = TBmakeGenerator (F_wl_le, F_wl_lt, newlb, newub, DUPdoDupTree (step),
                            DUPdoDupTree (width));
    newpart = TBmakePart (coden, DUPdoDupTree (withid), genn);
    CODE_INC_USED (coden);
    partz = AppendPart (partz, newpart);

    /* Case alpha, beta need partC */
    if (iub != pub) { /* this compare may have to be fancier */
        newlb = DUPdoDupTree (lb);
        newlb = WLSflattenBound (newlb, &INFO_VARDECS (arg_info),
                                 &INFO_PREASSIGNS (arg_info));

        newub = DUPdoDupTree (ubpart);
        newub = WLSflattenBound (newub, &INFO_VARDECS (arg_info),
                                 &INFO_PREASSIGNS (arg_info));

        genn = TBmakeGenerator (F_wl_le, F_wl_lt, newlb, newub, DUPdoDupTree (step),
                                DUPdoDupTree (width));
        newpart = TBmakePart (coden, DUPdoDupTree (withid), genn);
        CODE_INC_USED (coden);
        partz = AppendPart (partz, newpart);
    }
    DBUG_RETURN (partz);
}

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *SWLFfundef(node *arg_node, info *arg_info)
 *
 * @brief applies SWLF to a given fundef.
 *
 *****************************************************************************/
node *
SWLFfundef (node *arg_node, info *arg_info)
{
    bool old_onefundef;

    DBUG_ENTER ("SWLFfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {

        DBUG_PRINT ("SWLF", ("Symbolic With-Loop folding in %s %s begins",
                             (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                             FUNDEF_NAME (arg_node)));

        INFO_FUNDEF (arg_info) = arg_node;

        arg_node = WLNCdoWLNeedCount (arg_node);
        arg_node = WLCCdoWLCostCheck (arg_node);

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        /* If new vardecs were made, append them to the current set */
        if (INFO_VARDECS (arg_info) != NULL) {
            BLOCK_VARDEC (FUNDEF_BODY (arg_node))
              = TCappendVardec (INFO_VARDECS (arg_info),
                                BLOCK_VARDEC (FUNDEF_BODY (arg_node)));
            INFO_VARDECS (arg_info) = NULL;
        }

        old_onefundef = INFO_ONEFUNDEF (arg_info);
        INFO_ONEFUNDEF (arg_info) = FALSE;
        FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
        INFO_ONEFUNDEF (arg_info) = old_onefundef;

        DBUG_PRINT ("SWLF", ("Symbolic With-Loop folding in %s %s ends",
                             (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                             FUNDEF_NAME (arg_node)));
    }
    INFO_FUNDEF (arg_info) = NULL;

    if (NULL != FUNDEF_NEXT (arg_node)) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node SWLFassign( node *arg_node, info *arg_info)
 *
 * @brief performs a top-down traversal.
 *        For a foldable WL, arg_node is x = _sel_VxA_(iv, foldee).
 *
 *****************************************************************************/
node *
SWLFassign (node *arg_node, info *arg_info)
{
    node *foldablefoldeepart;

    DBUG_ENTER ("SWLFassign");

#ifdef VERBOSE
    DBUG_PRINT ("SWLF", ("Traversing N_assign"));
#endif // VERBOSE
    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    foldablefoldeepart = INFO_SWLFOLDABLEFOLDEEPART (arg_info);
    INFO_SWLFOLDABLEFOLDEEPART (arg_info) = NULL;
    DBUG_ASSERT ((NULL == INFO_PREASSIGNS (arg_info)),
                 "SWLFassign INFO_PREASSIGNS not NULL");

    /*
     * Top-down traversal
     */
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    /*
     * Append the new cloned block
     */
    if (NULL != foldablefoldeepart) {
        arg_node = doSWLFreplace (arg_node, INFO_FUNDEF (arg_info), foldablefoldeepart,
                                  INFO_PART (arg_info), arg_info);

        global.optcounters.swlf_expr += 1;
    }

    if (NULL != INFO_PREASSIGNS (arg_info)) {
        arg_node = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
        INFO_PREASSIGNS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SWLFwith( node *arg_node, info *arg_info)
 *
 * @brief applies SWLF to a with-loop in a top-down manner.
 *
 *****************************************************************************/
node *
SWLFwith (node *arg_node, info *arg_info)
{
    node *nextop;
    node *genop;
    node *folderop;
    node *foldeeshape;
    info *old_info;

    DBUG_ENTER ("SWLFwith");

    DBUG_PRINT ("SWLF", ("Examining N_with"));
    old_info = arg_info;
    arg_info = MakeInfo (INFO_FUNDEF (arg_info));
    INFO_WL (arg_info) = arg_node;
    INFO_LUT (arg_info) = INFO_LUT (old_info);
    INFO_LEVEL (arg_info) = INFO_LEVEL (old_info) + 1;
    INFO_VARDECS (arg_info) = INFO_VARDECS (old_info);
    INFO_PREASSIGNS (arg_info) = INFO_PREASSIGNS (old_info);

    WITH_REFERENCED_FOLDERWL (arg_node) = NULL;
    WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);

    /* Try to replace modarray(foldeeWL) by genarray(shape(foldeeWL)).
     * This has no effect on the foldeeWL itself, but is needed to
     * eliminate the reference to foldeeWL, so it can be removed by DCR.
     *
     * If the foldeeWL has been folded, its result will have
     * AVIS_FOLDED set. Since there are, by the definition of
     * folding, no other references to the foldeeWL, we can
     * blindly replace the modarray by the genarray.
     */
    folderop = WITH_WITHOP (arg_node);
    if ((N_modarray == NODE_TYPE (folderop))
        && (NULL != AVIS_SHAPE (ID_AVIS (MODARRAY_ARRAY (folderop))))
        && (TRUE == AVIS_ISWLFOLDED (ID_AVIS (MODARRAY_ARRAY (folderop))))) {
        foldeeshape = AVIS_SHAPE (ID_AVIS (MODARRAY_ARRAY (folderop)));
        genop = TBmakeGenarray (DUPdoDupTree (foldeeshape), NULL);
        GENARRAY_NEXT (genop) = MODARRAY_NEXT (folderop);
        nextop = FREEdoFreeNode (folderop);
        WITH_WITHOP (arg_node) = genop;
        DBUG_PRINT ("SWLF", ("Replacing modarray by genarray"));
    }

    INFO_WL (old_info) = NULL;
    INFO_VARDECS (old_info) = INFO_VARDECS (arg_info);
    INFO_PREASSIGNS (old_info) = INFO_PREASSIGNS (arg_info);
    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SWLFcode( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
SWLFcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SWLFcode");

    DBUG_PRINT ("SWLF", ("Traversing N_code"));
    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SWLFpart( node *arg_node, info *arg_info)
 *
 * @brief Traverse each partition of a WL.
 *
 *****************************************************************************/
node *
SWLFpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SWLFpart");

    DBUG_PRINT ("SWLF", ("Traversing N_part"));
    INFO_PART (arg_info) = arg_node;
    PART_CODE (arg_node) = TRAVdo (PART_CODE (arg_node), arg_info);
    INFO_PART (arg_info) = NULL;

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SWLFids( node *arg_node, info *arg_info)
 *
 * @brief set current With-Loop level as ids defDepth attribute
 *
 *****************************************************************************/
node *
SWLFids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SWLFids");

#ifdef VERBOSE
    DBUG_PRINT ("SWLF", ("Traversing N_ids"));
#endif // VERBOSE
    AVIS_DEFDEPTH (IDS_AVIS (arg_node)) = INFO_LEVEL (arg_info);
    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SWLFprf( node *arg_node, info *arg_info)
 *
 * @brief
 *   Examine all _sel_VxA_( idx, foldeeWL)  primitives to see if
 *   the _sel_VxA_ is contained inside a WL, and that idx has
 *   intersect information attached to it.
 *   If so, foldeeWL may be a candidate for folding into this WL.
 *
 *   If idx does not have an SSAASSIGN, it means idx is a WITHID.
 *   If so, we will visit here again, after extrema have been
 *   attached to idx, and idx renamed.
 *
 *****************************************************************************/
node *
SWLFprf (node *arg_node, info *arg_info)
{
    node *arg1;

    DBUG_ENTER ("SWLFprf");

#ifdef VERBOSE
    DBUG_PRINT ("SWLF", ("Traversing N_prf"));
#endif // VERBOSE
    arg1 = PRF_ARG1 (arg_node);
    if ((INFO_PART (arg_info) != NULL) && (PRF_PRF (arg_node) == F_sel_VxA)
        && (isPrfArg1AttachIntersect (arg_node))) {
        INFO_SWLFOLDABLEFOLDEEPART (arg_info)
          = checkSWLFoldable (arg_node, arg_info, INFO_PART (arg_info),
                              INFO_LEVEL (arg_info));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/******************************************************************************
 *
 * function:
 *   node *SWLFcond(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse conditional parts in the given order.
 *
 ******************************************************************************/
node *
SWLFcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SWLFcond");

    DBUG_PRINT ("SWLF", ("Traversing N_cond"));
    COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);
    COND_THENINSTR (arg_node) = TRAVdo (COND_THENINSTR (arg_node), arg_info);
    COND_ELSEINSTR (arg_node) = TRAVdo (COND_ELSEINSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SWLFfuncond( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/
node *
SWLFfuncond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SWLFfuncond");

    DBUG_PRINT ("SWLF", ("Traversing N_funcond"));
    FUNCOND_IF (arg_node) = TRAVopt (FUNCOND_IF (arg_node), arg_info);
    FUNCOND_THEN (arg_node) = TRAVopt (FUNCOND_THEN (arg_node), arg_info);
    FUNCOND_ELSE (arg_node) = TRAVopt (FUNCOND_ELSE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SWLFwhile( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/
node *
SWLFwhile (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SWLFwhile");

    DBUG_PRINT ("SWLF", ("Traversing N_while"));
    WHILE_COND (arg_node) = TRAVopt (WHILE_COND (arg_node), arg_info);
    WHILE_BODY (arg_node) = TRAVopt (WHILE_BODY (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Symbolic with loop folding -->
 *****************************************************************************/
