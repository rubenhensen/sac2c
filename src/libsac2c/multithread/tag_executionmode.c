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

#include "tag_executionmode.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "multithread_lib.h"
#include "str.h"
#include "memory.h"

#define DBUG_PREFIX "TEM"
#include "debug.h"

#include "globals.h"
#include "new_types.h"
#include "shape.h"
#include "type_utils.h"

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
#define INFO_LETLHS(n) (n->lefthandside)
#define INFO_EXECMODE(n) (n->executionmode)
#define INFO_WITHDEEP(n) (n->withdeep)
#define INFO_TRAVMODE(n) (n->traversalmode)

#define TEM_TRAVMODE_DEFAULT 0
#define TEM_TRAVMODE_MUSTEX 1
#define TEM_TRAVMODE_MUSTST 2
#define TEM_TRAVMODE_COULDMT 3

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_LETLHS (result) = NULL;
    INFO_EXECMODE (result) = MUTH_ANY;
    INFO_WITHDEEP (result) = 0;
    INFO_TRAVMODE (result) = TEM_TRAVMODE_DEFAULT;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

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

    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_module,
                 "TagExecutionmode expects a N_module as arg_node");

    arg_info = MakeInfo ();

    /* some initialisation */
    INFO_LETLHS (arg_info) = NULL;
    INFO_EXECMODE (arg_info) = MUTH_ANY;
    INFO_WITHDEEP (arg_info) = 0;

    TRAVpush (TR_tem);
    MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    TRAVpop ();

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
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_assign,
                 "TEMassign expects a N_assign as arg_node");

    /* initialize the executionmode */
    ASSIGN_EXECMODE (arg_node) = MUTH_ANY;

    /* we are on top-level, so let's work out the executionmode for this
       assignment */
    if (INFO_WITHDEEP (arg_info) == 0) {
        if (MustExecuteExclusive (arg_node, arg_info)) {
            ASSIGN_EXECMODE (arg_node) = MUTH_EXCLUSIVE;
        } else if (CouldExecuteMulti (arg_node, arg_info)) {
            ASSIGN_EXECMODE (arg_node) = MUTH_MULTI;
            /* this assignment will be done MT -> let's mark the allocations as
               single-threaded */
            DBUG_ASSERT (NODE_TYPE (ASSIGN_STMT (arg_node)) == N_let,
                         "TEMassign expects a N_let here");
            DBUG_ASSERT (NODE_TYPE (LET_EXPR (ASSIGN_STMT (arg_node))) == N_with2,
                         "TEMassign expects a N_Nwith2 here");
            /* set the calcparallel-flag */

            WITH2_PARALLELIZE (LET_EXPR (ASSIGN_STMT (arg_node))) = TRUE;

            /* tag the allocations of the withloop */
            MUTHLIBtagAllocs (LET_EXPR (ASSIGN_STMT (arg_node)), MUTH_MULTI);
        } else if (MustExecuteSingle (arg_node, arg_info)) {
            ASSIGN_EXECMODE (arg_node) = MUTH_SINGLE;
        }
    } else {
        ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
    }

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
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
 *   @return syntax branch; arg_info with (perhaps changed) INFO_EXECMODE
 *
 *****************************************************************************/
node *
TEMwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_with2,
                 "TEMwith2 expects a N_with2 as argument");

    if (INFO_TRAVMODE (arg_info) == TEM_TRAVMODE_COULDMT) {
        DBUG_PRINT ("traversalmode is COULDMT - analyse with-loop");

        /* INFO_LETLHS(arg_info) must contain a minimum of 1 chain member */
        DBUG_ASSERT (INFO_LETLHS (arg_info) != NULL,
                     "INFO_LETLHS(arg_info) must not be NULL");

        /* check for permission to execute this with-loop in MT
           (an withdeep > 0 implicits the permission, because the current
           with-loop is nested in an top-level with-loop, which MT-execution
           is permitted */
        if ((IsMTAllowed (arg_node)) || (INFO_WITHDEEP (arg_info) > 0)) {
            if (IsMTClever (INFO_LETLHS (arg_info))) {

                /* store information about the executionmode in arg_info */
                INFO_EXECMODE (arg_info) = MUTH_MULTI;
            }
            /* this with-loop is not big enough to be parallellized - but perhaps
               someone a level deeper...*/
            else {
                /*
                 * the generator of this with-loop must be big enough to be
                 * partitioned into max_threads
                 */
                if (IsGeneratorBigEnough (INFO_LETLHS (arg_info))) {
                    /* the deepness rises... */
                    INFO_WITHDEEP (arg_info)++;

                    WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);

                    /* the deepness falls... */
                    INFO_WITHDEEP (arg_info)--;
                }
            }
        } else { /*no MTallowed -> doomed */
        }
    } else if (INFO_TRAVMODE (arg_info) == TEM_TRAVMODE_MUSTEX) {
        /* not in COULDMT-traversalmode
           => continue traversal only in MUSTEX-mode (it's impossible to
           mark an assignment as MUTH_ST inside a with-loop */
        WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
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
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_prf, "TEMprf expects a N_prf as argument");

    if (INFO_TRAVMODE (arg_info) == TEM_TRAVMODE_MUSTST
        && IsSTClever (INFO_LETLHS (arg_info))) {
        INFO_EXECMODE (arg_info) = MUTH_SINGLE;
    } else {
        PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TEMlet(node *arg_node, info *arg_info)
 *
 *   @brief stores the LHS of let in INFO_LETLHS(arg_info)
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
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_let, "TEMlet expects a N_let as argument");

    /* store the ids of the lhs into arg_info */
    INFO_LETLHS (arg_info) = LET_IDS (arg_node);

    /* are we on top-level, searching for must-st assignments and found
       any unique type on the left handside? -> let's mark it MUTH_SINGLE
       otherwise: continue traversal
    */
    if (INFO_TRAVMODE (arg_info) == TEM_TRAVMODE_MUSTST && INFO_WITHDEEP (arg_info) == 0
        && AnyUniqueTypeInThere (LET_IDS (arg_node))) {
        DBUG_PRINT ("N_let with unique type => MUTH_SINGLE");
        INFO_EXECMODE (arg_info) = MUTH_SINGLE;
    } else {
        EXPRS_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
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
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_ap, "TEMap expects a N_ap as argument");

    if (INFO_TRAVMODE (arg_info) == TEM_TRAVMODE_MUSTEX
        && FUNDEF_BODY (AP_FUNDEF (arg_node)) == NULL) {
        DBUG_PRINT ("N_ap with unknown Body => MUTH_EXCLUSIVE");
        INFO_EXECMODE (arg_info) = MUTH_EXCLUSIVE;
    } else if (INFO_TRAVMODE (arg_info) == TEM_TRAVMODE_MUSTST
               && INFO_WITHDEEP (arg_info) == 0 && IsSTClever (INFO_LETLHS (arg_info))) {
        INFO_EXECMODE (arg_info) = MUTH_SINGLE;
    } else { /* do nothing special -> continue traversal */
        if (AP_ARGS (arg_node) != NULL) {
            ASSIGN_STMT (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
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
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_array,
                 "TEMarray expects a N_array as argument");

    if (INFO_TRAVMODE (arg_info) == TEM_TRAVMODE_MUSTST && INFO_WITHDEEP (arg_info) == 0
        && IsSTClever (INFO_LETLHS (arg_info))) {
        INFO_EXECMODE (arg_info) = MUTH_SINGLE;
    }
    /* otherwise continue traversal */
    else if (ARRAY_AELEMS (arg_node) != NULL) {
        ASSIGN_STMT (arg_node) = TRAVdo (ARRAY_AELEMS (arg_node), arg_info);
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
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_cond, "TEMcond expects a N_cond as argument");

    if (INFO_TRAVMODE (arg_info) == TEM_TRAVMODE_MUSTST && INFO_WITHDEEP (arg_info) == 0
        && IsSTClever (INFO_LETLHS (arg_info))) {
        INFO_EXECMODE (arg_info) = MUTH_SINGLE;
    }
    if (COND_THEN (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
    }
    if (COND_ELSE (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
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
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (withloop) == N_with2,
                 "IsMTAllowed expects a N_with2 as argument");

    if ((NODE_TYPE (WITH2_WITHOP (withloop)) == N_fold) && global.no_fold_parallel) {
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
    int var_dim;
    long long var_size;
    DBUG_ENTER ();

    /* some initializations */
    is_bigenough = FALSE;
    iterator = test_variables;

    /* TODO perhaps some adaptions for multigenerator-with-loop needed */
    /* TODO handling of AUD and AKD arrays */
    while (iterator != NULL) {

        var_dim = TYgetDim (IDS_NTYPE (iterator));
        var_size = SHgetUnrLen (TYgetShape (IDS_NTYPE (iterator)));

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
    int var_dim;  /* dimension and size of an actual variable */
    double var_size; /* size of an actual variable */
    double carry;
    node *iterator;
    DBUG_ENTER ();

    /* some initialization */
    is_clever = FALSE;
    iterator = test_variables;
    carry = 0;

    while ((is_clever == FALSE) && (iterator != NULL)) {

        var_dim = TYgetDim (IDS_NTYPE (iterator));
        var_size = (double)SHgetUnrLen (TYgetShape (IDS_NTYPE (iterator)));

        /* add the size of the actual variable to the sum of the sizes of the
           former variables */
        carry += var_size;

        if (carry >= (double)(global.min_parallel_size_per_thread * global.max_threads)) {
            is_clever = TRUE;
            DBUG_PRINT ("Found a variable, big enough for parallel execution");
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
    int var_dim;  /* dimension and size of an actual variable */
    double var_size; /* size of an actual variable */
    node *iterator;
    DBUG_ENTER ();

    /* some initialization */
    is_clever = FALSE;
    iterator = test_variables;

    while ((is_clever == FALSE) && (iterator != NULL)) {

        var_dim = TYgetDim (IDS_NTYPE (iterator));
        var_size = (double)SHgetUnrLen (TYgetShape (IDS_NTYPE (iterator)));

        if (var_size >= (double)(global.max_replication_size)) {
            is_clever = TRUE;
            DBUG_PRINT ("Found variable, #elements > max_replication_size");
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
    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (assign) == N_assign,
                 "MustExecuteExclusive expects a N_assign");

    /* some initialization */
    INFO_TRAVMODE (arg_info) = TEM_TRAVMODE_MUSTEX;
    INFO_EXECMODE (arg_info) = MUTH_ANY;

    /* traverse into the instruction to analyse it */
    ASSIGN_STMT (assign) = TRAVdo (ASSIGN_STMT (assign), arg_info);

    exclusive = (INFO_EXECMODE (arg_info) == MUTH_EXCLUSIVE);

    INFO_TRAVMODE (arg_info) = TEM_TRAVMODE_DEFAULT;
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
    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (assign) == N_assign, "CouldExecuteMulti expects a N_assign");

    /* some initialization */
    INFO_TRAVMODE (arg_info) = TEM_TRAVMODE_COULDMT;
    INFO_EXECMODE (arg_info) = MUTH_ANY;

    /* traverse into the instruction to analyse it */
    ASSIGN_STMT (assign) = TRAVdo (ASSIGN_STMT (assign), arg_info);

    multi = (INFO_EXECMODE (arg_info) == MUTH_MULTI);

    INFO_TRAVMODE (arg_info) = TEM_TRAVMODE_DEFAULT;
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
    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (assign) == N_assign, "MustExecuteSingle expects a N_assign");

    /* some initialization */
    INFO_TRAVMODE (arg_info) = TEM_TRAVMODE_MUSTST;
    INFO_EXECMODE (arg_info) = MUTH_ANY;

    /* traverse into the instruction to analyse it */
    ASSIGN_STMT (assign) = TRAVdo (ASSIGN_STMT (assign), arg_info);

    single = (INFO_EXECMODE (arg_info) == MUTH_SINGLE);

    INFO_TRAVMODE (arg_info) = TEM_TRAVMODE_DEFAULT;
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

    DBUG_ENTER ();

    unique_found = FALSE;

    while (letids != NULL && !unique_found) {
        unique_found |= TUisUniqueUserType (AVIS_TYPE (IDS_AVIS (letids)));
        letids = IDS_NEXT (letids);
    }

    DBUG_RETURN (unique_found);
}

/**
 * @}
 **/

#undef DBUG_PREFIX
