/*
 * $Log$
 * Revision 1.4  2004/07/28 23:37:23  skt
 * improved the handling of the indexvectors
 *
 * Revision 1.3  2004/07/28 22:45:22  skt
 * changed CRECEAddIv into CRECEHandleIv,
 * implementation changed & tested
 *
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
        if (NODE_TYPE (LET_EXPR (ASSIGN_INSTR (arg_node))) == N_Nwith2) {
            CRECEHandleIv (LET_EXPR (ASSIGN_INSTR (arg_node)), arg_info);
        }
        arg_node = MUTHInsertMT (arg_node, arg_info);
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
 * @fn void CRECEHandleIv(node *withloop, node *arg_info)
 *
 *   @brief creates a MT-cell for the allocation(s) of the indexvector(s) of
 *          an parallel executed with-loop
 *
 *   @param withloop a N_Nwith2
 *   @param arg_info
 *   @return nothing
 *
 *****************************************************************************/
void
CRECEHandleIv (node *withloop, node *arg_info)
{
    ids *iterator;
    int executionmode;
    DBUG_ENTER ("CRECEAddIv");
    DBUG_ASSERT ((NODE_TYPE (withloop) == N_Nwith2),
                 "CRECEAddIv expects a N_Nwith2 as argument withloop");

    iterator = NWITHID_VEC (NWITH2_WITHID (withloop));
    while (iterator != NULL) {
        executionmode = ASSIGN_EXECMODE (AVIS_SSAASSIGN (IDS_AVIS (iterator)));
        DBUG_ASSERT ((executionmode == MUTH_ANY) || (executionmode == MUTH_MULTI),
                     "Executionmode of iv-alloc must be MUTH_ANY or MUTH_MULTI");
        ASSIGN_EXECMODE (AVIS_SSAASSIGN (IDS_AVIS (iterator))) = MUTH_MULTI;

        AVIS_SSAASSIGN (IDS_AVIS (iterator))
          = MUTHInsertMT (AVIS_SSAASSIGN (IDS_AVIS (iterator)), arg_info);
        iterator = IDS_NEXT (iterator);
    }

    DBUG_VOID_RETURN;
}
