/** <!--********************************************************************-->
 *
 * @file cuda_create_cells.c
 *
 * prefix: CUCC
 *
 * description:
 *
 *****************************************************************************/

#include "cuda_create_cells.h"
#include "tree_basic.h"
#include "traverse.h"
#include "str.h"
#include "dbug.h"
#include "memory.h"
#include "tree_compound.h"

/*
 * INFO structure
 */
struct INFO {
};

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

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
 *
 * @fn node *CUCCdoCreateCells( node *syntax_tree)
 *
 *   @brief  Inits the traversal for this phase
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CUCCdoCreateCells (node *syntax_tree)
{
    info *arg_info;

    DBUG_ENTER ("CUCCdoCreateCells");

    arg_info = MakeInfo ();

    TRAVpush (TR_cucc);

    syntax_tree = TRAVdo (syntax_tree, arg_info);

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUCCfundef( node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CUCCfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUCCfundef");

    if (!FUNDEF_ISCUDALACFUN (arg_node)) {
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUCCassign( node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CUCCassign (node *arg_node, info *arg_info)
{
    node *cell, *last_cellassign;

    DBUG_ENTER ("CUCCassign");

    /* First assign with CUDA_DEVICE_SINGLE execution mode */
    if (ASSIGN_EXECMODE (arg_node) == CUDA_DEVICE_SINGLE) {
        cell = TBmakeAssign (TBmakeCudast (TBmakeBlock (arg_node, NULL)), NULL);
        ASSIGN_EXECMODE (cell) = CUDA_DEVICE_SINGLE;

        /* Keep searching for consecutive CUDA_DEVICE_SINGLE N_assigns */
        last_cellassign = arg_node;
        while (ASSIGN_NEXT (last_cellassign) != NULL
               && ASSIGN_EXECMODE (ASSIGN_NEXT (last_cellassign)) == CUDA_DEVICE_SINGLE) {
            last_cellassign = ASSIGN_NEXT (last_cellassign);
        }
        ASSIGN_NEXT (cell) = ASSIGN_NEXT (last_cellassign);
        ASSIGN_NEXT (last_cellassign) = NULL;
        arg_node = cell;
    }

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}
