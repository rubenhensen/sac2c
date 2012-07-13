/*
 * $Id$
 *
 */

/**
 *
 * @defgroup concel Consolidate Cells
 * @ingroup muth
 *
 * @brief Each functions (except the MUTH_ANY ones) by now must have just cells
 *        If a function has only one cell, the cell will be deleted an its
 *        assignment-chain will be restored into the FUNDEF_BODY
 * @{
 **/

/** <!--********************************************************************-->
 *
 * @file consolidate_cells.c
 *
 * prefix: CONCEL
 *
 * description:
 *
 *****************************************************************************/

#include "tree_basic.h"
#include "traverse.h"
#include "consolidate_cells.h"
#include "str.h"

#define DBUG_PREFIX "CONCEL"
#include "debug.h"

#include "memory.h"

/*
 * INFO structure
 */
struct INFO {
    node *cellassign;
};

/*
 * INFO macros
 *    node    CONCEL_CELLASSIGN
 */
#define INFO_CONCEL_CELLASSIGN(n) (n->cellassign)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_CONCEL_CELLASSIGN (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn node *CONCELdoConsolidateCells(node *arg_node)
 *
 *   @brief  Inits the traversal for this phase
 *
 *   @param arg_node a N_module
 *   @return the N_modul with consolidated cells (the cell of each function
 *           with only one cell will be deleted and its assignment chain
 *           restored into the FUNDEF_BODY
 *
 *****************************************************************************/
node *
CONCELdoConsolidateCells (node *arg_node)
{
    info *arg_info;
    trav_t traversaltable;
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_module,
                 "CONCELdoConsolidateCells expects a N_module as arg_node");

    arg_info = MakeInfo ();

    TRAVpush (TR_concel);

    DBUG_PRINT ("trav into module-funs");
    arg_node = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    DBUG_PRINT ("trav from module-funs");

    traversaltable = TRAVpop ();
    DBUG_ASSERT (traversaltable == TR_concel, "Popped incorrect traversal table");

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CONCELfundef(node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param arg_node a N_fundef
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CONCELfundef (node *arg_node, info *arg_info)
{
    node *myblock;
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef, "N_fundef expected");

    myblock = FUNDEF_BODY (arg_node);
    if (myblock != NULL) {
        if (NODE_TYPE (BLOCK_ASSIGNS (myblock)) == N_assign) {
            if (ASSIGN_NEXT (BLOCK_ASSIGNS (myblock)) == NULL) {
                /* traverse into the function to get the correct N_assign in
                 *  INFO_CONCEL_XTASSIGN */
                DBUG_PRINT ("trav into fundef-body");
                myblock = TRAVdo (myblock, arg_info);
                DBUG_PRINT ("trav from fundef-body");

                /*BLOCK_ASSIGNS(myblock) = MEMfree(BLOCK_ASSIGNS(myblock));*/
                /*BLOCK_ASSIGNS(myblock) = INFO_CONCEL_CELLASSIGN(arg_info);*/
            }
        }
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("trav into fundef-next");
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        DBUG_PRINT ("trav from fundef-next");
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CONCELex(node *arg_node, info *arg_info)
 *
 *   @brief stores the assignment-chain into INFO_CONCEL_CELLASSIGN and
 *          frees the cell arg_node
 *
 *   @param arg_node a N_ex
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CONCELex (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_ex, "N_ex expected");

    /* store the assignment-chain for reuse in CONCELfundef */
    INFO_CONCEL_CELLASSIGN (arg_info) = BLOCK_ASSIGNS (EX_REGION (arg_node));

    BLOCK_ASSIGNS (EX_REGION (arg_node)) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CONCELst(node *arg_node, info *arg_info)
 *
 *   @brief stores the assignment-chain into INFO_CONCEL_CELLASSIGN and
 *          frees the cell arg_node
 *
 *   @param arg_node a N_st
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CONCELst (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_st, "N_st expected");

    /* store the assignment-chain for reuse in CONCELfundef */
    INFO_CONCEL_CELLASSIGN (arg_info) = BLOCK_ASSIGNS (ST_REGION (arg_node));

    BLOCK_ASSIGNS (ST_REGION (arg_node)) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CONCELmt(node *arg_node, info *arg_info)
 *
 *   @brief stores the assignment-chain into INFO_CONCEL_CELLASSIGN and
 *          frees the cell arg_node
 *
 *   @param arg_node a N_mt
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CONCELmt (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_mt, "N_mt expected");

    /* store the assignment-chain for reuse in CONCELfundef */
    INFO_CONCEL_CELLASSIGN (arg_info) = BLOCK_ASSIGNS (MT_REGION (arg_node));

    BLOCK_ASSIGNS (MT_REGION (arg_node)) = NULL;

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
