/*
 *
 * $Log$
 * Revision 1.3  2000/02/11 16:21:01  jhs
 * Expanded traversals ...
 *
 * Revision 1.2  2000/02/04 14:44:43  jhs
 * Added infrastructure.
 *
 * Revision 1.1  2000/02/04 13:48:36  jhs
 * Initial revision
 *
 *
 */

/******************************************************************************
 *
 * file:   blocks_init.c
 *
 * prefix: RFIN
 *
 * description:
 *   ####
 *
 ******************************************************************************/

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "scheduling.h"
#include "DupTree.h"
#include "generatemasks.h"
#include "globals.h"
#include "free.h"

#include "internal_lib.h"

/******************************************************************************
 *
 * function:
 *   node *RepfunsInit(node *arg_node, node *arg_info)
 *
 * description:
 *   ####
 *
 ******************************************************************************/
node *
RepfunsInit (node *arg_node, node *arg_info)
{
    funtab *old_tab;
    int old_withinwith;
    node *old_firstfundef;
    node *old_lastfundef;
    int old_search;

    DBUG_ENTER ("RepfunsInit");

    old_tab = act_tab;
    act_tab = rfin_tab;

    old_withinwith = INFO_RFIN_WITHINWITH (arg_info);
    old_firstfundef = INFO_RFIN_FIRSTFUNDEF (arg_info);
    old_lastfundef = INFO_RFIN_LASTFUNDEF (arg_info);
    old_search = INFO_RFIN_SEARCH (arg_info);
    INFO_RFIN_WITHINWITH (arg_info) = FALSE;
    INFO_RFIN_FIRSTFUNDEF (arg_info) = NULL;
    INFO_RFIN_LASTFUNDEF (arg_info) = NULL;
    INFO_RFIN_SEARCH (arg_info) = FALSE;

    DBUG_PRINT ("RFIN", ("trav into body"));
    FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    DBUG_PRINT ("RFIN", ("trav from body"));

    /*
      if (FUNDEF_NEXT( arg_node) != NULL) {
        INFO_RFIN_SEARCH( arg_info) = TRUE;

        DBUG_PRINT( "RFIN", ("indirect hang in"));
        DBUG_PRINT( "RFIN", ("trav into next"));
        FUNDEF_NEXT( arg_node) = Trav( FUNDEF_NEXT( arg_node), arg_info);
        DBUG_PRINT( "RFIN", ("trav from next"));

      } else {
        DBUG_PRINT( "RFIN", ("direct hang in"));
        FUNDEF_NEXT( arg_node) = INFO_RFIN_FIRSTFUNDEF( arg_info);
      }
    */

    INFO_RFIN_WITHINWITH (arg_info) = old_withinwith;
    INFO_RFIN_FIRSTFUNDEF (arg_info) = old_firstfundef;
    INFO_RFIN_LASTFUNDEF (arg_info) = old_lastfundef;
    INFO_RFIN_SEARCH (arg_info) = old_search;

    act_tab = old_tab;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *ReplicateFundef (node *fundef, node* arg_info)
 *
 * description:
 *   returns the N_fundef of the replicated function,
 *   creates replicated version if not alredy done.
 *
 ******************************************************************************/
node *
ReplicateFundef (node *fundef, node *arg_info)
{
    node *result;
    char *old_name;
    char *new_name;
    char *suffix;

    DBUG_ENTER ("ReplicateFundef");

    if (FUNDEF_REPFUN (fundef) == NULL) {
        /*
         *  This function is not replicated yet, this is done now.
         */
        result = DupNode (fundef);
        FUNDEF_STATUS (result) = ST_repfun;

        /*
         *  note reference to replicated version, needed for further reuse.
         */
        FUNDEF_REPFUN (fundef) = result;

        /*
         *  Change name.
         */
        old_name = FUNDEF_NAME (result);
        suffix = "__REPLICATED__";
        new_name = Malloc (strlen (old_name) + strlen (suffix) + 1);
        strcpy (new_name, suffix);
        strcat (new_name, old_name);
        FUNDEF_NAME (result) = new_name;
        FREE (old_name);

        /*
         *  Add new replicated function to table of new replicated functions.
         */
        FUNDEF_NEXT (result) = FUNDEF_NEXT (fundef);
        FUNDEF_NEXT (fundef) = result;
        /*
            if (INFO_RFIN_FIRSTFUNDEF( arg_info) == NULL) {
              INFO_RFIN_FIRSTFUNDEF( arg_info) = result;
              INFO_RFIN_LASTFUNDEF( arg_info) = result;
            } else {
              FUNDEF_NEXT( INFO_RFIN_LASTFUNDEF( arg_info)) = result;
              INFO_RFIN_LASTFUNDEF( arg_info) = result;
            }
        */
    } else {
        /*
         *  This function is alredy replicated, reuse it.
         */
        result = FUNDEF_REPFUN (fundef);
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *RFINnwith2( node *arg_node, node *arg_info)
 *
 * description:
 *   ####
 *
 ******************************************************************************/
node *
RFINnwith2 (node *arg_node, node *arg_info)
{
    int old_withinwith;

    DBUG_ENTER ("RFINnwith2");

    NOTE (("hitrfinnwith2"));

    old_withinwith = INFO_RFIN_WITHINWITH (arg_info);

    if (NWITH2_ISSCHEDULED (arg_node)) {
        INFO_RFIN_WITHINWITH (arg_info) = TRUE;
    } else {
        /* reuse value of arg_info (from wl above) */
    }

    NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);

    INFO_RFIN_WITHINWITH (arg_info) = old_withinwith;
    ;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *RFINlet( node* arg_node, node *arg_info)
 *
 * description:
 *   ####
 *
 ******************************************************************************/
node *
RFINlet (node *arg_node, node *arg_info)
{
    node *ap;
    node *old_fundef;
    node *new_fundef;

    DBUG_ENTER ("RFINlet");

    NOTE (("hitrfinlet"));
    /* if lhs application then lift */

    if (INFO_RFIN_WITHINWITH (arg_info)) {
        if (NODE_TYPE (LET_EXPR (arg_node)) == N_ap) {
            NOTE (("hit lift %s", FUNDEF_NAME (AP_FUNDEF (LET_EXPR (arg_node)))));

            ap = LET_EXPR (arg_node);

            old_fundef = AP_FUNDEF (ap);

            new_fundef = ReplicateFundef (old_fundef, arg_info);

            AP_FUNDEF (ap) = new_fundef;

            FREE (AP_NAME (ap));
            AP_NAME (ap) = StringCopy (FUNDEF_NAME (new_fundef));
            FREE (AP_MOD (ap));
            AP_MOD (ap) = StringCopy (FUNDEF_MOD (new_fundef));
        }
    } else {
        if (NODE_TYPE (LET_EXPR (arg_node)) == N_Nwith2) {
            LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

node *
RFINfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("RFINfundef");

    if (FUNDEF_NEXT (arg_node) == NULL) {
        /*    if (INFO_RFIN_SEARCH( arg_info)) {
              FUNDEF_NEXT( arg_node) = INFO_RFIN_FIRSTFUNDEF( arg_info);
            } */
    } else {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
