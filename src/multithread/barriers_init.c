/*
 *
 * $Log$
 * Revision 1.2  2000/03/23 14:04:08  jhs
 * Added handling of fold-wl's, barriers only in mt-blocks and -functions.
 *
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
    int /* bool */ old_withinmt;

    DBUG_ENTER ("BarriersInit");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef), ("wrong type of arg_node"));

    if ((FUNDEF_BODY (arg_node) != NULL) && (FUNDEF_STATUS (arg_node) != ST_foldfun)
        && (FUNDEF_ATTRIB (arg_node) != ST_call_rep)) {
        old_tab = act_tab;
        act_tab = barin_tab;

        old_withinmt = INFO_BARIN_WITHINMT (arg_info);
        INFO_BARIN_WITHINMT (arg_info) = FALSE;

        arg_node = Trav (arg_node, arg_info);

        INFO_BARIN_WITHINMT (arg_info) = old_withinmt;

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
    char *name;

    DBUG_ENTER ("BARINassign");

    instr = ASSIGN_INSTR (arg_node);

    if (NODE_TYPE (instr) == N_let) {
        rhs = LET_EXPR (instr);

        if (NODE_TYPE (rhs) == N_Nwith2) {
            if ((INFO_BARIN_WITHINMT (arg_info)) /* && (NWITH2_ISSCHEDULED( rhs)) */) {

                if ((NWITHOP_TYPE (NWITH2_WITHOP (rhs)) == WO_genarray)
                    || (NWITHOP_TYPE (NWITH2_WITHOP (rhs)) == WO_modarray)) {
                    DBUG_PRINT ("BARIN", ("gen/mod wl scheduled"));

                    alloc = MakeMTalloc ();
                    MTALLOC_IDSET (alloc) = DFMGenMaskCopy (LET_DEFMASK (instr));

                    signal = MakeMTsignal ();
                    MTSIGNAL_IDSET (signal) = DFMGenMaskCopy (LET_DEFMASK (instr));

                    sync = MakeMTsync ();
                    MTSYNC_WAIT (sync) = DFMGenMaskCopy (LET_DEFMASK (instr));

                    before = MakeAssign (alloc, arg_node);
                    behind
                      = MakeAssign (signal, MakeAssign (sync, ASSIGN_NEXT (arg_node)));
                    ASSIGN_NEXT (arg_node) = behind;

                    assign = before;
                } else {
                    DBUG_PRINT ("BARIN", ("fold-wl scheduled"));

                    /* there can be one variable on the left side only! */
                    name = StringCopy (DFMGetMaskEntryNameSet (LET_DEFMASK (instr)));

                    signal = MakeMTsignal ();
                    MTSIGNAL_IDSET (signal) = DFMGenMaskCopy (LET_DEFMASK (instr));

                    sync = MakeMTsync ();
                    MTSYNC_WAIT (sync) = DFMGenMaskCopy (LET_DEFMASK (instr));
                    MTSYNC_FOLD (sync)
                      = MakeDFMfoldmask (name, NWITH2_WITHOP (rhs), NULL);

                    behind
                      = MakeAssign (signal, MakeAssign (sync, ASSIGN_NEXT (arg_node)));
                    ASSIGN_NEXT (arg_node) = behind;

                    assign = arg_node;
                }

            } else {
                DBUG_PRINT ("BARIN", ("not within mt or wl not scheduled"));
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

/* #### */
node *
BARINfundef (node *arg_node, node *arg_info)
{
    int old_withinmt; /* bool */

    DBUG_ENTER ("BARINfundef");

    old_withinmt = INFO_BARIN_WITHINMT (arg_info);
    if (FUNDEF_ATTRIB (arg_node) == ST_call_st) {
        INFO_BARIN_WITHINMT (arg_info) = TRUE;
    }

    FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

    INFO_BARIN_WITHINMT (arg_info) = old_withinmt;

    DBUG_RETURN (arg_node);
}

/* #### */
node *
BARINmt (node *arg_node, node *arg_info)
{
    int old_withinmt; /* bool */

    DBUG_ENTER ("BARINmt");

    old_withinmt = INFO_BARIN_WITHINMT (arg_info);
    INFO_BARIN_WITHINMT (arg_info) = TRUE;

    MT_REGION (arg_node) = Trav (MT_REGION (arg_node), arg_info);

    INFO_BARIN_WITHINMT (arg_info) = old_withinmt;

    DBUG_RETURN (arg_node);
}
