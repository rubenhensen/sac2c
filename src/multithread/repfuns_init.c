/*
 *
 * $Log$
 * Revision 3.1  2000/11/20 18:03:13  sacbase
 * new release made
 *
 * Revision 1.8  2000/10/31 23:21:41  dkr
 * Trav: NWITH2_CODE might be NULL
 *
 * Revision 1.7  2000/04/18 14:02:24  jhs
 * Added assertion to be aware of functions with NULL-body.
 *
 * Revision 1.6  2000/03/09 18:33:54  jhs
 * Brushing
 *
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
 *   Creation of replicated-functions (repfuns).
 *   Each function that is called within a scheduled with-loop is duplicated
 *   and all calls to that function from within schedules with-loops
 *   is changed to call the copy.
 *   The duplicated functions are called "repfuns".
 *   Within all repfuns all calls to functions (even if not in a with-loop)
 *   will be changed to call new created repfuns,
 *   also all schedulings of further with-loops will be eliminated.
 *   Each function will copied only once.
 *
 *   The repfuns will be expanded with an extra argument, which will
 *   transport the ThreadId into the functions called within
 *   multi-threading. So the ThreadID will be available for the
 *   HeapManager.
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
 *   Initiate the repfuns-initialisation as described above.
 *
 *   Traverses *only* the function handed over via arg_node with rfin_tab,
 *   will not traverse FUNDEF_NEXT( arg_node).
 *
 *   The flag INFO_RFIN_WITHINWITH( arg_info) will set to FALSE.
 *
 *   This routine ignores (returns without changes):
 *   - functions f with no body (FUNDEF_BODY( f) == NULL)
 *   - functions f with FUNDEF_STATUS( f) = ST_foldfun
 *   - repfuns
 *     functions f with FUNDEF_ATTRIB( f) = ST_call_rep
 *     (this ignores especially the new created repfuns).
 *
 ******************************************************************************/
node *
RepfunsInit (node *arg_node, node *arg_info)
{
    funtab *old_tab;
    int old_withinwith;

    DBUG_ENTER ("RepfunsInit");

    if ((FUNDEF_BODY (arg_node) != NULL) && (FUNDEF_STATUS (arg_node) != ST_foldfun)
        && (FUNDEF_ATTRIB (arg_node) != ST_call_rep)) {
        old_tab = act_tab;
        act_tab = rfin_tab;

        old_withinwith = INFO_RFIN_WITHINWITH (arg_info);
        INFO_RFIN_WITHINWITH (arg_info) = FALSE;

        DBUG_PRINT ("RFIN", ("trav into body"));
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
        DBUG_PRINT ("RFIN", ("trav from body"));

        INFO_RFIN_WITHINWITH (arg_info) = old_withinwith;

        act_tab = old_tab;
    }

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
 *   If the traversal hits a *scheduled with-loop* on the first level the
 *   INFO_RFIN_WITHINWITH flag will be set to TRUE. This indicates, that
 *   the traversal is within a scheduled with-loop.
 *   Then the with-loop-body is traversed and the flag restore afterwards.
 *
 ******************************************************************************/
node *
RFINnwith2 (node *arg_node, node *arg_info)
{
    int old_withinwith;

    DBUG_ENTER ("RFINnwith2");
    DBUG_PRINT ("RFIN", ("begin"));

    DBUG_PRINT ("RFIN", ("withinwith before %i", INFO_RFIN_WITHINWITH (arg_info)));

    old_withinwith = INFO_RFIN_WITHINWITH (arg_info);
    if (NWITH2_ISSCHEDULED (arg_node)) {
        INFO_RFIN_WITHINWITH (arg_info) = TRUE;
    } else {
        /* reuse value of arg_info (from wl above) */
    }

    DBUG_PRINT ("RFIN", ("withinwith while %i", INFO_RFIN_WITHINWITH (arg_info)));

    DBUG_PRINT ("RFIN", ("traverse into code"));
    if (NWITH2_CODE (arg_node) != NULL) {
        NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);
    }
    DBUG_PRINT ("RFIN", ("traverse from code"));

    INFO_RFIN_WITHINWITH (arg_info) = old_withinwith;
    ;

    DBUG_PRINT ("RFIN", ("withinwith after %i", INFO_RFIN_WITHINWITH (arg_info)));

    DBUG_PRINT ("RFIN", ("end"));
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *RFINlet( node* arg_node, node *arg_info)
 *
 * description:
 *   Depending on the kind of LET_EXPR that is found this routine takes
 *   special actions:
 *   - N_ap    : The call is changed, if necessary the function will be
 *               copied (if already done this is skipped)
 *   - N_prf   : No body is existent, so we can't copy ...
 *   - N_Nwith2: Traversal into the body
 *   - else    : Nothing is done.
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
            if (FUNDEF_BODY (AP_FUNDEF (LET_EXPR (arg_node))) != NULL) {
                DBUG_PRINT ("RFIN", ("ap-let != NULL: lift %s",
                                     FUNDEF_NAME (AP_FUNDEF (LET_EXPR (arg_node)))));

                ap = LET_EXPR (arg_node);

                old_fundef = AP_FUNDEF (ap);

                new_fundef = ReplicateFundef (old_fundef, arg_info);

                ap = MUTHExchangeApplication (ap, new_fundef);
            } else {
                DBUG_ASSERT (0, "ap-let == NULL");
            }
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
