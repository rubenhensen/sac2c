/*    $Id$
 *
 * $Log$
 * Revision 1.2  1998/04/07 16:50:06  srs
 * new WLF functions
 *
 * Revision 1.1  1998/03/22 18:21:46  srs
 * Initial revision
 *
 */

/*******************************************************************************

 This file realizes the withlop folding.

 *******************************************************************************

 Usage of arg_info:
 - node[0]: store old information in nested WLs
 - node[1]: reference to base node of current WL (N_Nwith)
 - node[2]: always the last N_assign node
 - node[3]: pointer to last fundef node. needed to access vardecs.
 - flag: collects information in the wlfm_search phase if there is
   at least one foldable reference in the WL.
 - varno  : number of variables in this function, see optimize.c
 - mask[0]: DEF mask, see optimize.c
 - mask[1]: USE mask, see optimize.c

 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "tree.h"
#include "types.h"
#include "internal_lib.h"
#include "free.h"
#include "print.h"
#include "DupTree.h"
#include "globals.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "traverse.h"
#include "optimize.h"
#include "ConstantFolding.h"
#include "WithloopFolding.h"
#include "WLF.h"

typedef enum {
    wlfm_search_WL,  /* find new WLs */
    wlfm_search_ref, /* find references within WL to fold. */
    wlfm_rename,     /* traverse copied body of substWL and
                        rename variables to avoid name clashes. */
    wlfm_replace     /* traverse copied body of targetWL and
                        replace substituted reference. */
} wlf_mode_type;

wlf_mode_type wlf_mode;

/******************************************************************************
 *
 * function:
 *   node *WLFfundef(node *arg_node, node *arg_info)
 *
 * description:
 *   initializations to arg_info.
 *
 *
 ******************************************************************************/

node *
WLFfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLFfundef");

    INFO_WLI_WL (arg_info) = NULL;
    INFO_WLI_FUNDEF (arg_info) = arg_node;
    wlf_mode = wlfm_search_WL;

    arg_node = TravSons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLFassign(node *arg_node, node *arg_info)
 *
 * description:
 *   set arg_info to remember this N_assign node.
 *
 *
 ******************************************************************************/

node *
WLFassign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLFassign");

    INFO_WLI_ASSIGN (arg_info) = arg_node;

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    if (ASSIGN_NEXT (arg_node))
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLFid(node *arg_node, node *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
WLFid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLFid");

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

    /* inside the body of this WL we may find another WL. So we better
       save the old arg_info information. */
    tmpn = MakeInfo ();
    tmpn->mask[0] = INFO_DEF; /* DEF and USE information have */
    tmpn->mask[1] = INFO_USE; /* to be identical. */
    tmpn->varno = INFO_VARNO;
    INFO_WLI_FUNDEF (tmpn) = INFO_WLI_FUNDEF (arg_info);
    INFO_WLI_ASSIGN (tmpn) = INFO_WLI_ASSIGN (arg_info);
    INFO_WLI_NEXT (tmpn) = arg_info;
    arg_info = tmpn;

    INFO_WLI_WL (arg_info) = arg_node; /* store the current node for later */

    if (wlf_mode == wlfm_search_WL) {
        /* It's faster to
           1. traverse into bodies to find WL within and fold them first
           2. and then try to fold references to other WLs.

           */
        INFO_WLI_FLAG (arg_info) = 0;
        arg_node = TravSons (arg_node, arg_info);

        if (INFO_WLI_FLAG (arg_info)) {
            /* traverse bodies of this WL and fold. */
            wlf_mode == wlfm_search_ref;

            /* this WL is finisched. Search other WLs on same level. */
            wlf_mode == wlfm_search_WL;
        }
    }

    /* restore arg_info */
    tmpn = arg_info;
    arg_info = INFO_WLI_NEXT (arg_info);
    INFO_DEF = tmpn->mask[0];
    INFO_USE = tmpn->mask[1];
    FREE (tmpn);

    DBUG_RETURN (arg_node);
}

/*
   Voraussetzungen zum Falten:
   REF == REF_FOLD
   FOLDABLE != 0
   ist modarray/genarray (kein fold)
   NO_CHANCE == 0
 */
