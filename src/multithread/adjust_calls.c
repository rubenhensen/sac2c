/*
 *
 * $Log$
 * Revision 1.2  2000/03/30 15:07:24  jhs
 * Tried to build in removal of st-interior ...
 *
 * Revision 1.1  2000/03/30 10:18:05  jhs
 * Initial revision
 *
 *
 */

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

        DBUG_ASSERT ((INFO_ADJCA_ATTRIB (arg_info) == ST_call_mt_worker),
                     "actual attrib wrong");
        DBUG_PRINT ("ADJCA", ("%s", mdb_statustype[FUNDEF_ATTRIB (arg_node)]));
        DBUG_ASSERT ((FUNDEF_ATTRIB (arg_node) == ST_call_mt_master),
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
