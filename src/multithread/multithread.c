/*
 *
 * $Log$
 * Revision 1.8  2000/02/04 14:44:24  jhs
 * Added repfuns-traversel.
 *
 * Revision 1.7  2000/02/02 12:28:18  jhs
 *  Added INFO_MUTH_FUNDEF, improved BLKIN.
 *
 * Revision 1.6  2000/01/28 13:50:16  jhs
 * blocks_init added.
 *
 * Revision 1.5  2000/01/26 17:25:24  dkr
 * type of traverse-function-table changed.
 *
 * Revision 1.4  2000/01/24 18:24:21  jhs
 * Added some infrastructure ...
 *
 * Revision 1.3  2000/01/21 14:28:09  jhs
 * Added MUTHmodul and MUTHfundef.
 *
 * Revision 1.2  2000/01/21 13:10:12  jhs
 * Added infrastructure for new mt support
 *
 * Revision 1.1  2000/01/21 11:11:38  jhs
 * Initial revision
 *
 *
 */

/******************************************************************************
 *
 * file:        multithread.c
 *
 * prefix:      MUTH
 *
 * description:
 *   This file initiates and guides the compilation process of the new
 *   multithread-support.
 *   ... The entire process is still under development ...
 *
 ******************************************************************************/

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "globals.h"
#include "free.h"
#include "Error.h"

#include "schedule_init.h"
#include "repfuns_init.h"
#include "blocks_init.h"

/******************************************************************************
 *
 * function:
 *   node *BuildMultiThread( node *syntax_tree)
 *
 * description:
 *   This function starts the process of building the *new* support for
 *   multithread. Throughout this process arg_info points to an N_info which
 *   is generated here.
 *
 ******************************************************************************/
node *
BuildMultiThread (node *syntax_tree)
{
    funtab *old_tab;

    node *arg_info;

    DBUG_ENTER ("BuildMultiThread");

    arg_info = MakeInfo ();

    old_tab = act_tab;
    act_tab = muth_tab;

    syntax_tree = Trav (syntax_tree, arg_info);

    act_tab = old_tab;

    FREE (arg_info);

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * function:
 *   node *MUTHmodul(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   muth_tab traversal function for N_fundef node.
 *
 *   This function assures that only function definitions are traversed
 *   during the process of exploiting concurrency.
 *
 ******************************************************************************/
node *
MUTHmodul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("MUTHmodul");

    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *MUTHfundef(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   muth_tab traversal function for N_fundef node.
 *
 *   This function traverses the function definitions and controls the
 *   entire ... step process of exploiting concurrency:
 *     - PHASE 1 - Scheduling-Inference (schedule_init.[ch])
 *       Decides which with-loops will be executed multithreaded an
 *       which not:
 *       > At each with-loop *to be* executed multithreaded a scheduling is
 *         annotated, schedulings annotated by pragmas are also considered,
 *         if annotated scheduling is not suitable error messages occur
 *       > At each with-loop *not to be* executed multithreaded no scheduling
 *         is annotated, warnings will be displayed if schedulings
 *         are annotatated at such with-loops by pragmas
 *     - PHASE 2 - Creation of REPfunctions
 *          #### to be done  ...
 *     - PHASE 3 - Creation of MT- and ST-Blocks
 *       ####
 *       > Creates a MT-Block around each assigment to be executed
 *         multithreaded, these are only the with-loops with schedulings
 *         annotated in Phase 1.
 *       > Creates a ST Block around each assigment to be executed
 *         singlethreaded, because it is not allowed to execute it
 *         multithreaded
 *         - usage of class-function ####
 *         - application of (primitive) function with unknown body, returning
 *           an array result > threshold ####
 *         - assignments of an array-constant > threshold ####
 *         - ???? application of a known function, with st-block before mt-block,
 *                including loopi- and condi-functions resp. loops an conditionals
 *           ####
 *       > Traverses also with-loops without scheduling ####
 *     - ...
 *        .
 *        .     #### to be done
 *        .
 *     - ...
 *
 ******************************************************************************/
node *
MUTHfundef (node *arg_node, node *arg_info)
{
    node *old_fundef;

    DBUG_ENTER ("MUTHfundef");

    old_fundef = INFO_MUTH_FUNDEF (arg_info);
    INFO_MUTH_FUNDEF (arg_info) = arg_node;

    NOTE (("%s", FUNDEF_NAME (arg_node)));

    if ((FUNDEF_BODY (arg_node) != NULL) && (FUNDEF_STATUS (arg_node) != ST_foldfun)) {

        arg_node = ScheduleInit (arg_node, arg_info);

        arg_node = RepfunsInit (arg_node, arg_info);

        arg_node = BlocksInit (arg_node, arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    INFO_MUTH_FUNDEF (arg_info) = old_fundef;

    DBUG_RETURN (arg_node);
}
