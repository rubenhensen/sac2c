/*
 * $Id: symb_wlf.c pde $
 */

/** <!--********************************************************************-->
 *
 * @defgroup swlf symbolic With-Loop folding
 *
 * TODO: write Module description here.
 *       describe preconditions
 *
 * Example
 *
 * For an example, take a look at src/refcount/explicitcopy.c
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

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    node *code;
    int level;
};

/**
 * Macro definitions for INFO structure
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_LEVEL(n) ((n)->level)
#define INFO_CODE(n) ((n)->code)

static info *
MakeInfo (node *fundef)
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_FUNDEF (result) = fundef;
    INFO_CODE (result) = NULL;
    INFO_LEVEL (result) = 0;

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
 * @param fundef Fundef-Node to start SWLF.
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
 * @fn node *DummyStaticHelper(node *arg_node)
 *
 * @brief A dummy static helper functions used only in your traversal
 *
 *****************************************************************************/
static node *
DummyStaticHelper (node *arg_node)
{
    DBUG_ENTER ("DummyStaticHelper");

    DBUG_RETURN (DummyStaticHelper (arg_node));
}

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
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

    if (FUNDEF_BODY (arg_node)) {
        DBUG_PRINT ("SWLF", ("Symbolic With-Loops fusion in function %s",
                             FUNDEF_NAME (arg_node)));

        arg_node = INFNCdoInferNeedCountersOneFundef (arg_node);

        arg_info = MakeInfo (arg_node);

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        arg_info = FreeInfo (arg_info);

        DBUG_PRINT ("SWLF", ("Symbolic With-Loops fusion in function %s complete",
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
    DBUG_ENTER ("SWLFassign");

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    /*
     * Top-down traversal
     */
    if (ASSIGN_NEXT (arg_node)) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
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

    INFO_LEVEL (arg_info) += 1;
    INFO_CODE (arg_info) = WITH_CODE (arg_node);

    /* Traverses sons */
    arg_node = TRAVcont (arg_node, arg_info);

    INFO_LEVEL (arg_info) -= 1;

    /**
     * check the preconditions
     */

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SWLFids( node *arg_node, info *arg_info)
 *
 * @brief set current With-Loop depth as ids defDepth attribute
 *
 *****************************************************************************/
node *
SWLFids (node *arg_node, info *arg_info)
{
    node *avis;

    DBUG_ENTER ("SWLFids");

    avis = IDS_AVIS (arg_node);

    AVIS_DEFDEPTH (avis) = INFO_LEVEL (arg_info);

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

    if (PRF_PRF (arg_node) == F_sel) {
        DBUG_PRINT ("SWLF", ("_sel_(...)"));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Symbolic with loop folding -->
 *****************************************************************************/
