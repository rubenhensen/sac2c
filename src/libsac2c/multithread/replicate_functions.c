/**
 *
 * @defgroup repfun Replicate Functions
 * @ingroup muth
 *
 * @brief all functions (and its members rekursive) which are called MUTH_ANY
 *        are replicated into special versions for MUTH_SINGLE, MUTH_MULTI and
 *        MUTH_EXCLUSIVE.
 * @{
 **/

/** <!--********************************************************************-->
 *
 * @file replicate_functions.c
 *
 * prefix: REPFUN
 *
 * description:
 *
 *****************************************************************************/

#include "tree_basic.h"
#include "tree_compound.h"
#include "DupTree.h"
#include "traverse.h"
#include "replicate_functions.h"
#include "multithread_lib.h"
#include "str.h"
#include "memory.h"

#define DBUG_PREFIX "REPFUN"
#include "debug.h"

/*
 * INFO structure
 */
struct INFO {
    node *actassign;
    mtexecmode_t execmode;
};

/*
 * INFO macros
 *    node    REPFUN_ACTASSIGN
 *    node    REPFUN_EXECMODE
 */
#define INFO_REPFUN_ACTASSIGN(n) (n->actassign)
#define INFO_REPFUN_EXECMODE(n) (n->execmode)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_REPFUN_ACTASSIGN (result) = NULL;
    INFO_REPFUN_EXECMODE (result) = MUTH_ANY;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn node *REPFUNdoReplicateFunctions(node *arg_node)
 *
 *   @brief  Inits the traversal for this phase
 *
 *   @param arg_node a N_module
 *   @return the N_module with replicated, correct assigned functions
 *
 *****************************************************************************/
node *
REPFUNdoReplicateFunctions (node *arg_node)
{
    info *arg_info;
    trav_t traversaltable;

    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_module,
                 "REPFUNdoReplicateFunctions expects a N_module as arg_node");

    arg_info = MakeInfo ();

    TRAVpush (TR_repfun);

    DBUG_PRINT ("trav into module");
    arg_node = TRAVdo (arg_node, arg_info);
    DBUG_PRINT ("trav from module");

    traversaltable = TRAVpop ();
    DBUG_ASSERT (traversaltable == TR_repfun, "Popped incorrect traversal table");

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *REPFUNmodule(node *arg_node, info *arg_info)
 *
 *   @brief  Inits the traversal for this phase
 *
 *   @param arg_node a N_module
 *   @return the N_module with replicated, correct assigned functions
 *
 *****************************************************************************/
node *
REPFUNmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt(MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *REPFUNfundef(node *arg_node, info *arg_info)
 *
 *   @brief after traversal into FUNDEF_BODY (if != NULL) it continues
 *          traversal into FUNDEF_NEXT, but don't assign it to FUNDEF_NEXT,
 *          because of the adding of new functions in meantime
 *
 *   @param arg_node a N_fundef
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
REPFUNfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef, "N_fundef expected");

    if (FUNDEF_BODY (arg_node) != NULL) {
        if (FUNDEF_EXECMODE (arg_node) != MUTH_ANY) {
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        }
    }

    if (FUNDEF_NEXT (arg_node) == NULL) {
        FUNDEF_NEXT (arg_node) = DUPgetCopiedSpecialFundefs ();
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("trav into fundef-next");
        /*
         * FUNDEF_NEXT(arg_node) =
         */
        TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        /*
         * (the FUNDEF_NEXT could change during the traversal - the pointer
         * is handled correct during REPFUNap)
         */
        DBUG_PRINT ("trav from fundef-next");
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *REPFUNex(node *arg_node, info *arg_info)
 *
 *   @brief switches the INFO_REPFUN_EXECMODE into MUTH_EXCLUSIVE
 *
 *   @param arg_node a N_ex
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
REPFUNex (node *arg_node, info *arg_info)
{
    mtexecmode_t old;
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_ex, "N_ex expected");

    /* push info */
    old = INFO_REPFUN_EXECMODE (arg_info);

    INFO_REPFUN_EXECMODE (arg_info) = MUTH_EXCLUSIVE;

    DBUG_PRINT ("trav into ex-region");
    EX_REGION (arg_node) = TRAVdo (EX_REGION (arg_node), arg_info);
    DBUG_PRINT ("trav from ex-region");

    /* pop info */
    INFO_REPFUN_EXECMODE (arg_info) = old;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *REPFUNst(node *arg_node, info *arg_info)
 *
 *   @brief switches the INFO_REPFUN_EXECMODE into MUTH_SINGLE
 *
 *   @param arg_node a N_st
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
REPFUNst (node *arg_node, info *arg_info)
{
    mtexecmode_t old;
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_st, "N_st expected");

    /* push info */
    old = INFO_REPFUN_EXECMODE (arg_info);

    INFO_REPFUN_EXECMODE (arg_info) = MUTH_SINGLE;

    DBUG_PRINT ("trav into st-region");
    ST_REGION (arg_node) = TRAVdo (ST_REGION (arg_node), arg_info);
    DBUG_PRINT ("trav from st-region");

    /* pop */
    INFO_REPFUN_EXECMODE (arg_info) = old;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *REPFUNmt(node *arg_node, info *arg_info)
 *
 *   @brief  switches the INFO_REPFUN_EXECMODE into MUTH_MULTI
 *
 *   @param arg_node a N_mt
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
REPFUNmt (node *arg_node, info *arg_info)
{
    mtexecmode_t old;
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_mt, "N_mt expected");

    /* push info */
    old = INFO_REPFUN_EXECMODE (arg_info);

    INFO_REPFUN_EXECMODE (arg_info) = MUTH_MULTI;

    DBUG_PRINT ("trav into mt-region");
    MT_REGION (arg_node) = TRAVdo (MT_REGION (arg_node), arg_info);
    DBUG_PRINT ("trav from mt-region");

    /* pop info */
    INFO_REPFUN_EXECMODE (arg_info) = old;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *REPFUNassign(node *arg_node, info *arg_info)
 *
 *   @brief stores the arg_node into INFO_REPFUN_ACTASSIGN and continues
 *          traversal into the instruction
 *
 *   @param arg_node a N_assign
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
REPFUNassign (node *arg_node, info *arg_info)
{
    node *old_actassign;
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_assign, "arg_node is no a N_assign");

    /* push info */
    old_actassign = INFO_REPFUN_ACTASSIGN (arg_info);

    INFO_REPFUN_ACTASSIGN (arg_info) = arg_node;

    DBUG_PRINT ("trav into instruction");
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
    DBUG_PRINT ("trav from instruction");

    /* pop info */
    INFO_REPFUN_ACTASSIGN (arg_info) = old_actassign;

    if (ASSIGN_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("trav into next");
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
        DBUG_PRINT ("trav from next");
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *REPFUNap(node *arg_node, info *arg_info)
 *
 *   @brief sets the AP_FUNDEF to its replication with the same executionmode
 *          as the ex/st/mt-block the traversal is in. Replicates the N_fundefs
 *          if it's neccessary
 *
 *   @param arg_node a N_ap
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
REPFUNap (node *arg_node, info *arg_info)
{
    node *my_fundef; /* shortcut to avoid AP_FUNDEF(arg_node) x-times */
    node *tmp_1, *tmp_2;
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_ap, "arg_node is no a N_ap");

    /*
     * only function applications within MUTH_ANY assignments have to be handled
     */
    if (ASSIGN_EXECMODE (INFO_REPFUN_ACTASSIGN (arg_info)) == MUTH_ANY) {
        ASSIGN_EXECMODE (INFO_REPFUN_ACTASSIGN (arg_info))
          = INFO_REPFUN_EXECMODE (arg_info);
        my_fundef = AP_FUNDEF (arg_node);

        /* EX/ST/MT-companions available => do not replicate function */

        if ((FUNDEF_COMPANION (my_fundef) == NULL)
            || (FUNDEF_EXECMODE (my_fundef) == MUTH_ANY)) {
            /* LaC-functions are handled seperatly */
            if (!FUNDEF_ISLACFUN (my_fundef)) {
                tmp_1 = DUPdoDupNode (my_fundef);
                tmp_2 = DUPdoDupNode (my_fundef);

                FUNDEF_EXECMODE (my_fundef) = MUTH_EXCLUSIVE;
                ASSIGN_EXECMODE (FUNDEF_RETURN (my_fundef)) = MUTH_EXCLUSIVE;
                FUNDEF_EXECMODE (tmp_1) = MUTH_SINGLE;
                ASSIGN_EXECMODE (FUNDEF_RETURN (tmp_1)) = MUTH_SINGLE;
                FUNDEF_EXECMODE (tmp_2) = MUTH_MULTI;
                ASSIGN_EXECMODE (FUNDEF_RETURN (tmp_2)) = MUTH_MULTI;

                my_fundef = MUTHLIBexpandFundefName (my_fundef, "__EX_");
                tmp_1 = MUTHLIBexpandFundefName (tmp_1, "__ST_");
                tmp_2 = MUTHLIBexpandFundefName (tmp_2, "__MT_");

                if (FUNDEF_COMPANION (my_fundef) == NULL) {
                    FUNDEF_NEXT (tmp_2) = FUNDEF_NEXT (my_fundef);
                    FUNDEF_NEXT (tmp_1) = tmp_2;
                    FUNDEF_NEXT (my_fundef) = tmp_1;

                    FUNDEF_COMPANION (my_fundef) = tmp_1;
                    FUNDEF_COMPANION (tmp_1) = tmp_2;
                    FUNDEF_COMPANION (tmp_2) = my_fundef;
                }
                /* there's already a specialisation (MUTH_MULTI_SPECIALIZED) */
                else {
                    DBUG_ASSERT (FUNDEF_EXECMODE (FUNDEF_COMPANION (my_fundef))
                                   == MUTH_MULTI_SPECIALIZED,
                                 "companion must have executionmode "
                                 "MUTH_MULTI_SPECIALIZED");
                    FUNDEF_NEXT (tmp_2) = FUNDEF_NEXT (FUNDEF_COMPANION (my_fundef));
                    FUNDEF_NEXT (tmp_1) = tmp_2;
                    FUNDEF_NEXT (FUNDEF_COMPANION (my_fundef)) = tmp_1;

                    FUNDEF_COMPANION (FUNDEF_COMPANION (my_fundef)) = tmp_1;
                    FUNDEF_COMPANION (tmp_1) = tmp_2;
                    FUNDEF_COMPANION (tmp_2) = my_fundef;
                }

                /* search for the fundef with correct executionmode */
                if (INFO_REPFUN_EXECMODE (arg_info) != MUTH_ANY) {
                    while (FUNDEF_EXECMODE (my_fundef)
                           != INFO_REPFUN_EXECMODE (arg_info)) {
                        my_fundef = FUNDEF_COMPANION (my_fundef);
                    }
                    AP_FUNDEF (arg_node) = my_fundef;
                }
            } else {
#if 0
        /*
         * This way copies of special functions are made, but as they are
         * inserted in the beginning of the fundef chain, they will never
         * be traversed.
         * Does that make sense?
         */
        INFO_REPFUN_MODULE(arg_info)
          = DUPcheckAndDupSpecialFundef(INFO_REPFUN_MODULE(arg_info),
                                        my_fundef,
                                        INFO_REPFUN_ACTASSIGN(arg_info));
#endif
            }
        } else {
            /* do not build replications) */
            /* search for the fundef with correct executionmode */
            if (INFO_REPFUN_EXECMODE (arg_info) != MUTH_ANY) {
                while (FUNDEF_EXECMODE (my_fundef) != INFO_REPFUN_EXECMODE (arg_info)) {
                    my_fundef = FUNDEF_COMPANION (my_fundef);
                }
                AP_FUNDEF (arg_node) = my_fundef;
            }
        }

        /* time to check the body of the function - perhaps we have to duplicate
         * somebody within it */
        DBUG_PRINT ("Duplicate: trav into function-body");
        FUNDEF_BODY (AP_FUNDEF (arg_node))
          = TRAVdo (FUNDEF_BODY (AP_FUNDEF (arg_node)), arg_info);
        DBUG_PRINT ("Duplicate: trav from function-body");
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
