/*
 * $Log$
 * Revision 1.14  2004/11/23 20:52:11  skt
 * big compiler brushing during SACDevCampDK 2k4
 *
 * Revision 1.13  2004/11/23 14:38:13  skt
 * SACDevCampDK 2k4
 *
 * Revision 1.12  2004/08/31 11:57:28  skt
 * handling of CALCPARALLEL-flag added
 *
 * Revision 1.11  2004/08/18 13:24:31  skt
 * switch to mtexecmode_t done
 *
 * Revision 1.10  2004/08/06 11:14:54  skt
 * DecodeExecmode deleted - it had been moved into multithread.[ch]
 *
 * Revision 1.9  2004/08/05 17:42:19  skt
 * moved TagAllocs into multithread_lib
 *
 * Revision 1.8  2004/08/05 13:50:18  skt
 * welcome to the new INFO structure
 *
 * Revision 1.7  2004/07/23 10:05:08  skt
 * TEMfundef added
 *
 * Revision 1.6  2004/07/06 18:06:41  skt
 * Parameter of IsGeneratorBigEnough changed from node* to ids*
 *
 * Revision 1.5  2004/07/06 12:37:54  skt
 * TEMreturn removed
 * several functions new implemented
 *
 * Revision 1.4  2004/06/25 09:36:21  skt
 * added TEMlet and some helper functions
 *
 * Revision 1.3  2004/06/23 15:45:17  skt
 * TEMreturn, TEMap, TEMarray added and some debugging done
 *
 * Revision 1.2  2004/06/23 10:18:41  skt
 * a bouquet of new functions added
 *
 * Revision 1.1  2004/06/08 14:17:29  skt
 * Initial revision
 *
 */

/**
 *
 * @defgroup tem Tag Executionmode
 * @ingroup muth
 *
 * @brief tags the mode of execution on an assignment
 * @{
 **/

/** <!--********************************************************************-->
 *
 * @file tag_executionmode.c
 *
 * prefix: TEM
 *
 * description:
 *   tags the assignments, wheter their executionmode is MUTH_ANY,
 *   MUTH_EXCLUSIVE, MUTH_SINGLE or MUTH_MULTI
 *   MUTH_EXCLUSIVE: execution by one thread, all other threads idle
 *   MUTH_SINGLE: execution by one thread, all other threads work
 *   MUTH_MULTI: execution by all threads
 *   MUTH_ANY: decision whether ST, OT or MT to be done later (in another
 *             subphase of the compiler)
 *
 *****************************************************************************/

#define NEW_INFO

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "tag_executionmode.h"
#include "multithread_lib.h"
#include "internal_lib.h"

/*
 * INFO structure
 */
struct INFO {
    node *lefthandside;
    mtexecmode_t executionmode;
    int withdeep;
    int traversalmode;
};

/* access macros for arg_info
 *
 *   node*      ORIGLHS    (left-hand-side of the assignemt, before F_fill was
 *                         added / args 2..n of fill())
 *   mtexecmode_t EXECMODE  (the current execution mode)
 *   int          WITHDEEP  (the current with-loop-deepness)
 *   int          TRAVMODE  (the current traversalmode MUSTEX, MUSTST or
 *                           COULDMT)
 */
#define INFO_TEM_LETLHS(n) (n->lefthandside)
#define INFO_TEM_EXECMODE(n) (n->executionmode)
#define INFO_TEM_WITHDEEP(n) (n->withdeep)
#define INFO_TEM_TRAVMODE(n) (n->traversalmode)

#define TEM_TRAVMODE_DEFAULT 0
#define TEM_TRAVMODE_MUSTEX 1
#define TEM_TRAVMODE_MUSTST 2
#define TEM_TRAVMODE_COULDMT 3

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_TEM_LETLHS (result) = NULL;
    INFO_TEM_EXECMODE (result) = MUTH_ANY;
    INFO_TEM_WITHDEEP (result) = 0;
    INFO_TEM_TRAVMODE (result) = TEM_TRAVMODE_DEFAULT;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/* some declarations */
static bool IsMTAllowed (node *withloop);

static bool IsGeneratorBigEnough (node *test_variables);

static bool IsMTClever (node *test_variables);

static bool IsSTClever (node *test_variables);

static bool MustExecuteExclusive (node *assign, info *arg_info);

static bool CouldExecuteMulti (node *assign, info *arg_info);

static bool MustExecuteSingle (node *assign, info *arg_info);

static bool AnyUniqueTypeInThere (node *letids);

/** <!--********************************************************************-->
 *
 * @fn node *TEMdoTagExecutionmode(node *arg_node, info)
 *
 *   @brief  Inits the traversal for this phase
 *
 *   @param arg_node a N_Module
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
TEMdoTagExecutionmode (node *arg_node)
{
    info *arg_info;
    trav_t traversaltable;

    DBUG_ENTER ("TagExecutionmode");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_module),
                 "TagExecutionmode expects a N_module as arg_node");

    arg_info = MakeInfo ();

    /* some initialisation */
    INFO_TEM_LETLHS (arg_info) = NULL;
    INFO_TEM_EXECMODE (arg_info) = MUTH_ANY;
    INFO_TEM_WITHDEEP (arg_info) = 0;

    TRAVpush (TR_tem);

    DBUG_PRINT ("TEM", ("trav into modul-funs"));
    MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    DBUG_PRINT ("TEM", ("trav from modul-funs"));

    traversaltable = TRAVpop ();
    DBUG_ASSERT ((traversaltable == TR_tem), "Popped incorrect traversal table");

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TEMassign(node *arg_node, info *arg_info)
 *
 *   @brief tags the N_assign with its executionmode
 *<pre>
 *          calculation of the executionmode occurs in an if/else-chain:
 *          if the assignment must be executed in exclusive-mode
 *             => MUTH_EXCLUSIVE
 *          else if the assignment could be executed in multithread-mode
 *             => MUTH_MULTI
 *          else if the assignment must be executed in singlethread-mode
 *             => MUTH_SINGLE
 *          else MUTH_ANY
 *</pre>
 *
 *   @param arg_node a N_assign
 *   @param arg_info
 *   @return N_assign with tagged executionmode
 *
 *****************************************************************************/
node *
TEMassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("TEMassign");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_assign),
                 "TEMassign expects a N_assign as arg_node");

    /* initialize the executionmode */
    ASSIGN_EXECMODE (arg_node) = MUTH_ANY;

    /* we are on top-level, so let's work out the executionmode for this
       assignment */
    if (INFO_TEM_WITHDEEP (arg_info) == 0) {
        if (MustExecuteExclusive (arg_node, arg_info)) {
            ASSIGN_EXECMODE (arg_node) = MUTH_EXCLUSIVE;
        } else if (CouldExecuteMulti (arg_node, arg_info)) {
            ASSIGN_EXECMODE (arg_node) = MUTH_MULTI;
            /* this assignment will be done MT -> let's mark the allocations as
               single-threaded */
            DBUG_ASSERT ((NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_let),
                         "TEMassign expects a N_let here");
            DBUG_ASSERT ((NODE_TYPE (LET_EXPR (ASSIGN_INSTR (arg_node))) == N_with2),
                         "TEMassign expects a N_Nwith2 here");
            /* set the calcparallel-flag */

            WITH2_CALCPARALLEL (LET_EXPR (ASSIGN_INSTR (arg_node))) = TRUE;

            /* tag the allocations of the withloop */
            MUTHLIBtagAllocs (LET_EXPR (ASSIGN_INSTR (arg_node)), MUTH_MULTI);
        } else if (MustExecuteSingle (arg_node, arg_info)) {
            ASSIGN_EXECMODE (arg_node) = MUTH_SINGLE;
        }
    } else {
        DBUG_PRINT ("TEM", ("trav into instruction"));
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
        DBUG_PRINT ("TEM", ("trav from instruction"));
    }

    if (ASSIGN_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("TEM", ("trav into next"));
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
        DBUG_PRINT ("TEM", ("trav from next"));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TEMwith2(node *arg_node, info *arg_info)
 *
 *   @brief check with-loop on possibility of being parallelizable
 *
 *    First:. Is the parallelization of this with-loop allowed?
 *    Second: Is the parallelization of this with-loop clever?
 *      if everything apply => change executionmode in arg_info to MUTH_MULTI
 *
 *   @param arg_node a N_with2
 *   @param arg_info
 *   @return syntax branch; arg_info with (perhaps changed) INFO_TEM_EXECMODE
 *
 *****************************************************************************/
node *
TEMwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("TEMwith2");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_with2),
                 "TEMwith2 expects a N_with2 as argument");

    if (INFO_TEM_TRAVMODE (arg_info) == TEM_TRAVMODE_COULDMT) {
        DBUG_PRINT ("TEM", ("traversalmode is COULDMT - analyse with-loop"));

        /* INFO_TEM_LETLHS(arg_info) must contain a minimum of 1 chain member */
        DBUG_ASSERT ((INFO_TEM_LETLHS (arg_info) != NULL),
                     "INFO_TEM_LETLHS(arg_info) must not be NULL");

        /* check for permission to execute this with-loop in MT
           (an withdeep > 0 implicits the permission, because the current
           with-loop is nested in an top-level with-loop, which MT-execution
           is permitted */
        if ((IsMTAllowed (arg_node)) || (INFO_TEM_WITHDEEP (arg_info) > 0)) {
            if (IsMTClever (INFO_TEM_LETLHS (arg_info))) {

                /* store information about the executionmode in arg_info */
                INFO_TEM_EXECMODE (arg_info) = MUTH_MULTI;
            }
            /* this with-loop is not big enough to be parallellized - but perhaps
               someone a level deeper...*/
            else {
                /*
                 * the generator of this with-loop must be big enough to be
                 * partitioned into max_threads
                 */
                if (IsGeneratorBigEnough (INFO_TEM_LETLHS (arg_info))) {
                    /* the deepness rises... */
                    INFO_TEM_WITHDEEP (arg_info)++;

                    DBUG_PRINT ("TEM", ("trav into with-loop code"));
                    WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
                    DBUG_PRINT ("TEM", ("trav from with-loop code"));

                    /* the deepness falls... */
                    INFO_TEM_WITHDEEP (arg_info)--;
                }
            }
        } else { /*no MTallowed -> doomed */
        }
    } else if (INFO_TEM_TRAVMODE (arg_info) == TEM_TRAVMODE_MUSTEX) {
        /* not in COULDMT-traversalmode
           => continue traversal only in MUSTEX-mode (it's impossible to
           mark an assignment as MUTH_ST inside a with-loop */
        DBUG_PRINT ("TEM", ("trav into with-loop code"));
        WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
        DBUG_PRINT ("TEM", ("trav from with-loop code"));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TEMprf(node *arg_node, info *arg_info)
 *
 *   @brief in MUSTST-traversal: test, whether it would be clever to execute
 *                               this primitive function ST or not,
 *          otherwise: continue traversal
 *
 *   @param arg_node a N_prf
 *   @param arg_info
 *   @return syntax branch with
 *
 *****************************************************************************/
node *
TEMprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("TEMprf");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_prf), "TEMprf expects a N_prf as argument");

    if (INFO_TEM_TRAVMODE (arg_info) == TEM_TRAVMODE_MUSTST
        && IsSTClever (INFO_TEM_LETLHS (arg_info))) {
        INFO_TEM_EXECMODE (arg_info) = MUTH_SINGLE;
    } else {
        DBUG_PRINT ("TEM", ("trav into args"));
        PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
        DBUG_PRINT ("TEM", ("trav from args"));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TEMlet(node *arg_node, info *arg_info)
 *
 *   @brief stores the LHS of let in INFO_TEM_LETLHS(arg_info)
 *          checks lhs for unique type, if in musts-st traversal
 *
 *   @param arg_node a N_let
 *   @param arg_info
 *   @return syntax branch
 *
 *****************************************************************************/
node *
TEMlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("TEMlet");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_let), "TEMlet expects a N_let as argument");

    /* store the ids of the lhs into arg_info */
    INFO_TEM_LETLHS (arg_info) = LET_IDS (arg_node);

    /* are we on top-level, searching for must-st assignments and found
       any unique type on the left handside? -> let's mark it MUTH_SINGLE
       otherwise: continue traversal
    */
    if (INFO_TEM_TRAVMODE (arg_info) == TEM_TRAVMODE_MUSTST
        && INFO_TEM_WITHDEEP (arg_info) == 0
        && AnyUniqueTypeInThere (LET_IDS (arg_node))) {
        DBUG_PRINT ("TEM", ("N_let with unique type => MUTH_SINGLE"));
        INFO_TEM_EXECMODE (arg_info) = MUTH_SINGLE;
    } else {
        DBUG_PRINT ("TEM", ("trav into expr"));
        EXPRS_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
        DBUG_PRINT ("TEM", ("trav from expr"));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TEMap(node *arg_node, info *arg_info)
 *
 *   @brief the behaviour depends on the traversal mode:
 *          MUSTEX: check for empty function body => executionmode:=MUTH_EX
 *          MUSTST: check for Cleverness of ST => executionmode:=MUTH_SINGLE
 *
 *   @param arg_node a N_ap
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
TEMap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("TEMap");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_ap), "TEMap expects a N_ap as argument");

    if (INFO_TEM_TRAVMODE (arg_info) == TEM_TRAVMODE_MUSTEX
        && FUNDEF_BODY (AP_FUNDEF (arg_node)) == NULL) {
        DBUG_PRINT ("TEM", ("N_ap with unknown Body => MUTH_EXCLUSIVE"));
        INFO_TEM_EXECMODE (arg_info) = MUTH_EXCLUSIVE;
    } else if (INFO_TEM_TRAVMODE (arg_info) == TEM_TRAVMODE_MUSTST
               && INFO_TEM_WITHDEEP (arg_info) == 0
               && IsSTClever (INFO_TEM_LETLHS (arg_info))) {
        INFO_TEM_EXECMODE (arg_info) = MUTH_SINGLE;
    } else { /* do nothing special -> continue traversal */
        if (AP_ARGS (arg_node) != NULL) {
            DBUG_PRINT ("TEM", ("trav into arguments"));
            ASSIGN_INSTR (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
            DBUG_PRINT ("TEM", ("trav from arguments"));
        }
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TEMarray(node *arg_node, info *arg_info)
 *
 *   @brief set the executionmode to MUTH_SINGLE, if it would be better to
 *          create the array in one thread than in all
 *
 *   @param arg_node a N_array
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
TEMarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("TEMarray");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_array),
                 "TEMarray expects a N_array as argument");

    if (INFO_TEM_TRAVMODE (arg_info) == TEM_TRAVMODE_MUSTST
        && INFO_TEM_WITHDEEP (arg_info) == 0 && IsSTClever (INFO_TEM_LETLHS (arg_info))) {
        INFO_TEM_EXECMODE (arg_info) = MUTH_SINGLE;
    }
    /* otherwise continue traversal */
    else if (ARRAY_AELEMS (arg_node) != NULL) {
        DBUG_PRINT ("TEM", ("trav into array-elements"));
        ASSIGN_INSTR (arg_node) = TRAVdo (ARRAY_AELEMS (arg_node), arg_info);
        DBUG_PRINT ("TEM", ("trav from array-elements"));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TEMcond(node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param arg_node a N_cond
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
TEMcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("TEMcond");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_cond),
                 "TEMcond expects a N_cond as argument");

    if (INFO_TEM_TRAVMODE (arg_info) == TEM_TRAVMODE_MUSTST
        && INFO_TEM_WITHDEEP (arg_info) == 0 && IsSTClever (INFO_TEM_LETLHS (arg_info))) {
        INFO_TEM_EXECMODE (arg_info) = MUTH_SINGLE;
    }
    if (COND_THEN (arg_node) != NULL) {
        DBUG_PRINT ("TEM", ("trav into then-branch"));
        ASSIGN_NEXT (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
        DBUG_PRINT ("TEM", ("trav from then-branch"));
    }
    if (COND_ELSE (arg_node) != NULL) {
        DBUG_PRINT ("TEM", ("trav into else-branch"));
        ASSIGN_NEXT (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
        DBUG_PRINT ("TEM", ("trav from else-branch"));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn static bool *IsMTAllowed(node *withloop)
 *
 *   @brief This function decides whether a with-loop is actually allowed to be
 *   executed concurrently or not.
 *
 *   the only reason for a negative result could be a ban on parallel execution
 *   of fold-with-loops
 *
 *   @param withloop
 *   @return int interpretated as boolean - whether MT is allowed or not
 *
 *****************************************************************************/
static bool
IsMTAllowed (node *withloop)
{
    bool is_allowed;
    DBUG_ENTER ("IsMTAllowed");
    DBUG_ASSERT ((NODE_TYPE (withloop) == N_with2),
                 "IsMTAllowed expects a N_with2 as argument");

    /* max_sync_fold is a global variable - for further info see globals.c
       if 0 -> no parallelization of fold-with-loops */
    if ((NODE_TYPE (WITH2_WITHOP (withloop)) == N_fold) && (global.max_sync_fold == 0)) {
        is_allowed = FALSE;
    } else {
        is_allowed = TRUE;
    }

    DBUG_RETURN (is_allowed);
}

/** <!--********************************************************************-->
 *
 * @fn static bool *IsGeneratorBigEnough(node *test_variables)
 *
 *   @brief This function decides whether the generator of a with-loop big
 *          enough to be partitioned into max_threads segments
 *
 *
 *   @param test_variables this variables are tested for parallel execution
 *   @return int interpretated as boolean - whether all variables are big
 *           enough or not
 *
 *****************************************************************************/
static bool
IsGeneratorBigEnough (node *test_variables)
{
    node *iterator;
    bool is_bigenough;
    int var_dim, var_size; /* dimension and size of an actual variable */
    int i;
    node *vardec;
    DBUG_ENTER ("IsGeneratorBigEnough");

    /* some initializations */
    is_bigenough = FALSE;
    iterator = test_variables;

    /* TODO perhaps some adaptions for multigenerator-with-loop needed */
    /* TODO handling of AUD and AKD arrays */
    while (iterator != NULL) {

        vardec = IDS_VARDEC (iterator);
        var_dim = VARDEC_DIM (vardec);
        var_size = 1;
        for (i = 0; i < var_dim; i++) {
            var_size *= VARDEC_SHAPE (vardec, i);
        }

        if (var_size >= global.max_threads) {
            is_bigenough = TRUE;
            iterator = IDS_NEXT (iterator);
        }
        /* If one variable on the left-hand-side is not big enough to be
         * partitioned into max_threads threads, the whole generator ist to small
         */
        else {
            is_bigenough = FALSE;
            iterator = NULL;
        }
    }

    DBUG_RETURN (is_bigenough);
}

/** <!--********************************************************************-->
 *
 * @fn static bool *IsMTClever(node *test_variables)
 *
 *   @brief Tests the test_variables whether any of them is big enough to cal-
 *          culate it parallel
 *
 *   @param test_variables this variables are tested for parallel execution
 *   @return int interpretated as boolean - whether calculation should be
 *               parallelized or not
 *
 *****************************************************************************/
static bool
IsMTClever (node *test_variables)
{
    bool is_clever;
    int i, var_dim;  /* dimension and size of an actual variable */
    double var_size; /* size of an actual variable */
    double carry;
    node *iterator, *vardec;
    DBUG_ENTER ("IsMTClever");

    /* some initialization */
    is_clever = FALSE;
    iterator = test_variables;
    carry = 0;

    while ((is_clever == FALSE) && (iterator != NULL)) {

        vardec = IDS_VARDEC (iterator);
        var_dim = VARDEC_DIM (vardec);
        var_size = 1.0;
        for (i = 0; i < var_dim; i++) {
            var_size *= (double)VARDEC_SHAPE (vardec, i);
        }
        /* add the size of the actual variable to the sum of the sizes of the
           former variables */
        carry += var_size;

        if (carry >= (double)(global.min_parallel_size_per_thread * global.max_threads)) {
            is_clever = TRUE;
            DBUG_PRINT ("TEM", ("Found a variable, big enough for parallel execution"));
        }
        iterator = IDS_NEXT (iterator);
    }

    DBUG_RETURN (is_clever);
}

/** <!--********************************************************************-->
 *
 * @fn bool static *IsSTClever(node *test_variables)
 *
 *   @brief Tests the test_variables whether any of them is an array with more
 *          elements than max_replication_size
 *
 *   @param test_variables this variables are tested
 *   @return int interpretated as boolean
 *
 *****************************************************************************/
static bool
IsSTClever (node *test_variables)
{
    /* implementation is like IsMTClever, except of the absence carry-variable */
    bool is_clever;
    int i, var_dim;  /* dimension and size of an actual variable */
    double var_size; /* size of an actual variable */
    node *iterator, *vardec;
    DBUG_ENTER ("IsSTClever");

    /* some initialization */
    is_clever = FALSE;
    iterator = test_variables;

    while ((is_clever == FALSE) && (iterator != NULL)) {

        vardec = IDS_VARDEC (iterator);
        var_dim = VARDEC_DIM (vardec);
        var_size = 1.0;
        for (i = 0; i < var_dim; i++) {
            var_size *= (double)VARDEC_SHAPE (vardec, i);
        }
        if (var_size >= (double)(global.max_replication_size)) {
            is_clever = TRUE;
            DBUG_PRINT ("TEM", ("Found variable, #elements > max_replication_size"));
        }
        iterator = IDS_NEXT (iterator);
    }

    DBUG_RETURN (is_clever);
}

/** <!--********************************************************************-->
 *
 * @fn static bool MustExecuteExclusive(node * assign, info *arg_info)
 *
 *   @brief decide, whether a assignment has to be executed exclusive or not
 *
 *   reason(s) for exclusive executionmode:
 *         - N_ap with unknown body
 *
 *   @param assign the assign-node, which is to analyse
 *   @return int interpretated as boolean - whether the assignment must be
 *           executed in exclusive mode or not
 *
 *****************************************************************************/
static bool
MustExecuteExclusive (node *assign, info *arg_info)
{
    bool exclusive;
    DBUG_ENTER (MustExecuteExclusive);
    DBUG_ASSERT ((NODE_TYPE (assign) == N_assign),
                 "MustExecuteExclusive expects a N_assign");

    /* some initialization */
    INFO_TEM_TRAVMODE (arg_info) = TEM_TRAVMODE_MUSTEX;
    INFO_TEM_EXECMODE (arg_info) = MUTH_ANY;

    /* traverse into the instruction to analyse it */
    DBUG_PRINT ("TEM", ("trav into instruction"));
    ASSIGN_INSTR (assign) = TRAVdo (ASSIGN_INSTR (assign), arg_info);
    DBUG_PRINT ("TEM", ("trav from instruction"));

    if (INFO_TEM_EXECMODE (arg_info) == MUTH_EXCLUSIVE) {
        exclusive = TRUE;
    } else {
        exclusive = FALSE;
    }

    INFO_TEM_TRAVMODE (arg_info) = TEM_TRAVMODE_DEFAULT;
    DBUG_RETURN (exclusive);
}

/** <!--********************************************************************-->
 *
 * @fn static bool CouldExecuteMulti(node * assign, info* arg_info)
 *
 *   @brief decide, whether a assignment could be executed multi-threaded
 *          or not
 *
 *   reason(s) for multi-threaded executionmode:
 *         - with-loop, where MT allowed (IsMTAllowed) and
 *           clever (IsMTCLever) is
 *
 *
 *   @param assign the assign-node, which is to analyse
 *   @return int interpretated as boolean - whether the assignment could be
 *           executed in multi-threaded mode or not
 *
 *****************************************************************************/
static bool
CouldExecuteMulti (node *assign, info *arg_info)
{
    bool multi;
    DBUG_ENTER (CouldExecuteMulti);
    DBUG_ASSERT ((NODE_TYPE (assign) == N_assign),
                 "CouldExecuteMulti expects a N_assign");

    /* some initialization */
    INFO_TEM_TRAVMODE (arg_info) = TEM_TRAVMODE_COULDMT;
    INFO_TEM_EXECMODE (arg_info) = MUTH_ANY;

    /* traverse into the instruction to analyse it */
    DBUG_PRINT ("TEM", ("trav into instruction"));
    ASSIGN_INSTR (assign) = TRAVdo (ASSIGN_INSTR (assign), arg_info);
    DBUG_PRINT ("TEM", ("trav from instruction"));

    if (INFO_TEM_EXECMODE (arg_info) == MUTH_MULTI) {
        multi = TRUE;
    } else {
        multi = FALSE;
    }

    INFO_TEM_TRAVMODE (arg_info) = TEM_TRAVMODE_DEFAULT;
    DBUG_RETURN (multi);
}

/** <!--********************************************************************-->
 *
 * @fn static bool MustExecuteSingle(node * assign, info* arg_info)
 *
 *   @brief decide, whether a assignment has to be executed single-threaded
 *          or not
 *
 *   reason(s) for single-threaded executionmode:
 *         - unique type in N_ap, (not in a withloop)
 *         - result on the lhs is an array bigger than max_replication_size in
 *           N_ap, N_array or N_prf, exceptions:
 *           ~ N_prf is a F_fill which contains a with-loop
 *           ~ N_ap, N_array or N_prf are in an already MUTH_MULTI tagged wl
 *
 *
 *   @param assign the assign-node, which is to analyse
 *   @return int interpretated as boolean - whether the assignment must be
 *           executed in single-threaded mode or not
 *
 *****************************************************************************/
static bool
MustExecuteSingle (node *assign, info *arg_info)
{
    bool single;
    DBUG_ENTER (MustExecuteSingle);
    DBUG_ASSERT ((NODE_TYPE (assign) == N_assign),
                 "MustExecuteSingle expects a N_assign");

    /* some initialization */
    INFO_TEM_TRAVMODE (arg_info) = TEM_TRAVMODE_MUSTST;
    INFO_TEM_EXECMODE (arg_info) = MUTH_ANY;

    /* traverse into the instruction to analyse it */
    DBUG_PRINT ("TEM", ("trav into instruction"));
    ASSIGN_INSTR (assign) = TRAVdo (ASSIGN_INSTR (assign), arg_info);
    DBUG_PRINT ("TEM", ("trav from instruction"));

    if (INFO_TEM_EXECMODE (arg_info) == MUTH_SINGLE) {
        single = TRUE;
    } else {
        single = FALSE;
    }

    INFO_TEM_TRAVMODE (arg_info) = TEM_TRAVMODE_DEFAULT;
    DBUG_RETURN (single);
}

/** <!--********************************************************************-->
 *
 * @fn static bool AnyUniqueTypeInThere(node *letids)
 *
 *   @brief checks the ids-chain for an unique type
 *
 *   @param letids the ids chain to analyse
 *   @return int interpretated as boolean - whether the ids-chain includes an
 *               unique type or not
 *
 *****************************************************************************/
static bool
AnyUniqueTypeInThere (node *letids)
{
    bool unique_found;
    node *iterator;
    types *type;
    DBUG_ENTER ("AnyUniqueTypeInThere");

    /* some initializations */
    unique_found = FALSE;
    iterator = letids;

    while (iterator != NULL && !unique_found) {
        type = VARDEC_TYPE (IDS_VARDEC (letids));
        unique_found |= TCisUnique (type);

        iterator = IDS_NEXT (letids);
    }

    DBUG_RETURN (unique_found);
}

/**
 * @}
 **/
