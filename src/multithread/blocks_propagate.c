/*
 *
 * $Log$
 * Revision 3.2  2004/09/28 13:22:48  ktr
 * Removed generatemasks.
 *
 * Revision 3.1  2000/11/20 18:03:06  sacbase
 * new release made
 *
 * Revision 1.3  2000/07/13 08:25:25  jhs
 * Added comments.
 *
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
 *   While- and do-loops are represented as special functions while
 *   executing this phase. These pseudo-loops wil the encapsulated in
 *   mt-blocks (N_mt) if the first mt-block within the pseudo-loop
 *   is in front of any st-block (N_st) in this function. If there is no N_mt
 *   block the pseudo-loop will not be embedded.
 *
 ******************************************************************************/

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "scheduling.h"
#include "DupTree.h"
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
 *   Inits this traversal, see file comment for further information.
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
 *   functions representing a while- or do-loop (FUNDEF_STATUS == ST_whilefun
 *   or ST_dofun) and containing an N_mt before any N_st block in this
 *   function will be embedded within an N_mt.
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
        DBUG_PRINT ("BLKPP", ("instr is %s", NODE_TEXT (LET_EXPR (let))));
        ap = LET_EXPR (let);
        if (NODE_TYPE (ap) == N_ap) {
            fundef = AP_FUNDEF (ap);
            if (fundef != NULL) {
                DBUG_PRINT ("BLKPP", ("hit"));
                state = FUNDEF_STATUS (fundef);
                DBUG_PRINT ("BLKPP", ("hit"));

                /*
                 *  FUNDEF_COMPANION contains block information (not a fundef)
                 */
                if (FUNDEF_COMPANION (fundef) != NULL) {
                    embedding = (NODE_TYPE (FUNDEF_COMPANION (fundef)) == N_mt);
                } else {
                    embedding = FALSE;
                }
                DBUG_PRINT ("BLKPP", ("embedding is %i", embedding));
                DBUG_PRINT ("BLKPP", ("found %s", mdb_statustype[state]));

                if (state == ST_dofun) {
                    if (embedding) {
                        arg_node = MUTHInsertMT (arg_node, arg_info);
                    }
                } else if (state == ST_whilefun) {
                    if (embedding) {
                        arg_node = MUTHInsertMT (arg_node, arg_info);
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
