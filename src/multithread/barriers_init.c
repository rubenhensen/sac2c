/*
 *
 * $Log$
 * Revision 1.5  2000/07/11 15:39:53  jhs
 * Added creation of MTSYNC_(FOLD|ALLOC).:w
 *
 * Revision 1.4  2000/04/14 17:43:26  jhs
 * Comments ...
 *
 * Revision 1.3  2000/04/10 15:45:35  jhs
 * Added BARINmt and BARINassign to traversal.
 *
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

/******************************************************************************
 *
 * function:
 *   node *BarriersInit(node *arg_node, node *arg_info)
 *
 * description:
 *   inits this traversal ... ####
 *
 ******************************************************************************/
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

/******************************************************************************
 *
 * function:
 *   node *BARINassign( node *arg_node, node *arg_info)
 *
 * description:
 *   This function looks big, but isn't (at least not until now, when I write
 *   this comment ...). Most parts are debugging and building of new nodes.
 *
 *   The function does the following things:
 *   - inspects lets
 *     - puts components of barriers around with-loops on rhs
 *       (see comments and action below).
 *     - ignores all other lets
 *   - traverses into N_mt
 *   - ignores everything else
 *
 * attention:
 *   THIS FUNCTION DOES NOT RETURN arg_node BUT assign!
 *   (see declaration of local variables).
 *
 ******************************************************************************/
node *
BARINassign (node *arg_node, node *arg_info)
{
    node *instr;  /* the instr at arg_node */
    node *before; /* nodes to be inserted before a wl */
    node *behind; /* nodes to be inserted behind a wl */
    node *assign; /*
                   * The result of this function, if components of barriers
                   * are added this one may be != argnode, otherwise it will
                   * be == arg_node.
                   * THIS FUNCTION DOES NOT RETURN arg_node BUT assign!
                   */
    node *rhs;    /* rhs of an let in the actual assign */
    node *signal; /* a new MTsignal */
    node *alloc;  /* a new MTalloc */
    node *sync;   /* a new MTsync */
    char *name;   /* the name of a variable containing a fold result */
    node *vardec; /* its vardec */

    DBUG_ENTER ("BARINassign");

    instr = ASSIGN_INSTR (arg_node);

    if (NODE_TYPE (instr) == N_let) {
        rhs = LET_EXPR (instr);

        if (NODE_TYPE (rhs) == N_Nwith2) {
            if ((INFO_BARIN_WITHINMT (arg_info)) /* && (NWITH2_ISSCHEDULED( rhs)) */) {

                if ((NWITHOP_TYPE (NWITH2_WITHOP (rhs)) == WO_genarray)
                    || (NWITHOP_TYPE (NWITH2_WITHOP (rhs)) == WO_modarray)) {
                    DBUG_PRINT ("BARIN", ("gen/mod wl scheduled"));

                    /*
                     *  We put a MTalloc before each gen/mod-wl.
                     *  We put a MTsignal and a MTsync after each gen/mod-wl.
                     */
                    alloc = MakeMTalloc ();
                    MTALLOC_IDSET (alloc) = DFMGenMaskCopy (LET_DEFMASK (instr));

                    signal = MakeMTsignal ();
                    MTSIGNAL_IDSET (signal) = DFMGenMaskCopy (LET_DEFMASK (instr));

                    sync = MakeMTsync ();
                    MTSYNC_WAIT (sync) = DFMGenMaskCopy (LET_DEFMASK (instr));
                    MTSYNC_FOLD (sync) = NULL;
                    MTSYNC_ALLOC (sync)
                      = DFMGenMaskClear (FUNDEF_DFM_BASE (INFO_MUTH_FUNDEF (arg_info)));

                    before = MakeAssign (alloc, arg_node);
                    behind
                      = MakeAssign (signal, MakeAssign (sync, ASSIGN_NEXT (arg_node)));
                    ASSIGN_NEXT (arg_node) = behind;

                    assign = before;
                } else if (NWITHOP_TYPE (NWITH2_WITHOP (rhs)) == WO_foldfun) {
                    DBUG_PRINT ("BARIN", ("fold-wl scheduled"));

                    /*
                     *  We put a MTsignal and a MTsync behind each fold-wl.
                     */

                    /* there can be one variable on the left side only! */
                    name = StringCopy (DFMGetMaskEntryNameSet (LET_DEFMASK (instr)));
                    vardec = DFMGetMaskEntryDeclSet (LET_DEFMASK (instr));

                    signal = MakeMTsignal ();
                    MTSIGNAL_IDSET (signal) = DFMGenMaskCopy (LET_DEFMASK (instr));

                    sync = MakeMTsync ();
                    MTSYNC_WAIT (sync) = DFMGenMaskCopy (LET_DEFMASK (instr));
                    MTSYNC_FOLD (sync)
                      = MakeDFMfoldmask (vardec, NWITH2_WITHOP (rhs), NULL);
                    MTSYNC_ALLOC (sync)
                      = DFMGenMaskClear (FUNDEF_DFM_BASE (INFO_MUTH_FUNDEF (arg_info)));

                    behind
                      = MakeAssign (signal, MakeAssign (sync, ASSIGN_NEXT (arg_node)));
                    ASSIGN_NEXT (arg_node) = behind;

                    assign = arg_node;
                } else {
                    DBUG_PRINT ("BARIN", ("unknown kind of withloop"));
                }
            } else {
                /* nothing do be done */
                DBUG_PRINT ("BARIN", ("not within mt or wl not scheduled"));
                assign = arg_node;
            }
        } else {
            /* nothing do be done */
            DBUG_PRINT ("BARIN", ("not a wl but %s", NODE_TEXT (instr)));
            assign = arg_node;
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

/******************************************************************************
 *
 * function:
 *   node *BARINfundef( node *arg_node, node *arg_info)
 *
 * description:
 *   Sets flag to show the traversal enters an mt-function, traverses the
 *   mt-function, and sets the flag back.
 *   The flag meant is INFO_BARIN_WITHINMT, used to show the traversal is
 *   in some mt-region (a mt-block or a mt-function).
 *
 ******************************************************************************/
node *
BARINfundef (node *arg_node, node *arg_info)
{
    int old_withinmt; /* bool */

    DBUG_ENTER ("BARINfundef");

    DBUG_PRINT ("BARIN", ("BARINfundef"));

    old_withinmt = INFO_BARIN_WITHINMT (arg_info);
    if (FUNDEF_ATTRIB (arg_node) == ST_call_mt_master) {
        INFO_BARIN_WITHINMT (arg_info) = TRUE;
    }

    FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

    INFO_BARIN_WITHINMT (arg_info) = old_withinmt;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *BARINmt( node *arg_node, node *arg_info)
 *
 * description:
 *   Sets flag to show the traversal enters an mt-block, traverses the
 *   mt-block, and sets the flag back.
 *   The flag meant is INFO_BARIN_WITHINMT, used to show the traversal is
 *   in some mt-region (a mt-block or a mt-function).
 *
 ******************************************************************************/
node *
BARINmt (node *arg_node, node *arg_info)
{
    int old_withinmt; /* bool */

    DBUG_ENTER ("BARINmt");

    DBUG_PRINT ("BARIN", ("BARINmt"));

    old_withinmt = INFO_BARIN_WITHINMT (arg_info);
    INFO_BARIN_WITHINMT (arg_info) = TRUE;

    MT_REGION (arg_node) = Trav (MT_REGION (arg_node), arg_info);

    INFO_BARIN_WITHINMT (arg_info) = old_withinmt;

    DBUG_RETURN (arg_node);
}
