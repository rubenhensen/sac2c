/*
 *
 * $Log$
 * Revision 2.10  1999/11/11 10:30:36  jhs
 * Added some DBUG_PRINTs to check which phases fail (if they do).
 *
 * Revision 2.9  1999/08/27 11:55:09  jhs
 * Added DBUG_PRINTS.
 *
 * Revision 2.8  1999/08/09 11:32:20  jhs
 * Cleaned up info-macros for concurrent-phase.
 *
 * Revision 2.7  1999/08/05 13:36:25  jhs
 * Added optimization of sequential assignments between spmd-blocks, main work
 * happens in spmdinit and ist steered by OPT_MTI (default now: off), some
 * traversals were needed and added in spmd_trav.
 *
 * Revision 2.6  1999/07/30 13:46:40  jhs
 * Brushed some Notes.
 *
 * Revision 2.5  1999/07/28 13:03:35  jhs
 * Added seventh phase: spmd-constraining.
 *
 * Revision 2.4  1999/07/21 16:30:27  jhs
 * needed_sync_fold introduced, max_sync_fold_adjusted.
 *
 * Revision 2.3  1999/07/01 13:01:02  jhs
 * Inserted handling of INFO_SYNCI_LAST.
 *
 * Revision 2.2  1999/05/26 14:32:23  jhs
 * Added options MTO and SBE for multi-thread optimsation and
 * synchronisation barrier elimination, both options are by
 * default disabled.
 *
 * Revision 2.1  1999/02/23 12:44:04  sacbase
 * new release made
 *
 * Revision 1.4  1999/02/15 15:13:02  cg
 * Reordering of functions corrected: Fodfuns remain in front
 * of N_fundef chain.
 *
 * Revision 1.3  1998/06/25 08:04:18  cg
 * sequence of fundefs will now be reordered so that spmd-functions
 * appear before the original functions they are lifted from.
 *
 * Revision 1.2  1998/06/23 12:56:32  cg
 * added handling of new attribute NWITH2_MT
 *
 * Revision 1.1  1998/06/18 14:35:53  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   concurrent.c
 *
 * prefix: CONC
 *
 * description:
 *
 *   This file initiates and guides the compilation process of building
 *   SPMD regions within the compiled SAC code. This entire process consists
 *   of 7 consecutive steps:
 *    - building spmd-blocks
 *    - optimizing/enlarging spmd-blocks
 *    - lifting spmd-blocks to spmd-functions
 *    - building synchronisation blocks
 *    - optimizing/enlarging synchronisation blocks
 *    - scheduling synchronisation blocks and with-loop segments
 *    - constraining spmd-blocks/spmd-functions
 *
 *   This process may be interruppted after each step by using one of the
 *   break specifiers "spmdinit", "spmdopt", "spmdlift", "syncinit", or
 *   "syncopt".
 *
 *****************************************************************************/

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "globals.h"
#include "free.h"
#include "Error.h"

/******************************************************************************
 *
 * function:
 *   node *BuildSpmdRegions(arg_node *syntax_tree)
 *
 * description:
 *
 *   This function starts the process of building spmd- and synchronisation
 *   blocks. Throughout this process arg_info points to an N_info node which
 *   is generated here.
 *
 ******************************************************************************/

node *
BuildSpmdRegions (node *syntax_tree)
{
    node *arg_info;

    DBUG_ENTER ("BuildSpmdRegions");

    arg_info = MakeInfo ();

    act_tab = conc_tab;

    syntax_tree = Trav (syntax_tree, arg_info);

    FREE (arg_info);

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * function:
 *   node *CONCmodul(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   conc_tab traversal function for N_fundef node.
 *
 *   This function assures that only function definitions are traversed
 *   during the process of exploiting concurrency.
 *
 ******************************************************************************/

node *
CONCmodul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CONCmodul");

    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }

    if (max_sync_fold == -1) {
        NOTE (
          ("  (Inferred) maximum folds per sync-block is set to %i", needed_sync_fold));
    } else {
        NOTE (("  Maximum folds per sync-block is set to %i", max_sync_fold));
    }
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CONCfundef(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   conc_tab traversal function for N_fundef node.
 *
 *   This function traverses the function definitions and controls the
 *   entire 7 step process of exploiting concurrency:
 *    - building spmd-blocks
 *    - optimizing/enlarging spmd-blocks
 *    - lifting spmd-blocks to spmd-functions
 *    - building synchronisation blocks
 *    - optimizing/enlarging synchronisation blocks
 *    - scheduling synchronisation blocks and with-loop segments
 *    - constraining spmd-blocks/spmd-functions
 *
 ******************************************************************************/

node *
CONCfundef (node *arg_node, node *arg_info)
{
    node *first_spmdfun, *last_spmdfun, *current_fun;

    DBUG_ENTER ("CONCfundef");

    INFO_CONC_FUNDEF (arg_info) = arg_node;

    if ((FUNDEF_BODY (arg_node) != NULL) && (FUNDEF_STATUS (arg_node) != ST_foldfun)) {

        if (FUNDEF_STATUS (arg_node) != ST_spmdfun) {

            /*
             * First, spmd-blocks are built around with-loops.
             */
            act_tab = spmdinit_tab;
            if (optimize & OPT_MTI) {
                INFO_SPMDI_LASTSPMD (arg_info) = -1;
                INFO_SPMDI_NEXTSPMD (arg_info) = -1;
                INFO_SPMDI_CONTEXT (arg_info) = N_fundef;
                INFO_SPMDI_EXPANDCONTEXT (arg_info) = TRUE;
                INFO_SPMDI_EXPANDSTEP (arg_info) = FALSE;
            }
            DBUG_PRINT ("CONC", ("--- begin a SPMDI traversal ---"));
            DBUG_PRINT ("SPMDI", ("--- begin a SPMDI traversal ---"));
            FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
            DBUG_PRINT ("SPMDI", ("--- end a SPMDI traversal ---"));
            DBUG_PRINT ("CONC", ("--- end a SPMDI traversal ---"));

            if ((break_after == PH_spmdregions)
                && (0 == strcmp ("spmdinit", break_specifier))) {
                goto cont;
            }

            /*
             * Second, spmd-blocks are optimized, i.e. two or several adjacent spmd-blocks
             * are combined into a single larger one.
             *
             * Executed only if optimisation MTO is set.
             */
            act_tab = spmdopt_tab;
            if (optimize & OPT_MTO) {
                DBUG_PRINT ("CONC", ("--- begin a SPMDO traversal ---"));
                DBUG_PRINT ("SPMDO", ("--- begin a SPMDO traversal ---"));
                FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
                DBUG_PRINT ("SPMDO", ("--- end a SPMDO traversal ---"));
                DBUG_PRINT ("CONC", ("--- end a SPMDO traversal ---"));
            } else {
                DBUG_PRINT ("CONC", ("--- no SPMDO traversal ---"));
                DBUG_PRINT ("SPMDO", ("--- no SPMDO traversal ---"));
            }

            if ((break_after == PH_spmdregions)
                && (0 == strcmp ("spmdopt", break_specifier))) {
                goto cont;
            }

            /*
             * Third, the contents of each spmd-block are copied into a separate function,
             * called spmd-function.
             */
            act_tab = spmdlift_tab;
            INFO_SPMDL_MT (arg_info) = 0;
            DBUG_PRINT ("CONC", ("--- begin a SPMDL (mt = 0) traversal ---"));
            DBUG_PRINT ("SPMDL", ("--- begin a SPMDL (mt = 0) traversal ---"));
            FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
            DBUG_PRINT ("SPMDL", ("--- end a SPMDL (mt = 0) traversal ---"));
            DBUG_PRINT ("CONC", ("--- end a SPMDL (mt = 0) traversal ---"));

            if ((break_after == PH_spmdregions)
                && (0 == strcmp ("spmdlift", break_specifier))) {
                goto cont;
            }

            /*
             * Sixth, scheduling specifications outside from spmd-functions are removed.
             */
            act_tab = sched_tab;
            DBUG_PRINT ("CONC", ("--- begin a SCHED traversal ---"));
            DBUG_PRINT ("SCHED", ("--- begin a SCHED traversal ---"));
            FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), NULL);
            DBUG_PRINT ("SCHED", ("--- end a SCHED traversal ---"));
            DBUG_PRINT ("CONC", ("--- end a SCHED traversal ---"));
        } else {

            if ((break_after == PH_spmdregions)
                && ((0 == strcmp ("spmdinit", break_specifier))
                    || (0 == strcmp ("spmdopt", break_specifier)))) {
                goto cont;
            }

            /*
             * Third, local back references within spmd-functions are adjusted, e.g.
             * references to identifer declarations or data flow masks.
             */

            act_tab = spmdlift_tab;
            INFO_SPMDL_MT (arg_info) = 1;
            DBUG_PRINT ("CONC", ("--- begin a SPMDL (mt = 1) traversal ---"));
            DBUG_PRINT ("SPMDL", ("--- begin a SPMDL (mt = 1) traversal ---"));
            FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
            DBUG_PRINT ("SPMDL", ("--- end a SPMDL (mt = 1) traversal ---"));
            DBUG_PRINT ("CONC", ("--- end a SPMDL (mt = 1) traversal ---"));

            if ((break_after == PH_spmdregions)
                && (0 == strcmp ("spmdlift", break_specifier))) {
                goto cont;
            }

            /*
             * Fourth, synchronisation blocks are built around each assignment within
             * the body of an spmd-function.
             */
            act_tab = syncinit_tab;
            INFO_SYNCI_FIRST (arg_info) = 1;
            INFO_SYNCI_LAST (arg_info) = 1;
            DBUG_PRINT ("CONC", ("--- begin a SYNCI traversal ---"));
            DBUG_PRINT ("SYNCI", ("--- begin a SYNCI traversal ---"));
            FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
            DBUG_PRINT ("SYNCI", ("--- end a SYNCI traversal ---"));
            DBUG_PRINT ("CONC", ("--- end a SYNCI traversal ---"));

            if ((break_after == PH_spmdregions)
                && (0 == strcmp ("syncinit", break_specifier))) {
                goto cont;
            }

            /*
             * Fifth, synchronisation blocks are optimized, i.e. two or several adjacent
             * synchronisation blocks are combined into a single larger one.
             *
             * Executed only if optimisation SBE is set.
             */
            act_tab = syncopt_tab;
            if (optimize & OPT_SBE) {
                DBUG_PRINT ("CONC", ("--- begin a SYNCO traversal ---"));
                DBUG_PRINT ("SYNCO", ("--- begin a SYNCO traversal ---"));
                FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
                DBUG_PRINT ("SYNCO", ("--- end a SYNCO traversal ---"));
                DBUG_PRINT ("CONC", ("--- end a SYNCO traversal ---"));
            } else {
                DBUG_PRINT ("CONC", ("--- no SYNCO traversal ---"));
                DBUG_PRINT ("SYNCO", ("--- no SYNCO traversal ---"));
            }

            if ((break_after == PH_spmdregions)
                && (0 == strcmp ("syncopt", break_specifier))) {
                goto cont;
            }

            /*
             * Sixth, each synchronisation block and each segment specification within are
             * given scheduling specifications. These are either infered from the context
             * or extracted from wlcomp pragma information. Scheduling specifications
             * outside of the context of a synchronisation block are removed.
             */
            act_tab = sched_tab;
            DBUG_PRINT ("CONC", ("--- begin a SCHED traversal ---"));
            DBUG_PRINT ("SCHED", ("--- begin a SCHED traversal ---"));
            FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
            DBUG_PRINT ("SCHED", ("--- end a SCHED traversal ---"));
            DBUG_PRINT ("CONC", ("--- end a SCHED traversal ---"));
        }

    cont:

        /*
         * The compilation continues with the next function definition.
         */

        act_tab = conc_tab;
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    /*
     *  On the back direction of the recursion we do some extra work:
     */
    if ((FUNDEF_BODY (arg_node) != NULL) && (FUNDEF_STATUS (arg_node) != ST_foldfun)) {

        if ((break_after == PH_spmdregions)
            && ((0 == strcmp ("spmdinit", break_specifier))
                || (0 == strcmp ("spmdopt", break_specifier))
                || (0 == strcmp ("spmdlift", break_specifier))
                || (0 == strcmp ("syncinit", break_specifier))
                || (0 == strcmp ("syncopt", break_specifier))
                || (0 == strcmp ("scheduling", break_specifier)))) {
            goto cont2;
        }

        if (FUNDEF_STATUS (arg_node) != ST_spmdfun) {
            /*
             *  Seventh, each spmd-block and the belonging spmd-function get
             *  additional in-parameters, to be able to pass values from extracted
             *  prolog icms to the spmd-function.
             */
            act_tab = spmdcons_tab;

            DBUG_PRINT ("CONC", ("--- begin a SPMDC traversal ---"));
            DBUG_PRINT ("SPMDC", ("--- begin a SPMDC traversal ---"));
            FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
            DBUG_PRINT ("SPMDC", ("--- end a SPMDC traversal ---"));
            DBUG_PRINT ("CONC", ("--- end a SPMDC traversal ---"));
            if ((break_after == PH_spmdregions)
                && (0 == strcmp ("spmdcons", break_specifier))) {
                goto cont2;
            }
        }

    cont2:

        act_tab = conc_tab;
    }

    /*
     * For several purposes it is advantageous for the remaining compilation
     * process to have the spmd-functions stored in front of the original function
     * from which they have been lifted.
     * Therefore, the sequence of N_fundef nodes is reordered at the end of this
     * compiler phase.
     */

    if ((FUNDEF_STATUS (arg_node) != ST_spmdfun) && (FUNDEF_NEXT (arg_node) != NULL)
        && (FUNDEF_STATUS (FUNDEF_NEXT (arg_node)) == ST_spmdfun)
        && (FUNDEF_LIFTEDFROM (FUNDEF_NEXT (arg_node)) == arg_node)) {
        current_fun = arg_node;
        first_spmdfun = FUNDEF_NEXT (arg_node);
        last_spmdfun = first_spmdfun;

        while ((FUNDEF_NEXT (last_spmdfun) != NULL)
               && (FUNDEF_STATUS (FUNDEF_NEXT (last_spmdfun)) == ST_spmdfun)
               && (FUNDEF_LIFTEDFROM (FUNDEF_NEXT (last_spmdfun)) == arg_node)) {
            last_spmdfun = FUNDEF_NEXT (last_spmdfun);
        }

        arg_node = first_spmdfun;
        FUNDEF_NEXT (current_fun) = FUNDEF_NEXT (last_spmdfun);
        FUNDEF_NEXT (last_spmdfun) = current_fun;
    }

    DBUG_RETURN (arg_node);
}
