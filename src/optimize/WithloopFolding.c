/* 	$Id$
 *
 * $Log$
 * Revision 1.5  1998/02/27 13:38:10  srs
 * checkin with deactivated traversal
 *
 * Revision 1.4  1998/02/24 14:19:20  srs
 * *** empty log message ***
 *
 * Revision 1.3  1998/02/09 15:58:20  srs
 * *** empty log message ***
 *
 *
 */

/*******************************************************************************
 This file realizes the WL-folding for the new SAC-WLs.

 what happens:
 we search for withloops in the body of all functions.  Any found WL will be
 put in the list wl_found. Only the latest (reachable) WLs are in this list.
   E.g. a WL "A = with..." is replaced by another "A = with2..." or removed
   after "A = nonwithexpr".

 Every time we find a new WL we check if we can fold is with one of the
 already known WLs from the wl_found list.

 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "tree.h"
#include "types.h"
#include "internal_lib.h"
#include "free.h"
#include "globals.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "traverse.h"
#include "optimize.h"
#include "WithloopFolding.h"

/******************************************************************************
 *
 * function:
 *   node *WLFfundef(node *arg_node, node *arg_info)
 *
 * description:
 *   we search in every function separately for withloops.
 *   The folding always happens exclusively in a function body.
 *
 *   The optimization traversal OPTTrav is included in WLF traversal to
 *   modify USE and DEF and to create MRD masks.
 *
 ******************************************************************************/

node *
WLFfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLFfundef");

    FUNDEF_INSTR (arg_node) = OPTTrav (FUNDEF_INSTR (arg_node), arg_info, arg_node);
    FUNDEF_NEXT (arg_node) = OPTTrav (FUNDEF_NEXT (arg_node), arg_info, arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLFassign(node *arg_node, node *arg_info)
 *
 * description:
 *   only needed to apply OPTTrav
 *
 *
 ******************************************************************************/

node *
WLFassign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLFassign");

    ASSIGN_INSTR (arg_node) = OPTTrav (ASSIGN_INSTR (arg_node), arg_info, arg_node);
    ASSIGN_NEXT (arg_node) = OPTTrav (ASSIGN_NEXT (arg_node), arg_info, arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLFcond(node *arg_node, node *arg_info)
 *
 * description:
 *   only needed to apply OPTTrav
 *
 *
 ******************************************************************************/

node *
WLFcond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLFcond");

    /* we do not need to traverse the condition
       COND_COND(arg_node) = OPTTrav(COND_COND(arg_node), arg_info, arg_node);
    */

    /* traverse bodies. MRDs are build locally in the bodies. The DEF mask is
       evaluated in the superior OPTTrav to modify the actual MRD list. */
    COND_THENINSTR (arg_node) = OPTTrav (COND_THENINSTR (arg_node), arg_info, arg_node);
    COND_ELSEINSTR (arg_node) = OPTTrav (COND_ELSEINSTR (arg_node), arg_info, arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLFdo(node *arg_node, node *arg_info)
 *
 * description:
 *   only needed to apply OPTTrav
 *
 *
 ******************************************************************************/

node *
WLFdo (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLFdo");

    DO_INSTR (arg_node) = OPTTrav (DO_INSTR (arg_node), arg_info, arg_node);
    DO_COND (arg_node) = OPTTrav (DO_COND (arg_node), arg_info, arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLFwhile(node *arg_node, node *arg_info)
 *
 * description:
 *   only needed to apply OPTTrav
 *
 *
 ******************************************************************************/

node *
WLFwhile (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLFwhile");

    WHILE_COND (arg_node) = OPTTrav (WHILE_COND (arg_node), arg_info, arg_node);
    WHILE_INSTR (arg_node) = OPTTrav (WHILE_INSTR (arg_node), arg_info, arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLFwith(node *arg_node, node *arg_info)
 *
 * description:
 *   only needed to apply OPTTrav
 *
 *
 ******************************************************************************/

node *
WLFwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLFwith");

    WITH_GEN (arg_node) = OPTTrav (WITH_GEN (arg_node), arg_info, arg_node);
    switch (NODE_TYPE (WITH_OPERATOR (arg_node))) {
    case N_genarray:
        BLOCK_INSTR (GENARRAY_BODY (WITH_OPERATOR (arg_node)))
          = OPTTrav (BLOCK_INSTR (GENARRAY_BODY (WITH_OPERATOR (arg_node))), arg_info,
                     arg_node);
        break;
    case N_modarray:
        BLOCK_INSTR (MODARRAY_BODY (WITH_OPERATOR (arg_node)))
          = OPTTrav (BLOCK_INSTR (MODARRAY_BODY (WITH_OPERATOR (arg_node))), arg_info,
                     arg_node);
        break;
    case N_foldprf:
        BLOCK_INSTR (FOLDPRF_BODY (WITH_OPERATOR (arg_node)))
          = OPTTrav (BLOCK_INSTR (FOLDPRF_BODY (WITH_OPERATOR (arg_node))), arg_info,
                     arg_node);
        break;
    case N_foldfun:
        BLOCK_INSTR (FOLDFUN_BODY (WITH_OPERATOR (arg_node)))
          = OPTTrav (BLOCK_INSTR (FOLDFUN_BODY (WITH_OPERATOR (arg_node))), arg_info,
                     arg_node);
        break;
    default:
        DBUG_ASSERT (0, "Operator not implemented for with_node");
        break;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLFNwith(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

node *
WLFNwith (node *arg_node, node *arg_info)
{
    node *tmpn;

    DBUG_ENTER ("WLFNwith");

    INFO_IS_WL (arg_info) = 1; /* we found a WL here */
    /* inside the body of this WL we may find another WL. So we better
       save this information. */
    tmpn = MakeInfo ();
    INFO_NEXT (tmpn) = arg_info;
    arg_info = tmpn;

    INFO_WL (arg_info) = arg_node; /* store the current node for later */

    /* restore arg_info */
    tmpn = arg_info;
    arg_info = INFO_NEXT (arg_info);
    FREE (tmpn);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLFWithloopFolding(node *arg_node, node* arg_info)
 *
 * description:
 *   starting point for the withloop folding.
 *
 *
 ******************************************************************************/

node *
WLFWithloopFolding (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLFWithloopFolding");

    act_tab = wlf_tab;

    DBUG_ASSERT (!arg_info, ("at the beginning of WLF: arg_info != NULL"));
    arg_info = MakeInfo ();
    /*   Trav(arg_node,arg_info); */
    FREE (arg_info);

    DBUG_RETURN (arg_node);
}
