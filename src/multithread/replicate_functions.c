/*
 * $Log$
 * Revision 1.4  2004/11/23 14:38:13  skt
 * SACDevCampDK 2k4
 *
 * Revision 1.3  2004/09/02 16:01:35  skt
 * comments added
 *
 * Revision 1.2  2004/09/01 16:02:27  skt
 * implementation finished
 *
 * Revision 1.1  2004/08/31 16:56:18  skt
 * Initial revision
 *
 */

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

#define NEW_INFO

#include "dbug.h"

#include "tree_basic.h"
#include "DupTree.h"
#include "traverse.h"
#include "replicate_functions.h"
#include "multithread_lib.h"

/*
 * INFO structure
 */
struct INFO {
    node *actassign;
    node *module;
    mtexecmode_t execmode;
};

/*
 * INFO macros
 *    node    REPFUN_ACTASSIGN
 *    node    REPFUN_MODUL
 *    node    REPFUN_EXECMODE
 */
#define INFO_REPFUN_ACTASSIGN(n) (n->actassign)
#define INFO_REPFUN_MODULE(n) (n->module)
#define INFO_REPFUN_EXECMODE(n) (n->execmode)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_REPFUN_ACTASSIGN (result) = NULL;
    INFO_REPFUN_MODULE (result) = NULL;
    INFO_REPFUN_EXECMODE (result) = MUTH_ANY;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

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
    funtab *old_tab;
    info *arg_info;
    DBUG_ENTER ("REPFUNdoReplicateFunctions");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_module),
                 "REPFUNdoReplicateFunctions expects a N_module as arg_node");

    arg_info = MakeInfo ();
    /* push info ... */
    old_tab = act_tab;
    act_tab = repfun_tab;

    INFO_REPFUN_MODULE (arg_info) = arg_node;

    DBUG_PRINT ("REPFUN", ("trav into modul-funs"));
    Trav (MODUL_FUNS (arg_node), arg_info);
    DBUG_PRINT ("REPFUN", ("trav from modul-funs"));

    arg_node = INFO_REPFUN_MODULE (arg_info);

    /* pop info ... */
    act_tab = old_tab;

    arg_info = FreeInfo (arg_info);

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
    DBUG_ENTER ("REPFUNfundef");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef), "N_fundef expected");

    if (FUNDEF_BODY (arg_node) != NULL) {
        if (FUNDEF_EXECMODE (arg_node) != MUTH_ANY) {
            FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
        }
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("REPFUN", ("trav into fundef-next"));
        /*
         * FUNDEF_NEXT(arg_node) =
         */
        Trav (FUNDEF_NEXT (arg_node), arg_info);
        /*
         * (the FUNDEF_NEXT could change during the traversal - the pointer
         * is handled correct during REPFUNap)
         */
        DBUG_PRINT ("REPFUN", ("trav from fundef-next"));
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
    DBUG_ENTER ("REPFUNex");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_ex), "N_ex expected");

    /* push info */
    old = INFO_REPFUN_EXECMODE (arg_info);

    INFO_REPFUN_EXECMODE (arg_info) = MUTH_EXCLUSIVE;

    DBUG_PRINT ("REPFUN", ("trav into ex-region"));
    EX_REGION (arg_node) = Trav (EX_REGION (arg_node), arg_info);
    DBUG_PRINT ("REPFUN", ("trav from ex-region"));

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
    DBUG_ENTER ("REPFUNst");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_st), "N_st expected");

    /* push info */
    old = INFO_REPFUN_EXECMODE (arg_info);

    INFO_REPFUN_EXECMODE (arg_info) = MUTH_SINGLE;

    DBUG_PRINT ("REPFUN", ("trav into st-region"));
    ST_REGION (arg_node) = Trav (ST_REGION (arg_node), arg_info);
    DBUG_PRINT ("REPFUN", ("trav from st-region"));

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
    DBUG_ENTER ("REPFUNmt");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_mt), "N_mt expected");

    /* push info */
    old = INFO_REPFUN_EXECMODE (arg_info);

    INFO_REPFUN_EXECMODE (arg_info) = MUTH_MULTI;

    DBUG_PRINT ("REPFUN", ("trav into mt-region"));
    MT_REGION (arg_node) = Trav (MT_REGION (arg_node), arg_info);
    DBUG_PRINT ("REPFUN", ("trav from mt-region"));

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
    DBUG_ENTER ("REPFUNassign");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_assign), "arg_node is no a N_assign");

    /* push info */
    old_actassign = INFO_REPFUN_ACTASSIGN (arg_info);

    INFO_REPFUN_ACTASSIGN (arg_info) = arg_node;

    DBUG_PRINT ("REPFUN", ("trav into instruction"));
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    DBUG_PRINT ("REPFUN", ("trav from instruction"));

    /* pop info */
    INFO_REPFUN_ACTASSIGN (arg_info) = old_actassign;

    if (ASSIGN_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("REPFUN", ("trav into next"));
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
        DBUG_PRINT ("REPFUN", ("trav from next"));
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
    bool build_replications;
    node *my_fundef; /* shortcut to avoid AP_FUNDEF(arg_node) x-times */
    node *tmp_1, *tmp_2;
    DBUG_ENTER ("REPFUNap");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_ap), "arg_node is no a N_ap");

    /*
     * only function applications within MUTH_ANY assignments have to be handled
     */
    if (ASSIGN_EXECMODE (INFO_REPFUN_ACTASSIGN (arg_info)) == MUTH_ANY) {
        ASSIGN_EXECMODE (INFO_REPFUN_ACTASSIGN (arg_info))
          = INFO_REPFUN_EXECMODE (arg_info);
        my_fundef = AP_FUNDEF (arg_node);

        /* some initialization */
        build_replications = TRUE;

        /* EX/ST/MT-companions available => do not replicate function */
        if (FUNDEF_COMPANION (my_fundef) != NULL) {
            if (FUNDEF_EXECMODE (my_fundef) != MUTH_ANY) {
                build_replications = FALSE;
            }
        }

        if (build_replications == TRUE) {
            /* LaC-functions are handled seperatly */
            if (!FUNDEF_ISDOFUN (my_fundef) && !FUNDEF_ISCONDFUN (my_fundef)) {
                tmp_1 = DUPdupNode (my_fundef);
                tmp_2 = DUPdupNode (my_fundef);

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
                    DBUG_ASSERT ((FUNDEF_EXECMODE (FUNDEF_COMPANION (my_fundef))
                                  == MUTH_MULTI_SPECIALIZED),
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
                INFO_REPFUN_MODULE (arg_info)
                  = CheckAndDupSpecialFundef (INFO_REPFUN_MODULE (arg_info), my_fundef,
                                              INFO_REPFUN_ACTASSIGN (arg_info));
            }
        }
        /* => build_replications == FALSE) */
        else {
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
        DBUG_PRINT ("REPFUN", ("Duplicate: trav into function-body"));
        FUNDEF_BODY (AP_FUNDEF (arg_node))
          = Trav (FUNDEF_BODY (AP_FUNDEF (arg_node)), arg_info);
        DBUG_PRINT ("REPFUN", ("Duplicate: trav from function-body"));
    }

    DBUG_RETURN (arg_node);
}
