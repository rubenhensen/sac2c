/*
 * $Log$
 * Revision 1.9  2004/08/16 16:52:35  skt
 * implementation expanded
 *
 * Revision 1.8  2004/08/13 16:17:40  skt
 * *** empty log message ***
 *
 * Revision 1.7  2004/08/05 17:42:19  skt
 * moved handling of the allocation around the withloop into propagate_executionmode
 *
 * Revision 1.6  2004/08/05 13:50:18  skt
 * welcome to the new INFO structure
 *
 * Revision 1.5  2004/07/29 13:45:19  skt
 * Handling of iv & its elements enhanced
 *
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

#define NEW_INFO

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "print.h"
#include "create_cells.h"
#include "multithread.h"
#include "multithread_lib.h"

/*
 * INFO structure
 */
struct INFO {
    node *cellassign;
    node *firstanyassign;
    node *lastpointer;
    node *newblock;
};

/*
 * INFO macros
 *    node*    CRECE_ACTCELLASSIGN     (the actual, last assign in the actuell
 *                                      cell, where the next assignment hat to
 *                                      be added; NULL, if there's no cell yet)
 *    node*    CRECE_FIRSTANYASSIGN    (the first assignment of the block, that
 *                                      is any-threaded; only != NULL, if no
 *                                      EXCLUSIVE/SINGLE/MULTI-cell exsits
 *                                      before)
 *    node*    CRECE_LASTPOINTER
 */
#define INFO_CRECE_ACTCELLASSIGN(n) (n->cellassign)
#define INFO_CRECE_FIRSTANYASSIGN(n) (n->firstanyassign)
#define INFO_CRECE_LASTPOINTER(n) (n->lastpointer)
#define INFO_CRECE_NEWBLOCK(n) (n->newblock)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_CRECE_ACTCELLASSIGN (result) = NULL;
    INFO_CRECE_FIRSTANYASSIGN (result) = NULL;
    INFO_CRECE_LASTPOINTER (result) = NULL;
    INFO_CRECE_NEWBLOCK (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = Free (info);

    DBUG_RETURN (info);
}

/* TODO: TravNone in node_info.mac for:
 *                nwith2, N_let, N_return, N_ex, N_st, N_mt
 */

/** <!--********************************************************************-->
 *
 * @fn node *CreateCells(node *arg_node)
 *
 *   @brief  Inits the traversal for this phase
 *
 *   @param arg_node a N_Modul
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CreateCells (node *arg_node)
{
    funtab *old_tab;
    info *arg_info;
    DBUG_ENTER ("CreateCells");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_modul),
                 "CreateCells expects a N_modul as arg_node");

    arg_info = MakeInfo ();
    /* push info ... */
    old_tab = act_tab;
    act_tab = crece_tab;

    DBUG_PRINT ("CRECE", ("trav into modul-funs"));
    MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    DBUG_PRINT ("CRECE", ("trav from modul-funs"));

    /* pop info ... */
    act_tab = old_tab;

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CRECEfundef(node *arg_node, info *arg_info)
 *
 *   @brief stores the actual function into INFO_CRECE_FUNDEF (needed by
 *          MUTHInsertXX()
 *   @param arg_node a N_fundef
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
/*node *CRECEfundef(node *arg_node, info *arg_info) {
  node *old_fundef;
  DBUG_ENTER("CRECEfundef");
  DBUG_ASSERT((NODE_TYPE(arg_node) == N_fundef),
             "CRECEfundef expects a N_fundef as arg_node");

  old_fundef = INFO_CRECE_FUNDEF(arg_info);
  INFO_CRECE_FUNDEF(arg_info) = arg_node;

  if (FUNDEF_BODY(arg_node) != NULL) {
    DBUG_PRINT("CRECE",("trav into function-body"));
    FUNDEF_BODY(arg_node) = Trav(FUNDEF_BODY(arg_node),arg_info);
    DBUG_PRINT("CRECE",("trav from function-body"));
  }

  if (FUNDEF_NEXT(arg_node) != NULL) {
    DBUG_PRINT("CRECE",("trav into function-next"));
    FUNDEF_NEXT(arg_node) = Trav(FUNDEF_NEXT(arg_node),arg_info);
    DBUG_PRINT("CRECE",("trav from function-next"));
  }
  INFO_CRECE_FUNDEF(arg_info) = old_fundef;

  DBUG_RETURN(arg_node);
  }*/

node *
CRECEblock (node *arg_node, info *arg_info)
{
    node *old_lastpointer;
    node *old_newblock;
    node *dummy;
    DBUG_ENTER ("CRECEblock");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_block), "arg_node is not a N_block");

    /* push info */
    old_lastpointer = INFO_CRECE_LASTPOINTER (arg_info);
    old_newblock = INFO_CRECE_NEWBLOCK (arg_info);
    INFO_CRECE_LASTPOINTER (arg_info) = arg_node;

    /* initialization */
    INFO_CRECE_ACTCELLASSIGN (arg_info) = NULL;
    INFO_CRECE_FIRSTANYASSIGN (arg_info) = NULL;

    if (BLOCK_INSTR (arg_node) != NULL) {
        /* continue traversal */
        DBUG_PRINT ("CRECE", ("trav into instruction(s)"));
        dummy = Trav (BLOCK_INSTR (arg_node), arg_info);
        DBUG_PRINT ("CRECE", ("trav from instruction(s)"));
    }

    BLOCK_INSTR (arg_node) = INFO_CRECE_NEWBLOCK (arg_info);

    /* pop info */
    INFO_CRECE_LASTPOINTER (arg_info) = old_lastpointer;
    INFO_CRECE_NEWBLOCK (arg_info) = old_newblock;

    fprintf (stdout, "This block:\n");
    PrintNode (arg_node);
    fprintf (stdout, "Block end\n\n");

    DBUG_RETURN (arg_node);
}

node *
CRECEassign (node *arg_node, info *arg_info)
{
    node *last_pointer;
    node *act_cellassign;
    node *old_firstanyassign;
    node *dummy;
    DBUG_ENTER ("CRECEassign");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_assign), "arg_node is no a N_assign");

    /* push info */
    act_cellassign = INFO_CRECE_ACTCELLASSIGN (arg_info);

    if (ASSIGN_EXECMODE (arg_node) == MUTH_ANY) {
        /* we've got an any-threaded assignment - add it to the actual cell, if it
         * exists */
        if (act_cellassign != NULL) {
            ASSIGN_EXECMODE (arg_node) = ASSIGN_EXECMODE (act_cellassign);
            ASSIGN_NEXT (act_cellassign) = arg_node;
            /* we have to update the arg_info structure, not the local variable
             * act_celllassign ! */
            INFO_CRECE_ACTCELLASSIGN (arg_info) = arg_node;
        }
        /* so, no cell exists yet - let's look, if this assign is the first assign
         * with MUTH_ANY execmode in this block */
        else if (INFO_CRECE_FIRSTANYASSIGN (arg_info) == NULL) {
            /* yes - set the firstassign */
            INFO_CRECE_FIRSTANYASSIGN (arg_info) = arg_node;
        }
        /* no - do nothing */
        else {
        }
    } else {
        /* so this assignment has a MUTH_EXCLUSIVE, MUTH_SINLE or MUTH_MULTI
         * execmode */

        /* Does a cell exist and has it the correct executionmode? */
        if ((act_cellassign != NULL)
            && (ASSIGN_EXECMODE (act_cellassign) == ASSIGN_EXECMODE (arg_node))) {
            /* yes - that's fine; just add the current assignment to the
             * assign-chain */
            ASSIGN_NEXT (act_cellassign) = arg_node;

            /* we have to update the arg_info structure, not the local variable
             * act_celllassign ! */
            INFO_CRECE_ACTCELLASSIGN (arg_info) = arg_node;
        } else {
            /* no - well, we've got to build a new cell */

            /* update info-structure before you get a new arg_node */
            INFO_CRECE_ACTCELLASSIGN (arg_info) = arg_node;

            arg_node = CRECEInsertCell (arg_node, INFO_CRECE_FIRSTANYASSIGN (arg_info));

            last_pointer = INFO_CRECE_LASTPOINTER (arg_info);

            if (NODE_TYPE (last_pointer) == N_block) {
                INFO_CRECE_NEWBLOCK (arg_info) = arg_node;
                BLOCK_INSTR (last_pointer) = arg_node;
            } else {
                DBUG_ASSERT ((NODE_TYPE (last_pointer) == N_assign),
                             "N_assign as LASTPOINTER expected");
                ASSIGN_NEXT (last_pointer) = arg_node;
            }

            /* now the arg_node isn't the same as former */
            INFO_CRECE_LASTPOINTER (arg_info) = arg_node;
            INFO_CRECE_FIRSTANYASSIGN (arg_info) = NULL;
        }
    }

    fprintf (stdout, "act-assign:\n");
    PrintNode (arg_node);
    fprintf (stdout, "********************\n\n");

    /* push info... */
    old_firstanyassign = INFO_CRECE_FIRSTANYASSIGN (arg_info);
    act_cellassign = INFO_CRECE_ACTCELLASSIGN (arg_info);

    /* kill info */
    INFO_CRECE_ACTCELLASSIGN (arg_info) = NULL;
    INFO_CRECE_FIRSTANYASSIGN (arg_info) = NULL;

    /* traverse into the instruction - it could be a conditional */
    /* if (ASSIGN_INSTR(arg_node) != NULL) {
      DBUG_PRINT( "CRECE", ("trav into instruction"));
      dummy = Trav(ASSIGN_INSTR(arg_node), arg_info);
      DBUG_PRINT( "CRECE", ("trav from instruction"));
      } */

    /* pop info... */
    INFO_CRECE_ACTCELLASSIGN (arg_info) = act_cellassign;
    INFO_CRECE_FIRSTANYASSIGN (arg_info) = old_firstanyassign;

    if (ASSIGN_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("CRECE", ("trav into next"));
        dummy = Trav (ASSIGN_NEXT (arg_node), arg_info);
        DBUG_PRINT ("CRECE", ("trav from next"));
    }

    DBUG_RETURN (arg_node);
}

node *
CRECEInsertCell (node *act_assign, node *first_anyassign)
{
    node *old_assign;
    node *new_assign;
    DBUG_ENTER ("MUTHInsertCell");
    DBUG_ASSERT ((NODE_TYPE (act_assign) == N_assign), "N_assign expected");

    /* set the executionmode of the any_assign-chain */
    if (first_anyassign != NULL) {
        /* set the old assign to the first_anyassign, cause its not NULL */
        old_assign = first_anyassign;
        while (first_anyassign != act_assign) {
            ASSIGN_EXECMODE (first_anyassign) = ASSIGN_EXECMODE (act_assign);
            first_anyassign = ASSIGN_NEXT (first_anyassign);
        }
    } else {
        /* set the old_assign to the act_assign, cause there's no first_anyassign*/
        old_assign = act_assign;
    }

    switch (ASSIGN_EXECMODE (old_assign)) {
    case MUTH_EXCLUSIVE:
        new_assign = MakeAssign (MakeEX (MakeBlock (old_assign, NULL)), NULL);
        break;
    case MUTH_SINGLE:
        new_assign = MakeAssign (MakeST (MakeBlock (old_assign, NULL)), NULL);
        break;
    case MUTH_MULTI:
        new_assign = MakeAssign (MakeMT (MakeBlock (old_assign, NULL)), NULL);
        break;
    }

    ASSIGN_EXECMODE (new_assign) = ASSIGN_EXECMODE (act_assign);
    ASSIGN_NEXT (new_assign) = ASSIGN_NEXT (act_assign);
    ASSIGN_NEXT (act_assign) = NULL;

    DBUG_RETURN (new_assign);
}

/** <!--********************************************************************-->
 *
 * @fn node *CRECEassign(node *arg_node, info *arg_info)
 *
 *   @brief creates an Cell out of each (non-MUTH_ANY) tagged assignment
 *<pre>
 *          additional: the allocation of the indexvector for an MUTH_MULTI
 *          with-loop is integrated into the MT-Cell of the with-loop.
 *</pre>
 *
 *   @param arg_node a N_assign
 *   @param arg_info
 *   @return N_assign with probably added cell
 *
 *****************************************************************************/
/*node *CRECEassign(node *arg_node, info *arg_info) {
  DBUG_ENTER("CRECEassign");
  DBUG_ASSERT((NODE_TYPE(arg_node) == N_assign),
             "CRECEassign expects a N_assign as arg_node");

  switch (ASSIGN_EXECMODE(arg_node)) {
  case MUTH_ANY: break;
  case MUTH_EXCLUSIVE:
    DBUG_PRINT("CRECE", ("Executionmode is MUTH_EXCLUSIVE"));
    arg_node = MUTHInsertEX(arg_node, INFO_CRECE_FUNDEF(arg_info));
    break;
  case MUTH_SINGLE:
    DBUG_PRINT("CRECE", ("Executionmode is MUTH_SINGLE"));
    arg_node = MUTHInsertST(arg_node, INFO_CRECE_FUNDEF(arg_info));
    break;
  case MUTH_MULTI:
    DBUG_PRINT("CRECE", ("Executionmode is MUTH_MULTI"));
    arg_node = MUTHInsertMT(arg_node, INFO_CRECE_FUNDEF(arg_info));
    break;
  default: DBUG_ASSERT(0,"CRECEassign expects an assignment with valid executionmode");
    break;
  }

  if(ASSIGN_NEXT(arg_node) != NULL) {
    DBUG_PRINT( "CRECE", ("trav into next"));
    ASSIGN_NEXT(arg_node) = Trav(ASSIGN_NEXT(arg_node), arg_info);
    DBUG_PRINT( "CRECE", ("trav from next"));
  }

  DBUG_RETURN (arg_node);
  }*/
