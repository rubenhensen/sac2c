/*
 *
 * $Log$
 * Revision 3.14  2004/11/25 14:25:09  khf
 * SacDevCamp04
 *
 * Revision 3.13  2004/11/24 19:29:17  skt
 * Compiler Switch during SACDevCampDK 2k4
 *
 * Revision 3.12  2004/11/23 23:09:06  skt
 * codebrushing during SACDevCampDK 2k4
 *
 * Revision 3.11  2004/11/21 17:32:02  skt
 * make it runable with the new info structure
 *
 * Revision 3.10  2004/09/28 16:33:12  ktr
 * cleaned up concurrent (removed everything not working / not working with emm)
 *
 * Revision 3.9  2004/09/18 16:01:04  ktr
 * Moved old multithread to phase 21 and added spmdemm traversal.
 * Although EMM and old MT work for genarray-wls, further work is needed
 * for fold-wls.
 *
 * Revision 3.8  2004/03/26 14:36:23  khf
 * OPT_MTO deaktivated
 *
 * Revision 3.7  2004/03/02 16:51:22  mwe
 * OPT_SBE deactivated
 *
 * Revision 3.6  2001/05/17 11:41:33  dkr
 * FREE replaced by FreeTree
 *
 * Revision 3.5  2001/01/25 10:18:01  dkr
 * PH_spmdregions renamed into PH_multithread
 *
 * Revision 3.4  2000/12/15 18:24:54  dkr
 * infer_dfms.h renamed into InferDFMs.h
 *
 * Revision 3.3  2000/12/15 10:43:06  dkr
 * signature of InferDFMs() modified
 *
 * Revision 3.2  2000/12/12 12:13:15  dkr
 * call of InferDFMs added
 *
 * Revision 3.1  2000/11/20 18:02:24  sacbase
 * new release made
 *
 * Revision 2.11  2000/01/21 13:22:44  jhs
 * Added some comments ...
 *
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
#include "Error.h"
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
    trav_t traversaltable;
    info *arg_info;

    DBUG_ENTER ("CONCdoConcurrent");

    syntax_tree = INFDFMSdoInferDfms (syntax_tree, HIDE_LOCALS_NEVER);

    arg_info = MakeInfo ();

    TRAVpush (TR_conc);

    syntax_tree = TRAVdo (syntax_tree, arg_info);

    traversaltable = TRAVpop ();
    DBUG_ASSERT ((traversaltable == TR_conc), "Popped incorrect traversal table");

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
        NOTE (("  (Inferred) maximum folds per sync-block is set to %i",
               global.needed_sync_fold));
    } else {
        NOTE (("  Maximum folds per sync-block is set to %i", global.max_sync_fold));
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
    trav_t traversaltable;

    DBUG_ENTER ("CONCfundef");

    INFO_CONC_FUNDEF (arg_info) = arg_node;

    if ((FUNDEF_BODY (arg_node) != NULL) && FUNDEF_ISFOLDFUN (arg_node)) {
        if (!FUNDEF_ISSPMDFUN (arg_node)) {

            /*
             * First, spmd-blocks are built around with-loops.
             */
            TRAVpush (TR_spmdi);

            DBUG_PRINT ("CONC", ("--- begin a SPMDI traversal ---"));
            DBUG_PRINT ("SPMDI", ("--- begin a SPMDI traversal ---"));
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
            DBUG_PRINT ("SPMDI", ("--- end a SPMDI traversal ---"));
            DBUG_PRINT ("CONC", ("--- end a SPMDI traversal ---"));

            traversaltable = TRAVpop ();
            DBUG_ASSERT ((traversaltable == TR_spmdi),
                         "Popped incorrect traversal table");

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
            TRAVpush (TR_spmdl);

            INFO_SPMDL_MT (arg_info) = 0;
            DBUG_PRINT ("CONC", ("--- begin a SPMDL (mt = 0) traversal ---"));
            DBUG_PRINT ("SPMDL", ("--- begin a SPMDL (mt = 0) traversal ---"));
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
            DBUG_PRINT ("SPMDL", ("--- end a SPMDL (mt = 0) traversal ---"));
            DBUG_PRINT ("CONC", ("--- end a SPMDL (mt = 0) traversal ---"));

            traversaltable = TRAVpop ();
            DBUG_ASSERT ((traversaltable == TR_spmdl),
                         "Popped incorrect traversal table");

            if ((global.break_after == PH_multithread_finish)
                && (0 == strcmp ("spmdlift", global.break_specifier))) {
                goto cont;
            }

            /*
             * scheduling specifications outside from spmd-functions are removed.
             */
            TRAVpush (TR_sched);

            DBUG_PRINT ("CONC", ("--- begin a SCHED traversal ---"));
            DBUG_PRINT ("SCHED", ("--- begin a SCHED traversal ---"));
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), NULL);
            DBUG_PRINT ("SCHED", ("--- end a SCHED traversal ---"));
            DBUG_PRINT ("CONC", ("--- end a SCHED traversal ---"));

            traversaltable = TRAVpop ();
            DBUG_ASSERT ((traversaltable == TR_sched),
                         "Popped incorrect traversal table");
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
            TRAVpush (TR_spmdl);

            INFO_SPMDL_MT (arg_info) = 1;
            DBUG_PRINT ("CONC", ("--- begin a SPMDL (mt = 1) traversal ---"));
            DBUG_PRINT ("SPMDL", ("--- begin a SPMDL (mt = 1) traversal ---"));
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
            DBUG_PRINT ("SPMDL", ("--- end a SPMDL (mt = 1) traversal ---"));
            DBUG_PRINT ("CONC", ("--- end a SPMDL (mt = 1) traversal ---"));

            traversaltable = TRAVpop ();
            DBUG_ASSERT ((traversaltable == TR_spmdi),
                         "Popped incorrect traversal table");

            if ((global.break_after == PH_multithread_finish)
                && (0 == strcmp ("spmdlift", global.break_specifier))) {
                goto cont;
            }

            /*
             * Fourth, synchronisation blocks are built around each assignment within
             * the body of an spmd-function.
             */
            TRAVpush (TR_synci);

            INFO_SYNCI_FIRST (arg_info) = 1;
            INFO_SYNCI_LAST (arg_info) = 1;
            DBUG_PRINT ("CONC", ("--- begin a SYNCI traversal ---"));
            DBUG_PRINT ("SYNCI", ("--- begin a SYNCI traversal ---"));
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
            DBUG_PRINT ("SYNCI", ("--- end a SYNCI traversal ---"));
            DBUG_PRINT ("CONC", ("--- end a SYNCI traversal ---"));

            traversaltable = TRAVpop ();
            DBUG_ASSERT ((traversaltable == TR_synci),
                         "Popped incorrect traversal table");

            if ((global.break_after == PH_multithread_finish)
                && (0 == strcmp ("syncinit", global.break_specifier))) {
                goto cont;
            }

            /*
             * synchronisation blocks are optimized, i.e. two or several adjacent
             * synchronisation blocks are combined into a single larger one.
             */
            TRAVpush (TR_synco);

            DBUG_PRINT ("CONC", ("--- begin a SYNCO traversal ---"));
            DBUG_PRINT ("SYNCO", ("--- begin a SYNCO traversal ---"));
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
            DBUG_PRINT ("SYNCO", ("--- end a SYNCO traversal ---"));
            DBUG_PRINT ("CONC", ("--- end a SYNCO traversal ---"));

            traversaltable = TRAVpop ();
            DBUG_ASSERT ((traversaltable == TR_synco),
                         "Popped incorrect traversal table");

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
            TRAVpush (TR_sched);

            DBUG_PRINT ("CONC", ("--- begin a SCHED traversal ---"));
            DBUG_PRINT ("SCHED", ("--- begin a SCHED traversal ---"));
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
            DBUG_PRINT ("SCHED", ("--- end a SCHED traversal ---"));
            DBUG_PRINT ("CONC", ("--- end a SCHED traversal ---"));

            traversaltable = TRAVpop ();
            DBUG_ASSERT ((traversaltable == TR_sched),
                         "Popped incorrect traversal table");
        }

        /*
         * The compilation continues with the next function definition.
         */
    }

cont:

    if (FUNDEF_NEXT (arg_node) != NULL) {
        TRAVpush (TR_conc);

        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);

        traversaltable = TRAVpop ();
        DBUG_ASSERT ((traversaltable == TR_conc), "Popped incorrect traversal table");
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
