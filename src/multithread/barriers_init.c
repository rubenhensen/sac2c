/*
 *
 * $Log$
 * Revision 1.1  2000/03/22 17:29:43  jhs
 * Initial revision
 *
 *
 */

/******************************************************************************
 *
 * file:   barriers_init.c
 *
 * prefix: BARIN
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
#include "DataFlowMask.h"

#include "internal_lib.h"
#include "multithread_lib.h"

/* #### */
node *
BarriersInit (node *arg_node, node *arg_info)
{
    funtab *old_tab;

    DBUG_ENTER ("BarriersInit");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef), ("wrong type of arg_node"));

    if ((FUNDEF_BODY (arg_node) != NULL) && (FUNDEF_STATUS (arg_node) != ST_foldfun)
        && (FUNDEF_ATTRIB (arg_node) != ST_call_rep)) {
        old_tab = act_tab;
        act_tab = barin_tab;

        arg_node = Trav (arg_node, arg_info);

        act_tab = old_tab;
    }

    DBUG_RETURN (arg_node);
}

/* #### */
node *
BARINassign (node *arg_node, node *arg_info)
{
    node *instr;
    node *before;
    node *behind;
    node *assign;
    node *rhs;
    node *signal;
    node *alloc;
    node *sync;

    DBUG_ENTER ("BARINassign");

    instr = ASSIGN_INSTR (arg_node);

    if (NODE_TYPE (instr) == N_let) {
        rhs = LET_EXPR (instr);

        if (NODE_TYPE (rhs) == N_Nwith2) {
            if (NWITH2_ISSCHEDULED (rhs)) {

                DBUG_PRINT ("BARIN", ("wl scheduled"));

                alloc = MakeMTalloc ();
                MTALLOC_IDSET (alloc) = DFMGenMaskCopy (LET_DEFMASK (instr));

                signal = MakeMTsignal ();
                MTSIGNAL_IDSET (signal) = DFMGenMaskCopy (LET_DEFMASK (instr));

                sync = MakeMTsync ();
                MTSYNC_WAIT (sync) = DFMGenMaskCopy (LET_DEFMASK (instr));

                before = MakeAssign (alloc, arg_node);
                behind = MakeAssign (signal, MakeAssign (sync, ASSIGN_NEXT (arg_node)));
                ASSIGN_NEXT (arg_node) = behind;

                assign = before;

            } else {
                DBUG_PRINT ("BARIN", ("wl not scheduled"));
                assign = arg_node;
            }
        } else {
            DBUG_PRINT ("BARIN", ("not a wl but %s", NODE_TEXT (instr)));
            assign = arg_node;
            /* nothing do be done */
        }

    } else if (NODE_TYPE (instr) == N_mt) {
        DBUG_PRINT ("BARIN", ("trav into %s", NODE_TEXT (instr)));
        instr = Trav (instr, arg_info);
        DBUG_PRINT ("BARIN", ("trav from %s", NODE_TEXT (instr)));
        assign = arg_node;
    } else {
        DBUG_PRINT ("BARIN", ("not a let but %s", NODE_TEXT (instr)));
        assign = arg_node;
    }

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (assign);
}
