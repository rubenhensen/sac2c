/*****************************************************************************
 *
 * $Id$
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
 *    - EMM: Move alloc and dec_rc operations of local vars into spmd-blocks
 *    - optimizing/enlarging spmd-blocks
 *    - lifting spmd-blocks to spmd-functions
 *    - building synchronisation blocks
 *    - optimizing/enlarging synchronisation blocks
 *    - scheduling synchronisation blocks and with-loop segments
 *    - constraining spmd-blocks/spmd-functions
 *
 *   This process may be interruppted after each step by using one of the
 *   break specifiers "spmdinit", "syncemm", "spmdopt", "spmdlift",
 *   "syncinit", or "syncopt".
 *
 *****************************************************************************/

#include "dbug.h"
#include "tree_basic.h"
#include "traverse.h"
#include "ctinfo.h"
#include "InferDFMs.h"
#include "spmd_emm.h"
#include "concurrent_info.h"
#include "internal_lib.h"
#include <string.h>

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_CONC_FUNDEF (result) = NULL;
    INFO_SPMDL_MT (result) = 0;
    INFO_SYNCI_FIRST (result) = 0;
    INFO_SYNCI_LAST (result) = 0;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * function:
 *   node *CONCdoConcurrent(arg_node *syntax_tree)
 *
 * description:
 *
 *   This function starts the process of building spmd- and synchronisation
 *   blocks. Throughout this process arg_info points to an N_info node which
 *   is generated here.
 *
 *   *** This is the process building the old support for multithread. ***
 *
 ******************************************************************************/

node *
CONCdoConcurrent (node *syntax_tree)
{
    info *arg_info;

    DBUG_ENTER ("CONCdoConcurrent");

    syntax_tree = INFDFMSdoInferDfms (syntax_tree, HIDE_LOCALS_NEVER);

    arg_info = MakeInfo ();

    TRAVpush (TR_conc);
    syntax_tree = TRAVdo (syntax_tree, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * function:
 *   node *CONCmodule(node *arg_node, info *arg_info)
 *
 * description:
 *
 *   conc_tab traversal function for N_fundef node.
 *
 *   This function assures that only function definitions are traversed
 *   during the process of exploiting concurrency.
 *
 *****************************************************************************/

node *
CONCmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CONCmodule");

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    if (global.max_sync_fold == -1) {
        CTInote ("(Inferred) maximum folds per sync-block is set to %i",
                 global.needed_sync_fold);
    } else {
        CTInote ("Maximum folds per sync-block is set to %i", global.max_sync_fold);
    }
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CONCfundef(node *arg_node, info *arg_info)
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
 *****************************************************************************/
node *
CONCfundef (node *arg_node, info *arg_info)
{
    node *first_spmdfun, *last_spmdfun, *current_fun;

    DBUG_ENTER ("CONCfundef");

    INFO_CONC_FUNDEF (arg_info) = arg_node;

    if ((FUNDEF_BODY (arg_node) != NULL) && (!FUNDEF_ISFOLDFUN (arg_node))) {
        if (!FUNDEF_ISSPMDFUN (arg_node)) {

            /*
             * First, spmd-blocks are built around with-loops.
             */
            DBUG_PRINT ("CONC", ("--- begin a SPMDI traversal ---"));
            DBUG_PRINT ("SPMDI", ("--- begin a SPMDI traversal ---"));
            TRAVpush (TR_spmdi);
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
            TRAVpop ();
            DBUG_PRINT ("SPMDI", ("--- end a SPMDI traversal ---"));
            DBUG_PRINT ("CONC", ("--- end a SPMDI traversal ---"));

            if ((global.break_after == PH_multithread_finish)
                && (0 == strcmp ("spmdinit", global.break_specifier))) {
                goto cont;
            }

            /*
             * For EMM it is necessary to move alloc and dec_rc operations of
             * local variables into SPMD blocks
             */
            DBUG_PRINT ("CONC", ("--- begin a SPMDEMM traversal ---"));
            DBUG_PRINT ("SPMDEMM", ("--- begin a SPMDEMM traversal ---"));
            arg_node = SPMDEMMdoSpmdEmm (arg_node);
            DBUG_PRINT ("SPMDEMM", ("--- end a SPMDEMM traversal ---"));
            DBUG_PRINT ("CONC", ("--- end a SPMDEMM traversal ---"));

            if ((global.break_after == PH_multithread_finish)
                && (0 == strcmp ("spmdemm", global.break_specifier))) {
                goto cont;
            }

            /*
             * the contents of each spmd-block are copied into a separate function,
             * called spmd-function.
             */

            INFO_SPMDL_MT (arg_info) = 0;
            DBUG_PRINT ("CONC", ("--- begin a SPMDL (mt = 0) traversal ---"));
            DBUG_PRINT ("SPMDL", ("--- begin a SPMDL (mt = 0) traversal ---"));
            TRAVpush (TR_spmdl);
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
            TRAVpop ();
            DBUG_PRINT ("SPMDL", ("--- end a SPMDL (mt = 0) traversal ---"));
            DBUG_PRINT ("CONC", ("--- end a SPMDL (mt = 0) traversal ---"));

            if ((global.break_after == PH_multithread_finish)
                && (0 == strcmp ("spmdlift", global.break_specifier))) {
                goto cont;
            }

            /*
             * scheduling specifications outside from spmd-functions are removed.
             */
            DBUG_PRINT ("CONC", ("--- begin a SCHED traversal ---"));
            DBUG_PRINT ("SCHED", ("--- begin a SCHED traversal ---"));
            TRAVpush (TR_sched);
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), NULL);
            TRAVpop ();
            DBUG_PRINT ("SCHED", ("--- end a SCHED traversal ---"));
            DBUG_PRINT ("CONC", ("--- end a SCHED traversal ---"));

        } else {

            if ((global.break_after == PH_multithread_finish)
                && ((0 == strcmp ("spmdinit", global.break_specifier))
                    || (0 == strcmp ("spmdopt", global.break_specifier)))) {
                goto cont;
            }

            /*
             * Third, local back references within spmd-functions are adjusted, e.g.
             * references to identifer declarations or data flow masks.
             */
            INFO_SPMDL_MT (arg_info) = 1;
            DBUG_PRINT ("CONC", ("--- begin a SPMDL (mt = 1) traversal ---"));
            DBUG_PRINT ("SPMDL", ("--- begin a SPMDL (mt = 1) traversal ---"));
            TRAVpush (TR_spmdl);
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
            TRAVpop ();
            DBUG_PRINT ("SPMDL", ("--- end a SPMDL (mt = 1) traversal ---"));
            DBUG_PRINT ("CONC", ("--- end a SPMDL (mt = 1) traversal ---"));

            if ((global.break_after == PH_multithread_finish)
                && (0 == strcmp ("spmdlift", global.break_specifier))) {
                goto cont;
            }

            /*
             * Fourth, synchronisation blocks are built around each assignment within
             * the body of an spmd-function.
             */
            INFO_SYNCI_FIRST (arg_info) = 1;
            INFO_SYNCI_LAST (arg_info) = 1;
            DBUG_PRINT ("CONC", ("--- begin a SYNCI traversal ---"));
            DBUG_PRINT ("SYNCI", ("--- begin a SYNCI traversal ---"));
            TRAVpush (TR_synci);
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
            TRAVpop ();
            DBUG_PRINT ("SYNCI", ("--- end a SYNCI traversal ---"));
            DBUG_PRINT ("CONC", ("--- end a SYNCI traversal ---"));

            if ((global.break_after == PH_multithread_finish)
                && (0 == strcmp ("syncinit", global.break_specifier))) {
                goto cont;
            }

            /*
             * synchronisation blocks are optimized, i.e. two or several adjacent
             * synchronisation blocks are combined into a single larger one.
             */
            DBUG_PRINT ("CONC", ("--- begin a SYNCO traversal ---"));
            DBUG_PRINT ("SYNCO", ("--- begin a SYNCO traversal ---"));
            TRAVpush (TR_synco);
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
            TRAVpop ();
            DBUG_PRINT ("SYNCO", ("--- end a SYNCO traversal ---"));
            DBUG_PRINT ("CONC", ("--- end a SYNCO traversal ---"));

            if ((global.break_after == PH_multithread_finish)
                && (0 == strcmp ("syncopt", global.break_specifier))) {
                goto cont;
            }

            /*
             * each synchronisation block and each segment specification within are
             * given scheduling specifications.
             * These are either infered from the context or extracted from
             * wlcomp pragma information. Scheduling specifications
             * outside of the context of a synchronisation block are removed.
             */
            DBUG_PRINT ("CONC", ("--- begin a SCHED traversal ---"));
            DBUG_PRINT ("SCHED", ("--- begin a SCHED traversal ---"));
            TRAVpush (TR_sched);
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
            TRAVpop ();
            DBUG_PRINT ("SCHED", ("--- end a SCHED traversal ---"));
            DBUG_PRINT ("CONC", ("--- end a SCHED traversal ---"));
        }

        /*
         * The compilation continues with the next function definition.
         */
    }

cont:

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    /*
     *  On the back direction of the recursion we do some extra work:
     */
    if ((FUNDEF_BODY (arg_node) != NULL) && FUNDEF_ISFOLDFUN (arg_node)) {

        if ((global.break_after == PH_multithread_finish)
            && ((0 == strcmp ("spmdinit", global.break_specifier))
                || (0 == strcmp ("spmdemm", global.break_specifier))
                || (0 == strcmp ("spmdlift", global.break_specifier))
                || (0 == strcmp ("syncinit", global.break_specifier))
                || (0 == strcmp ("syncopt", global.break_specifier))
                || (0 == strcmp ("scheduling", global.break_specifier)))) {
        }
    }

    /*
     * For several purposes it is advantageous for the remaining compilation
     * process to have the spmd-functions stored in front of the original function
     * from which they have been lifted.
     * Therefore, the sequence of N_fundef nodes is reordered at the end of this
     * compiler phase.
     */

    if (!FUNDEF_ISSPMDFUN (arg_node) && (FUNDEF_NEXT (arg_node) != NULL)
        && (FUNDEF_ISSPMDFUN (FUNDEF_NEXT (arg_node)))
        && (FUNDEF_LIFTEDFROM (FUNDEF_NEXT (arg_node)) == arg_node)) {
        current_fun = arg_node;
        first_spmdfun = FUNDEF_NEXT (arg_node);
        last_spmdfun = first_spmdfun;

        while ((FUNDEF_NEXT (last_spmdfun) != NULL)
               && FUNDEF_ISSPMDFUN (FUNDEF_NEXT (last_spmdfun))
               && (FUNDEF_LIFTEDFROM (FUNDEF_NEXT (last_spmdfun)) == arg_node)) {
            last_spmdfun = FUNDEF_NEXT (last_spmdfun);
        }

        arg_node = first_spmdfun;
        FUNDEF_NEXT (current_fun) = FUNDEF_NEXT (last_spmdfun);
        FUNDEF_NEXT (last_spmdfun) = current_fun;
    }

    DBUG_RETURN (arg_node);
}
