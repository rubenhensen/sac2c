/*
 *
 * $Log$
 * Revision 1.5  2000/03/02 13:02:28  jhs
 * Build some abstraction (in multithreaded_lib), tested and repaired.
 *
 * Revision 1.4  2000/02/21 17:54:20  jhs
 * Testing ...
 *
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
 * file:   repfuns_init.c
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
#include "multithread_lib.h"

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
    node *old_lastfundef;
    int old_search;

    DBUG_ENTER ("RepfunsInit");

    old_tab = act_tab;
    act_tab = rfin_tab;

    old_withinwith = INFO_RFIN_WITHINWITH (arg_info);
    old_lastfundef = INFO_RFIN_LASTFUNDEF (arg_info);
    old_search = INFO_RFIN_SEARCH (arg_info);
    INFO_RFIN_WITHINWITH (arg_info) = FALSE;
    INFO_RFIN_LASTFUNDEF (arg_info) = NULL;
    INFO_RFIN_SEARCH (arg_info) = FALSE;

    DBUG_PRINT ("RFIN", ("trav into body"));
    FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    DBUG_PRINT ("RFIN", ("trav from body"));

    INFO_RFIN_WITHINWITH (arg_info) = old_withinwith;
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

    DBUG_ENTER ("ReplicateFundef");

    if (FUNDEF_COMPANION (fundef) == NULL) {
        DBUG_PRINT ("RFIN", ("replicate: new copy"));
        /*
         *  This function is not replicated yet, this is done now.
         */
        result = DupNode (fundef);
        FUNDEF_ATTRIB (result) = ST_call_rep;

        /*
         *  note reference to replicated version, needed for further reuse.
         */
        FUNDEF_COMPANION (fundef) = result;

        /*
         *  Change name.
         */
        result = MUTHExpandFundefName (result, "__CALL_REP__");

        /*
         *  Add new replicated function to table of new replicated functions.
         */
        FUNDEF_NEXT (result) = FUNDEF_NEXT (fundef);
        FUNDEF_NEXT (fundef) = result;

        /*
         *  We want to traverse the function only,
         *  hopefully RFINfundef prevents us to traverse endless and recursive.
         */
        DBUG_PRINT ("RFIN", ("traverse into body"));
        result = Trav (result, arg_info);
        DBUG_PRINT ("RFIN", ("traverse from body"));
    } else {
        DBUG_PRINT ("RFIN", ("replicate: reuse copy"));
        /*
         *  This function is alredy replicated, reuse it.
         */
        result = FUNDEF_COMPANION (fundef);
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

    DBUG_PRINT ("RFIN", ("hit"));

    old_withinwith = INFO_RFIN_WITHINWITH (arg_info);

    if (INFO_RFIN_WITHINWITH (arg_info)) {
        DBUG_PRINT ("RFIN", ("before true"));
    } else {
        DBUG_PRINT ("RFIN", ("before false"));
    }

    if (NWITH2_ISSCHEDULED (arg_node)) {
        INFO_RFIN_WITHINWITH (arg_info) = TRUE;
    } else {
        /* reuse value of arg_info (from wl above) */
    }

    if (INFO_RFIN_WITHINWITH (arg_info)) {
        DBUG_PRINT ("RFIN", ("while true"));
    } else {
        DBUG_PRINT ("RFIN", ("while false"));
    }

    DBUG_PRINT ("RFIN", ("traverse into code"));
    NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);
    DBUG_PRINT ("RFIN", ("traverse from code"));

    INFO_RFIN_WITHINWITH (arg_info) = old_withinwith;
    ;

    if (INFO_RFIN_WITHINWITH (arg_info)) {
        DBUG_PRINT ("RFIN", ("after true"));
    } else {
        DBUG_PRINT ("RFIN", ("after false"));
    }

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

    /* if lhs application then lift */
    if (INFO_RFIN_WITHINWITH (arg_info)) {
        if (NODE_TYPE (LET_EXPR (arg_node)) == N_ap) {
            DBUG_PRINT ("RFIN", ("ap-let: lift %s",
                                 FUNDEF_NAME (AP_FUNDEF (LET_EXPR (arg_node)))));

            ap = LET_EXPR (arg_node);

            old_fundef = AP_FUNDEF (ap);

            new_fundef = ReplicateFundef (old_fundef, arg_info);

            ap = MUTHExchangeApplication (ap, new_fundef);
        }
    } else if (NODE_TYPE (LET_EXPR (arg_node)) == N_Nwith2) {
        DBUG_PRINT ("RFIN", ("with-let: traverse into"));
        LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
        DBUG_PRINT ("RFIN", ("with-let: traverse from"));
    } else {
        DBUG_PRINT ("RFIN", ("any-let: ignore"));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* RFINfundef (node *arg_node, node *arg_info)
 *
 * description:
 *   prevents traversals into FUNDEF_NEXT to avoid endless recursion
 *
 ******************************************************************************/
node *
RFINfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("RFINfundef");

    FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

    /* DO NOT TRAVERSE INTO NEXT!!! */

    DBUG_RETURN (arg_node);
}
