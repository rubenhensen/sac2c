/*
 *
 * $Log$
 * Revision 1.6  2000/03/15 15:50:19  dkr
 * fixed a bug:
 *   MT_OR_ST_REGION on left hand side is replaced by L_MT_OR_ST_REGION
 *
 * Revision 1.5  2000/03/02 12:56:37  jhs
 * Added comments.
 *
 * Revision 1.4  2000/02/23 13:28:22  jhs
 * Expansion stops above first block in a function now.
 *
 * Revision 1.3  2000/02/22 15:49:33  jhs
 * Melting now works for N_mt and N_st.
 *
 * Revision 1.2  2000/02/21 17:52:07  jhs
 * Expansion on N_mt's finished.
 *
 * Revision 1.1  2000/02/21 11:02:13  jhs
 * Initial revision
 *
 *
 */

/******************************************************************************
 *
 * file:   blocks_expand.c
 *
 * prefix: BLKEX
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
#include "my_debug.h"
#include "multithread_lib.h"
#include "free.h"

#include "internal_lib.h"

/******************************************************************************
 *
 * function:
 *   node *BlocksExpand(node *arg_node, node *arg_info)
 *
 * description:
 *   ####
 *
 ******************************************************************************/
node *
BlocksExpand (node *arg_node, node *arg_info)
{
    int old_blockabove; /* bool */
    funtab *old_tab;

    DBUG_ENTER ("BlocksExpand");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef),
                 "BlocksExpand expects a N_fundef as arg_node");

    old_tab = act_tab;
    act_tab = blkex_tab;

    /* push info ... */
    old_blockabove = INFO_BLKEX_BLOCKABOVE (arg_info);
    INFO_BLKEX_BLOCKABOVE (arg_info) = FALSE;

    FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

    /* pop info ... */
    INFO_BLKEX_BLOCKABOVE (arg_info) = old_blockabove;

    act_tab = old_tab;

    DBUG_RETURN (arg_node);
}

/* #### comment missing */
node *
BLKEXassign (node *arg_node, node *arg_info)
{
    node *this;
    node *this_instr;
    node *next;
    node *next_instr;
    int old_blockabove; /* bool */

    DBUG_ENTER ("BLKEXassign");

    /*
     *  push the block above information
     *  note the occurence of an N_mt or N_st
     */
    old_blockabove = INFO_BLKEX_BLOCKABOVE (arg_info);
    if ((NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_mt)
        || (NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_st)) {
        INFO_BLKEX_BLOCKABOVE (arg_info) = TRUE;
    }

    /* bottom-up-traversal!!! */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    /*
     *  pop the block above information
     */
    INFO_BLKEX_BLOCKABOVE (arg_info) = old_blockabove;

    this = arg_node;
    this_instr = ASSIGN_INSTR (arg_node);

    if (NODE_TYPE (this_instr) == N_let) {
        if (INFO_BLKEX_BLOCKABOVE (arg_info)) {
            if (ASSIGN_NEXT (arg_node) != NULL) {
                next = ASSIGN_NEXT (arg_node);
                next_instr = ASSIGN_INSTR (next);

                if ((NODE_TYPE (next_instr) == N_mt)
                    || (NODE_TYPE (next_instr) == N_st)) {
                    /* swap this assignment into the N_mt/N_st */
                    DBUG_PRINT ("BLKEX", ("swap into %s", NODE_TEXT (next_instr)));

                    ASSIGN_INSTR (this) = next_instr;
                    ASSIGN_NEXT (this) = ASSIGN_NEXT (next);

                    ASSIGN_INSTR (next) = this_instr;
                    ASSIGN_NEXT (next) = BLOCK_INSTR (MT_OR_ST_REGION (next_instr));
                    BLOCK_INSTR (MT_OR_ST_REGION (next_instr)) = next;
                }
            }
        }
    } else if ((NODE_TYPE (this_instr) == N_mt) || (NODE_TYPE (this_instr) == N_st)) {
        if (ASSIGN_NEXT (arg_node) != NULL) {
            next = ASSIGN_NEXT (arg_node);
            next_instr = ASSIGN_INSTR (next);

            /*
             *  this is a N_mt or N_st, if the next one is of the same type
             *  then both blocks will be melted.
             *  By the way: next could be something not N_mt or N_st while
             *  traversing the last part of a function behind the last (N_mt
             *  or N_st) and the return as end of the function.
             */
            if (NODE_TYPE (this_instr) == NODE_TYPE (next_instr)) {
                /* melt these blocks */
                DBUG_PRINT ("BLKEX", ("melt %s", NODE_TEXT (this_instr)));

                L_MT_OR_ST_REGION (this_instr,
                                   MUTHMeltBlocks (MT_OR_ST_REGION (this_instr),
                                                   MT_OR_ST_REGION (next_instr)));
                L_MT_OR_ST_REGION (next_instr, NULL);
                FreeTree (next_instr);

                ASSIGN_NEXT (this) = ASSIGN_NEXT (next);
                ASSIGN_NEXT (next) = NULL;
                ASSIGN_INSTR (next) = NULL;
                FreeTree (next);

            } else {
                /* nothing happens */
            }
        }
    } else if (NODE_TYPE (this_instr) == N_return) {
        /* nothing happens, returns are ignored */
    } else {
        DBUG_PRINT ("BLKEX", ("node_type: %s", NODE_TEXT (this_instr)));
        DBUG_ASSERT (0, ("unhandled type of ASSIGN_INSTR (watch BLKEX)"));
    }

    DBUG_RETURN (arg_node);
}
