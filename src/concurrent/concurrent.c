/*
 *
 * $Log$
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
 *   of 6 consecutive steps:
 *    - building spmd-blocks
 *    - optimizing/enlarging spmd-blocks
 *    - lifting spmd-blocks to spmd-functions
 *    - building synchronisation blocks
 *    - optimizing/enlarging synchronisation blocks
 *    - scheduling synchronisation blocks and with-loop segments
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
 *   entire 6 step process of exploiting concurrency:
 *    - building spmd-blocks
 *    - optimizing/enlarging spmd-blocks
 *    - lifting spmd-blocks to spmd-functions
 *    - building synchronisation blocks
 *    - optimizing/enlarging synchronisation blocks
 *    - scheduling synchronisation blocks and with-loop segments
 *
 ******************************************************************************/

node *
CONCfundef (node *arg_node, node *arg_info)
{
    node *first_spmdfun, *last_spmdfun, *current_fun;

    DBUG_ENTER ("CONCfundef");

    INFO_SPMD_FUNDEF (arg_info) = arg_node;

    if ((FUNDEF_BODY (arg_node) != NULL) && (FUNDEF_STATUS (arg_node) != ST_foldfun)) {

        if (FUNDEF_STATUS (arg_node) != ST_spmdfun) {

            INFO_SPMD_MT (arg_info) = 0;

            /*
             * First, spmd-blocks are built around with-loops.
             */
            act_tab = spmdinit_tab;
            FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

            if ((break_after == PH_spmdregions)
                && (0 == strcmp ("spmdinit", break_specifier))) {
                goto cont;
            }

            /*
             * Second, spmd-blocks are optimized, i.e. two or several adjacent spmd-blocks
             * are combined into a single larger one.
             */
            act_tab = spmdopt_tab;
            FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

            if ((break_after == PH_spmdregions)
                && (0 == strcmp ("spmdopt", break_specifier))) {
                goto cont;
            }

            /*
             * Third, the contents of each spmd-block are copied into a separate function,
             * called spmd-function.
             */
            act_tab = spmdlift_tab;
            FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

            if ((break_after == PH_spmdregions)
                && (0 == strcmp ("spmdlift", break_specifier))) {
                goto cont;
            }

            /*
             * Sixth, scheduling specifications outside from spmd-functions are removed.
             */
            act_tab = sched_tab;
            FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), NULL);
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
            INFO_SPMD_MT (arg_info) = 1;

            act_tab = spmdlift_tab;
            FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

            if ((break_after == PH_spmdregions)
                && (0 == strcmp ("spmdlift", break_specifier))) {
                goto cont;
            }

            /*
             * Fourth, synchronisation blocks are built around each assignment within
             * the body of an spmd-function.
             */
            act_tab = syncinit_tab;
            INFO_SPMD_FIRST (arg_info) = 1;
            FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

            if ((break_after == PH_spmdregions)
                && (0 == strcmp ("syncinit", break_specifier))) {
                goto cont;
            }

            /*
             * Fifth, synchronisation blocks are optimized, i.e. two or several adjacent
             * synchronisation blocks are combined into a single larger one.
             */
            act_tab = syncopt_tab;
            FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

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
            FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
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
