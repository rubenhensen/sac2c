/*
 * $Log$
 * Revision 1.2  2004/07/28 17:46:14  skt
 * CRECEfundef added
 *
 * Revision 1.1  2004/07/26 16:11:55  skt
 * Initial revision
 *
 */

/**
 *
 * @defgroup crece Create Cells
 * @ingroup muth
 *
 * @brief tags the mode of execution on an assignment
 * @{
 **/

/** <!--********************************************************************-->
 *
 * @file create_cells.c
 *
 * prefix: CRECE
 *
 * description:
 *   creates a seperate cell around each MUTH_EXCLUSIVE, MUTH_SINGLE and
 *   MUTH_MULTI tagged assignment. Includes the corresponding allocation for
 *   an MUTH_MULTI withloop into the same with-loop, too.
 *****************************************************************************/

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "print.h"
#include "create_cells.h"
#include "multithread.h"
#include "multithread_lib.h"

/** <!--********************************************************************-->
 *
 * @fn node *CreateCells(node *arg_node, node *arg_info)
 *
 *   @brief  Inits the traversal for this phase
 *
 *   @param arg_node a N_Modul
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CreateCells (node *arg_node, node *arg_info)
{
    funtab *old_tab;

    DBUG_ENTER ("CreateCells");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_modul),
                 "CreateCells expects a N_modul as arg_node");

    /* push info ... */
    old_tab = act_tab;
    act_tab = crece_tab;

    DBUG_PRINT ("CRECE", ("trav into modul-funs"));
    MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    DBUG_PRINT ("CRECE", ("trav from modul-funs"));

    /* pop info ... */
    act_tab = old_tab;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CRECEfundef(node *arg_node, node *arg_info)
 *
 *   @brief stores the actual function into INFO_MUTH_FUNDEF (needed by
 *          MUTHInsertXX()
 *   @param arg_node a N_fundef
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CRECEfundef (node *arg_node, node *arg_info)
{
    node *old_fundef;
    DBUG_ENTER ("CRECEfundef");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef),
                 "CRECEfundef expects a N_fundef as arg_node");

    old_fundef = INFO_MUTH_FUNDEF (arg_info);
    INFO_MUTH_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node) != NULL) {
        DBUG_PRINT ("CRECE", ("trav into function-body"));
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
        DBUG_PRINT ("CRECE", ("trav from function-body"));
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("CRECE", ("trav into function-next"));
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
        DBUG_PRINT ("CRECE", ("trav from function-next"));
    }
    INFO_MUTH_FUNDEF (arg_info) = old_fundef;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CRECEassign(node *arg_node, node *arg_info)
 *
 *   @brief creates an Cell out of each (non-MUTH_ANY) tagged assignment
 *<pre>
 *          additional: the allocation of the indexvector for an MUTH_MULTI
 *          with-loop is integrated into the MT-Cell of the with-loop.
 *</pre>
 *
 *   @param arg_node a N_assign
 *   @param arg_info
 *   @return N_assign with probably added Cell
 *
 *****************************************************************************/
node *
CRECEassign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CRECEassign");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_assign),
                 "CRECEassign expects a N_assign as arg_node");

    switch (ASSIGN_EXECMODE (arg_node)) {
    case MUTH_ANY:
        break;
    case MUTH_EXCLUSIVE:
        DBUG_PRINT ("CRECE", ("Executionmode is MUTH_EXCLUSIVE"));
        arg_node = MUTHInsertEX (arg_node, arg_info);
        break;
    case MUTH_SINGLE:
        DBUG_PRINT ("CRECE", ("Executionmode is MUTH_SINGLE"));
        arg_node = MUTHInsertST (arg_node, arg_info);
        break;
    case MUTH_MULTI:
        DBUG_PRINT ("CRECE", ("Executionmode is MUTH_MULTI"));
        arg_node = MUTHInsertMT (arg_node, arg_info);
        if (NODE_TYPE (ASSIGN_INSTR (LET_EXPR (arg_node))) == N_Nwith2) {
            arg_node = CRECEAddIv (arg_node, arg_info);
        }
        break;
    default:
        DBUG_ASSERT (0, "CRECEassign expects an assignment with valid executionmode");
        break;
    }

    if (ASSIGN_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("CRECE", ("trav into next"));
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
        DBUG_PRINT ("CRECE", ("trav from next"));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CRECEAddIv(node *arg_node, node *arg_info)
 *
 *   @brief adds the allocation of the indexvector for an MUTH_MULTI
 *          with-loop into the MT-Cell of the with-loop.
 *
 *   @param arg_node a N_assign
 *   @param arg_info
 *   @return N_assign with probably added Cell
 *
 *****************************************************************************/
node *
CRECEAddIv (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CRECEAddIv");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_assign),
                 "CRECEAddIv expects a N_assign as arg_node");

    /* TODO*/

    DBUG_RETURN (arg_node);
}
