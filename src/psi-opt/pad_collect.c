/*
 * $Log$
 * Revision 1.5  2000/06/15 14:38:01  mab
 * dummies for APC block and let added
 *
 * Revision 1.4  2000/06/14 10:43:19  mab
 * dummies for APC ap, exprs, id, prf, fundef added
 *
 * Revision 1.3  2000/06/08 11:13:37  mab
 * added functions for nodes arg, vardec, array
 *
 * Revision 1.2  2000/05/31 16:16:58  mab
 * initial version
 *
 * Revision 1.1  2000/05/26 13:42:00  sbs
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   pad_collect.c
 *
 * prefix: APC
 *
 * description:
 *
 *   This compiler module collects information needed to infer new array
 *   shapes for the inference-phase.
 *
 *
 *****************************************************************************/

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "globals.h"

#include "pad_info.h"
#include "pad_collect.h"

/*****************************************************************************
 *
 * function:
 *   void APcollect (node *arg_node)
 *
 * description:
 *   main function for collection-phase of array padding
 *
 *****************************************************************************/
/* main function */
void
APcollect (node *arg_node)
{

    node *arg_info;
    funtab *tmp_tab;

    DBUG_ENTER ("APcollect");

    DBUG_PRINT ("AP", ("Array Padding: collecting data..."));

    tmp_tab = act_tab;
    act_tab = apc_tab;

    arg_info = MakeInfo ();

    arg_node = Trav (arg_node, arg_info);

    FREE (arg_info);
    act_tab = tmp_tab;

    DBUG_VOID_RETURN;
}

/*****************************************************************************
 *
 * function:
 *   node *APCarg(node *arg_node, node *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/

node *
APCarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("APCarg");

    DBUG_PRINT ("AP", ("arg-node detected\n"));

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APCvardec(node *arg_node, node *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/

node *
APCvardec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("APCvardec");

    DBUG_PRINT ("AP", ("vardec-node detected\n"));

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APCarray(node *arg_node, node *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/

node *
APCarray (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("APCarray");

    DBUG_PRINT ("AP", ("array-node detected\n"));

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APCNwith(node *arg_node, node *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/

node *
APCNwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("APCNwith");

    DBUG_PRINT ("AP", ("Nwith-node detected\n"));

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APCap(node *arg_node, node *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/

node *
APCap (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("APCap");

    DBUG_PRINT ("AP", ("ap-node detected"));

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APCexprs(node *arg_node, node *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/

node *
APCexprs (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("APCexprs");

    DBUG_PRINT ("AP", ("exprs-node detected"));

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APCid(node *arg_node, node *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/

node *
APCid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("APCid");

    DBUG_PRINT ("AP", ("id-node detected"));

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APCprf(node *arg_node, node *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/

node *
APCprf (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("APCprf");

    DBUG_PRINT ("AP", ("prf-node detected"));

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APCfundef(node *arg_node, node *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/

node *
APCfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("APCfundef");

    DBUG_PRINT ("AP", ("fundef-node detected"));

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APCblock(node *arg_node, node *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/

node *
APCblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("APCblock");

    DBUG_PRINT ("AP", ("block-node detected"));

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APClet(node *arg_node, node *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/

node *
APClet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("APClet");

    DBUG_PRINT ("AP", ("let-node detected"));

    DBUG_RETURN (arg_node);
}
