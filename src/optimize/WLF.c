/*    $Id$
 *
 * $Log$
 * Revision 1.3  1998/04/08 20:33:53  srs
 * new WLF functions
 *
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

/******************************************************************************
 *
 *   typedefs
 *
 ******************************************************************************/

/* Several traverse functions of this file are traversed for different
   purposes. This enum determines ths function. */
typedef enum {
    wlfm_search_WL,  /* find new WLs */
    wlfm_search_ref, /* find references within WL to fold. */
    wlfm_rename,     /* traverse copied body of substWL and
                        rename variables to avoid name clashes. */
    wlfm_replace     /* traverse copied body of targetWL and
                        replace substituted reference. */
} wlf_mode_type;

/******************************************************************************
 *
 *   global variables
 *
 ******************************************************************************/

wlf_mode_type wlf_mode;

intern_gen *new_ig;     /* new generators derived by a traversel
                           of one Ncode block. */
intern_gen *all_new_ig; /* new generators derived from all Ncode
                           nodes. */
intern_gen *orig_ig;    /* original generators of the target WL */

node *new_codes; /* This is a list of created N_Ncode nodes */

/******************************************************************************
 *
 * function:
 *   int FoldDecision(node *target_wl, node* subst_wl)
 *
 * description:
 *   decides whether subst_wl shall be folded into target_wl or not.
 *
 *
 ******************************************************************************/

int
FoldDecision (node *target_wl, node *subst_wl)
{
    int result;

    DBUG_ENTER ("FoldDecision");

    subst_wl = LET_EXPR (ASSIGN_INSTR (subst_wl));

    result
      = (NWITH_PARTS (target_wl) > 0 && NWITH_PARTS (subst_wl) > 0
         && NWITH_REFERENCED (subst_wl) == NWITH_REFERENCED_FOLD (subst_wl)
         && (WO_genarray != NWITH_TYPE (subst_wl) || WO_modarray != NWITH_TYPE (subst_wl))
         && !NWITH_NO_CHANCE (subst_wl));

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *CheckForSuperfuousCodes(node *wln)
 *
 * description:
 *   remove all unused N_Ncode nodes of the given WL.
 *
 *
 ******************************************************************************/

node *
CheckForSuperfuousCodes (node *wln)
{
    node **tmp, *tmpn;

    DBUG_ENTER ("CheckForSuperfuousCodes");

    tmp = &NWITH_CODE (wln);
    while (*tmp)
        if (!NCODE_USED ((*tmp))) {
            tmpn = *tmp;
            *tmp = FreeNode (tmpn);
        } else
            tmp = &NCODE_NEXT ((*tmp));

    DBUG_RETURN (wln);
}

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

    switch (wlf_mode) {
    case wlfm_search_WL:
        /* check if we can fold this Id.
           If no, clear ID_WL(). This is the sign for the following phases to fold.
           If yes, set INFO_WLI_FLAG. */
        if (INFO_WLI_WL (arg_info) && /* inside WL */
            ID_WL (arg_node))         /* a WL is referenced. This implies that
                                         Id is a reference to a WL. */
            if (FoldDecision (INFO_WLI_WL (arg_info), ID_WL (arg_node)))
                INFO_WLI_FLAG (arg_info) = 1;
            else
                ID_WL (arg_node) = NULL;

    default:
        DBUG_ASSERT (0, ("Not expected"));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLFlet(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

node *
WLFlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLFlet");

    switch (wlf_mode) {
    case wlfm_search_WL:
        LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
        break;

    default:
        DBUG_ASSERT (0, ("Not expected"));
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

    switch (wlf_mode) {
    case wlfm_search_WL:
        /* It's faster to
           1. traverse into bodies to find WL within and fold them first
           2. and then try to fold references to other WLs.

           */
        INFO_WLI_FLAG (arg_info) = 0;
        arg_node = TravSons (arg_node, arg_info);

        if (INFO_WLI_FLAG (arg_info)) {
            /* traverse bodies of this WL again and fold now.
               Do not traverse WithOp (Id would confuse us). */
            wlf_mode = wlfm_search_ref;

            all_new_ig = NULL;
            new_codes = NULL;
            NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);

            /* all codes have been traversed. Now append new_codes to WL and
             exchange old generators with all_new_ig. */
            if (new_codes) {
                /* We have new codes. This means at least one folding action has been
                   done and we have to replace the N_Nparts, too. */
                tmpn = NWITH_CODE (arg_node);
                while (NCODE_NEXT (tmpn))
                    tmpn = NCODE_NEXT (tmpn);
                NCODE_NEXT (tmpn) = new_codes;

                arg_node = InternGen2Tree (arg_node, all_new_ig);
                all_new_ig = FreeInternGenChain (all_new_ig);

                arg_node = CheckForSuperfuousCodes (arg_node);
            }

            /* this WL is finisched. Search other WLs on same level. */
            wlf_mode = wlfm_search_WL;
        }
        break;

    default:
        DBUG_ASSERT (0, ("Not expected"));
    }

    /* restore arg_info */
    tmpn = arg_info;
    arg_info = INFO_WLI_NEXT (arg_info);
    INFO_DEF = tmpn->mask[0];
    INFO_USE = tmpn->mask[1];
    FREE (tmpn);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLFNcode(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

node *
WLFNcode (node *arg_node, node *arg_info)
{
    intern_gen *ig;

    DBUG_ENTER ("WLFNcode");

    switch (wlf_mode) {
    case wlfm_search_WL:
        arg_node = TravSons (arg_node, arg_info);
        break;

    case wlfm_search_ref:
        /* create generator list orig_ig. Copy all generators of the target-WL
           to this list which point to the current N_Ncode node. Don't use
           the traversal mechanism because it's slow. */
        orig_ig = Tree2InternGen (INFO_WLI_WL (arg_info), arg_node);
        new_ig = NULL;

        /* traverse Code, create new_ig, fold. */
        NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);

        /* copy new generators to all_new_ig and clear new_ig. */
        if (!all_new_ig)
            all_new_ig = new_ig;
        else {
            ig = all_new_ig;
            while (ig->next)
                ig = ig->next;
            ig->next = new_ig;
        }
        new_ig = NULL;

        FreeInternGenChain (orig_ig);

        /* traverse next code block. */
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
        break;

    default:
        DBUG_ASSERT (0, ("Not expected"));
    }

    DBUG_RETURN (arg_node);
}
