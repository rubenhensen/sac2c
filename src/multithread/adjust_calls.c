/*
 *
 * $Log$
 * Revision 1.4  2000/04/14 17:43:26  jhs
 * Comments ...
 *
 * Revision 1.3  2000/03/31 16:24:33  jhs
 * improved
 *
 * Revision 1.2  2000/03/30 15:07:24  jhs
 * Tried to build in removal of st-interior ...
 *
 * Revision 1.1  2000/03/30 10:18:05  jhs
 * Initial revision
 *
 *
 */

/******************************************************************************
 *
 * file:   adjust_class.c
 *
 * prefix: ADJCA1 / ADJCA2
 *
 * description:
 *   This phase creates master und worker-functions. All functions to be
 *   executed multithreaded are tagged as ST_call_mt_master, these function
 *   will be the master functions. From each master-function a copy is created
 *   and tagged as ST_call_mt_worker. From now on the suffix "master" will
 *   have real significance.
 *   Within master-functions only masters are to be called, from within workers
 *   only workers. So the calls have to be adjusted after coping the
 *   functions.
 *
 *   This phase consist of two traversal, the first (ADJCA1) copies all the
 *   functions, and the second (ADJCA2) adjusts the calls.
 *   The two traversals are necessary: The adjustment has to be done in
 *   all workers, so we need to have this worker do the adjustment.
 *   And the function to be called must exist to. The easist way is to
 *   simple copy all functions first and then adjust.
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
 *   node* AdjustCalls1( node *arg_node, node *arg_info)
 *
 * description:
 *   Starts first traversal, for comments see file comments above.
 *
 ******************************************************************************/
node *
AdjustCalls1 (node *arg_node, node *arg_info)
{
    funtab *old_tab;

    DBUG_ENTER ("AdjustCalls1");
    DBUG_PRINT ("ADJCA", ("begin"));

    if (FUNDEF_ATTRIB (arg_node) == ST_call_mt_master) {
        old_tab = act_tab;
        act_tab = adjca1_tab;

        arg_node = Trav (arg_node, arg_info);

        act_tab = old_tab;
    }

    DBUG_PRINT ("ADJCA", ("end"));
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* AdjustCalls2( node *arg_node, node *arg_info)
 *
 * description:
 *   Starts second traversal, for comments see file comments above.
 *
 ******************************************************************************/
node *
AdjustCalls2 (node *arg_node, node *arg_info)
{
    funtab *old_tab;

    DBUG_ENTER ("AdjustCalls2");
    DBUG_PRINT ("ADJCA", ("begin"));

    if (FUNDEF_ATTRIB (arg_node) == ST_call_mt_worker) {
        old_tab = act_tab;
        act_tab = adjca2_tab;

        arg_node = Trav (arg_node, arg_info);

        act_tab = old_tab;
    }

    DBUG_PRINT ("ADJCA", ("end"));
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *ADJCA1fundef( node *arg_node, node *arg_info)
 *
 * description:
 *   (First part of this phase!)
 *   Creates a copy of each ST_call_mt[_master] function tagged as
 *   ST_call_mt_worker. Now the meaning of ST_call_mt_master is not only
 *   call_mt anymore, but also call_mt_master.
 *
 * attention:
 *   this traversal handles ST_call_mt_master functions only, but ignores
 *   the new ST_call_mt_workers. So we assert this within here.
 *
 ******************************************************************************/
node *
ADJCA1fundef (node *arg_node, node *arg_info)
{
    node *new_fundef;

    DBUG_ENTER ("ADJCA1fundef");
    DBUG_PRINT ("ADJCA", ("begin"));

    DBUG_ASSERT ((FUNDEF_ATTRIB (arg_node) == ST_call_mt_master),
                 "wrong attribs in fundef ...");

    new_fundef = DupNode (arg_node);

    FUNDEF_NEXT (new_fundef) = FUNDEF_NEXT (arg_node);
    FUNDEF_NEXT (arg_node) = new_fundef;
    arg_node = MUTHExpandFundefName (arg_node, "MASTER");
    new_fundef = MUTHExpandFundefName (new_fundef, "WORKER");
    FUNDEF_WORKER (arg_node) = new_fundef;
    FUNDEF_ATTRIB (new_fundef) = ST_call_mt_worker;

    DBUG_PRINT ("ADJCA", ("end"));
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *ADJCA1fundef( node *arg_node, node *arg_info)
 *
 * description:
 *   (Second part of this phase!)
 *   Here we adjust all calls, within workers only workers are called
 *   and within masters only masters are called.
 *   The masters are all correct we only change the workers.
 *   The first traversal has created a copy of each mt-function so there
 *   always must be a N_fundef tagged as worker to set the N_ap to.
 *
 * attention:
 *   this traversal handles ST_call_mt_worker functions only, but ignores
 *   the new ST_call_mt_masters. So we assert this within here.
 *
 ******************************************************************************/
node *
ADJCA2fundef (node *arg_node, node *arg_info)
{
    statustype old_attrib;

    DBUG_ENTER ("ADJCA2fundef");
    DBUG_PRINT ("ADJCA", ("begin"));

    DBUG_ASSERT ((FUNDEF_ATTRIB (arg_node) == ST_call_mt_worker),
                 "wrong attribs in fundef ...");

    old_attrib = INFO_ADJCA_ATTRIB (arg_info);
    INFO_ADJCA_ATTRIB (arg_info) = ST_call_mt_worker;

    DBUG_PRINT ("ADJCA", ("traverse into body"));
    FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    DBUG_PRINT ("ADJCA", ("traverse info body"));

    INFO_ADJCA_ATTRIB (arg_node) = old_attrib;

    DBUG_PRINT ("ADJCA", ("end"));
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *ADJCA2st( node *arg_node, node *arg_info)
 *
 * description:
 *   Within workers we do not need contents of ST-blocks anymore (but we still
 *   need the ST-block, with it's mask for code generation!!!). We remove
 *   contents here.
 *
 ******************************************************************************/
node *
ADJCA2st (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ADJCAst");
    DBUG_PRINT ("ADJCA", ("begin"));

    ST_REGION (arg_node) = FreeTree (ST_REGION (arg_node));
    ST_REGION (arg_node) = NULL;

    DBUG_PRINT ("ADJCA", ("end"));
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* ADJCA2let( node *arg_node, node *arg_info)
 *
 * description:
 *   Here is the part were the calls really are adjusted.
 *   This routine is able to change a call from a master to a worker, but
 *   not necessaryly the other way round.
 *
 ******************************************************************************/
node *
ADJCA2let (node *arg_node, node *arg_info)
{
    node *ap;
    node *old_fundef;
    node *new_fundef;

    DBUG_ENTER ("ADJCAlet");
    DBUG_PRINT ("ADJCA", ("begin"));

    if (NODE_TYPE (LET_EXPR (arg_node)) == N_ap) {
        ap = LET_EXPR (arg_node);

        old_fundef = AP_FUNDEF (ap);
        DBUG_PRINT ("ADJCA", ("old_fundef=%s", NODE_TEXT (old_fundef)));

        DBUG_ASSERT ((INFO_ADJCA_ATTRIB (arg_info) == ST_call_mt_worker),
                     "actual attrib wrong");
        DBUG_PRINT ("ADJCA", ("%s status=%s attrib=%s", FUNDEF_NAME (old_fundef),
                              mdb_statustype[FUNDEF_STATUS (old_fundef)],
                              mdb_statustype[FUNDEF_ATTRIB (old_fundef)]));
        DBUG_ASSERT ((FUNDEF_ATTRIB (old_fundef) == ST_call_mt_master),
                     "fundef attrib wrong");

        if (FUNDEF_ATTRIB (old_fundef) != INFO_ADJCA_ATTRIB (arg_info)) {
            new_fundef = FUNDEF_WORKER (old_fundef);
            DBUG_ASSERT ((new_fundef != NULL), "new_fundef is NULL");
            ap = MUTHExchangeApplication (ap, new_fundef);
        }
    }

    DBUG_PRINT ("ADJCA", ("end"));
    DBUG_RETURN (arg_node);
}
