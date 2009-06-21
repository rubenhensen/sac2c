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
 * @fn node *SWLFdoSymbolicWithLoopFolding( node *fundef)
 *
 * @brief global entry point of symbolic With-Loop folding
 *
 * @param fundef N_module to apply SWLF.
 *
 * @return optimized N_module
 *
 *****************************************************************************/
node *
SWLFdoSymbolicWithLoopFolding (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("SWLFdoSymbolicWithLoopFolding");

    arg_info = MakeInfo (NULL);
    INFO_LUT (arg_info) = LUTgenerateLut ();

    TRAVpush (TR_swlf);
    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);
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
    bool mat = FALSE;
    ;

    DBUG_ENTER ("matchGeneratorField");

    if (fa == fb) { /* SAA should do it this way most of the time */
        mat = TRUE;
    } else {
        if ((NULL != fa) && (NULL != fb)) {
            if (fa == fb) {
                mat = TRUE;
            } else {
                if (PM (PMarray (&fafs, &fav, fa)) && PM (PMarray (&fbfs, &fbv, fb))) {
                    mat = fav == fbv;
                }
            }
        }
    }

    fafs = (NULL != fafs) ? COfreeConstant (fafs) : fafs;
    fbfs = (NULL != fbfs) ? COfreeConstant (fbfs) : fbfs;

    if (mat) {
        DBUG_PRINT ("SWLF", ("matchGeneratorField matched PMarray"));
    } else {
        DBUG_PRINT ("SWLF", ("matchGeneratorField could not match PMarray"));
    }
    DBUG_RETURN (mat);
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

    DBUG_ENTER ("ExtractNthWLIntersection");

    dfg = AVIS_SSAASSIGN (ID_AVIS (idx));
    dfg = LET_EXPR (ASSIGN_INSTR (dfg));
    DBUG_ASSERT ((F_attachintersect == PRF_PRF (dfg)),
                 ("FindMatchingPart wanted F_attachintersect as idx parent"));
    /* expressions are bound1, bound2 for each partition. */
    bnd = TCgetNthExprsExpr (((2 * partno) + boundnum + 2), PRF_ARGS (dfg));
    DBUG_RETURN (bnd);
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
FindMatchingPart (node *arg_node, node *folderpart, node *foldeeWL)
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
    bool m1 = FALSE;
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
          && (idxbound1 == intersectb1) && (idxbound2 == intersectb2) &&

          (matchGeneratorField (GENERATOR_STEP (folderpg), GENERATOR_STEP (gee)))
          && (matchGeneratorField (GENERATOR_WIDTH (folderpg), GENERATOR_WIDTH (gee)))) {
            m1 = TRUE;
            DBUG_PRINT ("SWLF", ("FindMatchingPart referents all match"));
        } else { /* Ye olde school way */
            m1 = (CMPT_EQ == CMPTdoCompareTree (folderpg, gee));
            DBUG_PRINT ("SWLF", ("FindMatchingPart referents all match, olde school"));
        }

        if (m1 && (WITHOP_NEXT (WITH_WITHOP (foldeeWL)) == NULL)
            && ((NODE_TYPE (WITH_WITHOP (foldeeWL)) == N_genarray)
                || (NODE_TYPE (WITH_WITHOP (foldeeWL)) == N_modarray))) {
            matched = TRUE;
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
                foldeepart = FindMatchingPart (arg_node, folderpart, foldeewl);
            }
        }
    } else {
        DBUG_PRINT ("SWLF", ("WL %s will not fold. AVIS_DEFDEPTH: %d, lavel: %d",
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
 * @brief Populate one element of a look up table for mapping names
 *        in the copied WL code block. See caller for description. Basically,
 *        we have a foldeeWL with generator of this form:
 *
 *    foldeeWL = with...  elb = _sel_VxA_(iv=[i,j], AAA) ...
 *
 *        We want to perform renames of the foldeeWL code block as follows:
 *
 *        iv --> iv'
 *        i  --> i'
 *        j  --> j'
 *
 * @param: arg_node: one N_ids node of the foldeeWL generator (e.g., iv),
 *                   to serve as RHS for above assigns.
 *         arg_info: your basic arg_info.
 *         shp:      the shape descriptor of the new LHS.
 *
 * @result: New LHS N_avis node, e.g, iv'.
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
    navis
      = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (IDS_AVIS (arg_node))),
                    TYmakeAKS (TYcopyType (TYgetScalar (AVIS_TYPE (IDS_AVIS (arg_node)))),
                               shp));

    if (isSAAMode ()) {
        AVIS_DIM (navis) = DUPdoDupTree (AVIS_DIM (IDS_AVIS (arg_node)));
        AVIS_SHAPE (navis) = DUPdoDupTree (AVIS_SHAPE (IDS_AVIS (arg_node)));
    }
    INFO_VARDECS (arg_info) = TBmakeVardec (navis, INFO_VARDECS (arg_info));
    LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (arg_node), navis);

    DBUG_PRINT ("SWLF", ("Inserted WITHID_VEC into lut: oldname: %s, newname %s",
                         AVIS_NAME (IDS_AVIS (arg_node)), AVIS_NAME (navis)));

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

        lhsavis = IDS_AVIS (ids);
        DBUG_PRINT ("SWLF", ("makeIdxAssigns created %s = _sel_VxA_(%d, %s)",
                             AVIS_NAME (lhsavis), k, AVIS_NAME (ID_AVIS (idxid))));
        sel = TBmakeAssign (TBmakeLet (DUPdoDupNode (ids),
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
    lhsavis = IDS_AVIS (lhsids);

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
    node *newblock2;
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
    newblock = (N_empty == NODE_TYPE (oldblock)) ? idxassigns
                                                 : TCappendAssign (idxassigns, oldblock);

    newblock2
      = DUPdoDupTreeLutSsa (newblock, INFO_LUT (arg_info), INFO_FUNDEF (arg_info));

    /* FIXME the following FREE is crap, as it kills DCR immediately,
     * but removing it causes memory leak.
  FREEdoFreeTree( idxassigns);
     */

    expravis = ID_AVIS (EXPRS_EXPR (CODE_CEXPRS (PART_CODE (foldee))));
    newavis = LUTsearchInLutPp (INFO_LUT (arg_info), expravis);

    LUTremoveContentLut (INFO_LUT (arg_info));

    /**
     * replace the code
     */
    FREEdoFreeNode (LET_EXPR (ASSIGN_INSTR (arg_node)));
    LET_EXPR (ASSIGN_INSTR (arg_node)) = TBmakeId (newavis);
    if (NULL != newblock2) {
        arg_node = TCappendAssign (newblock2, arg_node);
    }

    DBUG_RETURN (arg_node);
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

        DBUG_PRINT ("SWLF", ("Symbolic With-Loop folding in %s %s ends",
                             (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                             FUNDEF_NAME (arg_node)));

        FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
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

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    foldablefoldeepart = INFO_SWLFOLDABLEFOLDEEPART (arg_info);
    INFO_SWLFOLDABLEFOLDEEPART (arg_info) = NULL;

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

    old_info = arg_info;
    arg_info = MakeInfo (INFO_FUNDEF (arg_info));
    INFO_WL (arg_info) = arg_node;
    INFO_LUT (arg_info) = INFO_LUT (old_info);
    INFO_LEVEL (arg_info) = INFO_LEVEL (old_info) + 1;
    INFO_VARDECS (arg_info) = INFO_VARDECS (old_info);

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

    AVIS_DEFDEPTH (IDS_AVIS (arg_node)) = INFO_LEVEL (arg_info);
    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SWLFprf( node *arg_node, info *arg_info)
 *
 * @brief
 *   Examine all X[iv] primitives to see if iv is current folderWL iv.
 *   If the iv does not have an SSAASSIGN, it means iv is a WITHID,
 *   and so SWLFI has not yet inserted an F_attachextrema for it.
 *
 *****************************************************************************/
node *
SWLFprf (node *arg_node, info *arg_info)
{
    node *arg1;

    DBUG_ENTER ("SWLFprf");

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

/** <!--********************************************************************-->
 * @}  <!-- Symbolic with loop folding -->
 *****************************************************************************/
