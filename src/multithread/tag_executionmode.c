/*
 * $Log$
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
 *   MUTH_ANY: decision whether ST, OT or MT to be done later
 *
 *****************************************************************************/

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "print.h"
#include "tag_executionmode.h"
#include "multithread.h"

/** <!--********************************************************************-->
 *
 * @fn node *TagExecutionmode(node *arg_node, node *arg_info)
 *
 *   @brief  Inits the traversal for this phase
 *
 *   @param arg_node a N_Modul
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
TagExecutionmode (node *arg_node, node *arg_info)
{
    funtab *old_tab;

    DBUG_ENTER ("TagExecutionmode");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_modul),
                 "TagExecutionmode expects a N_modul as arg_node");

    /* some initialisation */
    INFO_TEM_ORIGLHS (arg_info) = NULL;
    INFO_TEM_EXECMODE (arg_info) = MUTH_ANY;
    INFO_TEM_WITHDEEP (arg_info) = 0;

    /* push info ... */
    old_tab = act_tab;
    act_tab = tem_tab;

    DBUG_PRINT ("TEM", ("trav into modul-funs"));
    MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    DBUG_PRINT ("TEM", ("trav from modul-funs"));

    /* pop info ... */
    act_tab = old_tab;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TEMassign(node *arg_node, node *arg_info)
 *
 *   @brief tags the N_assign with its executionmode
 *
 *   @param arg_node a N_assign
 *   @param arg_info
 *   @return N_assign with tagged
 *
 *****************************************************************************/
node *
TEMassign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("TEMassign");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_assign),
                 "TEMassign expects a N_assign as arg_node");

    /* reset to default */
    if (INFO_TEM_WITHDEEP (arg_info) == 0) {
        INFO_TEM_EXECMODE (arg_info) = MUTH_ANY;
    }

    DBUG_PRINT ("TEM", ("trav into instruction"));
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    DBUG_PRINT ("TEM", ("trav from instruction"));

    /* set the executionmode of the current assignment */
    if (INFO_TEM_WITHDEEP (arg_info) == 0) {
        ASSIGN_EXECMODE (arg_info) = INFO_TEM_EXECMODE (arg_info);

#if TEM_DEBUG
        if (INFO_TEM_EXECMODE (arg_info) != MUTH_ANY) {
            PrintNode (arg_node);
            fprintf (stdout, "The upper assignment is %s.\n",
                     DecodeExecmode (INFO_TEM_EXECMODE (arg_info)));
        }
#endif
    }
    if (ASSIGN_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("TEM", ("trav into next"));
        PrintNode (ASSIGN_NEXT (arg_node));
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
        DBUG_PRINT ("TEM", ("trav from next"));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TEMwith2(node *arg_node, node *arg_info)
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
TEMwith2 (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("TEMwith2");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_Nwith2),
                 "TEMwith2 expects a N_with2 as argument");

    /* INFO_TEM_ORIGLHS(arg_info) must contain a minimum of 1 chain member */
    DBUG_ASSERT ((INFO_TEM_ORIGLHS (arg_info) != NULL),
                 "INFO_TEM_ORIGLHS(arg_info) must not be NULL");

    if (IsMTAllowed (arg_node) && IsMTClever (INFO_TEM_ORIGLHS (arg_info))) {

        /* store information for N_assign in arg_info */
        INFO_TEM_EXECMODE (arg_info) = MUTH_MULTI;
    }
    /* this with-loop is not big enough to be parallellized - but perhaps
       someone a level deeper...*/
    else {
        /* the generator of this with-loop must be big enough to be partitioned
           into max_threads */
        if (IsGeneratorBigEnough (INFO_TEM_ORIGLHS (arg_info))) {
            /* the deepness rises... */
            INFO_TEM_WITHDEEP (arg_info)++;
            DBUG_PRINT ("TEM", ("trav into with-loop code"));
            NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);
            DBUG_PRINT ("TEM", ("trav from with-loop code"));
            /* the deepness falls... */
            INFO_TEM_WITHDEEP (arg_info)--;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TEMprf(node *arg_node, node *arg_info)
 *
 *   @brief stores PRF_ARGS(arg_node) into arg_info to use them in TEMwith2
 *
 *   SSARefCount() stores the original assignments in primitive functions
 *   called fill(). E.g.,
 *   A,B= with...
 *   SSARefCount => A',B' = fill(with...,A,B);
 *   To get access to the original lhs, one has to bring the
 *   reference through the traversal. So the PRF_ARGS are stored
 *   to INFO_TEM_PRFARGS(arg_info).
 *
 *   @param arg_node a N_prf
 *   @param arg_info
 *   @return syntax branch with
 *
 *****************************************************************************/
node *
TEMprf (node *arg_node, node *arg_info)
{
    node *old_lhs;
    DBUG_ENTER ("TEMprf");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_prf), "TEMprf expects a N_prf as argument");

    if (PRF_PRF (arg_node) == F_fill) {
        DBUG_ASSERT ((PRF_ARGS (arg_node) != NULL),
                     "PRF_ARGS(arg_node)) must not be NULL");
        old_lhs = INFO_TEM_ORIGLHS (arg_info);
        INFO_TEM_ORIGLHS (arg_info) = EXPRS_NEXT (PRF_ARGS (arg_node));
    }

    DBUG_PRINT ("TEM", ("trav into args"));
    PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);
    DBUG_PRINT ("TEM", ("trav from args"));

    if (PRF_PRF (arg_node) == F_fill) {

        /* Can the actual assignment executed in mt-mode and are we going back
         * to top-level? => Let's mark the allocations for this assignment as
         * exclusive-mode */
        if ((INFO_TEM_EXECMODE (arg_info) == MUTH_MULTI)
            && (INFO_TEM_WITHDEEP (arg_info) == 0)) {
            DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (PRF_ARGS (arg_node))) == N_Nwith2),
                         "TEMprf would expect a N_with2 here");
            DBUG_ASSERT ((EXPRS_NEXT (PRF_ARGS (arg_node)) != NULL),
                         "TEMprf do not expect a NULL here");
            EXPRS_NEXT (PRF_ARGS (arg_node))
              = TagAllocs (EXPRS_NEXT (PRF_ARGS (arg_node)));
        }
        INFO_TEM_ORIGLHS (arg_info) = old_lhs;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TEMlet(node *arg_node, node *arg_info)
 *
 *   @brief stores the LHS of let in INFO_TEM_LETLHS(arg_info)
 *
 *   @param arg_node a N_let
 *   @param arg_info
 *   @return syntax branch
 *
 *****************************************************************************/
node *
TEMlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("TEMlet");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_let), "TEMlet expects a N_let as argument");

    /*  if(arg_node != INFO_TEM_ORIGLHS(arg_info)) {*/
    DBUG_PRINT ("TEM", ("trav into expr"));
    EXPRS_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
    DBUG_PRINT ("TEM", ("trav from expr"));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TEMreturn(node *arg_node, node *arg_info)
 *
 *   @brief avoids traversing into the expressions of N_return
 *
 *   @param arg_node a N_return
 *   @param arg_info
 *   @return arg_node as it came into TEMreturn
 *
 *****************************************************************************/
node *
TEMreturn (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("TEMreturn");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_return),
                 "TEMreturn expects a N_return as argument");

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TEMap(node *arg_node, node *arg_info)
 *
 *   @brief
 *
 *   @param arg_node a N_ap
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
TEMap (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("TEMap");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_ap), "TEMap expects a N_ap as argument");
    if (FUNDEF_BODY (AP_FUNDEF (arg_node)) == NULL) {
        DBUG_PRINT ("TEM", ("N_ap with unknown Body => MUTH_EXCLUSIVE"));
        INFO_TEM_EXECMODE (arg_info) = MUTH_EXCLUSIVE;
    } else {
        DBUG_PRINT ("TEM", ("N_ap with known Body"));
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TEMarray(node *arg_node, node *arg_info)
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
TEMarray (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("TEMarray");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_array),
                 "TEMarray expects a N_array as argument");

    if (INFO_TEM_WITHDEEP (arg_info) == 0 &&   /* Top-Level ?*/
        INFO_TEM_ORIGLHS (arg_info) != NULL && /* inside a pr. function fill ? */
        IsSTClever (INFO_TEM_ORIGLHS (arg_info)) /* better to do ST ? */) {
        INFO_TEM_EXECMODE (arg_info) = MUTH_SINGLE;
    }

#if TEM_DEBUG
    fprintf (stdout, "TEMarray: act. execmode = %s\n",
             DecodeExecmode (INFO_TEM_EXECMODE (arg_info)));
#endif

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn int *IsMTAllowed(node *withloop)
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
int
IsMTAllowed (node *withloop)
{
    int is_allowed;
    DBUG_ENTER ("IsMTAllowed");
    DBUG_ASSERT ((NODE_TYPE (withloop) == N_Nwith2),
                 "IsMTAllowed expects a N_with2 as argument");

    /* max_sync_fold is a global variable - for further info see globals.c
       if 0 -> no parallelization of fold-with-loops */
    if (NWITHOP_IS_FOLD (NWITH2_WITHOP (withloop)) && (max_sync_fold == 0)) {
        is_allowed = FALSE;
    } else {
        is_allowed = TRUE;
    }

    DBUG_RETURN (is_allowed);
}

/** <!--********************************************************************-->
 *
 * @fn int *IsGeneratorBigEnough(node *exprs)
 *
 *   @brief This function decides whether the generator of a with-loop big
 *          enough to be partitioned into max_threads segments
 *
 *
 *   @param exprs expecs N_exprs-chain, the parameter 2..n of the primitive
 *                function fill(), added by SSARefCount
 *   @return int interpretated as boolean - whether the generator is big enough
 *           or not
 *
 *****************************************************************************/
int
IsGeneratorBigEnough (node *exprs)
{
    int is_bigenough;
    int var_dim, var_size; /* dimension and size of an actual variable */
    int i;
    node *vardec;
    DBUG_ENTER ("IsGeneratorBigEnough");
    DBUG_ASSERT ((NODE_TYPE (exprs) == N_exprs),
                 "IsGeneratorBigEnough expects a N_exprs as argument");

    /* some initialization */
    is_bigenough = FALSE;

    /* TODO perhaps some adaptions for multigenerator-with-loop needed */
    while ((exprs != NULL)) {

        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (exprs)) == N_id), "N_id expected");

        vardec = ID_VARDEC (EXPRS_EXPR (exprs));
        var_dim = VARDEC_DIM (vardec);
        var_size = 1;
        for (i = 0; i < var_dim; i++) {
            var_size *= VARDEC_SHAPE (vardec, i);
        }

        if (var_size >= max_threads) {
            is_bigenough = TRUE;
            exprs = EXPRS_NEXT (exprs);
        }
        /* If one variable on the left-hand-side is not big enough to be
         * partitioned into max_threads threads, the whole generator ist to small
         */
        else {
            is_bigenough = FALSE;
            exprs = NULL;
        }
    }

    DBUG_RETURN (is_bigenough);
}

/** <!--********************************************************************-->
 *
 * @fn int *IsMTClever(node *exprs)
 *
 *   @brief This function decides whether it is clever to parallelize a
 *          with-loop or not
 *
 *
 *   @param exprs expecs N_exprs-chain, the parameter 2..n of the primitive
 *                function fill(), added by SSARefCount
 *   @return int interpretated as boolean - whether calculation should be
 *               parallelized or not
 *
 *****************************************************************************/
int
IsMTClever (node *exprs)
{
    int is_clever;
    int var_dim;     /* dimension and size of an actual variable */
    double var_size; /* size of an actual variable */
    int i;
    node *vardec;
    DBUG_ENTER ("IsMTClever");

    DBUG_ASSERT ((NODE_TYPE (exprs) == N_exprs),
                 "IsMTClever expects a N_exprs as argument");

    /* some initialization */
    is_clever = FALSE;

    /* TODO perhaps some adaptions for multigenerator-with-loop needed */

    while ((is_clever == FALSE) && (exprs != NULL)) {

        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (exprs)) == N_id), "N_id expected");

        vardec = ID_VARDEC (EXPRS_EXPR (exprs));
        var_dim = VARDEC_DIM (vardec);
        var_size = 1.0;
        for (i = 0; i < var_dim; i++) {
            var_size *= (double)VARDEC_SHAPE (vardec, i);
        }

        if (var_size >= (double)(min_parallel_size_per_thread * max_threads))
            is_clever = TRUE;

        exprs = EXPRS_NEXT (exprs);
    }

    /*min_parallel_size_per_thread*/

    DBUG_RETURN (is_clever);
}

/** <!--********************************************************************-->
 *
 * @fn int *IsSTClever(node *exprs)
 *
 *   @brief This function decides whether it is clever to execute the
 *          assignment of the variables in the given exprs-chain in single-mode
 *
 *   @param exprs expecs N_exprs-chain, the parameter 2..n of the primitive
 *                function fill(), added by SSARefCount
 *   @return int interpretated as boolean - whether the assignment should be
 *               done single-threaded or not
 *
 *****************************************************************************/
int
IsSTClever (node *exprs)
{
    int is_clever;
    int var_dim;     /* dimension and size of an actual variable */
    double var_size; /* dimension and size of an actual variable */
    int i;
    node *vardec;
    DBUG_ENTER ("IsMTClever");

    DBUG_ASSERT ((NODE_TYPE (exprs) == N_exprs),
                 "IsSTClever expects a N_exprs as argument");

    /* some initialization */
    is_clever = FALSE;

    while ((is_clever == FALSE) && (exprs != NULL)) {

        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (exprs)) == N_id), "N_id expected");

        vardec = ID_VARDEC (EXPRS_EXPR (exprs));
        var_dim = VARDEC_DIM (vardec);
        var_size = 1.0;
        for (i = 0; i < var_dim; i++) {
            var_size *= (double)VARDEC_SHAPE (vardec, i);
        }

        if (var_size >= (double)max_replication_size)
            is_clever = TRUE;

        exprs = EXPRS_NEXT (exprs);
    }

    DBUG_RETURN (is_clever);
}

/** <!--********************************************************************-->
 *
 * @fn node *TagAllocs(node *exprs)
 *
 *   @brief This function tags the executionmode of the allocation-assignments
 *          for the variables in the exprs-chain into MUTH_SINGLE, if their
 *          executionmode was not strict enough
 *
 *   @param exprs chain of expressions, consists of Variables, which are the
 *                left-hand-side of a multi-threaded with-loop
 *   @return the chain of expressions, which alloc-assignments are tagged
 *
 *****************************************************************************/
node *
TagAllocs (node *exprs)
{
    node *tmp;
    node *expr;
    node *assign;
    DBUG_ENTER ("TagAllocs");
    DBUG_ASSERT ((NODE_TYPE (exprs) == N_exprs),
                 "TagAllocs expects a N_exprs as argument");

    tmp = exprs;
#if TEM_DEBUG
    fprintf (stdout, "********** TagAllocs start **********\n");
#endif
    while (tmp != NULL) {
        expr = EXPRS_EXPR (exprs);

        DBUG_ASSERT ((NODE_TYPE (expr) == N_id), "TagAllocs expects a N_id here");
        assign = AVIS_SSAASSIGN (ID_AVIS (expr));

        DBUG_ASSERT ((ASSIGN_EXECMODE (assign) = MUTH_MULTI),
                     "The executionmode of this assignment must'n be MUTH_MULTI");
        /* change executionmode of the assignment, if it's not as strict as
           MUTH_SINGLE */
        ASSIGN_EXECMODE (assign)
          = StrongestRestriction (MUTH_SINGLE, ASSIGN_EXECMODE (assign));

#if TEM_DEBUG
        PrintNode (assign);
        fprintf (stdout, "Executionmode changed into %s\n",
                 DecodeExecmode (ASSIGN_EXECMODE (assign)));
#endif

        tmp = EXPRS_NEXT (tmp);
    }
#if TEM_DEBUG
    fprintf (stdout, "********** TagAllocs end **********\n");
#endif

    DBUG_RETURN (exprs);
}

/** <!--********************************************************************-->
 *
 * @fn int StrongestRestriction(int execmode1, int execmode2)
 *
 *   @brief gets two executionmodes and returns the most restricted of them
 *
 *   @param execmode1 the first executionmode
 *   @param execmode2 the second executionmode
 *   @return the most restricted executionmode of the two arguments
 *
 *****************************************************************************/
int
StrongestRestriction (int execmode1, int execmode2)
{
    int result;
    DBUG_ENTER (StrongestRestriction);
    DBUG_ASSERT (((execmode1 == MUTH_ANY) || (execmode1 == MUTH_EXCLUSIVE)
                  || (execmode1 == MUTH_SINGLE) || (execmode1 == MUTH_MULTI)),
                 "StrongestRestriction expects a valid executionmode in #1 arg.");
    DBUG_ASSERT (((execmode2 == MUTH_ANY) || (execmode2 == MUTH_EXCLUSIVE)
                  || (execmode2 == MUTH_SINGLE) || (execmode2 == MUTH_MULTI)),
                 "StrongestRestriction expects a valid executionmode in #2 arg.");

    if ((execmode1 == MUTH_EXCLUSIVE) || (execmode2 == MUTH_EXCLUSIVE)) {
        result = MUTH_EXCLUSIVE;
    } else if ((execmode1 == MUTH_SINGLE) || (execmode2 == MUTH_SINGLE)) {
        result = MUTH_SINGLE;
    } else if ((execmode1 == MUTH_MULTI) || (execmode2 == MUTH_MULTI)) {
        result = MUTH_MULTI;
    } else {
        result = MUTH_ANY;
    }

    DBUG_RETURN (result);
}

#if TEM_DEBUG
/** <!--********************************************************************-->
 *
 * @fn char *DecodeExecmode(int execmode)
 *
 *   @brief A small helper function to make debug-output more readable
 *          !It must be adapted if the names of the modes change!
 *
 *   @param execmode the executionmode to decode
 *   @return the name of the executionmode as a string
 *
 *****************************************************************************/
char *
DecodeExecmode (int execmode)
{
    switch (execmode) {
    case MUTH_ANY:
        return ("AT");
    case MUTH_EXCLUSIVE:
        return ("EX");
    case MUTH_SINGLE:
        return ("ST");
    case MUTH_MULTI:
        return ("MT");
    default:
        DBUG_ASSERT (0, "DecodeExecmode expects a valid executionmode");
    }
    return "NN";
}
#endif

/**
 * @}
 **/
