/*
 *
 * $Log$
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

/* intro comment missing #### */

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
BlocksCons (node *arg_node, node *arg_info)
{
    statustype old_attrib;
    funtab *old_tab;

    DBUG_ENTER ("BlocksCons");
    DBUG_PRINT ("BLKCO", ("begin"));

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef), ("wrong type of arg_node"));

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

    DBUG_PRINT ("BLKCO", ("begin"));
    DBUG_RETURN (arg_node);
}

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

node *
BLKCOfundef (node *arg_node, node *arg_info)
{
    statustype old_attrib;

    DBUG_ENTER ("BLKCOfundef");
    DBUG_PRINT ("BLKCO", ("begin"));

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef), ("wrong type of arg_node"));

    if ((FUNDEF_ATTRIB (arg_node) == ST_call_mt)
        || (FUNDEF_ATTRIB (arg_node) == ST_call_st)) {
        old_attrib = INFO_BLKCO_CURRENTATTRIB (arg_info);
        INFO_BLKCO_CURRENTATTRIB (arg_info) = FUNDEF_ATTRIB (arg_node);

        DBUG_PRINT ("BLKCO", ("traverse into body with %s",
                              mdb_statustype[FUNDEF_ATTRIB (arg_node)]));
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
        DBUG_PRINT ("BLKCO", ("traverse from body with %s",
                              mdb_statustype[FUNDEF_ATTRIB (arg_node)]));

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
