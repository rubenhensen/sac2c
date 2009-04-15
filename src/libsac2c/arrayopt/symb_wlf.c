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
 *        The features of extended WLF are:
 *
 *            - Ability to fold arrays whose shapes are not
 *              known statically. Specifically, if the index
 *              set of the folder-WL is known to be identical to,
 *              or a subset of, the index set of the foldee-WL,
 *              folding will occur. FIXME: This will have to be
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
 *             the generator of the foldee-WL.
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
    bool modarray2genarray;
    /* modarray2genarray is TRUE iff folderWL's modarray(foldee)
     * can be converted to a enarray(shape(foldee)).
     */
};

/**
 * Macro definitions for INFO structure
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_PART(n) ((n)->part)
#define INFO_WL(n) ((n)->wl)
#define INFO_LEVEL(n) ((n)->level)
#define INFO_SWLFOLDABLEFOLDEEPART(n) ((n)->swlfoldablefoldeepart)
#define INFO_MODARRAY2GENARRAY(n) ((n)->modarray2genarray)

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
    INFO_MODARRAY2GENARRAY (result) = FALSE;

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

/******************************************************************************
 *
 * function:
 *   node *SWLFdoSymbolicWithLoopFoldingModule(node *module)
 *
 * description:
 *   Applies symbolic WL folding to a module.
 *
 *****************************************************************************/
node *
SWLFdoSymbolicWithLoopFoldingModule (node *arg_node)
{

    DBUG_ENTER ("SWLFdoSymbolicWithLoopFoldingModule");

    TRAVpush (TR_swlf);
    arg_node = TRAVdo (arg_node, NULL);
    TRAVpop ();

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SWLFdoSymbolicWithLoopFolding( node *fundef)
 *
 * @brief global entry point of symbolic With-Loop folding
 *
 * @param fundef Fundef-Node to apply SWLF.
 *
 * @return optimized fundef
 *
 *****************************************************************************/
node *
SWLFdoSymbolicWithLoopFolding (node *arg_node)
{
    DBUG_ENTER ("SWLFdoSymbolicWithLoopFolding");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef),
                 "SWLFdoSymbolicWithLoopFolding called for non-fundef node");

    TRAVpush (TR_swlf);
    arg_node = TRAVdo (arg_node, NULL);
    TRAVpop ();

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
 *          foldeewl is the N_id of the result of the foldee WL.
 *
 * @return Boolean TRUE if both fields are the same or can
 *          be shown to represent the same shape vector.
 *
 *****************************************************************************/
static bool
matchGeneratorField (node *fa, node *fb, node *foldeewl)
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
 * @fn node * FindMatchingPart(...
 *
 * @brief check if a WL has a legal foldee partition.
 *        The requirements for folding are:
 *           - The WL operator is a genarray or modarray,
 *             or it is a fold, and the generated code does
 *             not include -mt support. See comments at
 *             head of this file for a rationale.
 *
 *           - The WL is a single-operator WL.
 *
 *           - The current partition of the folder-WL matches some generator
 *               of that of the foldee-WL.
 *               FIXME: This is overly strict. This function should
 *                      be extended to allow the current folder partition's
 *                      generator range to either match or be a subset
 *                      of a generator of the foldee.
 *
 *           - The sel(idx,y) idx is the WITHID of the
 *             putative foldee.
 *
 * @params *foldee:  the putative foldee -- the WL that
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
    node *pg;
    node *wg;
    node *wp;
    bool matched = FALSE;
    bool m1 = FALSE;

    DBUG_ENTER ("FindMatchingPart");
    DBUG_ASSERT (N_with == NODE_TYPE (foldee),
                 ("FindMatchingPart expected N_with foldee"));
    DBUG_ASSERT (N_part == NODE_TYPE (folderpart),
                 ("FindMatchingPart expected N_part folder"));

    pg = PART_GENERATOR (folderpart);
    wp = WITH_PART (foldee);

    while ((!matched) && wp != NULL) {
        wg = PART_GENERATOR (wp);
        if (
          /* Find and match Referents for generators */
          matchGeneratorField (GENERATOR_BOUND1 (pg), GENERATOR_BOUND1 (wg), foldee)
          && matchGeneratorField (GENERATOR_BOUND2 (pg), GENERATOR_BOUND2 (wg), foldee)
          && matchGeneratorField (GENERATOR_STEP (pg), GENERATOR_STEP (wg), foldee)
          && matchGeneratorField (GENERATOR_WIDTH (pg), GENERATOR_WIDTH (wg), foldee)) {
            m1 = TRUE;
            DBUG_PRINT ("SWLF", ("FindMatchingPart referents all match"));
        } else { /* Ye olde school way */
            m1 = (CMPT_EQ == CMPTdoCompareTree (pg, wg));
            DBUG_PRINT ("SWLF", ("FindMatchingPart referents all match: olde school"));
        }

        if (m1 && (WITHOP_NEXT (WITH_WITHOP (foldee)) == NULL)
            && ((NODE_TYPE (WITH_WITHOP (foldee)) == N_genarray)
                || (NODE_TYPE (WITH_WITHOP (foldee)) == N_modarray))) {
            matched = TRUE;
        } else {
            wp = PART_NEXT (wp);
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
 *        The requirements for folding are:
 *           - foldee has an SSAASSIGN (means that WITH_IDs are NOT legal),
 *           - foldee has a NEEDCOUNT of 1 (there are no other uses of foldee),
 *             or of 2 iff the folder WL is a modarray(foldee).
 *           - foldee has a DEFDEPTH value which I don't understand yet,
 *           - foldee is the result of a WL.
 *           - and checkWithLoop is happy with these arguments.
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
    node *foldeeavis;
    node *foldeeassign;
    node *idx;
    node *foldeepart = NULL;
    int nc;

    DBUG_ENTER ("checkSWLFoldable");

    idx = PRF_ARG1 (arg_node);
    foldeeavis = ID_AVIS (PRF_ARG2 (arg_node));

    /* There must be only one data reference to the foldee WL.
     * However, if the folder WL is: modarray(foldee),
     * that is counted as a data reference, so we have to allow for
     * it here.
     */
    if ((NULL != INFO_WL (arg_info))
        && (N_modarray == NODE_TYPE (WITH_WITHOP (INFO_WL (arg_info))))
        && (foldeeavis == ID_AVIS (MODARRAY_ARRAY (WITH_WITHOP (INFO_WL (arg_info)))))) {
        nc = 2;
    } else {
        nc = 1;
    }

    if ((AVIS_SSAASSIGN (foldeeavis) != NULL) && (AVIS_NEEDCOUNT (foldeeavis) == nc)
        && (AVIS_DEFDEPTH (foldeeavis) + 1 == level)) {

        foldeeassign = ASSIGN_INSTR (AVIS_SSAASSIGN (foldeeavis));
        if ((NODE_TYPE (foldeeassign) == N_let)
            && (NODE_TYPE (LET_EXPR (foldeeassign)) == N_with)) {
            foldeepart = FindMatchingPart (LET_EXPR (foldeeassign), idx, folderpart);
        }
    }

    if (NULL != foldeepart) {
        INFO_MODARRAY2GENARRAY (arg_info) = TRUE;
        DBUG_PRINT ("SWLF", ("WLs are foldable"));
    } else {
        DBUG_PRINT ("SWLF", ("WLs are not foldable"));
    }

    DBUG_RETURN (foldeepart);
}

/** <!--********************************************************************-->
 *
 * @fn node *createLut( node *part1, node *part2)
 *
 * @brief Create a look up table and insert into vec and ids
 *
 *****************************************************************************/
static lut_t *
createLut (node *part1, node *part2)
{
    lut_t *lut;
    node *vec1, *ids1, *vec2, *ids2;

    DBUG_ENTER ("createLut");

    vec1 = WITHID_VEC (PART_WITHID (part1));
    ids1 = WITHID_IDS (PART_WITHID (part1));

    vec2 = WITHID_VEC (PART_WITHID (part2));
    ids2 = WITHID_IDS (PART_WITHID (part2));

    lut = LUTgenerateLut ();
    DBUG_PRINT ("SWLF", ("Inserting WITHID_VEC into lut: foldee: %s, folder %s",
                         AVIS_NAME (IDS_AVIS (vec1)), AVIS_NAME (IDS_AVIS (vec2))));
    LUTinsertIntoLutP (lut, IDS_AVIS (vec1), IDS_AVIS (vec2));
    while (ids1 != NULL) {
        DBUG_PRINT ("SWLF", ("Inserting WITHID_IDS into lut: foldee: %s, folder: %s",
                             AVIS_NAME (IDS_AVIS (ids1)), AVIS_NAME (IDS_AVIS (ids2))));
        LUTinsertIntoLutP (lut, IDS_AVIS (ids1), IDS_AVIS (ids2));

        ids1 = IDS_NEXT (ids1);
        ids2 = IDS_NEXT (ids2);
    }

    DBUG_RETURN (lut);
}

/** <!--********************************************************************-->
 *
 * @fn node *getAvisLetExpr( node *sel)
 *
 * @brief ...
 *
 *****************************************************************************/
static node *
getAvisLetExpr (node *sel)
{
    node *n;

    DBUG_ENTER ("getAvisLetExpr");

    n = ID_AVIS (PRF_ARG2 (sel));

    if (n != NULL) {
        n = AVIS_SSAASSIGN (n);

        if (n != NULL) {
            n = ASSIGN_INSTR (n);

            if (n != NULL) {
                n = LET_EXPR (n);
            }
        }
    }

    DBUG_RETURN (n);
}

/** <!--********************************************************************-->
 *
 * @fn node *doSWLFreplace( ... )
 *
 * @brief
 *   In
 *    BBB = with...  elb = _sel_VxA_(iv, AAA) ...    NB. foldee
 *    CCC = with...  elc = _sel_VxA_(jv, BBB) ...     NB. folder
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
doSWLFreplace (node *assign, node *fundef, node *foldee, node *folder)
{
    node *oldblock, *newblock;
    node *expravis, *newavis;
    lut_t *lut;

    DBUG_ENTER ("doSWLFreplace");

    /* Create a new Lut, to map names in foldee to corresponding names
     * in folder
     */
    lut = createLut (foldee, folder);

    oldblock = CODE_CBLOCK (PART_CODE (foldee));
    expravis = ID_AVIS (EXPRS_EXPR (CODE_CEXPRS (PART_CODE (foldee))));
    newblock = DUPdoDupTreeLutSsa (BLOCK_INSTR (oldblock), lut, fundef);
    newavis = LUTsearchInLutPp (lut, expravis);

    /**
     * replace the code
     */
    FREEdoFreeNode (LET_EXPR (ASSIGN_INSTR (assign)));
    LET_EXPR (ASSIGN_INSTR (assign)) = TBmakeId (newavis);
    assign = TCappendAssign (newblock, assign);

    DBUG_RETURN (assign);
}

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *    PRTdoPrintNode( sel);
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
    bool datarefonly;

    DBUG_ENTER ("SWLFfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {

        DBUG_PRINT ("SWLF", ("Symbolic With-Loops folding in %s %s begins",
                             (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                             FUNDEF_NAME (arg_node)));

#ifdef DAOEN
        arg_node = WLCCdoWLCostCheck (arg_node);
        arg_node = WLNCdoWLNeedCount (arg_node);
#endif // DAOEN

        datarefonly = global.ssaiv ? TRUE : FALSE;
        arg_node = INFNCdoInferNeedCountersOneFundef (arg_node, datarefonly);

        arg_info = MakeInfo (arg_node);

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        arg_info = FreeInfo (arg_info);

        DBUG_PRINT ("SWLF", ("Symbolic With-Loops folding in %s %s ends",
                             (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                             FUNDEF_NAME (arg_node)));

        FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    }

    if (NULL != FUNDEF_NEXT (arg_node)) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), NULL);
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
    node *foldee; /* FIXME probably obsolete ? */
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
        foldee = getAvisLetExpr (LET_EXPR (ASSIGN_INSTR (arg_node)));
        arg_node = doSWLFreplace (arg_node, INFO_FUNDEF (arg_info), foldablefoldeepart,
                                  INFO_PART (arg_info));

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

    DBUG_ENTER ("SWLFwith");

    INFO_WL (arg_info) = arg_node;
    INFO_MODARRAY2GENARRAY (arg_info) = FALSE;
    /* Increment the level counter */
    INFO_LEVEL (arg_info) += 1;

    if (INFO_PART (arg_info) == NULL) {
        WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), arg_info);
        WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);
    }

    /* Try to replace modarray(foldeeWL) by genarray(shape(foldeeWL)).
     * This has no effect on the foldeeWL itself, but is needed to
     * eliminate the reference to foldeeWL, so it can be removed by DCR.
     */
    folderop = WITH_WITHOP (arg_node);
    if ((N_modarray == NODE_TYPE (folderop))
        && (NULL != AVIS_SHAPE (ID_AVIS (MODARRAY_ARRAY (folderop))))
        && (TRUE == INFO_MODARRAY2GENARRAY (arg_info))) {
        foldeeshape = AVIS_SHAPE (ID_AVIS (MODARRAY_ARRAY (folderop)));
        genop = TBmakeGenarray (DUPdoDupTree (foldeeshape), NULL);
        GENARRAY_NEXT (genop) = MODARRAY_NEXT (folderop);
        nextop = FREEdoFreeNode (folderop);
        WITH_WITHOP (arg_node) = genop;
        DBUG_PRINT ("SWLF", ("Replacing modarray by genarray"));
    }

    /* Decrement the level counter */
    INFO_LEVEL (arg_info) -= 1;
    INFO_WL (arg_info) = NULL;
    INFO_MODARRAY2GENARRAY (arg_info) = FALSE;

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

    if (INFO_PART (arg_info) == NULL) {
        CODE_NEXT (arg_node) = TRAVopt (CODE_NEXT (arg_node), arg_info);
    }

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
 *
 *****************************************************************************/
node *
SWLFprf (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("SWLFprf");

    if ((INFO_PART (arg_info) != NULL) && (PRF_PRF (arg_node) == F_sel_VxA)
        && (NODE_TYPE (PRF_ARG1 (arg_node)) == N_id)
        && (NODE_TYPE (PRF_ARG1 (arg_node)) == N_id)) {

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
