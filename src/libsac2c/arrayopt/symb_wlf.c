/*
 * $Id$
 */

// #define DAOEN

/** <!--********************************************************************-->
 *
 * @defgroup swlf Symbolic With-Loop Folding
 *
 * @terminology:
 *        Foldee-WL, or foldee: The WL that will no longer
 *        exist after this phase completes. In the example
 *        below, A is the foldee-WL.
 *
 *        Folder-WL: the WL that will absorb the block(s) from
 *        the foldee-WL. In the example below, B is the folder-WL.
 *
 * @brief Extended With-Loop Folding
 *        This performs WLF on arrays that are not foldable
 *        by WLF.
 *
 *        The features of extended SWLF are:
 *
 *            - Ability to fold arrays whose shapes are not
 *              known statically. Specifically, if the index
 *              set of the folder-WL is known to be identical to,
 *              or a subset of, the index set of the foldee-WL,
 *              folding will occur.
 *
 *              FIXME: This will have to be
 *              updated once the indices can accept offsets.
 *
 *           - the foldee-WL must have an SSAASSIGN.
 *
 *           - the foldee-WL must have a NEEDCOUNT of 1. I.e.,
 *             there must be no other references to the foldee-WL.
 *             If CVP and friends have done their job, this should
 *             not be a severe restriction.
 *             This code includes a little trick to convert a modarray(foldee)
 *             into a genarray(shape(foldee)), but this happens
 *             after any folding. Hence, there is a kludge to
 *             allow a NEEDCOUNT of 2 if there is a modarray folder-WL
 *             present.
 *
 *           - the foldee-WL must have a DEPDEPTH value of 1??
 *             Not sure what this means yet...
 *
 *           - The folder-WL must refer to the foldee WL via
 *              _sel_VxA_(idx, foldee)
 *             and idx must be the folder-WL's WITHID.
 *
 *           - The foldee-WL operator is a genarray or modarray
 *             (NO folds, please).
 *
 *           - The WL is a single-operator WL. (These should have been
 *             eliminated by phase 10, I think.)
 *
 *           - The generator of the folder- WL matches
 *             the generator of the foldee-WL, or is a subset
 *             of it. Note that this implies that
 *             the WL-bounds are the same length, which
 *             may not be the case. Consider this example:
 *
 *               x = with([0] <= iv < [n]) ... : [1,2,3,4]; NB. x is int[.,.]
 *               z = with([0,0], <= iv < shape(x)) :  x[iv];
 *
 *             The bounds of x are int[1], while the bounds of z are int[2].
 *             It is important that SWLFI not generate a partition
 *             intersection expression such as:
 *
 *              _swlfi_789 = _max_VxV_( bound1(x), bound1(z));
 *
 *             because TUP will get very confused about the length
 *             error. As I did...
 *
 *         This phase now must run when SAA information is available,
 *         because I intend to kill the dicey code that attempted to make
 *         it work in the absence of -dosaa information.
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

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    node *part;
    /* This is the current partition in the folder-WL. */
    node *wl;
    /* This is the current folder-WL. */
    int level;
    /* This is the current nesting level of WLs */
    node *swlfoldablefoldeepart;
    lut_t *lut;
    /* This is the WITH_ID renaming lut */
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
 * @fn bool isPrfArg1DataFlowGuard( node *arg_node)
 *
 * @brief Predicate to check if arg1 of this N_prf is an F_dataflowguard op
 *
 * @param arg_node: an N_prf
 *
 * @return Boolean TRUE if PRF_ARG2 is an F_dataflowguard.
 *
 *****************************************************************************/
bool
isPrfArg1DataFlowGuard (node *arg_node)
{
    node *arg1;
    node *assgn;
    bool z = FALSE;

    DBUG_ENTER ("isPrfArg1DataFlowGuard");

    arg1 = PRF_ARG1 (arg_node);
    DBUG_ASSERT (N_id == NODE_TYPE (arg1),
                 "isPrfArg1DataFlowGuard expected N_id as PRF_ARG1");
    assgn = AVIS_SSAASSIGN (ID_AVIS (arg1));
    if ((NULL != assgn) && (N_prf == NODE_TYPE (LET_EXPR (ASSIGN_INSTR (assgn))))
        && (F_dataflowguard == PRF_PRF (LET_EXPR (ASSIGN_INSTR (assgn))))) {
        z = TRUE;
    }
    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @name Static helper functions
 * @{
 *
 *****************************************************************************/

#ifdef DICEYCODE
/** <!--********************************************************************-->
 *
 * @fn static bool sameWL( node *arg2, node *foldeewl)
 *
 * @brief Predicate to check if arg2 is the result of foldewl
 *
 * @param arg2: an N_id that may be the result of the WL foldeewl.
 *        foldeewl: an N_with.
 *
 * @return Boolean TRUE if N_id was formed by foldeewl
 *
 *****************************************************************************/
static bool
sameWL (node *arg2, node *foldeewl)
{
    bool z;
    node *idas;

    DBUG_ENTER ("sameWL");

    idas = AVIS_SSAASSIGN (ID_AVIS (arg2));
    z = (NULL != idas) && (foldeewl == LET_EXPR (ASSIGN_INSTR (idas)));

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn static bool checkElement( node *curel, int i, node *foldeewl)
 *
 * @brief Vets one element of an N_array generator bound.
 *
 * @param curel: an N_assign node from an ARRAY_AELEMS.
 *               If it's an N_num, we fail, because this case
 *               should be caught elsewhere.
 *        i:     the index into the ARRAY_AELEMS node.
 *               Hence, the index into the bound's shape vector
 *        foldeewl: the WL that is intended to be folded out of
 *               existence.
 *
 * @return Boolean TRUE if the generator bound is of the form:
 *                 _idx_shape_sel( i, arr)
 *                 where i is the appropriate index, and
 *                 arr is the foldee WL.
 *
 *****************************************************************************/
static bool
checkElement (node *curel, int i, node *foldeewl)
{
    node *el;
    node *arg1 = NULL;
    constant *arg1fs = NULL;
    node *arg2 = NULL;
    bool z = FALSE;

    DBUG_ENTER ("checkElement");

    if (N_id == NODE_TYPE (curel)) {
        el = AVIS_SSAASSIGN (ID_AVIS (curel));
        if (NULL != el) {
            el = LET_EXPR (ASSIGN_INSTR (el));
            if (PM (
                  PMvar (&arg2, PMintConst (&arg1fs, &arg1, PMprf (F_idx_shape_sel, el))))
                && (i == COconst2Int (arg1fs)) && /* Constant match */
                sameWL (arg2, foldeewl)) {
                z = TRUE;
            }
        }
    }

    arg1fs = (NULL != arg1fs) ? COfreeConstant (arg1fs) : NULL;

    if (z) {
        DBUG_PRINT ("SWLF", ("checkElement can fold %s", AVIS_NAME (ID_AVIS (arg2))));
    } else {
        DBUG_PRINT ("SWLF", ("checkElement can not fold %s", AVIS_NAME (ID_AVIS (arg2))));
        DBUG_PRINT ("SWLF", ("checkElement can fold %s", AVIS_NAME (ID_AVIS (arg2))));
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 * @fn static bool scalarCell( node *foldeewl)
 *
 * @brief Check that the foldee WL is either a modarray,
 *        or a genarray with scalar default cell.
 *
 * @param foldeewl: the foldee WL N_id node
 *
 * @result: TRUE if WL is modarray or has scalar (or NULL) default cell.
 *
 *****************************************************************************/
static bool
scalarCell (node *foldeewl)
{
    node *def;
    bool z;

    DBUG_ENTER ("scalarCell");

    z = N_modarray == NODE_TYPE (WITH_WITHOP (foldeewl));

    if (N_genarray == NODE_TYPE (WITH_WITHOP (foldeewl))) {
        def = GENARRAY_DEFAULT (WITH_WITHOP (foldeewl));
        z = z || (NULL == def)
            || ((N_id == NODE_TYPE (def) && TYisScalar (AVIS_TYPE (ID_AVIS (def)))));
        /* FIXME: above line may be less than swell if def is
         * an N_num or other scalar type
         */
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn static bool shapeMatchesArray( node *fa, node *fb, node *foldeewl)
 *
 * @brief Ensure that GENERATOR nodes fa and fb match non-trivially.
 *        fa and fb are not the same, nor do they have identical
 *        ancestors.
 *
 *        We do this by showing that fa == shape(foldeewl).
 *
 * @param fa: the folder WL GENERATOR N_id node.
 *        fb: the foldee WL GENERATOR N_id node.
 *        foldeewl: The WL that we want to fold out of existence.
 *
 *             If all is well, fa's ancestor is an N_array that
 *             is shape(foldeewl), taking the form:
 *
 *               fa = [ arr0, arr1, ...]
 *
 *             with these properties:
 *
 *             1.
 *                  arr0 = _idx_shape_sel(0, foldeewl);
 *                  arr1 = _idx_shape_sel(1, foldeewl);
 *                  arr2 = _idx_shape_sel(2, foldeewl);
 *                  ...
 *
 *             2. dim(fb) == shape(fa)[0].
 *
 *             3. The default cell shape of a genarray foldewl
 *                must be scalar, or the foldewl must be a modarray.
 *
 *             4. PRF_ARG2 of all the idx_shape_sel ops must be fb.
 *
 *
 * @return Boolean TRUE if fa == shape( fb).
 *
 *****************************************************************************/
static bool
shapeMatchesArray (node *fa, node *fb, node *foldeewl)
{
    node *nfa = NULL;
    constant *nfafs = NULL;
    node *nfb = NULL;
    constant *nfbfs = NULL;
    node *nextel;
    node *curel;
    bool mat = TRUE;
    int rnk;
    int i;

    DBUG_ENTER ("shapeMatchesArray");

    if (PM (PMarray (&nfbfs, &nfb, fb))) {
        COfreeConstant (nfbfs);
    } else {
        DBUG_ASSERT (FALSE, ("shapeMatches array did not find N_array for fb"));
    }

    /* 1. performed partly by PM:  fa is has an N_array ancestor */
    if (PM (PMarray (&nfafs, &nfa, fa))) {
        rnk = SHgetUnrLen (ARRAY_FRAMESHAPE (nfa));
        COfreeConstant (nfafs);

        /* 2. dim(fb) == shape(fa)[0] */
        if (rnk == TYgetDim (ARRAY_ELEMTYPE (nfb))) {
            nextel = ARRAY_AELEMS (nfa);
            for (i = 0; i < rnk; i++) {
                DBUG_ASSERT (NULL != nextel, ("shapeMatchesArray saw corrupt N_array"));
                curel = EXPRS_EXPR (nextel);
                /* 1. idx_shape_sel checks performed by checkElement */
                /* 4. performed by checkElement */
                mat = mat && checkElement (curel, i, foldeewl);
                nextel = EXPRS_NEXT (nextel);
            }
            /* 3. check for genarray scalar default cell, or modarray */
            mat = mat && scalarCell (foldeewl);
        } else {
            mat = FALSE;
        }

    } else {
        mat = FALSE;
    }

    DBUG_RETURN (mat);
}
#endif // DICEYCODE

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
#ifdef DICEYCODE
    mat = shapeMatchesArray (fa, fb, foldeewl);
#endif // DICEYCODE
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
 *         The intersection calculations hang off the F_dataflowguard
 *         that is the parent of idx.
 *
 * @result: The lower/upper bounds of the WL intersection.
 *
 *****************************************************************************/
static node *
ExtractNthWLIntersection (int partno, int boundnum, node *idx)
{
    node *bnd;
    node *dfg;

    DBUG_ENTER ("FindMatchingPart");

    dfg = AVIS_SSAASSIGN (ID_AVIS (idx));
    dfg = LET_EXPR (ASSIGN_INSTR (dfg));
    DBUG_ASSERT ((F_dataflowguard == PRF_PRF (dfg)),
                 ("FindMatchingPart expected F_dataflowguard as parent of idx"));
    /* expressions are bound1, bound2 for each partition. */
    bnd = TCgetNthExprsExpr (((2 * partno) + boundnum + 1), PRF_ARGS (dfg));
    DBUG_RETURN (bnd);
}

/** <!--********************************************************************-->
 *
 *
 * @fn node * FindMatchingPart(...
 *
 * @brief check if a WL has a legal foldee partition.
 *        The requirements for folding are:
 *           - The WL foldee-WL operator is a genarray or modarray.
 *
 *           - The WL is a single-operator WL.
 *
 *           - The current partition of the folder-WL matches some generator
 *               of that of the foldee-WL, or is a subset
 *               of it.
 *
 *           - The sel(idx,y) idx is the WITHID of the
 *             putative foldee, or a direct descendant of same.
 *
 * @params *foldee:  the putative foldee-WL: the WL that
 *                 would be subsumed into the one at "part".
 *         *index: The idx in sel(idx, y) of the WL that is selecting
 *                 from the result of WL.
 *         *folderpart: The partition of the folder WL.
 *
 * @result: The address of the matching foldee partition, if any.
 *          NULL if none is found.
 *
 *
 *****************************************************************************/
static node *
FindMatchingPart (node *foldee, node *idx, node *folderpart)
{
    node *folderpg;
    node *wg;
    node *wp;
    node *intersectb1;
    node *intersectb2;
    bool matched = FALSE;
    bool m1 = FALSE;
    int partno = 0;

    DBUG_ENTER ("FindMatchingPart");
    DBUG_ASSERT (N_with == NODE_TYPE (foldee),
                 ("FindMatchingPart expected N_with foldee"));
    DBUG_ASSERT (N_part == NODE_TYPE (folderpart),
                 ("FindMatchingPart expected N_part folder"));

    folderpg = PART_GENERATOR (folderpart);
    wp = WITH_PART (foldee);

    while ((!matched) && wp != NULL) {
        wg = PART_GENERATOR (wp);
        intersectb1 = ExtractNthWLIntersection (partno, 0, idx);
        intersectb2 = ExtractNthWLIntersection (partno, 1, idx);
        if (
          /* Find and match Referents for generators, skipping default partitions */
          ((N_generator == NODE_TYPE (folderpg)) && (N_generator == NODE_TYPE (wg)))
          && (matchGeneratorField (intersectb1, GENERATOR_BOUND1 (folderpg)))
          && (matchGeneratorField (intersectb2, GENERATOR_BOUND2 (folderpg)))
          && matchGeneratorField (GENERATOR_STEP (folderpg), GENERATOR_STEP (wg))
          && matchGeneratorField (GENERATOR_WIDTH (folderpg), GENERATOR_WIDTH (wg))) {
            m1 = TRUE;
            DBUG_PRINT ("SWLF", ("FindMatchingPart referents all match"));
        } else { /* Ye olde school way */
            m1 = (CMPT_EQ == CMPTdoCompareTree (folderpg, wg));
            DBUG_PRINT ("SWLF", ("FindMatchingPart referents all match: olde school"));
        }

        if (m1 && (WITHOP_NEXT (WITH_WITHOP (foldee)) == NULL)
            && ((NODE_TYPE (WITH_WITHOP (foldee)) == N_genarray)
                || (NODE_TYPE (WITH_WITHOP (foldee)) == N_modarray))) {
            matched = TRUE;
        } else {
            wp = PART_NEXT (wp);
            partno++;
        }
    }

    if (matched) {
        DBUG_PRINT ("SWLF", ("checkWithLoop matches"));
    } else {
        wp = NULL;
        DBUG_PRINT ("SWLF", ("checkWithLoop does not match"));
    }

    DBUG_RETURN (wp);
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
 *        and that the  only references to the foldeeWL result are
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
                foldeepart = FindMatchingPart (LET_EXPR (foldeeassign),
                                               PRF_ARG1 (arg_node), folderpart);
            }
        }
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
 * @fn node *populateLut( node *part1, node *part2, info *arg_info)
 *
 * @brief Populate a look up table for mapping WITH_VEC and
 *        WITH_ID references into temp names.
 *
 *****************************************************************************/
static void
populateLut (node *part1, node *part2, info *arg_info)
{
    node *vec1, *ids1, *vec2, *ids2;

    DBUG_ENTER ("populateLut");

    vec1 = WITHID_VEC (PART_WITHID (part1));
    ids1 = WITHID_IDS (PART_WITHID (part1));

    vec2 = WITHID_VEC (PART_WITHID (part2));
    ids2 = WITHID_IDS (PART_WITHID (part2));

    DBUG_PRINT ("SWLF", ("Inserting WITHID_VEC into lut: foldee: %s, folder %s",
                         AVIS_NAME (IDS_AVIS (vec1)), AVIS_NAME (IDS_AVIS (vec2))));
    LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (vec1), IDS_AVIS (vec2));

    while (ids1 != NULL) {
        DBUG_PRINT ("SWLF", ("Inserting WITHID_IDS into lut: foldee: %s, folder: %s",
                             AVIS_NAME (IDS_AVIS (ids1)), AVIS_NAME (IDS_AVIS (ids2))));
        LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (ids1), IDS_AVIS (ids2));

        ids1 = IDS_NEXT (ids1);
        ids2 = IDS_NEXT (ids2);
    }

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn node *doSWLFreplace( ... )
 *
 * @brief
 *   In
 *    BBB = with...  elb = _sel_VxA_(iv, AAA) ...    NB. foldee
 *    CCC = with...  elc = _sel_VxA_(jv, BBB) ...    NB. folder
 *
 *   Replace, in the folder WL:
 *     elc = _sel_VxA_( jv, BBB)
 *   by
 *     tmp = _sel_VxA_( iv, AAA)
 *     elc = tmp;
 *
 * @params
 *    assign:
 *    fundef: N_fundef node, so we can insert new avis node for
 *            temp assign being made here.
 *    foldee: N_part node of foldee.
 *    folder: N_part node of folder.
 *****************************************************************************/
static node *
doSWLFreplace (node *assign, node *fundef, node *foldee, node *folder, info *arg_info)
{
    node *oldblock, *newblock;
    node *expravis, *newavis;

    DBUG_ENTER ("doSWLFreplace");

    /* Populate lut, to map names in foldeeWL to corresponding names
     * in folderWL.
     */
    populateLut (foldee, folder, arg_info);

    oldblock = CODE_CBLOCK (PART_CODE (foldee));

    /* If foldeeWL is empty, don't do any code substitutions.
     * Just replace sel(iv, foldeeWL) by iv.
     */
    if (N_empty == NODE_TYPE (BLOCK_INSTR (oldblock))) {
        newblock = NULL;
    } else {
        newblock
          = DUPdoDupTreeLutSsa (BLOCK_INSTR (oldblock), INFO_LUT (arg_info), fundef);
    }
    expravis = ID_AVIS (EXPRS_EXPR (CODE_CEXPRS (PART_CODE (foldee))));
    newavis = LUTsearchInLutPp (INFO_LUT (arg_info), expravis);

    LUTremoveContentLut (INFO_LUT (arg_info));

    /**
     * replace the code
     */
    FREEdoFreeNode (LET_EXPR (ASSIGN_INSTR (assign)));
    LET_EXPR (ASSIGN_INSTR (assign)) = TBmakeId (newavis);
    if (NULL != newblock) {
        assign = TCappendAssign (newblock, assign);
    }

    DBUG_RETURN (assign);
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
#ifdef DAOEN
        arg_node = WLCCdoWLCostCheck (arg_node);
#endif // DAOEN

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

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
 *   Examine all X[iv] primitives to see if iv is current folder-WL iv.
 *   If the iv does not have an SSAASSIGN, it means iv is a WITHID,
 *   and so SWLFI has not yet inserted an F_dataflowguard for it.
 *
 *****************************************************************************/
node *
SWLFprf (node *arg_node, info *arg_info)
{
    node *arg1;

    DBUG_ENTER ("SWLFprf");

    arg1 = PRF_ARG1 (arg_node);
    if ((INFO_PART (arg_info) != NULL) && (PRF_PRF (arg_node) == F_sel_VxA)
        && (isPrfArg1DataFlowGuard (arg_node)) && (N_id == NODE_TYPE (arg1))
        && (NULL != AVIS_SSAASSIGN (ID_AVIS (arg1)))
        && (NULL != AVIS_MINVAL (ID_AVIS (arg1)))
        && (NULL != AVIS_MAXVAL (ID_AVIS (arg1)))
        && (N_id == NODE_TYPE (PRF_ARG2 (arg_node)))) {

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
