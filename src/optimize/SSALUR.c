/*
 *
 * $Log$
 * Revision 1.1  2001/04/20 11:20:56  nmw
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   SSALUR.c
 *
 * prefix: SSALUR
 *
 * description:
 *
 *   This module implements loop-unrolling for special do-functions in ssa
 *   form. all while loops have been removed and converted to do-loops before
 *   so we have to deal only with the do loops.
 *   To have all necessary information about constant data, you should do
 *   a SSAConstantFolding traversal first.
 *
 *****************************************************************************/

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "dbug.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "SSALUR.h"
#include "optimize.h"

static ids *TravIDS (ids *arg_ids, node *arg_info);
static ids *SSALURids (ids *arg_ids, node *arg_info);

/* helper functions for internal use only */
static int SSALURGetDoLoopUnrolling (node *fundef);

/******************************************************************************
 *
 * function:
 *   int SSALURGetDoLoopUnrolling(node *fundef)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
static int
SSALURGetDoLoopUnrolling (node *fundef)
{
    DBUG_ENTER ("SSALURGetDoLoopUnrolling");
    DBUG_RETURN (0);
}

/******************************************************************************
 *
 * function:
 *   node *SSALURfundef(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
node *
SSALURfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSALURfundef");

    INFO_SSALUR_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node) != NULL) {
        /* traverse block of fundef */
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSALURassign(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
node *
SSALURassign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSALURassign");

    DBUG_ASSERT ((ASSIGN_INSTR (arg_node) != NULL), "assign node without instruction");

    INFO_SSALUR_ASSIGN (arg_info) = arg_node;

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    /* traverse to next assignment in chain */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSALURap(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses the args and starts the traversal in the called fundef if it is
 *   a special fundef.
 *
 ******************************************************************************/
node *
SSALURap (node *arg_node, node *arg_info)
{
    node *new_arg_info;
    DBUG_ENTER ("SSALURap");

    DBUG_ASSERT ((AP_FUNDEF (arg_node) != NULL), "missing fundef in ap-node");

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
    }

    /* traverse special fundef without recursion */
    if ((FUNDEF_IS_LACFUN (AP_FUNDEF (arg_node)))
        && (AP_FUNDEF (arg_node) != INFO_SSALUR_FUNDEF (arg_info))) {
        DBUG_PRINT ("SSALUR", ("traverse in special fundef %s",
                               FUNDEF_NAME (AP_FUNDEF (arg_node))));

        INFO_SSALUR_MODUL (arg_info)
          = CheckAndDupSpecialFundef (INFO_SSALUR_MODUL (arg_info), AP_FUNDEF (arg_node),
                                      INFO_SSALUR_ASSIGN (arg_info));

        DBUG_ASSERT ((FUNDEF_USED (AP_FUNDEF (arg_node)) == 1),
                     "more than one instance of special function used.");

        /* stack arg_info frame for new fundef */
        new_arg_info = MakeInfo ();

        INFO_SSALUR_MODUL (new_arg_info) = INFO_SSALUR_MODUL (arg_info);

        /* start traversal of special fundef */
        AP_FUNDEF (arg_node) = Trav (AP_FUNDEF (arg_node), new_arg_info);

        DBUG_PRINT ("SSALUR", ("traversal of special fundef %s finished\n",
                               FUNDEF_NAME (AP_FUNDEF (arg_node))));
        FREE (new_arg_info);

    } else {
        DBUG_PRINT ("SSALUR", ("do not traverse in normal fundef %s",
                               FUNDEF_NAME (AP_FUNDEF (arg_node))));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSALURlet(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
node *
SSALURlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSALURlet");

    DBUG_ASSERT ((LET_EXPR (arg_node) != NULL), "let without expression");

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    if (LET_IDS (arg_node) != NULL) {
        LET_IDS (arg_node) = TravIDS (LET_IDS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSALURid(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
node *
SSALURid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSALURid");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   ids *SSALURids (ids *arg_ids, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
static ids *
SSALURids (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("SSALURids");

    /* traverse to next ids in chain */
    if (IDS_NEXT (arg_ids) != NULL) {
        IDS_NEXT (arg_ids) = TravIDS (IDS_NEXT (arg_ids), arg_info);
    }

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   ids *TravIDS(ids *arg_ids, node *arg_info)
 *
 * description:
 *   implements a simple TravIDS function like Trav for nodes to have
 *   an similar implementation.
 *
 ******************************************************************************/
static ids *
TravIDS (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("TravIDS");

    DBUG_ASSERT (arg_ids != NULL, "traversal in NULL ids");
    arg_ids = SSALURids (arg_ids, arg_info);

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   node* SSALoopUnrolling(node* fundef, node *modul)
 *
 * description:
 *   starts the LoopUnrolling traversal for the given fundef. Does not start
 *   in special fundefs.
 *
 ******************************************************************************/
node *
SSALoopUnrolling (node *fundef, node *modul)
{
    node *arg_info;
    funtab *old_tab;

    DBUG_ENTER ("SSALoopUnrolling");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef), "SSALUR called for non-fundef node");

    DBUG_PRINT ("OPT",
                ("starting loop unrolling (ssa) in function %s", FUNDEF_NAME (fundef)));

    /* do not start traversal in special functions */
    if (!(FUNDEF_IS_LACFUN (fundef))) {
        arg_info = MakeInfo ();
        INFO_SSALUR_MODUL (arg_info) = modul;

        old_tab = act_tab;
        act_tab = ssalur_tab;

        fundef = Trav (fundef, arg_info);

        act_tab = old_tab;

        FREE (arg_info);
    }

    DBUG_RETURN (fundef);
}
