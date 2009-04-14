/*
 * $Id: symb_wlf.c 16030 2009-04-09 19:40:34Z rbe $
 */

/** <!--********************************************************************-->
 *
 * @defgroup swlf Symbolic With-Loop Folding Inference
 *
 * @terminology:
 *        Foldee-WL, or foldee: The WL that will no longer
 *        exist after this phase completes. In the example
 *        below, A is the foldee-WL.
 *
 *        Folder-WL: the WL that will absorb the block(s) from
 *        the foldee-WL. In the example below, B is the folder-WL.
 *
 *        Intersection expression: the expression that
 *        specifies the set of index vectors in one WL partition
 *        that are present in a partition of another WL.
 *
 * @brief Extended With-Loop Folding
 *        SWLF performs WLF on some arrays that are not foldable
 *        by WLF. Features of extended WLF are described in symb_wlf.c
 *
 *        This phase inserts _dataflowguard "intersection expressions"
 *        before certain _sel_(iv, X) statements. Specifically, iv must
 *        be a function of a WITH_ID (the folder-WL),
 *        and X must have been created by a foldee-WL.
 *
 *        An intersection expression provides the information required
 *        to cut a folder-WL partition so that its index set
 *        is entirely contained with the index set of
 *        one partition of the foldee-WL.
 *
 *        Once the cutting has been performed, folding of the
 *        foldee-WL into the folder-WL can be performed directly,
 *        by replacing the above _sel_(iv, X) expression with
 *        the code from the foldee-WL partition, and performing
 *        appropriate renames of WITH_IDs.
 *
 * @ingroup opt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file symb_wlfi.c
 *
 * Prefix: SWLFI
 *
 *****************************************************************************/
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

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    node *vardecs;
    node *preassigns;
    node *part;             /* The current partition in the folder-WL */
    node *wl;               /* The current folder-WL we are looking at */
    int level;              /* The current nesting level of WLs. This
                             * is used to ensure that an index expression
                             * refers to an earlier WL in the same code
                             * block, rather than to a WL within this
                             * WL. I think...
                             */
    bool swlfoldablefoldee; /* foldee-WL may be legally foldable. */
                            /* (If index sets prove to be OK)     */
};

/**
 * Macro definitions for INFO structure
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_PREASSIGNS(n) ((n)->preassigns)
#define INFO_PART(n) ((n)->part)
#define INFO_WL(n) ((n)->wl)
#define INFO_LEVEL(n) ((n)->level)
#define INFO_SWLFOLDABLEFOLDEE(n) ((n)->swlfoldablefoldee)

static info *
MakeInfo (node *fundef)
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = fundef;
    INFO_VARDECS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_PART (result) = NULL;
    INFO_WL (result) = NULL;
    INFO_LEVEL (result) = 0;
    INFO_SWLFOLDABLEFOLDEE (result) = FALSE;

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
 *   node *SWLFIdoSymbolicWithLoopFoldingModule(node *module)
 *
 * description:
 *   Applies symbolic WL folding to a module.
 *
 *****************************************************************************/
node *
SWLFIdoSymbolicWithLoopFoldingModule (node *arg_node)
{

    DBUG_ENTER ("SWLFIdoSymbolicWithLoopFoldingModule");

    TRAVpush (TR_swlfi);
    arg_node = TRAVdo (arg_node, NULL);
    TRAVpop ();

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SWLFIdoSymbolicWithLoopFolding( node *fundef)
 *
 * @brief global entry point of symbolic With-Loop folding
 *
 * @param fundef Fundef-Node to apply SWLF.
 *
 * @return optimized fundef
 *
 *****************************************************************************/
node *
SWLFIdoSymbolicWithLoopFolding (node *arg_node)
{
    DBUG_ENTER ("SWLFIdoSymbolicWithLoopFolding");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef),
                 "SWLFIdoSymbolicWithLoopFolding called for non-fundef node");

    TRAVpush (TR_swlfi);
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

/** <!--********************************************************************-->
 *
 * @fn node *foo( node *arg_node, info *arg_info)
 *
 * @brief Construct an expression for simplification by the compiler.
 *        A typical expression used by SWLF to determine if
 *        a folder-WL partition's bounds are a subset of
 *        the putative foldee-WL's bounds is:
 *
 *          Intersection = _max_VxV_( GENERATOR_BOUND1(folder),
 *          xxx
 *
 * @params arg_node:
 * @return
 *
 *****************************************************************************/

static node *
foo (node *arg_node, info *arg_info)
{
}

/** <!--********************************************************************-->
 *
 * @fn node *createNewIV( node *arg_node, info *arg_info)
 *
 * @brief Create new iv' from iv.
 *        Also, create a guard expression:
 *         iv' = _dataflowguard_( iv, TRUE);
 *        Our caller will replace the selection:
 *         z = _sel_(iv, foldeeWL);
 *        by
 *         z = _sel_(iv', foldeeWL);
 *
 *        This function also appends a vardec to the INFO chain,
 *        and an assign to the preassigns chain.
 *
 * @params arg_node: the N_prf node of the sel() operation.
 * @return A pointer to the newly created N_id node for iv'.
 *
 *****************************************************************************/

static node *
createNewIV (node *arg_node, info *arg_info)
{
    node *ivavis;
    node *ivassign;
    int ivshape;
    node *ivid;
    node *ividprime;

    DBUG_ENTER ("createNewIV");
    DBUG_PRINT ("SWLFI", ("Inserting dataflow guard"));
    ivid = PRF_ARG1 (arg_node);
    ivshape = SHgetUnrLen (TYgetShape (AVIS_TYPE (ID_AVIS (ivid))));
    ivavis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (ID_AVIS (ivid))),
                         TYcopyType (AVIS_TYPE (ID_AVIS (ivid))));

    INFO_VARDECS (arg_info) = TBmakeVardec (ivavis, INFO_VARDECS (arg_info));
    ivassign = TBmakeAssign (TBmakeLet (TBmakeIds (ivavis, NULL),
                                        TCmakePrf2 (F_dataflowguard, DUPdoDupTree (ivid),
                                                    DUPdoDupTree (ivid))),
                             NULL);
    INFO_PREASSIGNS (arg_info) = TCappendAssign (INFO_PREASSIGNS (arg_info), ivassign);
    ividprime = TBmakeId (ivavis);
    AVIS_SSAASSIGN (ivavis) = ivassign;
    PRF_SELISSUEDDATAFLOWGUARD (arg_node) = TRUE;

    DBUG_RETURN (ividprime);
}

/** <!--********************************************************************-->
 *
 * @fn bool checkSWLFoldable( node *arg_node, info *arg_info)
 *
 * @brief We are looking at _sel_VxA_(idx, foldee), contained
 *        within a folder-WL. We want to determine if
 *        foldee is a WL that is a possible candidate for having some
 *        partition of itself folded into the folder-WL that
 *        contains the _sel_ expression.
 *
 *        This function concerns itself only with the characteristics
 *        of the entire foldee-WL: partition-dependent characteristics
 *        are determined by the SWLF phase itself.
 *
 *        The requirements for folding are:
 *
 *           - foldee-WL is the result of a WL.
 *
 *           - foldee-WL operator is a genarray or modarray.
 *
 *           - foldee WL is a single-operator WL.
 *
 *           - foldee-WL has an SSAASSIGN (means that WITH_IDs are NOT legal),
 *
 *           - foldee-WL has a NEEDCOUNT of 1 (there are no other uses
 *             of foldee), or of 2, iff the folder-WL is a modarray(foldeeWL).
 *
 *           - foldee-WL has a DEFDEPTH value (which I don't understand yet).
 *
 *        There is an added requirement, that the index set of
 *        the folder-WL partition match, or be a subset, of the
 *        foldee-WL partition index set.
 *        The expressions hanging from the dataflowguard inserted
 *        by createNewIV are intended to determine if this
 *        requirement is met. CF, CVP, and other optimizations
 *        should simplify those expressions and give SWLF
 *        the information it needs to determine if the index
 *        set requirements are met.
 *
 *
 * @param _sel_VxA_( idx, foldee-WL)
 * @result If some partition of the foldee-WL may be a legal
 *         candidate for folding into the folder-WL, return true.
 *         Else false.
 *
 *****************************************************************************/
static bool
checkSWLFoldable (node *arg_node, info *arg_info)
{
    node *foldeeavis;
    node *foldeeassign;
    node *foldeewl;
    int nc;
    bool z = FALSE;

    DBUG_ENTER ("checkSWLFoldable");

    foldeewl = PRF_ARG2 (arg_node);
    foldeeavis = ID_AVIS (foldeewl);

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
        && (AVIS_DEFDEPTH (foldeeavis) + 1 == INFO_LEVEL (arg_info))) {

        foldeeassign = ASSIGN_INSTR (AVIS_SSAASSIGN (foldeeavis));
        if ((NODE_TYPE (foldeeassign) == N_let)
            && (NODE_TYPE (LET_EXPR (foldeeassign)) == N_with)
            && (WITHOP_NEXT (WITH_WITHOP (LET_EXPR (foldeeassign))) == NULL)
            && ((NODE_TYPE (WITH_WITHOP (LET_EXPR (foldeeassign))) == N_genarray)
                || (NODE_TYPE (WITH_WITHOP (LET_EXPR (foldeeassign))) == N_modarray))) {
            z = TRUE;
        }
    }

    if (z) {
        DBUG_PRINT ("SWLFI", ("WL may be foldable"));
    } else {
        DBUG_PRINT ("SWLFI", ("WL is not foldable"));
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *SWLFIfundef(node *arg_node, info *arg_info)
 *
 * @brief applies SWLFI to a given fundef.
 *
 *****************************************************************************/
node *
SWLFIfundef (node *arg_node, info *arg_info)
{
    bool datarefonly;

    DBUG_ENTER ("SWLFIfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {

        DBUG_PRINT ("SWLFI", ("Symbolic-With-Loop-Folding Inference in %s %s begins",
                              (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                              FUNDEF_NAME (arg_node)));

        datarefonly = global.ssaiv ? TRUE : FALSE;
        arg_node = INFNCdoInferNeedCountersOneFundef (arg_node, datarefonly);

        arg_info = MakeInfo (arg_node);

        if (FUNDEF_BODY (arg_node) != NULL) {
            INFO_VARDECS (arg_info) = BLOCK_VARDEC (FUNDEF_BODY (arg_node));

            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

            BLOCK_VARDEC (FUNDEF_BODY (arg_node)) = INFO_VARDECS (arg_info);
            INFO_VARDECS (arg_info) = NULL;
        }

        arg_info = FreeInfo (arg_info);

        DBUG_PRINT ("SWLFI", ("Symbolic-With-Loop-Folding Inference in %s %s ends",
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
 * @fn node SWLFIassign( node *arg_node, info *arg_info)
 *
 * @brief performs a top-down traversal.
 *        For a foldable WL, arg_node is x = _sel_VxA_(iv, foldee).
 *
 *****************************************************************************/
node *
SWLFIassign (node *arg_node, info *arg_info)
{
    bool foldablefoldee; /* this may be dead wood FIXME*/

    DBUG_ENTER ("SWLFIassign");

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    foldablefoldee = INFO_SWLFOLDABLEFOLDEE (arg_info);
    INFO_SWLFOLDABLEFOLDEE (arg_info) = FALSE;

    if (INFO_PREASSIGNS (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
        INFO_PREASSIGNS (arg_info) = NULL;
    }

    /*
     * Top-down traversal
     */
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SWLFIwith( node *arg_node, info *arg_info)
 *
 * @brief applies SWLFI to a with-loop in a top-down manner.
 *
 *****************************************************************************/
node *
SWLFIwith (node *arg_node, info *arg_info)
{
    info *old_arg_info;

    DBUG_ENTER ("SWLFIwith");

    old_arg_info = arg_info;
    arg_info = MakeInfo (INFO_FUNDEF (arg_info));
    INFO_LEVEL (arg_info) = INFO_LEVEL (old_arg_info) + 1;
    INFO_VARDECS (arg_info) = INFO_VARDECS (old_arg_info);
    INFO_WL (arg_info) = arg_node;

    if (INFO_PART (arg_info) == NULL) {
        WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), arg_info);
        WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);
    }

    INFO_VARDECS (old_arg_info) = INFO_VARDECS (arg_info);

    /* Decrement the level counter */
    arg_info = FreeInfo (arg_info);
    arg_info = old_arg_info;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SWLFIcode( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
SWLFIcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SWLFIcode");

    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);

    if (INFO_PART (arg_info) == NULL) {
        CODE_NEXT (arg_node) = TRAVopt (CODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SWLFIpart( node *arg_node, info *arg_info)
 *
 * @brief Traverse each partition of a WL.
 *
 *****************************************************************************/
node *
SWLFIpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SWLFIpart");

    INFO_PART (arg_info) = arg_node;
    PART_CODE (arg_node) = TRAVdo (PART_CODE (arg_node), arg_info);
    INFO_PART (arg_info) = NULL;

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SWLFIids( node *arg_node, info *arg_info)
 *
 * @brief set current With-Loop level as ids defDepth attribute
 *
 *****************************************************************************/
node *
SWLFIids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SWLFIids");

    AVIS_DEFDEPTH (IDS_AVIS (arg_node)) = INFO_LEVEL (arg_info);
    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SWLFIprf( node *arg_node, info *arg_info)
 *
 * @brief
 *   Examine all X[iv] primitives to see if iv is current folder-WL iv.
 *
 *****************************************************************************/
node *
SWLFIprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SWLFIprf");

    if ((INFO_PART (arg_info) != NULL) && (PRF_PRF (arg_node) == F_sel_VxA)
        && (NODE_TYPE (PRF_ARG1 (arg_node)) == N_id)
        && (NODE_TYPE (PRF_ARG2 (arg_node)) == N_id)
        && (!PRF_SELISSUEDDATAFLOWGUARD (arg_node))) {

#ifdef OLDSCHOOL
        /* Next line restricts iv to WL ids - this has to go! */
        if (ID_AVIS (PRF_ARG1 (arg_node))
            == IDS_AVIS (WITHID_VEC (PART_WITHID (INFO_PART (arg_info))))) {
            INFO_SWLFOLDABLEFOLDEE (arg_info) = checkSWLFoldable (arg_node, arg_info);
        }
#else  // OLDSCHOOL
        INFO_SWLFOLDABLEFOLDEE (arg_info) = checkSWLFoldable (arg_node, arg_info);
#endif // OLDSCHOOL
        /* Replace iv by iv' */
        if (INFO_SWLFOLDABLEFOLDEE (arg_info)) {
            PRF_ARG1 (arg_node) = createNewIV (arg_node, arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Symbolic with loop folding -->
 *****************************************************************************/
