/*
 *
 * $Log$
 * Revision 1.8  1998/04/24 17:17:11  dkr
 * renamed Spmd...() to SPMD...()
 * fixed a bug in SPMDAssign
 *
 * Revision 1.7  1998/04/24 13:28:03  dkr
 * fixed a bug with LOCAL
 *
 * Revision 1.6  1998/04/24 12:16:11  dkr
 * added SPMD_LOCAL
 *
 * Revision 1.5  1998/04/24 01:15:58  dkr
 * added N_sync
 *
 * Revision 1.4  1998/04/20 02:39:33  dkr
 * includes now tree.h
 *
 * Revision 1.3  1998/04/19 23:18:00  dkr
 * changed comments
 *
 * Revision 1.2  1998/04/17 19:16:20  dkr
 * lifting of spmd-fun is performed in precompile now
 *
 * Revision 1.1  1998/04/17 17:22:07  dkr
 * Initial revision
 *
 *
 */

#include "tree.h"
#include "free.h"

#include "internal_lib.h"
#include "traverse.h"

#include "optimize.h"
#include "DupTree.h"
#include "refcount.h"
#include "typecheck.h"
#include "dbug.h"

#include "spmdregions.h"

/******************************************************************************
 * init SPMD/sync-regions
 *
 */

/******************************************************************************
 *
 * function:
 *   node *SPMDFundef(node *arg_node, node *arg_info)
 *
 * description:
 *   fills 'INFO_SPMD_FUNDEF(info_node)' with the current fundef
 *    --- needed for creation of used/defined-masks in 'SPMDInitAssign'.
 *
 ******************************************************************************/

node *
SPMDInitFundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SPMDInitFundef");

    /*
     * save current fundef in INFO_SPMD_FUNDEF
     */
    INFO_SPMD_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    }
    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }
    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SPMDInitAssign(node *arg_node, node *arg_info)
 *
 * description:
 *   At the moment we simply generate a SPMD-region and sync-region for each
 *    first level with-loop.

 *   Then we store in SPMD_IN, SPMD_OUT, S the used/defined-masks
 *    (occurrences of vars which scope is not the whole SPMD region
 *    --- e.g. local vars of with-loops --- are already subtracted).
 *
 ******************************************************************************/

node *
SPMDInitAssign (node *arg_node, node *arg_info)
{
    node *spmd_let, *spmd, *sync, *fundefs, *vardec;
    ids *spmd_ids, *new_inout, *new_in, *last_in, *new_local, *last_local;
    int varno, i;

    DBUG_ENTER ("SPMDInitAssign");

    spmd_let = ASSIGN_INSTR (arg_node);

    /* contains the current assignment a with-loop?? */
    if ((NODE_TYPE (spmd_let) == N_let) && (NODE_TYPE (LET_EXPR (spmd_let)) == N_Nwith)) {

        /*
         * current assignment contains a with-loop
         *  -> create a SPMD/sync-region containing the current assignment only
         *      and insert it into the syntaxtree.
         */
        sync = MakeSync (MakeBlock (MakeAssign (spmd_let, NULL), NULL));
        spmd = MakeSPMD (MakeBlock (MakeAssign (sync, NULL), NULL));
        ASSIGN_INSTR (arg_node) = spmd;

        /*
         * generate masks (for IN, OUT, INOUT)
         *
         * we must start at the fundef node to get the right value for VARNO
         *  in GenerateMasks().
         */
        INFO_SPMD_FUNDEF (arg_info) = GenerateMasks (INFO_SPMD_FUNDEF (arg_info), NULL);

        fundefs = INFO_SPMD_FUNDEF (arg_info);
        varno = FUNDEF_VARNO (fundefs);

        /*
         * INOUT (for genarray/modarray with-loops),
         * OUT (for fold with-loops):
         *
         * the only inout/out var is the let-id (N_ids) of the spmd-region.
         */

        spmd_ids = LET_IDS (spmd_let);
        DBUG_ASSERT ((IDS_NEXT (spmd_ids) == NULL), "more than one let-ids found");
        DBUG_ASSERT (((SPMD_DEFVARS (spmd))[IDS_VARNO (spmd_ids)] > 0),
                     "wrong mask entry for let-ids");

        new_inout = DupOneIds (spmd_ids, NULL);

        if ((NWITH_TYPE (LET_EXPR (spmd_let)) == WO_genarray)
            || (NWITH_TYPE (LET_EXPR (spmd_let)) == WO_modarray)) {
            SPMD_INOUT (spmd) = new_inout;
            SYNC_INOUT (sync) = DupIds (new_inout, NULL);
        } else {
            SPMD_OUT (spmd) = new_inout;
        }

        /*
         * IN:
         *
         * In-arguments of the spmd-region are all vars that are used in the region
         *  (SPMD_USEDVARS), except for local vars.
         * The smpd-region so far contains one with-loop-assignment only. Therefore
         *  all vars that are defined in the assignment (ASSIGN_MASK(0)) are
         *  (in)out- or local vars.
         */

        for (i = 0; i < varno; i++) {

            if (((SPMD_USEDVARS (spmd))[i] > 0)
                && ((ASSIGN_MASK (arg_node, 0))[i] == 0)) {
                vardec = FindVardec (i, fundefs);

                if (NODE_TYPE (vardec) == N_vardec) {
                    new_in = MakeIds (StringCopy (VARDEC_NAME (vardec)), NULL,
                                      VARDEC_STATUS (vardec));
                    IDS_ATTRIB (new_in) = VARDEC_ATTRIB (vardec);
                } else {
                    DBUG_ASSERT ((NODE_TYPE (vardec) == N_arg), "wrong node type");
                    new_in = MakeIds (StringCopy (ARG_NAME (vardec)), NULL,
                                      ARG_STATUS (vardec));
                    IDS_ATTRIB (new_in) = ARG_ATTRIB (vardec);
                }
                IDS_VARDEC (new_in) = vardec;

                if (SPMD_IN (spmd) == NULL) {
                    SPMD_IN (spmd) = new_in;
                } else {
                    IDS_NEXT (last_in) = new_in;
                }
                last_in = new_in;
            }
        }

        /*
         * LOCAL:
         *
         * All vars that are defined in the assignment (ASSIGN_MASK(0)), except
         *  for the let-var (OUT/INOUT), are local.
         */

        for (i = 0; i < varno; i++) {

            if ((ASSIGN_MASK (arg_node, 0))[i] > 0) {
                vardec = FindVardec (i, fundefs);
                if (IDS_VARDEC (new_inout) != vardec) {
                    DBUG_ASSERT ((NODE_TYPE (vardec) == N_vardec),
                                 "local var is arg of a fun");

                    new_local = MakeIds (StringCopy (VARDEC_NAME (vardec)), NULL,
                                         VARDEC_STATUS (vardec));
                    IDS_ATTRIB (new_local) = VARDEC_ATTRIB (vardec);
                    IDS_VARDEC (new_local) = vardec;

                    if (SPMD_LOCAL (spmd) == NULL) {
                        SPMD_LOCAL (spmd) = new_local;
                    } else {
                        IDS_NEXT (last_local) = new_local;
                    }
                    last_local = new_local;
                }
            }
        }

        /*
         * we only traverse the following assignments to prevent nested
         *  spmd regions
         */
    } else {
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    }

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 * optimize SPMD/sync-regions
 *
 */

/* not yet implemented :( */

/******************************************************************************
 *
 * function:
 *   node *SpmdRegions( node *syntax_tree)
 *
 * description:
 *   In a first traversal we generate the initial SPMD- and sync-regions
 *   (Every with-loop-assignment is lifted into a SPMD- and sync-region):
 *     A = with ( ... )  ->  SPMD {
 *                             do {
 *                               A = with ( ... )
 *                             }
 *                           }
 *   In a second traversal we optimize the SPMD- and sync-regions.
 *   (not yet implemented :(
 *
 ******************************************************************************/

node *
SpmdRegions (node *syntax_tree)
{
    node *info;

    DBUG_ENTER ("SpmdRegions");

    info = MakeInfo ();

    act_tab = spmdinit_tab; /* first traversal */
    syntax_tree = Trav (syntax_tree, info);

    act_tab = spmdopt_tab; /* second traversal */
    syntax_tree = Trav (syntax_tree, info);

    FREE (info);

    DBUG_RETURN (syntax_tree);
}
