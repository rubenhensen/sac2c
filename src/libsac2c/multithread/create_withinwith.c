/**
 *
 * @defgroup crwiw Create Withinwith-Functions
 * @ingroup muth
 *
 * @brief all functions (and its members rekursive) which are
 *        called within a withloop are duplicated and the duplicate is
 *        MUTH_MULTI_SPECIALISED tagged
 * @{
 **/

/** <!--********************************************************************-->
 *
 * @file create_withinwith.c
 *
 * prefix: CRWIW
 *
 * description:
 *
 *****************************************************************************/

#include "tree_basic.h"
#include "tree_compound.h"
#include "DupTree.h"
#include "traverse.h"
#include "create_withinwith.h"
#include "multithread_lib.h"
#include "str.h"
#include "memory.h"

#define DBUG_PREFIX "CRWIW"
#include "debug.h"

/*
 * INFO structure
 */
struct INFO {
    bool withinmulti;
    bool createspecialized;
    bool duplicatemode;
    node *actassign;
    node *module;
};

/*
 * INFO macros
 *    bool    CRWIW_WITHINMULTI
 */
#define INFO_CRWIW_WITHINMULTI(n) (n->withinmulti)
#define INFO_CRWIW_ACTASSIGN(n) (n->actassign)
#define INFO_CRWIW_MODULE(n) (n->module)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_CRWIW_WITHINMULTI (result) = FALSE;
    INFO_CRWIW_ACTASSIGN (result) = NULL;
    INFO_CRWIW_MODULE (result) = NULL;

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
 * @fn node *CRWIWdoCreateWithinwith(node *arg_node)
 *
 *   @brief  Inits the traversal for this phase
 *
 *   @param arg_node a N_module
 *   @return the N_module with duplicated/specialized functions within
 *           multithreaded withloops
 *
 *****************************************************************************/
node *
CRWIWdoCreateWithinwith (node *arg_node)
{
    info *arg_info;
    trav_t traversaltable;
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_module,
                 "CRWIWdoCreateWithinwith expects a N_module as arg_node");

    arg_info = MakeInfo ();

    TRAVpush (TR_crwiw);

    INFO_CRWIW_MODULE (arg_info) = arg_node;

    DBUG_PRINT ("trav into module-funs");
    TRAVdo (MODULE_FUNS (arg_node), arg_info);
    DBUG_PRINT ("trav from module-funs");

    arg_node = INFO_CRWIW_MODULE (arg_info);

    traversaltable = TRAVpop ();
    DBUG_ASSERT (traversaltable == TR_crwiw, "Popped incorrect traversal table");

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

node *
CRWIWfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    FUNDEF_BODY (arg_node) = TRAVopt(FUNDEF_BODY (arg_node), arg_info);

    if (FUNDEF_NEXT (arg_node) != NULL) {
        /*FUNDEF_NEXT(arg_node) = */
        TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        /* (the FUNDEF_NEXT could change during the traversal - the pointer
         * is handled correct during CRWIWap)
         */
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CRWIWassign(node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param arg_node a N_assign
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CRWIWassign (node *arg_node, info *arg_info)
{
    bool old_flag;
    node *old_actassign;
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_assign, "arg_node is no a N_assign");

    /* set the executionmode to MUTH_MULTI_SPECIALIZED if in MULTI-environment */
    if (INFO_CRWIW_WITHINMULTI (arg_info) == TRUE) {
        ASSIGN_EXECMODE (arg_node) = MUTH_MULTI_SPECIALIZED;
    }

    /* some initialization to avoid a compiler warning */
    old_flag = FALSE;

    if (ASSIGN_EXECMODE (arg_node) == MUTH_MULTI) {
        /* set the within_multi-flag */
        old_flag = INFO_CRWIW_WITHINMULTI (arg_info);
        INFO_CRWIW_WITHINMULTI (arg_info) = TRUE;
    }

    old_actassign = INFO_CRWIW_ACTASSIGN (arg_info);
    INFO_CRWIW_ACTASSIGN (arg_info) = arg_node;

    DBUG_PRINT ("trav into instruction");
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
    DBUG_PRINT ("trav from instruction");

    if (ASSIGN_EXECMODE (arg_node) == MUTH_MULTI) {
        /* reset the within_multi-flag */
        INFO_CRWIW_WITHINMULTI (arg_info) = old_flag;
    }

    INFO_CRWIW_ACTASSIGN (arg_info) = old_actassign;

    if (ASSIGN_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("trav into next");
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
        DBUG_PRINT ("trav from next");
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CRWIWap(node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param arg_node a N_ap
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CRWIWap (node *arg_node, info *arg_info)
{
    node *my_fundef;
    node *tmp;
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_ap, "arg_node is no a N_ap");

    my_fundef = AP_FUNDEF (arg_node);

    if (INFO_CRWIW_WITHINMULTI (arg_info) == TRUE) {
        if (FUNDEF_COMPANION (my_fundef) != NULL) {
            AP_FUNDEF (arg_node) = FUNDEF_COMPANION (my_fundef);
            DBUG_ASSERT (FUNDEF_EXECMODE (AP_FUNDEF (arg_node)) == MUTH_MULTI_SPECIALIZED,
                         "my_fundef mut have execmode MUTH_MULTI_SPECIALIZED");
        }
        /* let's duplicate it */
        else {
            /* LaC-functions are handled seperatly */
            if (!FUNDEF_ISLACFUN (my_fundef)) {
                tmp = DUPdoDupNode (my_fundef);

                /*
                 * insert copied fundef into fundef chain
                 */
                FUNDEF_NEXT (tmp) = FUNDEF_NEXT (my_fundef);
                FUNDEF_NEXT (my_fundef) = tmp;

                /*
                 * immediately insert indirectly copied special fundefs into
                 * fundef chain
                 */
                FUNDEF_NEXT (tmp)
                  = TCappendFundef (DUPgetCopiedSpecialFundefs (), FUNDEF_NEXT (tmp));
            } else {
                tmp
                  = AP_FUNDEF (LET_EXPR (ASSIGN_STMT (INFO_CRWIW_ACTASSIGN (arg_info))));
            }

            FUNDEF_EXECMODE (tmp) = MUTH_MULTI_SPECIALIZED;
            tmp = MUTHLIBexpandFundefName (tmp, "__MS_");

            FUNDEF_COMPANION (my_fundef) = tmp;
            FUNDEF_COMPANION (tmp) = my_fundef;

            AP_FUNDEF (arg_node) = tmp;

            /* time to check the body of the function - perhaps we have to duplicate
             * somebody within it */
            DBUG_PRINT ("Duplicate: trav into function-body");
            FUNDEF_BODY (AP_FUNDEF (arg_node))
              = TRAVdo (FUNDEF_BODY (AP_FUNDEF (arg_node)), arg_info);
            DBUG_PRINT ("Duplicate: trav from function-body");
        }
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
