/*
 * $Id$
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
#include "str.h"

#define DBUG_PREFIX "CRECE"
#include "debug.h"

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
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

    INFO_CRECE_LASTCELLID (result) = 0;
    INFO_CRECE_LASTEXECMODE (result) = MUTH_ANY;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

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
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_module,
                 "CRECEdoCreateCells expects a N_module as arg_node");

    arg_info = MakeInfo ();

    TRAVpush (TR_crece);

    DBUG_PRINT ("trav into module-funs");
    MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    DBUG_PRINT ("trav from module-funs");

    traversaltable = TRAVpop ();
    DBUG_ASSERT (traversaltable == TR_crece, "Popped incorrect traversal table");

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

node *
CRECEblock (node *arg_node, info *arg_info)
{
    int old_cellid;
    mtexecmode_t old_execmode;
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_block, "arg_node is not a N_block");

    /* push info... */
    old_cellid = INFO_CRECE_LASTCELLID (arg_info);
    old_execmode = INFO_CRECE_LASTEXECMODE (arg_info);

    INFO_CRECE_LASTCELLID (arg_info) = 0;
    INFO_CRECE_LASTEXECMODE (arg_info) = MUTH_ANY;

    /* continue traversal */
    if (BLOCK_ASSIGNS (arg_node) != NULL) {
        DBUG_PRINT ("trav into instruction(s)");
        BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);
        DBUG_PRINT ("trav from instruction(s)");
    }

    /* pop info... */
    INFO_CRECE_LASTCELLID (arg_info) = old_cellid;
    INFO_CRECE_LASTEXECMODE (arg_info) = old_execmode;

    DBUG_RETURN (arg_node);
}

node *
CRECEassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_assign, "arg_node is not a N_assign");

    /* traverse into the instruction - it could be a conditional */
    if (ASSIGN_STMT (arg_node) != NULL) {
        DBUG_PRINT ("trav into instruction");
        ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
        DBUG_PRINT ("trav from instruction");
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
        DBUG_PRINT ("trav into next");
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
        DBUG_PRINT ("trav from next");
    }

    DBUG_RETURN (arg_node);
}

static node *
InsertCell (node *act_assign)
{
    node *new_assign;
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (act_assign) == N_assign, "N_assign expected");

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
        DBUG_ASSERT (FALSE, "MUTH_MULTI_SPECIALIZED is impossible here");
    case MUTH_ANY:
        DBUG_ASSERT (FALSE, "MUTH_ANY is impossible here");
    }

    ASSIGN_EXECMODE (new_assign) = ASSIGN_EXECMODE (act_assign);
    ASSIGN_CELLID (new_assign) = ASSIGN_CELLID (act_assign);
    ASSIGN_NEXT (new_assign) = ASSIGN_NEXT (act_assign);
    ASSIGN_NEXT (act_assign) = NULL;

    DBUG_RETURN (new_assign);
}

#undef DBUG_PREFIX
