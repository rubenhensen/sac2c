/*
 *
 * $Log$
 * Revision 1.4  2000/03/21 13:06:14  jhs
 * Brushing, Comments.
 *
 * Revision 1.3  2000/03/15 15:52:44  dkr
 * fixed a bug:
 *   MT_OR_ST_REGION on left hand side is replaced by L_MT_OR_ST_REGION
 *
 * Revision 1.2  2000/03/02 14:13:58  jhs
 * Using mdb_statustype now.
 *
 * Revision 1.1  2000/03/02 12:54:53  jhs
 * Initial revision
 *
 *
 * constructed by jhs@dArtagnan at home
 */

/******************************************************************************
 *
 * file:   block_cons.c
 *
 * prefix: BLKCO
 *
 * description:
 *   (BlocksConsolidation)
 *   Each function by now must be classified as call_rep, call_mt or call_st.
 *   I.e. FUNDEFF_ATTRIB in {ST_call_rep, ST_call_mt, ST_call_st}.
 *   This traversal ignores call_rep-funs.
 *   By this traversal are
 *   - N_mt-blocks deleted in call_mt-functions
 *   - N_st-blocks deleted in call_st-functions
 *   - N_mt's nested in another N_mt deleted.
 *   - N_st's nested in another N_st deleted.
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
 *   node *BlocksCons(node *arg_node, node *arg_info)
 *
 * description:
 *   Initiate the Consolidation as described above.
 *
 *   Traverses *only* the function handed over via arg_node with dfa_tab,
 *   will not traverse FUNDEF_NEXT( arg_node).
 *
 *   This routine ignores (returns without changes):
 *   - functions f with no body (FUNDEF_BODY( f) == NULL)
 *   - functions f with FUNDEF_STATUS( f) = ST_foldfun
 *   - repfuns
 *     functions f with FUNDEF_ATTRIB( f) = ST_call_rep
 *
 ******************************************************************************/
node *
BlocksCons (node *arg_node, node *arg_info)
{
    statustype old_attrib;
    funtab *old_tab;

    DBUG_ENTER ("BlocksCons");
    DBUG_PRINT ("BLKCO", ("begin"));

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef), ("wrong type of arg_node"));

    if ((FUNDEF_BODY (arg_node) != NULL) && (FUNDEF_STATUS (arg_node) != ST_foldfun)
        && (FUNDEF_ATTRIB (arg_node) != ST_call_rep)) {
        old_tab = act_tab;
        act_tab = blkco_tab;

        old_attrib = INFO_BLKCO_CURRENTATTRIB (arg_info);
        /*
         *  ST_call_any has to be overwritten by BLKCOfundef!!!
         *  This is to initialize only!!!
         */
        INFO_BLKCO_CURRENTATTRIB (arg_info) = ST_call_any;

        arg_node = Trav (arg_node, arg_info);

        INFO_BLKCO_CURRENTATTRIB (arg_info) = old_attrib;

        act_tab = old_tab;
    }

    DBUG_PRINT ("BLKCO", ("begin"));
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *BLKCOxt(node *arg_node, node *arg_info)
 *
 * description:
 *   This is the traversal function for N_st and N_mt!!!
 *   One does not need explicit versions BLKCOst or BLKCOmt here.
 *
 *   - If we find a N_mt while traversing ST_call_mt the N_mt is deleted.
 *   - If we find a N_st while traversing ST_call_st the N_st is deleted.
 *   - If we find a N_mt while traversing ST_call_st we swap the current
 *     attribute and traverse the region, deleting all nested blocks of
 *     same type, restoring current afterwards.
 *   - If we find a N_st while traversing ST_call_mt we swap the current
 *     attribute and traverse the region, deleting all nested blocks of
 *     same type, restoring current afterwards.
 *
 ******************************************************************************/
node *
BLKCOxt (node *arg_node, node *arg_info)
{
    statustype old_attrib;
    node *this_assign;
    node *last_assign;
    node *block;

    DBUG_ENTER ("BLKCOxt");
    DBUG_PRINT ("BLKCO", ("begin %s", NODE_TEXT (arg_node)));

    DBUG_ASSERT (((NODE_TYPE (arg_node) == N_st) || (NODE_TYPE (arg_node) == N_mt)),
                 ("wrong type of arg_node"));

    if (((INFO_BLKCO_CURRENTATTRIB (arg_info) == ST_call_mt)
         && (NODE_TYPE (arg_node) == N_mt))
        || ((INFO_BLKCO_CURRENTATTRIB (arg_info) == ST_call_st)
            && (NODE_TYPE (arg_node) == N_st))) {
        DBUG_PRINT ("BLKCO", ("delete"));
        /*
         *  Delete this block.
         *  this_assign is the assignment holding this N_mt/N_st.
         *  last_assign is the last assignment ot MT_OR_ST_REGION.
         */
        this_assign = INFO_BLKCO_THISASSIGN (arg_info);
        DBUG_PRINT ("BLKCO", ("hit"));
        last_assign = MUTHBlocksLastInstruction (MT_OR_ST_REGION (arg_node));

        DBUG_PRINT ("BLKCO", ("hit"));
        block = arg_node;
        arg_node = ASSIGN_INSTR (BLOCK_INSTR (MT_OR_ST_REGION (block)));
        ASSIGN_INSTR (BLOCK_INSTR (MT_OR_ST_REGION (block))) = NULL;
        ASSIGN_NEXT (last_assign) = ASSIGN_NEXT (this_assign);
        ASSIGN_NEXT (this_assign) = ASSIGN_NEXT (BLOCK_INSTR (MT_OR_ST_REGION (block)));
        ASSIGN_NEXT (BLOCK_INSTR (MT_OR_ST_REGION (block))) = NULL;
        block = FreeTree (block);

        arg_node = Trav (arg_node, arg_info);

    } else {
        DBUG_PRINT ("BLKCO", ("swap current attrib"));
        old_attrib = INFO_BLKCO_CURRENTATTRIB (arg_info);
        if (NODE_TYPE (arg_node) == N_st) {
            INFO_BLKCO_CURRENTATTRIB (arg_info) = ST_call_st;
        } else if (NODE_TYPE (arg_node) == N_mt) {
            INFO_BLKCO_CURRENTATTRIB (arg_info) = ST_call_mt;
        } else {
            DBUG_ASSERT (0, ("this cannot be ..."));
        }

        L_MT_OR_ST_REGION (arg_node, Trav (MT_OR_ST_REGION (arg_node), arg_info));

        INFO_BLKCO_CURRENTATTRIB (arg_info) = old_attrib;
    }

    DBUG_PRINT ("BLKCO", ("end"));
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *BLKCOfundef (node *arg_node, node *arg_info)
 *
 * description:
 *   Fetches the actual attribut from the function and stores it at the
 *   arg_node.
 *   DO NOT TRAVERSE FUNDEF_NEXT!!!
 *
 ******************************************************************************/
node *
BLKCOfundef (node *arg_node, node *arg_info)
{
    statustype old_attrib;

    DBUG_ENTER ("BLKCOfundef");
    DBUG_PRINT ("BLKCO", ("begin"));

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef), ("wrong type of arg_node"));

    if ((FUNDEF_ATTRIB (arg_node) == ST_call_mt)
        || (FUNDEF_ATTRIB (arg_node) == ST_call_st)) {
        /* push current attribute, fetch actual attribute from fundtion */
        old_attrib = INFO_BLKCO_CURRENTATTRIB (arg_info);
        INFO_BLKCO_CURRENTATTRIB (arg_info) = FUNDEF_ATTRIB (arg_node);

        DBUG_PRINT ("BLKCO", ("traverse into body with %s",
                              mdb_statustype[FUNDEF_ATTRIB (arg_node)]));
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
        DBUG_PRINT ("BLKCO", ("traverse from body with %s",
                              mdb_statustype[FUNDEF_ATTRIB (arg_node)]));

        /* pop attrib */
        INFO_BLKCO_CURRENTATTRIB (arg_info) = old_attrib;
    } else if (FUNDEF_ATTRIB (arg_node) == ST_call_rep) {
        /* ignore repfuns */
    } else if (FUNDEF_ATTRIB (arg_node) == ST_call_any) {
        /*
         *  all functions should be set to something != ST_call_any,
         *  by RFIN or MTFIN by now ...
         */
        DBUG_ASSERT (0, ("ST_call_any not allowed for here!!!"));
    }

    /* DO NOT TRAV INTO NEXT!!! */

    DBUG_PRINT ("BLKCO", ("end"));
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *BLKCOassign( node *arg_node, node *arg_info)
 *
 * description:
 *   pops INFO_BLKCO_THISASSIGN, stores new value there, traverses
 *   ASSIGN_INSTR and ASSIGN_NEXT, pop again
 *
 ******************************************************************************/
node *
BLKCOassign (node *arg_node, node *arg_info)
{
    node *old_assign;
    DBUG_ENTER ("BLKCOassign");
    DBUG_PRINT ("BLKCO", ("begin"));

    old_assign = INFO_BLKCO_THISASSIGN (arg_info);
    INFO_BLKCO_THISASSIGN (arg_info) = arg_node;

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    INFO_BLKCO_THISASSIGN (arg_info) = old_assign;

    DBUG_PRINT ("BLKCO", ("end"));
    DBUG_RETURN (arg_node);
}
