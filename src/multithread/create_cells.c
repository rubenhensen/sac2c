/*
 * $Log$
 * Revision 1.14  2004/08/19 17:45:53  skt
 * initialization of new_assign added
 *
 * Revision 1.13  2004/08/18 13:24:31  skt
 * switch to mtexecmode_t done
 *
 * Revision 1.12  2004/08/18 12:55:33  skt
 * added case MUTH_ANY into CRECEInsertCell
 * changed int into mtexecmode_t at executionmodes
 *
 * Revision 1.11  2004/08/17 10:37:31  skt
 * push / pop added at N_block
 *
 * Revision 1.10  2004/08/16 18:15:26  skt
 * implementation finished
 *
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
 * @brief creates initial cells
 * @{
 **/

/** <!--********************************************************************-->
 *
 * @file create_cells.c
 *
 * prefix: CRECE
 *
 * description:
 *   creates a seperate cell around each first assignment of a CELLID, which
 *   is MUTH_EXCLUSIVE, MUTH_SINGLE or MUTH_MULTI tagged
 *
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
    int last_cellid;
    mtexecmode_t last_execmode;
};

/*
 * INFO macros
 *    int           CRECE_LASTCELLID        (the cellid of the last assignment)
 *    mtexecmode_t  CREEC_LASTEXECMODE      (the executiomode of the last cell)
 */
#define INFO_CRECE_LASTCELLID(n) (n->last_cellid)
#define INFO_CRECE_LASTEXECMODE(n) (n->last_execmode)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_CRECE_LASTCELLID (result) = 0;
    INFO_CRECE_LASTEXECMODE (result) = MUTH_ANY;

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

node *
CRECEblock (node *arg_node, info *arg_info)
{
    int old_cellid;
    mtexecmode_t old_execmode;
    DBUG_ENTER ("CRECEblock");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_block), "arg_node is not a N_block");

    /* push info... */
    old_cellid = INFO_CRECE_LASTCELLID (arg_info);
    old_execmode = INFO_CRECE_LASTEXECMODE (arg_info);

    INFO_CRECE_LASTCELLID (arg_info) = 0;
    INFO_CRECE_LASTEXECMODE (arg_info) = MUTH_ANY;

    /* continue traversal */
    if (BLOCK_INSTR (arg_node) != NULL) {
        DBUG_PRINT ("CRECE", ("trav into instruction(s)"));
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
        DBUG_PRINT ("CRECE", ("trav from instruction(s)"));
    }

    /* pop info... */
    INFO_CRECE_LASTCELLID (arg_info) = old_cellid;
    INFO_CRECE_LASTEXECMODE (arg_info) = old_execmode;

    DBUG_RETURN (arg_node);
}

node *
CRECEassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CRECEassign");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_assign), "arg_node is no a N_assign");

    /* traverse into the instruction - it could be a conditional */
    if (ASSIGN_INSTR (arg_node) != NULL) {
        DBUG_PRINT ("CRECE", ("trav into instruction"));
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
        DBUG_PRINT ("CRECE", ("trav from instruction"));
    }

    if (ASSIGN_EXECMODE (arg_node) != MUTH_ANY) {
        /* the number of initial cells depents on the usage of split-phase synchro-
         * nisation */
        if (MUTH_SPLITPHASE_ENABLED == TRUE) {
            if (ASSIGN_EXECMODE (arg_node) != INFO_CRECE_LASTEXECMODE (arg_info)) {
                arg_node = CRECEInsertCell (arg_node);
                INFO_CRECE_LASTEXECMODE (arg_info) = ASSIGN_EXECMODE (arg_node);
            }
        } else {
            if (ASSIGN_CELLID (arg_node) != INFO_CRECE_LASTCELLID (arg_info)) {
                arg_node = CRECEInsertCell (arg_node);
                INFO_CRECE_LASTCELLID (arg_info) = ASSIGN_CELLID (arg_node);
            }
        }
    }
    if (ASSIGN_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("CRECE", ("trav into next"));
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
        DBUG_PRINT ("CRECE", ("trav from next"));
    }

    DBUG_RETURN (arg_node);
}

node *
CRECEInsertCell (node *act_assign)
{
    node *new_assign;
    DBUG_ENTER ("MUTHInsertCell");
    DBUG_ASSERT ((NODE_TYPE (act_assign) == N_assign), "N_assign expected");

    new_assign = NULL;

    switch (ASSIGN_EXECMODE (act_assign)) {
    case MUTH_EXCLUSIVE:
        new_assign = MakeAssign (MakeEX (MakeBlock (act_assign, NULL)), NULL);
        break;
    case MUTH_SINGLE:
        new_assign = MakeAssign (MakeST (MakeBlock (act_assign, NULL)), NULL);
        break;
    case MUTH_MULTI:
        new_assign = MakeAssign (MakeMT (MakeBlock (act_assign, NULL)), NULL);
        break;
    case MUTH_ANY:
        DBUG_ASSERT ((FALSE), "MUTH_ANY is impossible here");
    }

    ASSIGN_EXECMODE (new_assign) = ASSIGN_EXECMODE (act_assign);
    ASSIGN_CELLID (new_assign) = ASSIGN_CELLID (act_assign);
    ASSIGN_NEXT (new_assign) = ASSIGN_NEXT (act_assign);
    ASSIGN_NEXT (act_assign) = NULL;

    DBUG_RETURN (new_assign);
}
