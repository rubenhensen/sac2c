/*
 * $Log$
 * Revision 1.2  2004/11/22 14:37:39  skt
 * code brushing in SACDevCampDK 2004
 *
 * Revision 1.1  2004/09/02 16:02:22  skt
 * Initial revision
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

#define NEW_INFO

#include "dbug.h"
#include "tree_basic.h"
#include "traverse.h"
#include "print.h"
#include "consolidate_cells.h"

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
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_CONCEL_CELLASSIGN (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = Free (info);

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
    funtab *old_tab;
    info *arg_info;
    DBUG_ENTER ("CONCELdoConsoldateCells");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_module),
                 "CONCELdoConsolidateCells expects a N_module as arg_node");

    arg_info = MakeInfo ();
    /* push info ... */
    old_tab = act_tab;
    act_tab = concel_tab;

    DBUG_PRINT ("CONCEL", ("trav into module-funs"));
    arg_node = Trav (MODULE_FUNS (arg_node), arg_info);
    DBUG_PRINT ("CONCEL", ("trav from module-funs"));

    /* pop info ... */
    act_tab = old_tab;

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
    DBUG_ENTER ("CONCELfundef");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef), "N_fundef expected");

    myblock = FUNDEF_BODY (arg_node);
    if (myblock != NULL) {
        if (NODE_TYPE (BLOCK_INSTR (myblock)) == N_assign) {
            if (ASSIGN_NEXT (BLOCK_INSTR (myblock)) == NULL) {
                /* traverse into the function to get the correct N_assign in
                 *  INFO_CONCEL_XTASSIGN */
                DBUG_PRINT ("CONCEL", ("trav into fundef-body"));
                myblock = Trav (myblock, arg_info);
                DBUG_PRINT ("CONCEL", ("trav from fundef-body"));

                /*BLOCK_INSTR(myblock) = Free(BLOCK_INSTR(myblock));*/
                /*BLOCK_INSTR(myblock) = INFO_CONCEL_CELLASSIGN(arg_info);*/
            }
        }
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("CONCEL", ("trav into fundef-next"));
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
        DBUG_PRINT ("CONCEL", ("trav from fundef-next"));
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
    DBUG_ENTER ("CONCELex");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_ex), "N_ex expected");

    /* store the assignment-chain for reuse in CONCELfundef */
    INFO_CONCEL_CELLASSIGN (arg_info) = BLOCK_INSTR (EX_REGION (arg_node));

    BLOCK_INSTR (EX_REGION (arg_node)) = NULL;

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
    DBUG_ENTER ("CONCELst");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_st), "N_st expected");

    /* store the assignment-chain for reuse in CONCELfundef */
    INFO_CONCEL_CELLASSIGN (arg_info) = BLOCK_INSTR (ST_REGION (arg_node));

    BLOCK_INSTR (ST_REGION (arg_node)) = NULL;

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
    DBUG_ENTER ("CONCELmt");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_mt), "N_mt expected");

    /* store the assignment-chain for reuse in CONCELfundef */
    INFO_CONCEL_CELLASSIGN (arg_info) = BLOCK_INSTR (MT_REGION (arg_node));

    BLOCK_INSTR (MT_REGION (arg_node)) = NULL;

    DBUG_RETURN (arg_node);
}
