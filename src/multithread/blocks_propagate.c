/*
 *
 * $Log$
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
        DBUG_ASSERT (0, ("no trav table"));
        /*    act_tab = blkpp_tab; */

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
    int embedding;

    DBUG_ENTER ("BLKPPassign");

    embedding = 1;

    let = ASSIGN_INSTR (arg_node);
    if (NODE_TYPE (let) == N_let) {
        ap = LET_EXPR (arg_node);
        if (NODE_TYPE (ap) == N_ap) {
            fundef = AP_FUNDEF (ap);
            state = FUNDEF_STATUS (fundef);
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

    DBUG_RETURN (arg_node);
}
