/*
 *
 * $Log$
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
 *   node *SpmdFundef(node *arg_node, node *arg_info)
 *
 * description:
 *   fills 'INFO_SPMD_FUNDEF(info_node)' with the current fundef
 *    --- needed for creation of used/defined-masks in 'SpmdInitAssign'.
 *
 ******************************************************************************/

node *
SpmdInitFundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SpmdInitFundef");

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
 *   node *SpmdInitAssign(node *arg_node, node *arg_info)
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
SpmdInitAssign (node *arg_node, node *arg_info)
{
    ids *spmd_ids;
    node *spmd, *sync, *spmd_let, *fundefs, *vardec, *new_in, *last_in, *new_inout;
    int varno, i;

    DBUG_ENTER ("SpmdInitAssign");

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

        /****************************************************
         * begin computation SPMD_USEDVARS, SPMD_DEFVARS
         */

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
         * traverse the 'spmd_let' node to generate INFO_SPMD_LOCALUSED,
         *  INFO_SPMD_LOCALDEF
         */
        INFO_SPMD_LOCALUSED (arg_info)
          = ReGenMask (INFO_SPMD_LOCALUSED (arg_info), varno);
        INFO_SPMD_LOCALDEF (arg_info) = ReGenMask (INFO_SPMD_LOCALDEF (arg_info), varno);
        spmd_let = Trav (spmd_let, arg_info);

        /*
         * subtract the local vars from SPMD_...VARS
         *  because these vars will not become args or returns of the
         *  lifted spmd-fun.
         */
        MinusMask (SPMD_USEDVARS (spmd), INFO_SPMD_LOCALUSED (arg_info), varno);
        MinusMask (SPMD_DEFVARS (spmd), INFO_SPMD_LOCALDEF (arg_info), varno);

        /*
         * end computation SPMD_USEDVARS, SPMD_DEFVARS
         ****************************************************/

        /****************************************************
         * begin building of IN, OUT, INOUT
         */

        /*
         * INOUT (for genarray/modarray with-loops)
         * OUT (for fold with-loops)
         */

        spmd_ids = LET_IDS (spmd_let);
        DBUG_ASSERT ((IDS_NEXT (spmd_ids) == NULL), "more than one let-ids found");
        DBUG_ASSERT (((SPMD_DEFVARS (spmd))[IDS_VARNO (spmd_ids)] > 0),
                     "wrong mask entry for let-ids");

        new_inout = MakeArg (StringCopy (IDS_NAME (spmd_ids)),
                             DuplicateTypes (IDS_TYPE (spmd_ids), 1),
                             IDS_STATUS (spmd_ids), IDS_ATTRIB (spmd_ids), NULL);
        ARG_REFCNT (new_inout) = IDS_REFCNT (spmd_ids);
        ARG_VARNO (new_inout) = IDS_VARNO (spmd_ids);

        if ((NWITH_TYPE (LET_EXPR (spmd_let)) == WO_genarray)
            || (NWITH_TYPE (LET_EXPR (spmd_let)) == WO_modarray)) {
            SPMD_INOUT (spmd) = new_inout;
            SYNC_INOUT (sync) = DupTree (new_inout, NULL);
        } else {
            SPMD_OUT (spmd) = new_inout;
        }

        /*
         * IN
         */

        for (i = 0; i < varno; i++) {
            DBUG_ASSERT ((((SPMD_USEDVARS (spmd))[i] >= 0)
                          && ((SPMD_DEFVARS (spmd))[i] >= 0)),
                         "wrong mask entry found");

            if ((SPMD_USEDVARS (spmd))[i] > 0) {
                vardec = FindVardec (i, fundefs);

                if (NODE_TYPE (vardec) == N_vardec) {
                    new_in
                      = MakeArg (StringCopy (VARDEC_NAME (vardec)),
                                 DuplicateTypes (VARDEC_TYPE (vardec), 1),
                                 VARDEC_STATUS (vardec), VARDEC_ATTRIB (vardec), NULL);
                    ARG_REFCNT (new_in) = VARDEC_REFCNT (vardec);
                } else {
                    DBUG_ASSERT ((NODE_TYPE (vardec) == N_arg), "wrong node type");
                    new_in = DupNode (vardec);
                }
                ARG_VARNO (new_in) = i;

                if (SPMD_IN (spmd) == NULL) {
                    SPMD_IN (spmd) = new_in;
                } else {
                    ARG_NEXT (last_in) = new_in;
                }
                last_in = new_in;
            }
        }

        /*
         * end building of IN, OUT, INOUT
         ****************************************************/

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
 *
 * function:
 *   node *SpmdInitNpart( node *arg_node, node *arg_info)
 *
 * description:
 *   all defined vars of the N_Npart node are local
 *    -> save them in INFO_SPMD_LOCALDEF.
 *
 ******************************************************************************/

node *
SpmdInitNpart (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SpmdInitNpart");

    PlusMask (INFO_SPMD_LOCALDEF (arg_info), NPART_MASK (arg_node, 0),
              FUNDEF_VARNO (INFO_SPMD_FUNDEF (arg_info)));

    NPART_WITHID (arg_node) = Trav (NPART_WITHID (arg_node), arg_info);
    NPART_GEN (arg_node) = Trav (NPART_GEN (arg_node), arg_info);
    if (NPART_NEXT (arg_node) != NULL) {
        NPART_NEXT (arg_node) = Trav (NPART_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SpmdInitNcode( node *arg_node, node *arg_info)
 *
 * description:
 *   all defined vars of the N_Ncode node are local
 *    -> save them in INFO_SPMD_LOCALUSED, INFO_SPMD_LOCALDEF.
 *
 ******************************************************************************/

node *
SpmdInitNcode (node *arg_node, node *arg_info)
{
    int i;

    DBUG_ENTER ("SpmdInitNcode");

    for (i = 0; i < FUNDEF_VARNO (INFO_SPMD_FUNDEF (arg_info)); i++) {
        if ((NPART_MASK (arg_node, 0))[i] > 0) {
            /*
             * this var is defined in the with-loop-code
             *  -> local var -> count all defs and uses in INFO_SPMD_LOCAL...
             */
            (INFO_SPMD_LOCALUSED (arg_info))[i] += (NPART_MASK (arg_node, 1))[i];
            (INFO_SPMD_LOCALDEF (arg_info))[i] += (NPART_MASK (arg_node, 0))[i];
        }
    }

    if (NCODE_CBLOCK (arg_node) != NULL) {
        NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
    }
    if (NCODE_CEXPR (arg_node) != NULL) {
        NCODE_CEXPR (arg_node) = Trav (NCODE_CEXPR (arg_node), arg_info);
    }

    if (NCODE_NEXT (arg_node) != NULL) {
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
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
