/*
 *
 * $Log$
 * Revision 3.17  2004/08/26 17:01:36  skt
 * moved MUTHDecodeExecmode from multithread to multithread_lib
 *
 * Revision 3.16  2004/08/24 16:50:24  skt
 * creation of specialized functions within parallel withloops enabled
 *
 * Revision 3.15  2004/08/19 10:16:04  skt
 * pushed the position of resetting executionmodes_available downwards
 *
 * Revision 3.14  2004/08/17 15:47:39  skt
 * cell_groth enabled
 *
 * Revision 3.13  2004/08/11 09:38:03  skt
 * assignments rearrange enabled
 *
 * Revision 3.12  2004/08/09 03:47:34  skt
 * master run warning fixed
 *
 * Revision 3.11  2004/08/06 10:45:41  skt
 * MUTHDecodeExecmode added
 *
 * Revision 3.10  2004/08/05 13:50:18  skt
 * welcome to the new INFO structure
 *
 * Revision 3.9  2004/08/05 12:04:55  skt
 * MUTHassugn added & removed some trash
 *
 * Revision 3.8  2004/07/29 00:40:54  skt
 * added support for creation of dataflowgraph (mtmode 3)
 *
 * Revision 3.7  2004/07/28 17:47:40  skt
 * superfluous FreeTree removed
 *
 * Revision 3.6  2004/07/26 17:04:37  skt
 * create_cells added
 *
 * Revision 3.5  2004/07/06 12:50:54  skt
 * support for propagate_executionmode added
 *
 * Revision 3.4  2004/06/08 14:51:39  skt
 * tag_executionmode added
 *
 * Revision 3.3  2001/05/17 11:47:00  dkr
 * FREE eliminated
 *
 * Revision 3.2  2001/01/25 10:17:57  dkr
 * PH_spmdregions renamed into PH_multithread
 *
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
 */

/**
 * @defgroup muth Multithread
 *
 * This group contains all those files/ modules that apply support for
 * multithreaded execution for sac-code (mtmode 3)
 */

/**
 * @file multithread.c
 *
 * @brief This file iniates and guides the compilation process of the new
 *        multihread-support (mtmode 3)
 * @attention The entire process is still under development
 *
 *
 * prefix:      MUTH
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
 *
 */

#define NEW_INFO

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "globals.h"
#include "free.h"
#include "Error.h"

/*#include "repfuns_init.h"
  #include "blocks_propagate.h"
  #include "blocks_expand.h"
  #include "mtfuns_init.h"
  #include "blocks_cons.h"
  #include "dataflow_analysis.h"
  #include "barriers_init.h"
  #include "blocks_lift.h"
  #include "adjust_calls.h" */

#include "multithread.h"
#include "tag_executionmode.h"
#include "create_withinwith.h"
#include "propagate_executionmode.h"
#include "create_dataflowgraph.h"
#include "assignments_rearrange.h"
#include "create_cells.h"
#include "cell_growth.h"

/*
 * INFO structure
 */
struct INFO {
    node *modul;
};

/*
 * INFO macros
 *    node*    MUTH_MODUL                  (just a placeholder)
 */
#define INFO_MUTH_MODUL(n) (n->modul)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_MUTH_MODUL (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = Free (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn node *BuildMultiThread( node *syntax_tree)
 *
 *   This function starts the process of building the *new* support for
 *   multithread. Throughout this process arg_info points to an N_info which
 *   is generated here.
 *
 * @param syntax_tree The whole syntax-tree of the SAC-program
 * @pre syntax_tree is flat and in SSA-form
 * @return The whole syntax-tree, prepared to be compiled as a multithreaded
 *         application
 *****************************************************************************/
node *
BuildMultiThread (node *syntax_tree)
{
    funtab *old_tab;

    info *arg_info;

    DBUG_ENTER ("BuildMultiThread");

    arg_info = MakeInfo ();

    old_tab = act_tab;
    act_tab = muth_tab;

    syntax_tree = Trav (syntax_tree, arg_info);

    act_tab = old_tab;

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *MUTHfundef(node *arg_node, info *arg_info)
 *
 *   @brief accomplish the initialization of the function-executionmode
 *
 *   @param arg_node a N_fundef
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
MUTHfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MUTHfundef");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef),
                 "MUTHfundef expects a N_fundef as arg_node");

    /* initialize the executionmode*/
    FUNDEF_EXECMODE (arg_node) = MUTH_ANY;
    /* initialize the companion */
    FUNDEF_COMPANION (arg_node) = NULL;

    if (FUNDEF_BODY (arg_node) != NULL) {
        DBUG_PRINT ("MUTH", ("trav into function-body"));
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
        DBUG_PRINT ("MUTH", ("trav from function-body"));
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("MUTH", ("trav into function-next"));
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
        DBUG_PRINT ("MUTH", ("trav from function-next"));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MUTHassign(node *arg_node, info *arg_info)
 *
 *   @brief accomplish the initialization of the assignment-executionmode
 *
 *   @param arg_node a N_assign
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
MUTHassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MUTHassign");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_assign),
                 "MUTHassign expects a N_assign as 1st argument");

    ASSIGN_EXECMODE (arg_node) = MUTH_ANY;
    if (ASSIGN_INSTR (arg_node) != NULL) {
        DBUG_PRINT ("MUTH", ("trav into assignment-instruction"));
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
        DBUG_PRINT ("MUTH", ("trav from assignment-instruction"));
    }
    if (ASSIGN_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("MUTH", ("trav into assignment-next"));
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
        DBUG_PRINT ("MUTH", ("trav from assignment-next"));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MUTHmodul(node *arg_node, info *arg_info)
 *
 *   @brief executes the first parts of the EX-/ST-/MT-multithreading
 *
 *   @param arg_node a N_modul
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
MUTHmodul (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MUTHmodul");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_modul),
                 "MUTHmodul expects a N_modul as 1st argument");

    /*
     *  --- initializing (init) ---
     */
    DBUG_PRINT ("MUTH", ("begin initializing"));
    if (MODUL_IMPORTS (arg_node) != NULL) {
        DBUG_PRINT ("MUTH", ("trav into modul-imports"));
        MODUL_IMPORTS (arg_node) = Trav (MODUL_IMPORTS (arg_node), arg_info);
        DBUG_PRINT ("MUTH", ("trav from modul-imports"));
    }
    if (MODUL_OBJS (arg_node) != NULL) {
        DBUG_PRINT ("MUTH", ("trav into modul-objs"));
        MODUL_OBJS (arg_node) = Trav (MODUL_OBJS (arg_node), arg_info);
        DBUG_PRINT ("MUTH", ("trav from modul-objs"));
    }
    if (MODUL_TYPES (arg_node) != NULL) {
        DBUG_PRINT ("MUTH", ("trav into modul-types"));
        MODUL_TYPES (arg_node) = Trav (MODUL_TYPES (arg_node), arg_info);
        DBUG_PRINT ("MUTH", ("trav from modul-types"));
    }
    if (MODUL_FUNS (arg_node) != NULL) {
        DBUG_PRINT ("MUTH", ("trav into modul-funs"));
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
        DBUG_PRINT ("MUTH", ("trav from modul-funs"));
    }
    executionmodes_available = TRUE;
    DBUG_PRINT ("MUTH", ("end initializing"));

    if ((break_after == PH_multithread) && (strcmp ("init", break_specifier) == 0)) {
        goto cont;
    }

    /*
     *  --- TagExecutionmode (tem) ---
     */
    DBUG_PRINT ("MUTH", ("begin TagExecutionmode"));

    arg_node = TagExecutionmode (arg_node);

    DBUG_PRINT ("MUTH", ("end TagExecutionmode"));

    if ((break_after == PH_multithread) && (strcmp ("tem", break_specifier) == 0)) {
        goto cont;
    }

    /*
     *  --- CreateWithinwith (crwiw) ---
     */
    DBUG_PRINT ("MUTH", ("begin CreateWithinwith"));

    arg_node = CreateWithinwith (arg_node);

    DBUG_PRINT ("MUTH", ("end CreateWihitinwith"));

    if ((break_after == PH_multithread) && (strcmp ("crwiw", break_specifier) == 0)) {
        goto cont;
    }

    /*
     *  --- PropagateExecutionmode (pem) ---
     */
    DBUG_PRINT ("MUTH", ("begin PropagateExecutionmode"));

    arg_node = PropagateExecutionmode (arg_node);

    DBUG_PRINT ("MUTH", ("end PropagateExecutionmode"));

    if ((break_after == PH_multithread) && (strcmp ("pem", break_specifier) == 0)) {
        goto cont;
    }

    /*
     *  --- CreateDataflowgraph (cdfg) ---
     */
    DBUG_PRINT ("MUTH", ("begin CreateDataflowgraph"));

    arg_node = CreateDataflowgraph (arg_node);

    DBUG_PRINT ("MUTH", ("end CreateDataflowgraph"));

    if ((break_after == PH_multithread) && (strcmp ("cdfg", break_specifier) == 0)) {
        goto cont;
    }

    /*
     *  --- AssignmentsRearange (asmra) ---
     */
    DBUG_PRINT ("MUTH", ("begin AssignmentsRearrange"));

    arg_node = AssignmentsRearrange (arg_node);

    DBUG_PRINT ("MUTH", ("end AssignmentsRearrange"));

    if ((break_after == PH_multithread) && (strcmp ("asmra", break_specifier) == 0)) {
        goto cont;
    }

    /*
     *  --- CreateCells (crece) ---
     */
    DBUG_PRINT ("MUTH", ("begin CreateCells"));

    arg_node = CreateCells (arg_node);

    DBUG_PRINT ("MUTH", ("end CreateCells"));
    if ((break_after == PH_multithread) && (strcmp ("crece", break_specifier) == 0)) {
        goto cont;
    }

    /*
     *  --- CellGrowth (cegro) ---
     */
    DBUG_PRINT ("MUTH", ("begin CellGrowth"));

    arg_node = CellGrowth (arg_node);

    DBUG_PRINT ("MUTH", ("end CellGrowth"));
    if ((break_after == PH_multithread) && (strcmp ("cegro", break_specifier) == 0)) {
        goto cont;
    }

    executionmodes_available = FALSE;

    /*
     *  --- MtfunsInit (mtfin) ---
     *
     *  FUNDEF_COMPANION only used within this traversal!!
     *  It can be reused afterwards
     */
    /*DBUG_PRINT( "MUTH", ("begin MtfunsInit"));
    MODUL_FUNS( arg_node) =
      MUTHdriver (MODUL_FUNS( arg_node), arg_info, FALSE, ClearCOMPANION,
    MUTHignore_none); MODUL_FUNS( arg_node) = MUTHdriver (MODUL_FUNS( arg_node), arg_info,
    TRUE, MtfunsInit, MUTHignore); DBUG_PRINT( "MUTH", ("end MtfunsInit"));

    if ((break_after == PH_multithread) &&
        (strcmp("mtfin", break_specifier)==0)) {
      goto cont;
      }*/

    /*
     *  --- BlocksCons (blkco) ---
     */
    /*DBUG_PRINT( "MUTH", ("begin BlocksCons"));
    MODUL_FUNS( arg_node) =
      MUTHdriver_ (MODUL_FUNS( arg_node), arg_info, FALSE, FALSE, BlocksCons, MUTHignore);
    DBUG_PRINT( "MUTH", ("end BlocksCons"));

    if ((break_after == PH_multithread) &&
        (strcmp("blkco", break_specifier)==0)) {
      goto cont;
      }*/

    /*
     *  --- DataflowAnalysis (dfa) ---
     */
    /* DBUG_PRINT( "MUTH", ("begin DataflowAnalysis"));
    MODUL_FUNS( arg_node) =
      MUTHdriver (MODUL_FUNS( arg_node), arg_info, FALSE, DataflowAnalysis, MUTHignore);
    DBUG_PRINT( "MUTH", ("end DataflowAnalysis"));

    if ((break_after == PH_multithread) &&
        (strcmp("dfa", break_specifier)==0)) {
      goto cont;
      }*/

    /*
     *  --- BarriersInit (barin) ---
     */
    /* DBUG_PRINT( "MUTH", ("begin BarriersInit"));
       MODUL_FUNS( arg_node) =
       MUTHdriver (MODUL_FUNS( arg_node), arg_info, FALSE, BarriersInit, MUTHignore);
       DBUG_PRINT( "MUTH", ("end BarriersInit"));

       if ((break_after == PH_multithread) &&
       (strcmp("barin", break_specifier)==0)) {
       goto cont;
      }*/

    /*
     *  --- BlocksLift (blkli) ---
     */
    /* DBUG_PRINT( "MUTH", ("begin BlocksLift"));
       MODUL_FUNS( arg_node) =
       MUTHdriver (MODUL_FUNS( arg_node), arg_info, TRUE, BlocksLift, MUTHignore);
       DBUG_PRINT( "MUTH", ("end BlocksLift"));

       if ((break_after == PH_multithread) &&
       (strcmp("blkli", break_specifier)==0)) {
       goto cont;
       }*/

    /*
     *  --- AdjustCalls (adjca) ---
     */
    /*DBUG_PRINT( "MUTH", ("begin AdjustCalls"));
      MODUL_FUNS( arg_node) =
      MUTHdriver (MODUL_FUNS( arg_node), arg_info, FALSE, AdjustCalls1, MUTHignore);
      MODUL_FUNS( arg_node) =
      MUTHdriver (MODUL_FUNS( arg_node), arg_info, FALSE, AdjustCalls2, MUTHignore);
      DBUG_PRINT( "MUTH", ("end AdjustCalls"));

      if ((break_after == PH_multithread) &&
      (strcmp("adjca", break_specifier)==0)) {
      goto cont;
      }*/

cont:

    DBUG_RETURN (arg_node);
}

/**
 * @}
 */
