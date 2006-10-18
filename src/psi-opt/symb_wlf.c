/*
 * $Id: symb_wlf.c dpa $
 */

/** <!--********************************************************************-->
 *
 * @defgroup swlf Symbolic With-Loop Folding
 *
 * @brief Symbolic With-Loop Folding
 *
 * TODO: write Module description here.
 *       describe preconditions
 *       what do "symbolic" means?
 *
 * An example for Symbolic With-Loop Folding is given below:
 *
 * <pre>
 *  1  a = with( iv)
 *  2          ( lb <= iv < ub) {
 *  3        <block_a>
 *  4      } : val;
 *  5      genarray( shp);
 *  6
 *  7  ...
 *  8
 *  9  b = with( jv)
 * 10          ( lb <= jv < ub) {
 * 11        <block_b1>
 * 12        ael = sel( jv, a);
 * 13        <block_b2>
 * 14      } : -
 * 15      genarray( shp);
 * </pre>
 *
 *   is transformed into
 *
 * <pre>
 *  1  a = with( iv)
 *  2          ( lb <= iv < ub) {
 *  3        <block_a>
 *  4      } : val;
 *  5      genarray( shp);
 *  6
 *  7  ...
 *  8
 *  9  b = with( jv)
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
#include "internal_lib.h"
#include "inferneedcounters.h"
#include "compare_tree.h"
#include "DupTree.h"
#include "free.h"
#include "LookUpTable.h"

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

    result = ILIBmalloc (sizeof (info));

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

    info = ILIBfree (info);

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
 * @brief global entry  point of symbolic With-Loop folding
 *
 * @param fundef Fundef-Node to apply SWLF.
 *
 * @return optimized fundef
 *
 *****************************************************************************/
node *
SWLFdoSymbolicWithLoopFolding (node *fundef)
{
    DBUG_ENTER ("SWLFdoSymbolicWithLoopFolding");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "SWLFdoSymbolicWithLoopFolding called for non-fundef node");

    TRAVpush (TR_swlf);
    fundef = TRAVdo (fundef, NULL);
    TRAVpop ();

    DBUG_RETURN (fundef);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper funcions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn bool checkWithLoop(...
 *
 * @brief check if it is fold-able.
 *
 *****************************************************************************/
static bool
checkWithLoop (node *with, node *index, node *part)
{
    DBUG_ENTER ("checkWithLoop");

    bool matched = FALSE;

    if (((NODE_TYPE (WITH_WITHOP (with)) == N_genarray)
         || (NODE_TYPE (WITH_WITHOP (with)) == N_modarray))
        && (WITHOP_NEXT (WITH_WITHOP (with)) == NULL)
        && (CMPT_EQ
            == CMPTdoCompareTree (PART_GENERATOR (part),
                                  PART_GENERATOR (WITH_PART (with))))
        && (ID_AVIS (index) == IDS_AVIS (WITHID_VEC (PART_WITHID (part))))) {

        matched = TRUE;
    }

    DBUG_RETURN (matched);
}

/** <!--********************************************************************-->
 *
 * @fn bool checkSWLFoldable( node *sel, info *arg_info, ...)
 *
 * @brief check if it is fold-able.
 *
 * @param sel
 * @param part
 * @param level
 *
 *****************************************************************************/
static bool
checkSWLFoldable (node *sel, node *part, int level)
{
    node *avis;
    node *assign;
    node *arg1;
    node *arg2;
    bool swlfable = FALSE;

    DBUG_ENTER ("checkSWLFoldable");

    arg1 = PRF_ARG1 (sel);
    arg2 = PRF_ARG2 (sel);
    avis = ID_AVIS (arg2);

    if ((AVIS_SSAASSIGN (avis) != NULL) && (AVIS_NEEDCOUNT (avis) == 1)
        && (AVIS_DEFDEPTH (avis) + 1 == level)) {

        assign = ASSIGN_INSTR (AVIS_SSAASSIGN (avis));

        if ((NODE_TYPE (assign) == N_let) && (NODE_TYPE (LET_EXPR (assign)) == N_with)
            && (checkWithLoop (LET_EXPR (assign), arg1, part))) {
            swlfable = TRUE;
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
    DBUG_ENTER ("SWLFfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {

        DBUG_PRINT ("SWLF", ("Symbolic With-Loops folding in function %s begins",
                             FUNDEF_NAME (arg_node)));

        /**
         * Get infer need counters
         */
        arg_node = INFNCdoInferNeedCountersOneFundef (arg_node);

        arg_info = MakeInfo (arg_node);

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        arg_info = FreeInfo (arg_info);

        DBUG_PRINT ("SWLF", ("Symbolic With-Loops folding in function %s completes",
                             FUNDEF_NAME (arg_node)));
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
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

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

        if (WITH_CODE (arg_node) != NULL) {
            WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
        }

        if (WITH_PART (arg_node) != NULL) {
            WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
        }
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

    if (CODE_CBLOCK (arg_node) != NULL) {
        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
    }

    if ((INFO_PART (arg_info) == NULL) && (CODE_NEXT (arg_node) != NULL)) {

        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
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

    if (PART_NEXT (arg_node) != NULL) {
        PART_NEXT (arg_node) = TRAVdo (PART_NEXT (arg_node), arg_info);
    }

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

    if (IDS_NEXT (arg_node)) {
        IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
    }

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

    if ((INFO_PART (arg_info) != NULL) && (PRF_PRF (arg_node) == F_sel)
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
