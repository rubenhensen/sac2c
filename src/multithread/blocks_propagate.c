/*
 *
 * $Log$
 * Revision 1.2  2000/03/21 13:07:38  jhs
 * Finished this traversal.
 *
 * Revision 1.1  2000/03/09 19:50:36  jhs
 * Initial revision
 *
 *
 *
 * constructed by jhs@dArtagnan at home
 */

/******************************************************************************
 *
 * file:   blocks_propagate.c
 *
 * prefix: BLKPP
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

#include "blocks_init.h"

#include "internal_lib.h"
#include "multithread_lib.h"

/******************************************************************************
 *
 * function:
 *
 * description:
 *   ####
 *
 ******************************************************************************/
node *
BlocksPropagate (node *arg_node, node *arg_info)
{
    funtab *old_tab;

    DBUG_ENTER ("BlocksPropagate");
    DBUG_PRINT ("BLKPP", ("begin"));

    if ((FUNDEF_BODY (arg_node) != NULL) && (FUNDEF_STATUS (arg_node) != ST_foldfun)
        && (FUNDEF_ATTRIB (arg_node) != ST_call_rep)) {
        old_tab = act_tab;
        act_tab = blkpp_tab;

        /* push info */

        arg_node = Trav (arg_node, arg_info);

        /* pop info */

        act_tab = old_tab;
    }

    DBUG_PRINT ("BLKPP", ("end"));
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node BLKPPfundef( node *arg_node, node *arg_info)
 *
 * description:
 *   DO NOT TRAVERSE FUNDEF_NEXT!!!
 *
 ******************************************************************************/
node *
BLKPPfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("BLKPPfundef");
    DBUG_PRINT ("BLKPP", ("begin"));

    FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

    DBUG_PRINT ("BLKPP", ("end"));
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *BLKPPassign( node *arg_node, node* arg_info)
 *
 * description:
 *   ####
 *
 ******************************************************************************/
node *
BLKPPassign (node *arg_node, node *arg_info)
{
    node *let;
    node *ap;
    node *fundef;
    statustype state;
    int embedding; /* bool */

    DBUG_ENTER ("BLKPPassign");

    DBUG_PRINT ("BLKPP", ("begin, instr is a %s", NODE_TEXT (ASSIGN_INSTR (arg_node))));

    let = ASSIGN_INSTR (arg_node);
    if (NODE_TYPE (let) == N_let) {
        DBUG_PRINT ("BLKIN", ("instr is %s", NODE_TEXT (LET_EXPR (let))));
        ap = LET_EXPR (let);
        if (NODE_TYPE (ap) == N_ap) {
            fundef = AP_FUNDEF (ap);
            if (fundef != NULL) {
                DBUG_PRINT ("BLKPP", ("hit"));
                state = FUNDEF_STATUS (fundef);
                DBUG_PRINT ("BLKPP", ("hit"));

                if (FUNDEF_COMPANION (fundef) != NULL) {
                    embedding = (NODE_TYPE (FUNDEF_COMPANION (fundef)) == N_mt);
                } else {
                    embedding = FALSE;
                }
                DBUG_PRINT ("BLKIN", ("embedding is %i", embedding));
                DBUG_PRINT ("BLKPP", ("found %s", mdb_statustype[state]));

                if (state == ST_dofun) {
                    if (embedding) {
                        arg_node = InsertMT (arg_node, arg_info);
                    }
                } else if (state == ST_whilefun) {
                    if (embedding) {
                        arg_node = InsertMT (arg_node, arg_info);
                    }
                } else {
                    DBUG_PRINT ("BLKPP", ("ignoring %s", mdb_statustype[state]));
                }
            }
        }
    } else if (NODE_TYPE (let) == N_cond) {
        DBUG_PRINT ("BLKPP", ("trav into cond"));
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
        DBUG_PRINT ("BLKPP", ("trav from cond"));
    }

    if (ASSIGN_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("BLKPP", ("trav info next"));
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
        DBUG_PRINT ("BLKPP", ("trav from next"));
    }

    DBUG_PRINT ("BLKPP", ("end"));
    DBUG_RETURN (arg_node);
}
