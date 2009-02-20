/*
 * $Id$
 */

// #define DAOEN

/** <!--********************************************************************-->
 *
 * @defgroup swlf Symbolic With-Loop Folding
 *
 * @terminology:
 *        Foldee WL, or foldee: The WL that will no longer
 *        exist after this phase completes. In the example
 *        below, A is the foldee.
 *
 *        Folder WL: the WL that will absorb the block(s) from
 *        the foldee WL. In the example below, B is the folder WL.
 *
 * @brief Symbolic With-Loop Folding
 *        This performs WLF on arrays that are not foldable
 *        by WLF, for some reason(s), such as some AKD and AUD arrays.
 *
 *        At present (2009-02-17), SWLF has several
 *        restrictions over WLF:
 *
 *           - the foldee WL must have an SSAASSIGN.
 *             NB. This means that references to WITH_IDS are
 *                 not foldable!
 *
 *           - the foldee WL must have a NEEDCOUNT of 1. I.e.,
 *             there are no other references to the foldee.
 *             If CVP and friends have done their job, this should
 *             not be a severe restriction.
 *
 *           - the foldee WL must have a DEPDEPTH value of 1??
 *             Not sure what this means yet...
 *
 *           - The folder WL must refer to the foldee WL via
 *              _sel_VxA_(idx, foldee)
 *             and idx must the folder WITHID.
 *
 *           - The WL operator is a genarray or modarray (NO folds, please).
 *
 *           - The WL is a single-operator WL. (These should have been
 *             eliminated by phase 10?)
 *
 *           - The generator of the folder WL matches
 *             the generator of the folder WL.
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
 * 12        <block_a>
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
    int level;
    bool swlfoldable;
};

/**
 * Macro definitions for INFO structure
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_PART(n) ((n)->part)
#define INFO_LEVEL(n) ((n)->level)
#define INFO_SWLFOLDABLE(n) ((n)->swlfoldable)

static info *
MakeInfo (node *fundef)
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = fundef;
    INFO_PART (result) = NULL;
    INFO_LEVEL (result) = 0;
    INFO_SWLFOLDABLE (result) = FALSE;

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
                DBUG_PRINT ("SWLF",
                            ("checkElement can fold %s", AVIS_NAME (ID_AVIS (arg2))));
            }
        }
    }

    arg1fs = (NULL != arg1fs) ? COfreeConstant (arg1fs) : NULL;
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
 *             with thes properties:
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
    node *fbv = NULL;
    constant *fbvfs = NULL;
    bool mat = FALSE;
    ;

    DBUG_ENTER ("matchGeneratorField");

    if (fa == fb) { /* SAA should do it this way most of the time */
        mat = TRUE;
    } else {
        if (PM (PMarray (&fbvfs, &fbv, fa)) && PM (PMarray (&fbvfs, &fbv, fb))) {
            DBUG_PRINT ("SWLF", ("matchGeneratorField matched PMarray"));
            fbvfs = COfreeConstant (fbvfs);
            mat = TRUE;
        } else {
            DBUG_PRINT ("SWLF", ("matchGeneratorField could not match PMarray"));
#ifdef DICEYCODE
            mat = shapeMatchesArray (fa, fb, foldeewl);
#endif // DICEYCODE
        }
    }
    DBUG_RETURN (mat);
}

/** <!--********************************************************************-->
 *
 * @fn bool checkWithLoop(...
 *
 * @brief check if a WL is fold-able.
 *        The requirements for folding are:
 *           - The WL operator is a genarray or modarray (NO folds, please).
 *           - The WL is a single-operator WL.
 *           - The generator of the WL matches the generator
 *               of that of the WL it is to be folded into.
 *           - The sel(idx,y) idx is the WITHID of the
 * @params *with:  the putative foldee -- the WL that
 *                 would be subsumed into the one at "part".
 *         *index: The idx in sel(idx, y) of the WL that is selecting
 *                 from the result of WL.
 *         *part: The partition of the folder WL.
 *
 *
 *****************************************************************************/
static bool
checkWithLoop (node *with, node *index, node *part)
{
    node *pg;
    node *wg;
    bool matched = FALSE;
    bool m1 = FALSE;

    DBUG_ENTER ("checkWithLoop");

    pg = PART_GENERATOR (part);
    wg = PART_GENERATOR (WITH_PART (with));

    if ((global.ssaiv) &&
        /* Find and match Referents for generators */
        matchGeneratorField (GENERATOR_BOUND1 (pg), GENERATOR_BOUND1 (wg), with)
        && matchGeneratorField (GENERATOR_BOUND2 (pg), GENERATOR_BOUND2 (wg), with)
        && matchGeneratorField (GENERATOR_STEP (pg), GENERATOR_STEP (wg), with)
        && matchGeneratorField (GENERATOR_WIDTH (pg), GENERATOR_WIDTH (wg), with)) {
        m1 = TRUE;
        DBUG_PRINT ("SWLF", ("checkWithLoop referents all match"));
    } else { /* Ye olde school way */
        m1 = (CMPT_EQ == CMPTdoCompareTree (pg, wg));
        DBUG_PRINT ("SWLF", ("checkWithLoop referents all match: olde school"));
    }

    if (((NODE_TYPE (WITH_WITHOP (with)) == N_genarray)
         || (NODE_TYPE (WITH_WITHOP (with)) == N_modarray))
        && (WITHOP_NEXT (WITH_WITHOP (with)) == NULL) && m1
        && (ID_AVIS (index) == IDS_AVIS (WITHID_VEC (PART_WITHID (part))))) {

        matched = TRUE;
        DBUG_PRINT ("SWLF", ("checkWithLoop matches"));
    }

    DBUG_RETURN (matched);
}

/** <!--********************************************************************-->
 *
 * @fn bool checkSWLFoldable( node *sel, node * part, info *arg_info, ...)
 *
 * @brief check if _sel_VxA_(idx, y) is foldable into part.
 *        The requirements for folding are:
 *           - y has an SSAASSIGN (means that WITH_IDs are NOT legal),
 *           - y has a NEEDCOUNT of 1 (there are no other uses of y),
 *           - y has a DEFDEPTH value which I don't understand yet,
 *           - y is the result of a WL.
 *           - and checkWithLoop is happy with these arguments.
 *
 * @param _sel_VxA_( idx, y)
 * @param The partition into which we would like to fold this sel().
 * @param level
 *
 *****************************************************************************/
static bool
checkSWLFoldable (node *arg_node, node *part, int level)
{
    node *avis;
    node *assign;
    node *arg1;
    node *arg2;
    bool swlfable = FALSE;

    DBUG_ENTER ("checkSWLFoldable");

    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);
    avis = ID_AVIS (arg2);

    if ((AVIS_SSAASSIGN (avis) != NULL) && (AVIS_NEEDCOUNT (avis) == 1)
        && (AVIS_DEFDEPTH (avis) + 1 == level)) {

        assign = ASSIGN_INSTR (AVIS_SSAASSIGN (avis));

        if ((NODE_TYPE (assign) == N_let) && (NODE_TYPE (LET_EXPR (assign)) == N_with)
            && (checkWithLoop (LET_EXPR (assign), arg1, part))) {
            swlfable = TRUE;
            DBUG_PRINT ("SWLF", ("WLs are foldable"));
        }
    }

    DBUG_RETURN (swlfable);
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
    LUTinsertIntoLutP (lut, IDS_AVIS (vec1), IDS_AVIS (vec2));
    while (ids1 != NULL) {
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
 *
 *****************************************************************************/
static node *
doSWLFreplace (node *assign, node *fundef, node *with1, node *part2)
{
    node *oldblock, *newblock;
    node *expravis, *newavis;
    lut_t *lut;

    DBUG_ENTER ("doSWLFreplace");

    /* Create a new Lut */
    lut = createLut (WITH_PART (with1), part2);

    oldblock = CODE_CBLOCK (WITH_CODE (with1));
    expravis = ID_AVIS (EXPRS_EXPR (CODE_CEXPRS (WITH_CODE (with1))));
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
 *
 *****************************************************************************/
node *
SWLFassign (node *arg_node, info *arg_info)
{
    node *with1;
    bool foldable;

    DBUG_ENTER ("SWLFassign");

    foldable = FALSE;
    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    foldable = INFO_SWLFOLDABLE (arg_info);
    INFO_SWLFOLDABLE (arg_info) = FALSE;

    /*
     * Top-down traversal
     */
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    /*
     * Append the new cloned block
     */
    if (foldable) {
        with1 = getAvisLetExpr (LET_EXPR (ASSIGN_INSTR (arg_node)));
        arg_node
          = doSWLFreplace (arg_node, INFO_FUNDEF (arg_info), with1, INFO_PART (arg_info));

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
    DBUG_ENTER ("SWLFwith");

    /* Increment the level counter */
    INFO_LEVEL (arg_info) += 1;

    if (INFO_PART (arg_info) == NULL) {
        WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), arg_info);
        WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);
    }

    /* Decrement the level counter */
    INFO_LEVEL (arg_info) -= 1;

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
 * @brief
 *
 *****************************************************************************/
node *
SWLFpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SWLFpart");

    if (CODE_USED (PART_CODE (arg_node)) == 1) {

        INFO_PART (arg_info) = arg_node;
        PART_CODE (arg_node) = TRAVdo (PART_CODE (arg_node), arg_info);
        INFO_PART (arg_info) = NULL;
    }

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
 *
 *****************************************************************************/
node *
SWLFprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SWLFprf");

    if ((INFO_PART (arg_info) != NULL) && (PRF_PRF (arg_node) == F_sel_VxA)
        && (NODE_TYPE (PRF_ARG1 (arg_node)) == N_id)
        && (NODE_TYPE (PRF_ARG2 (arg_node)) == N_id)) {

        INFO_SWLFOLDABLE (arg_info)
          = checkSWLFoldable (arg_node, INFO_PART (arg_info), INFO_LEVEL (arg_info));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Symbolic with loop folding -->
 *****************************************************************************/
