/*
 * $Log$
 * Revision 1.3  2004/08/31 16:59:10  skt
 * some comments added
 *
 * Revision 1.2  2004/08/26 17:05:04  skt
 * implementation finished
 *
 * Revision 1.1  2004/08/24 16:48:56  skt
 * Initial revision
 *
 */

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

#define NEW_INFO

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "DupTree.h"
#include "traverse.h"
#include "print.h"
#include "create_withinwith.h"
#include "multithread_lib.h"

/*
 * INFO structure
 */
struct INFO {
    bool withinmulti;
    bool createspecialized;
    bool duplicatemode;
    node *actassign;
    node *modul;
};

/*
 * INFO macros
 *    bool    CRWIW_WITHINMULTI
 */
#define INFO_CRWIW_WITHINMULTI(n) (n->withinmulti)
#define INFO_CRWIW_ACTASSIGN(n) (n->actassign)
#define INFO_CRWIW_MODUL(n) (n->modul)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_CRWIW_WITHINMULTI (result) = FALSE;
    INFO_CRWIW_ACTASSIGN (result) = NULL;
    INFO_CRWIW_MODUL (result) = NULL;

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
 * @fn node *CreateWithinwith(node *arg_node)
 *
 *   @brief  Inits the traversal for this phase
 *
 *   @param arg_node a N_modul
 *   @return the N_modul with duplicated/specialized functions within
 *           multithreaded withloops
 *
 *****************************************************************************/
node *
CreateWithinwith (node *arg_node)
{
    funtab *old_tab;
    info *arg_info;
    DBUG_ENTER ("CreateWithinwith");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_modul),
                 "CreateWithinwith expects a N_modul as arg_node");

    arg_info = MakeInfo ();
    /* push info ... */
    old_tab = act_tab;
    act_tab = crwiw_tab;

    INFO_CRWIW_MODUL (arg_info) = arg_node;

    DBUG_PRINT ("CRWIW", ("trav into modul-funs"));
    Trav (MODUL_FUNS (arg_node), arg_info);
    DBUG_PRINT ("CRWIW", ("trav from modul-funs"));

    arg_node = INFO_CRWIW_MODUL (arg_info);

    /* pop info ... */
    act_tab = old_tab;

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

node *
CRWIWfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CRWIWfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        /*FUNDEF_NEXT(arg_node) = */
        Trav (FUNDEF_NEXT (arg_node), arg_info);
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
    DBUG_ENTER ("CRWIWassign");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_assign), "arg_node is no a N_assign");

    /* set the executionmode to MUTH_MULTI_SPECIALIZED if in MULTI-environment */
    if (INFO_CRWIW_WITHINMULTI (arg_info) == TRUE) {
        ASSIGN_EXECMODE (arg_node) = MUTH_MULTI_SPECIALIZED;
    }

    if (ASSIGN_EXECMODE (arg_node) == MUTH_MULTI) {
        /* set the within_multi-flag */
        old_flag = INFO_CRWIW_WITHINMULTI (arg_info);
        INFO_CRWIW_WITHINMULTI (arg_info) = TRUE;
    }

    old_actassign = INFO_CRWIW_ACTASSIGN (arg_info);
    INFO_CRWIW_ACTASSIGN (arg_info) = arg_node;

    DBUG_PRINT ("CRWIW", ("trav into instruction"));
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    DBUG_PRINT ("CRWIW", ("trav from instruction"));

    if (ASSIGN_EXECMODE (arg_node) == MUTH_MULTI) {
        /* reset the within_multi-flag */
        INFO_CRWIW_WITHINMULTI (arg_info) = old_flag;
    }

    INFO_CRWIW_ACTASSIGN (arg_info) = old_actassign;

    if (ASSIGN_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("CRWIW", ("trav into next"));
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
        DBUG_PRINT ("CRWIW", ("trav from next"));
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
    DBUG_ENTER ("CRWIWap");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_ap), "arg_node is no a N_ap");

    my_fundef = AP_FUNDEF (arg_node);

    if (INFO_CRWIW_WITHINMULTI (arg_info) == TRUE) {
        if (FUNDEF_COMPANION (my_fundef) != NULL) {
            AP_FUNDEF (arg_node) = FUNDEF_COMPANION (my_fundef);
            DBUG_ASSERT ((FUNDEF_EXECMODE (AP_FUNDEF (arg_node))
                          == MUTH_MULTI_SPECIALIZED),
                         "my_fundef mut have execmode MUTH_MULTI_SPECIALIZED");
        }
        /* let's duplicate it */
        else {
            /* LaC-functions are handled seperatly */
            if (((FUNDEF_STATUS (my_fundef) != ST_dofun)
                 && (FUNDEF_STATUS (my_fundef) != ST_whilefun)
                 && (FUNDEF_STATUS (my_fundef) != ST_condfun))) {
                tmp = DupNode (my_fundef);
                FUNDEF_NEXT (tmp) = FUNDEF_NEXT (my_fundef);
                FUNDEF_NEXT (my_fundef) = tmp;
            } else {
                INFO_CRWIW_MODUL (arg_info)
                  = CheckAndDupSpecialFundef (INFO_CRWIW_MODUL (arg_info), my_fundef,
                                              INFO_CRWIW_ACTASSIGN (arg_info));
                tmp
                  = AP_FUNDEF (LET_EXPR (ASSIGN_INSTR (INFO_CRWIW_ACTASSIGN (arg_info))));
            }

            FUNDEF_EXECMODE (tmp) = MUTH_MULTI_SPECIALIZED;
            tmp = MUTHExpandFundefName (tmp, "__MS_");

            FUNDEF_COMPANION (my_fundef) = tmp;
            FUNDEF_COMPANION (tmp) = my_fundef;

            AP_FUNDEF (arg_node) = tmp;

            /* time to check the body of the function - perhaps we have to duplicate
             * somebody within it */
            DBUG_PRINT ("CRWIW", ("Duplicate: trav into function-body"));
            FUNDEF_BODY (AP_FUNDEF (arg_node))
              = Trav (FUNDEF_BODY (AP_FUNDEF (arg_node)), arg_info);
            DBUG_PRINT ("CRWIW", ("Duplicate: trav from function-body"));
        }
    }

    DBUG_RETURN (arg_node);
}
