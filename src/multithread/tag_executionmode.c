/*
 * $Log$
 * Revision 1.1  2004/06/08 14:17:29  skt
 * Initial revision
 *
 */

/**
 *
 * @defgroup tem Tag Executionmode
 * @ingroup muth
 *
 * @brief tags the mode of execution on an assignment
 * @{
 **/

/** <!--********************************************************************-->
 *
 * @file tag_executionmode.c
 *
 * prefix: TEM
 *
 * description:
 *   tags the assignments, wheter their executionmode is MUTH_ANY,
 *   MUTH_SINGLE, MUTH_ONCE or MUTH_MULTI
 *   MUTH_SINGLE: execution by one thread, all other threads idle
 *   MUTH_ONCE: execution by one thread, all other threads work
 *   MUTH_MULTI: execution by all threads
 *   MUTH_ANY: decision whether ST, OT or MT to be done later
 *
 *****************************************************************************/

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "print.h"
#include "tag_executionmode.h"
#include "multithread.h"

/** <!--********************************************************************-->
 *
 * @fn node *TagExecutionmode(node *arg_node, node *arg_info)
 *
 *   @brief  Inits the traversal for this phase
 *
 *   @param arg_node a N_Modul
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
TagExecutionmode (node *arg_node, node *arg_info)
{
    funtab *old_tab;

    DBUG_ENTER ("TagExecutionmode");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_modul),
                 "TagExecutionmode expects a N_modul as arg_node");

    /* push info ... */
    old_tab = act_tab;
    act_tab = tem_tab;

    DBUG_PRINT ("TEM", ("trav into modul-funs"));
    MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    DBUG_PRINT ("TEM", ("trav from modul-funs"));

    /* pop info ... */
    act_tab = old_tab;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TEMassign(node *arg_node, node *arg_info)
 *
 *   @brief tags the N_assign with its executionmode
 *
 *   @param arg_node a N_assign
 *   @param arg_info
 *   @return N_assign with tagged
 *
 *****************************************************************************/
node *
TEMassign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("TEMassign");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_assign),
                 "TEMassign expects a N_assign as arg_node");

    /* reset to default */
    INFO_TEM_EXECMODE (arg_info) = MUTH_ANY;

    DBUG_PRINT ("TEM", ("trav into instruction"));
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    DBUG_PRINT ("TEM", ("trav from instruction"));

    /* set the executionmode of the current assignment */
    ASSIGN_EXECMODE (arg_info) = INFO_TEM_EXECMODE (arg_info);

    if (ASSIGN_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("TEM", ("trav into next"));
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
        DBUG_PRINT ("TEM", ("trav from next"));
    }

    DBUG_RETURN (arg_node);
}

node *
TEMwith2 (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("TEMwith2");
    DBUG_ASSERT ((NODE_TYPE (arg_info) == N_Nwith2),
                 "TEMwith2 expects a N_with2 as argument");

    DBUG_RETURN (arg_node);
}

int
IsMTAllowed (node *withloop)
{
    int is_allowed;
    DBUG_ENTER ("IsMTAllowes");
    DBUG_ASSERT ((NODE_TYPE (withloop) == N_Nwith2),
                 "IsMTAllowed expects a N_with2 as argument");

    if (NWITHOP_IS_FOLD (NWITH2_WITHOP (withloop)) && (max_sync_fold == 0)) {
        is_allowed = FALSE;
    } else {
        is_allowed = TRUE;
    }

    DBUG_RETURN (is_allowed);
}

/**
 * @}
 **/
