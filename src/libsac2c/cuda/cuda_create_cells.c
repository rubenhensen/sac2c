/** <!--********************************************************************-->
 *
 * @file cuda_create_cells.c
 *
 * prefix: CUCC
 *
 * description:
 *      searches for N_assign nodes that are tagged as CUDA_DEVICE_SINGLE.
 *      If a sequence of these is found, wrap them up in a N_cudast node
 *      and make that node the RHS of an N_assign.
 *
 *****************************************************************************/

#include "cuda_create_cells.h"
#include "tree_basic.h"
#include "traverse.h"
#include "str.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "memory.h"
#include "tree_compound.h"

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
    DBUG_ENTER ();

    TRAVpush (TR_cucc);

    syntax_tree = TRAVdo (syntax_tree, NULL);

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
    DBUG_ENTER ();

    /* We do not traverse cuda lac fun as they itself will
     * be executed on cuda, and therefore no cuda cells can
     * possibly be created within them */
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

    DBUG_ENTER ();

    /* First N_assign with CUDA_DEVICE_SINGLE execution mode */
    if (ASSIGN_CUDAEXECMODE (arg_node) == CUDA_DEVICE_SINGLE) {
        cell = TBmakeAssign (TBmakeCudast (TBmakeBlock (arg_node, NULL)), NULL);
        ASSIGN_CUDAEXECMODE (cell) = CUDA_DEVICE_SINGLE;

        /* Keep searching for consecutive CUDA_DEVICE_SINGLE N_assigns */
        last_cellassign = arg_node;
        while (ASSIGN_NEXT (last_cellassign) != NULL
               && ASSIGN_CUDAEXECMODE (ASSIGN_NEXT (last_cellassign))
                    == CUDA_DEVICE_SINGLE) {
            last_cellassign = ASSIGN_NEXT (last_cellassign);
        }
        ASSIGN_NEXT (cell) = ASSIGN_NEXT (last_cellassign);
        ASSIGN_NEXT (last_cellassign) = NULL;
        arg_node = cell;
    }

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
