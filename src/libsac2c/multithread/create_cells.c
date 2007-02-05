/*
 * $Log$
 * Revision 1.19  2004/11/24 19:40:47  skt
 * SACDevCampDK 2k4
 *
 * Revision 1.18  2004/11/23 20:52:11  skt
 * big compiler brushing during SACDevCampDK 2k4
 *
 * Revision 1.17  2004/11/23 14:38:13  skt
 * SACDevCampDK 2k4
 *
 * Revision 1.16  2004/11/22 14:59:51  skt
 * code brushing in SACDevCampDK 2004
 *
 * Revision 1.15  2004/08/26 17:05:04  skt
 * handling of MUTH_MULTI_SPECIALIZED added
 *
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

#include "tree_basic.h"
#include "traverse.h"
#include "create_cells.h"
#include "internal_lib.h"
#include "str.h"
#include "memory.h"
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

    result = MEMmalloc (sizeof (info));

    INFO_CRECE_LASTCELLID (result) = 0;
    INFO_CRECE_LASTEXECMODE (result) = MUTH_ANY;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/* some declaration */
static node *InsertCell (node *act_assign);

/** <!--********************************************************************-->
 *
 * @fn node *CRECEdoCreateCells(node *arg_node)
 *
 *   @brief  Inits the traversal for this phase
 *
 *   @param arg_node a N_Modul
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CRECEdoCreateCells (node *arg_node)
{
    info *arg_info;
    trav_t traversaltable;
    DBUG_ENTER ("CRECEdoCreateCells");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_module),
                 "CRECEdoCreateCells expects a N_module as arg_node");

    arg_info = MakeInfo ();

    TRAVpush (TR_crece);

    DBUG_PRINT ("CRECE", ("trav into module-funs"));
    MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    DBUG_PRINT ("CRECE", ("trav from module-funs"));

    traversaltable = TRAVpop ();
    DBUG_ASSERT ((traversaltable == TR_crece), "Popped incorrect traversal table");

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
        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
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
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_assign), "arg_node is not a N_assign");

    /* traverse into the instruction - it could be a conditional */
    if (ASSIGN_INSTR (arg_node) != NULL) {
        DBUG_PRINT ("CRECE", ("trav into instruction"));
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
        DBUG_PRINT ("CRECE", ("trav from instruction"));
    }

    if ((ASSIGN_EXECMODE (arg_node) != MUTH_ANY)
        && (ASSIGN_EXECMODE (arg_node) != MUTH_MULTI_SPECIALIZED)) {
        /* the number of initial cells depents on the usage of split-phase synchro-
         * nisation */
        if (MUTH_SPLITPHASE_ENABLED == TRUE) {
            if (ASSIGN_EXECMODE (arg_node) != INFO_CRECE_LASTEXECMODE (arg_info)) {
                arg_node = InsertCell (arg_node);
                INFO_CRECE_LASTEXECMODE (arg_info) = ASSIGN_EXECMODE (arg_node);
            }
        } else {
            if (ASSIGN_CELLID (arg_node) != INFO_CRECE_LASTCELLID (arg_info)) {
                arg_node = InsertCell (arg_node);
                INFO_CRECE_LASTCELLID (arg_info) = ASSIGN_CELLID (arg_node);
            }
        }
    }
    if (ASSIGN_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("CRECE", ("trav into next"));
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
        DBUG_PRINT ("CRECE", ("trav from next"));
    }

    DBUG_RETURN (arg_node);
}

static node *
InsertCell (node *act_assign)
{
    node *new_assign;
    DBUG_ENTER ("InsertCell");
    DBUG_ASSERT ((NODE_TYPE (act_assign) == N_assign), "N_assign expected");

    new_assign = NULL;

    switch (ASSIGN_EXECMODE (act_assign)) {
    case MUTH_EXCLUSIVE:
        new_assign = TBmakeAssign (TBmakeEx (TBmakeBlock (act_assign, NULL)), NULL);
        break;
    case MUTH_SINGLE:
        new_assign = TBmakeAssign (TBmakeSt (TBmakeBlock (act_assign, NULL)), NULL);
        break;
    case MUTH_MULTI:
        new_assign = TBmakeAssign (TBmakeMt (TBmakeBlock (act_assign, NULL)), NULL);
        break;
    case MUTH_MULTI_SPECIALIZED:
        DBUG_ASSERT ((FALSE), "MUTH_MULTI_SPECIALIZED is impossible here");
    case MUTH_ANY:
        DBUG_ASSERT ((FALSE), "MUTH_ANY is impossible here");
    }

    ASSIGN_EXECMODE (new_assign) = ASSIGN_EXECMODE (act_assign);
    ASSIGN_CELLID (new_assign) = ASSIGN_CELLID (act_assign);
    ASSIGN_NEXT (new_assign) = ASSIGN_NEXT (act_assign);
    ASSIGN_NEXT (act_assign) = NULL;

    DBUG_RETURN (new_assign);
}
