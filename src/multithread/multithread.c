/*
 *
 * $Log$
 * Revision 3.1  2000/11/20 18:03:10  sacbase
 * new release made
 *
 * Revision 1.19  2000/04/14 17:43:26  jhs
 * Comments ...
 *
 * Revision 1.18  2000/04/10 15:44:42  jhs
 * Fixed dbprint
 *
 * Revision 1.17  2000/03/30 15:11:47  jhs
 * added AdjustCalls
 *
 * Revision 1.16  2000/03/29 16:09:33  jhs
 * BlocksLift added.
 *
 * Revision 1.15  2000/03/22 17:31:16  jhs
 * Added BarriersInit.
 *
 * Revision 1.14  2000/03/21 16:11:19  jhs
 * Comments.
 *
 * Revision 1.13  2000/03/21 13:10:10  jhs
 * Added another CleanCOMPANION.
 * Comments.
 *
 * Revision 1.12  2000/03/09 18:34:22  jhs
 * Additional mini-phases.
 *
 * Revision 1.11  2000/03/02 12:58:36  jhs
 * Rearranged the traversal, each miniphase is apllied to all functions
 * before the next one is executed.
 *
 * Revision 1.10  2000/02/21 17:54:43  jhs
 * New mini-phase BLKEX.
 *
 * Revision 1.9  2000/02/11 16:21:01  jhs
 * Expanded traversals ...
 *
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
 *   The step of exploiting concurrency is done step by step in several
 *   miniphases. Each miniphase will be done on *all* functions before
 *   the next step is done.
 *
 *   - PHASE 1 - Scheduling-Inference (schedule_init.[ch])
 *     Decides which with-loops will be executed multithreaded an
 *     which not:
 *     > At each with-loop *to be* executed multithreaded a scheduling is
 *       annotated, schedulings annotated by pragmas are also considered,
 *       if annotated scheduling is not suitable error messages occur
 *     > At each with-loop *not to be* executed multithreaded no scheduling
 *       is annotated, warnings will be displayed if schedulings
 *       are annotatated at such with-loops by pragmas
 *   - PHASE 2 - Creation of REPfunctions
 *        #### to be done  ...
 *   - PHASE 3 - Creation of MT- and ST-Blocks
 *     ####
 *     > Creates a MT-Block around each assigment to be executed
 *       multithreaded, these are only the with-loops with schedulings
 *       annotated in Phase 1.
 *     > Creates a ST Block around each assigment to be executed
 *       singlethreaded, because it is not allowed to execute it
 *       multithreaded
 *       - usage of class-function ####
 *       - application of (primitive) function with unknown body, returning
 *         an array result > threshold ####
 *       - assignments of an array-constant > threshold ####
 *       - ???? application of a known function, with st-block before
 *         mt-block, including loopi- and condi-functions resp. loops and
 *         conditionals ####
 *     > Traverses also with-loops without scheduling ####
 *   - ...
 *      .
 *      .     #### to be done
 *      .
 *   - ...
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
#include "blocks_propagate.h"
#include "blocks_expand.h"
#include "mtfuns_init.h"
#include "blocks_cons.h"
#include "dataflow_analysis.h"
#include "barriers_init.h"
#include "blocks_lift.h"
#include "adjust_calls.h"

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

typedef int (*ignorefun) (node *arg_node, node *arg_info);

/******************************************************************************
 *
 * function:
 *   node *MUTHdriver(node *arg_node,
 *                    node *arg_info,
 *                    int allowOOOCs,  / * bool * /
 *                    int topdown, / * bool * /
 *                    funptr driver,
 *                    ignorefun ignore)
 *
 * description:
 *   This is not function of the traversal itself, but behaves like a
 *   normal traversal function.
 *
 *   - first driver is stored in INFO_MUTH_DRIVER and
 *           ignore           in INFO_MUTH_IGNORE,
 *   - then arg_node will be traversed,
 *   - and the arg_info is retored afterwards
 *
 *   While traversing the driver and ignore can be used to customize
 *   the actual traversal on the fly.
 *   Ignore says which nodes can be applied to the driver function,
 *   these node will be applied to the driver function then,
 *
 *   The *driver* function is a simple function that behaves like an
 *   ordinary traversal function, but can contain whole mini-phases or
 *   simple actions.
 *   The *ignore* function takes an arg_node and an arg_info and decides
 *   if the node should be ignored (Result == TRUE == 1) or not
 *   (Result == FALSE = 0).
 *
 *   This function is used to customize the traversal for these many many
 *   miniphases simply on-the-fly.
 *
 * attention:
 *   This function can handle OOOCs on demand, which are explained below at
 *   MUTHfundef.
 *
 ******************************************************************************/
node *
MUTHdriver_ (node *arg_node, node *arg_info, int allowOOOCs, /* bool */
             int topdown,                                    /* bool */
             funptr driver, ignorefun ignore)
{
    int old_allowOOOCs; /* bool */
    int old_topdown;    /* bool */
    funptr old_driver;
    ignorefun old_ignore;

    DBUG_ENTER ("MUTHdriver");

    if (arg_node != NULL) {
        DBUG_PRINT ("MUTH", ("begin driving"));
        old_driver = INFO_MUTH_DRIVER (arg_info);
        old_ignore = INFO_MUTH_IGNORE (arg_info);
        old_allowOOOCs = INFO_MUTH_ALLOW_OOOC (arg_info);
        old_topdown = INFO_MUTH_TOPDOWN (arg_info);
        INFO_MUTH_DRIVER (arg_info) = driver;
        INFO_MUTH_IGNORE (arg_info) = ignore;
        INFO_MUTH_ALLOW_OOOC (arg_info) = allowOOOCs;
        INFO_MUTH_TOPDOWN (arg_info) = topdown;

        /* OOOC is explained at MUTHfundef */
        if (INFO_MUTH_ALLOW_OOOC (arg_info)) {
            /* arg_node = */ Trav (arg_node, arg_info);
        } else {
            arg_node = Trav (arg_node, arg_info);
        }

        INFO_MUTH_DRIVER (arg_info) = old_driver;
        INFO_MUTH_IGNORE (arg_info) = old_ignore;
        INFO_MUTH_ALLOW_OOOC (arg_info) = old_allowOOOCs;
        INFO_MUTH_TOPDOWN (arg_info) = old_topdown;
        DBUG_PRINT ("MUTH", ("end driving"));
    }

    DBUG_RETURN (arg_node);
}

node *
MUTHdriver (node *arg_node, node *arg_info, int allowOOOCs, /* bool */
            funptr driver, ignorefun ignore)
{
    return (MUTHdriver_ (arg_node, arg_info, allowOOOCs, TRUE, driver, ignore));
}

/******************************************************************************
 *
 * function:
 *   node *MUTHfundef(node *arg_node, node *arg_info)
 *
 * description:
 *   This function
 *   - first stores the arg_node (or the actual N_fundef-node) in
 *     INFO_MUTH_FUNDEF,
 *   - then checks if the actual ignore-function INFO_MUTH_IGNORE
 *     says the actual fundef has to be applied by the
 *     actual driver-function INFO_MUTH_driver, or not,
 *   - and restores the old arg_info.
 *
 *   This function reuses information stored by MUTHdriver,
 *   MUTHdriver has to be called before the traversal reaches this function!
 *   On demand this function can handle OOOCs (see below).
 *
 * attention: --- OutOfOrderChanges (OOOCs) ---
 *   The action and perhaps the changes a traversal function applied to some
 *   node n of the syntax-tree done on part of the tree from n on, i.e. n
 *   and everything that can be reached by the sons of n.
 *   The whole travsersal started at some node s in the tree upwards (not
 *   necessary that s is the root of the tree). From s to n exists a path
 *   that is described exactly by the actual call stack, if some thing is
 *   changed in the tree structure in this path this is called an
 *   OutOfOrderChange (OOOC).
 *   - Example - Chain of functions:
 *     (a,b,c,d,e and x are some functions, n is the next attribute)
 *
 *     Before traversal ... a to e are chained x does not exist ...
 *     (1) a.n -> b
 *                b.n -> c
 *                       c.n -> d
 *                              d.n -> e
 *
 *     After the traversal we like to have this tree ...
 *     ... where x is inserted between b and c.
 *     And we know what x shall look like if we traverse e!
 *     (2) a.n -> b
 *                b.n -> x
 *                       x.n -> c
 *                              c.n -> d
 *                                     d.n -> e
 *
 *     We write a simple traversal-function ...
 *     ANYfundef (node arg_node, ...)
 *     {
 *       if (arg_node) == e) {
 *         x = new_node_by_e( e)
 *         x.n = b.n
 *         b.n = x;
 *       } else {
 *         FUNDEF_NEXT( arg_node) = Trav(FUNDEF_NEXT( arg_node), arg_info);
 *       }
 *       RETURN( arg_node);
 *     }
 *
 *     THIS WILL NOT DO WHAT WE WANT!!!
 *     The result locks (not is!) like in (1) not like in (2)!
 *
 *     Problem is when we traverse into e everything is ok, b.n == x.
 *     Then we go back up reaching c, the function returns c, and was called
 *     from b, so b.n is set to c again ... we get the tree like in (3)
 *     where x is unreachable ...
 *
 *     (3) a.n -> b
 *                b.n -> c
 *                x.n -> c
 *                       c.n -> d
 *                              d.n -> e
 *
 *     To do this traversal in this way we need to ignore the function result
 *     like in this traversal ...
 *     ANYfundef2 (node arg_node, ...)
 *     {
 *       if (arg_node) == e) {
 *         x = new_node_by_e( e)
 *         x.n = b.n
 *         b.n = x;
 *       } else {
 *         Trav(FUNDEF_NEXT( arg_node), arg_info);
 *       }
 *       RETURN( arg_node);
 *     }
 *
 ******************************************************************************/
node *
MUTHfundef (node *arg_node, node *arg_info)
{
    node *old_fundef;
    node *before;
    node *result;
    node *after;

    DBUG_ENTER ("MUTHfundef");

    old_fundef = INFO_MUTH_FUNDEF (arg_info);
    INFO_MUTH_FUNDEF (arg_info) = arg_node;

    if (INFO_MUTH_TOPDOWN (arg_info)) {
        if (!INFO_MUTH_IGNORE (arg_info) (arg_node, arg_info)) {
            DBUG_PRINT ("MUTH",
                        ("(top-down) into driver fun (%s)", FUNDEF_NAME (arg_node)));
            arg_node = INFO_MUTH_DRIVER (arg_info) (arg_node, arg_info);
            DBUG_PRINT ("MUTH",
                        ("(top-down) from driver fun (%s)", FUNDEF_NAME (arg_node)));
        } else {
            DBUG_PRINT ("MUTH", ("(top-down) ignore %s", FUNDEF_NAME (arg_node)));
        }
    }
    if (FUNDEF_NEXT (arg_node) != NULL) {
        /*
         *  Are OutOfOrderChanges (OOOCs) allowed???
         *  What are OOOCs??? See comment above!
         */
        if (INFO_MUTH_ALLOW_OOOC (arg_info)) {
            /*
             *  There is a problem here ... Compare OOOC comment above ...
             *  If a new function is inserted by a traversal-function (that is not
             *  applied to FUNDEF_NEXT( arg_node)) that changes the value of
             *  FUNDEF_NEXT( arg_node), we would overwrite this here, so we cannot
             *  do this ... and so we don't by ignoring the return value.
             */
            /* FUNDEF_NEXT( arg_node) = */ Trav (FUNDEF_NEXT (arg_node), arg_info);
        } else {
            /*
             *  Let's check if there is a OOOC ...
             */
            before = FUNDEF_NEXT (arg_node);
            result = Trav (FUNDEF_NEXT (arg_node), arg_info);
            after = FUNDEF_NEXT (arg_node);
            if (before != after) {
                DBUG_ASSERT (0, "not allowed OOOC occured");
            }
            FUNDEF_NEXT (arg_node) = result;
        }
    }
    if (!INFO_MUTH_TOPDOWN (arg_info)) {
        if (!INFO_MUTH_IGNORE (arg_info) (arg_node, arg_info)) {
            DBUG_PRINT ("MUTH",
                        ("(bottom-up) into driver fun (%s)", FUNDEF_NAME (arg_node)));
            arg_node = INFO_MUTH_DRIVER (arg_info) (arg_node, arg_info);
            DBUG_PRINT ("MUTH",
                        ("(bottom-up) from driver fun (%s)", FUNDEF_NAME (arg_node)));
        } else {
            DBUG_PRINT ("MUTH", ("(bottom-up) ignore %s", FUNDEF_NAME (arg_node)));
        }
    }

    INFO_MUTH_FUNDEF (arg_info) = old_fundef;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *ClearATTRIB(node *arg_node, node *arg_info)
 *
 * description:
 *   >>driver<< function, used to clean the N_fundef큦 before the first
 *   mini-phase is executed.
 *   FUNDEF_ATTRIB is set to ST_call_any for all N_fundef큦.
 *
 ******************************************************************************/
node *
ClearATTRIB (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ClearATTRIB");

    FUNDEF_ATTRIB (arg_node) = ST_call_any;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *ClearCOMPANION(node *arg_node, node *arg_info)
 *
 * description:
 *   >>driver<< function, used to clean the N_fundef큦 before mini-phases
 *   rfin and mtfin are executed.
 *   FUNDEF_COMPANION is set to NULL for all N_fundef큦.
 *
 ******************************************************************************/
node *
ClearCOMPANION (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ClearCOMPANION");

    FUNDEF_COMPANION (arg_node) = NULL;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   int MUTHignore(node *arg_node, node *arg_info)
 *
 * description:
 *   >>ignore<< function, used for most (all?) of the miniphases,
 *   functions with empty body, fold_funs and rep_funs are ignored.
 *
 ******************************************************************************/
int
MUTHignore (node *arg_node, node *arg_info)
{
    int result;

    DBUG_ENTER ("MUTHignore");

    result = (FUNDEF_BODY (arg_node) == NULL) || (FUNDEF_STATUS (arg_node) == ST_foldfun)
             || (FUNDEF_ATTRIB (arg_node) == ST_call_rep);

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   int MUTHignore_none(node *arg_node, node *arg_info)
 *
 * description:
 *   >>ignore<< function, ignores nothing
 *
 ******************************************************************************/
int
MUTHignore_none (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("MUTHignore_none");

    DBUG_RETURN (FALSE);
}

/******************************************************************************
 *
 * function:
 *   node *MUTHmodul(node *arg_node, node *arg_info)
 *
 * description:
 *   muth_tab traversal function for N_modul node.
 *   Executes miniphases on all N_fundefs.
 *
 *   ** Each phase is done on all N_fundefs, before the next miniphase **
 *   ** is started!!!!                                                 **
 *
 ******************************************************************************/
node *
MUTHmodul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("MUTHmodul");

    /*
     *  --- initializing (init) ---
     */
    DBUG_PRINT ("MUTH", ("begin initializing"));
    MODUL_FUNS (arg_node)
      = MUTHdriver (MODUL_FUNS (arg_node), arg_info, FALSE, ClearATTRIB, MUTHignore_none);
    DBUG_PRINT ("MUTH", ("end initializing"));

    if ((break_after == PH_spmdregions) && (strcmp ("init", break_specifier) == 0)) {
        goto cont;
    }

    /*
     *  --- ScheduleInit (schin) ---
     */
    DBUG_PRINT ("MUTH", ("begin ScheduleInit"));
    MODUL_FUNS (arg_node)
      = MUTHdriver (MODUL_FUNS (arg_node), arg_info, FALSE, ScheduleInit, MUTHignore);
    DBUG_PRINT ("MUTH", ("end ScheduleInit"));

    if ((break_after == PH_spmdregions) && (strcmp ("schin", break_specifier) == 0)) {
        goto cont;
    }

    /*
     *  --- RepfunsInit (rfin) ---
     *
     *  FUNDEF_COMPANION only used within this traversal!!
     *  It can be reused afterwards
     */
    DBUG_PRINT ("MUTH", ("begin RepfunsInit"));
    MODUL_FUNS (arg_node) = MUTHdriver (MODUL_FUNS (arg_node), arg_info, FALSE,
                                        ClearCOMPANION, MUTHignore_none);
    MODUL_FUNS (arg_node)
      = MUTHdriver (MODUL_FUNS (arg_node), arg_info, TRUE, RepfunsInit, MUTHignore);
    DBUG_PRINT ("MUTH", ("end RepfunsInit"));

    if ((break_after == PH_spmdregions) && (strcmp ("rfin", break_specifier) == 0)) {
        goto cont;
    }

    /*
     *  --- BlocksInit (blkin) ---
     *
     *  FUNDEF_COMPANION used to transport information from blkin to blkpp,
     *  it can be reused after blkpp.
     */
    DBUG_PRINT ("MUTH", ("begin BlocksInit"));
    MODUL_FUNS (arg_node) = MUTHdriver (MODUL_FUNS (arg_node), arg_info, FALSE,
                                        ClearCOMPANION, MUTHignore_none);
    MODUL_FUNS (arg_node)
      = MUTHdriver (MODUL_FUNS (arg_node), arg_info, FALSE, BlocksInit, MUTHignore);
    DBUG_PRINT ("MUTH", ("end BlocksInit"));

    if ((break_after == PH_spmdregions) && (strcmp ("blkin", break_specifier) == 0)) {
        goto cont;
    }

    /*
     *  --- BlocksPropagate (blkpp) ---
     *
     *  FUNDEF_COMPANION used to get information from blkin, but can be
     *  reused after blkpp.
     */
    DBUG_PRINT ("MUTH", ("begin BlocksPropagate"));
    MODUL_FUNS (arg_node)
      = MUTHdriver (MODUL_FUNS (arg_node), arg_info, FALSE, BlocksPropagate, MUTHignore);
    DBUG_PRINT ("MUTH", ("end BlocksPropagate"));

    if ((break_after == PH_spmdregions) && (strcmp ("blkpp", break_specifier) == 0)) {
        goto cont;
    }

    /*
     *  --- BlocksExpand (blkex) ---
     */
    DBUG_PRINT ("MUTH", ("begin BlocksExpand"));
    MODUL_FUNS (arg_node)
      = MUTHdriver (MODUL_FUNS (arg_node), arg_info, FALSE, BlocksExpand, MUTHignore);
    DBUG_PRINT ("MUTH", ("end BlocksExpand"));

    if ((break_after == PH_spmdregions) && (strcmp ("blkex", break_specifier) == 0)) {
        goto cont;
    }

    /*
     *  --- MtfunsInit (mtfin) ---
     *
     *  FUNDEF_COMPANION only used within this traversal!!
     *  It can be reused afterwards
     */
    DBUG_PRINT ("MUTH", ("begin MtfunsInit"));
    MODUL_FUNS (arg_node) = MUTHdriver (MODUL_FUNS (arg_node), arg_info, FALSE,
                                        ClearCOMPANION, MUTHignore_none);
    MODUL_FUNS (arg_node)
      = MUTHdriver (MODUL_FUNS (arg_node), arg_info, TRUE, MtfunsInit, MUTHignore);
    DBUG_PRINT ("MUTH", ("end MtfunsInit"));

    if ((break_after == PH_spmdregions) && (strcmp ("mtfin", break_specifier) == 0)) {
        goto cont;
    }

    /*
     *  --- BlocksCons (blkco) ---
     */
    DBUG_PRINT ("MUTH", ("begin BlocksCons"));
    MODUL_FUNS (arg_node) = MUTHdriver_ (MODUL_FUNS (arg_node), arg_info, FALSE, FALSE,
                                         BlocksCons, MUTHignore);
    DBUG_PRINT ("MUTH", ("end BlocksCons"));

    if ((break_after == PH_spmdregions) && (strcmp ("blkco", break_specifier) == 0)) {
        goto cont;
    }

    /*
     *  --- DataflowAnalysis (dfa) ---
     */
    DBUG_PRINT ("MUTH", ("begin DataflowAnalysis"));
    MODUL_FUNS (arg_node)
      = MUTHdriver (MODUL_FUNS (arg_node), arg_info, FALSE, DataflowAnalysis, MUTHignore);
    DBUG_PRINT ("MUTH", ("end DataflowAnalysis"));

    if ((break_after == PH_spmdregions) && (strcmp ("dfa", break_specifier) == 0)) {
        goto cont;
    }

    /*
     *  --- BarriersInit (barin) ---
     */
    DBUG_PRINT ("MUTH", ("begin BarriersInit"));
    MODUL_FUNS (arg_node)
      = MUTHdriver (MODUL_FUNS (arg_node), arg_info, FALSE, BarriersInit, MUTHignore);
    DBUG_PRINT ("MUTH", ("end BarriersInit"));

    if ((break_after == PH_spmdregions) && (strcmp ("barin", break_specifier) == 0)) {
        goto cont;
    }

    /*
     *  --- BlocksLift (blkli) ---
     */
    DBUG_PRINT ("MUTH", ("begin BlocksLift"));
    MODUL_FUNS (arg_node)
      = MUTHdriver (MODUL_FUNS (arg_node), arg_info, TRUE, BlocksLift, MUTHignore);
    DBUG_PRINT ("MUTH", ("end BlocksLift"));

    if ((break_after == PH_spmdregions) && (strcmp ("blkli", break_specifier) == 0)) {
        goto cont;
    }

    /*
     *  --- AdjustCalls (adjca) ---
     */
    DBUG_PRINT ("MUTH", ("begin AdjustCalls"));
    MODUL_FUNS (arg_node)
      = MUTHdriver (MODUL_FUNS (arg_node), arg_info, FALSE, AdjustCalls1, MUTHignore);
    MODUL_FUNS (arg_node)
      = MUTHdriver (MODUL_FUNS (arg_node), arg_info, FALSE, AdjustCalls2, MUTHignore);
    DBUG_PRINT ("MUTH", ("end AdjustCalls"));

    if ((break_after == PH_spmdregions) && (strcmp ("adjca", break_specifier) == 0)) {
        goto cont;
    }

cont:

    DBUG_RETURN (arg_node);
}
