/*
 * $Log$
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

#define MUTH_MULTI_SPECIALIZED 4

/*
 * INFO structure
 */
struct INFO {
    bool withinmulti;
    bool createspecialized;
    bool duplicatemode;
};

/*
 * INFO macros
 *    bool    CRWIW_WITHINMULTI
 *    bool    CRWIW_CREATESPECIALIZED
 *    bool    CRWIW_DUPLICATEMODE
 */
#define INFO_CRWIW_WITHINMULTI(n) (n->withinmulti)
#define INFO_CRWIW_CREATESPECIALIZED(n) (n->createspecialized)
#define INFO_CRWIW_DUPLICATEMODE(n) (n->duplicatemode)

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
    INFO_CRWIW_CREATESPECIALIZED (result) = FALSE;
    INFO_CRWIW_DUPLICATEMODE (result) = FALSE;

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
 *   @return the N_modul with duplicated/specialised functions within
 *           multithreaded withloops
 *
 *****************************************************************************/
node *
CreateWithinwith (node *arg_node)
{
    funtab *old_tab;
    info *arg_info;
    DBUG_ENTER ("CellGrowth");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_modul),
                 "CellGrowth expects a N_modul as arg_node");

    arg_info = MakeInfo ();
    /* push info ... */
    old_tab = act_tab;
    act_tab = crwiw_tab;
    fprintf (stdout, "Hello again\n");
    DBUG_PRINT ("CRWIW", ("trav into modul-funs"));
    MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    DBUG_PRINT ("CRWIW", ("trav from modul-funs"));

    /* pop info ... */
    act_tab = old_tab;

    arg_info = FreeInfo (arg_info);

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
    DBUG_ENTER ("CRWIWassign");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_assign), "arg_node is no a N_assign");

    if (INFO_CRWIW_DUPLICATEMODE (arg_info) == FALSE) {
        /* traverse only into MUTH_MULTI-assignments, cause you are on the hunt
         * for with-loops, that will be executed parallel*/
        if ((ASSIGN_EXECMODE (arg_node) == MUTH_MULTI)
            && (ASSIGN_INSTR (arg_node) != NULL)) {

            /* set the within_multi-flag */
            INFO_CRWIW_WITHINMULTI (arg_info) = TRUE;

            DBUG_PRINT ("CRWIW", ("trav into instruction"));
            ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
            DBUG_PRINT ("CRWIW", ("trav from instruction"));

            /* reset the within_multi-flag */
            INFO_CRWIW_WITHINMULTI (arg_info) = FALSE;
        } else if (INFO_CRWIW_WITHINMULTI (arg_info) == TRUE) {
            DBUG_PRINT ("CRWIW", ("trav into instruction"));
            ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
            DBUG_PRINT ("CRWIW", ("trav from instruction"));
        }
    } else {
        DBUG_PRINT ("CRWIW", ("Duplicate :trav into instruction"));
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
        DBUG_PRINT ("CRWIW", ("Duplicate: trav from instruction"));
    }

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
    DBUG_ENTER ("CRWIWap");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_ap), "arg_node is no a N_ap");

    fprintf (stdout, "Hello again\n");

    if (INFO_CRWIW_DUPLICATEMODE (arg_info) == FALSE) {
        /* replicate the function, if needed */
        AP_FUNDEF (arg_node) = CRWIWBuildReplication (AP_FUNDEF (arg_node), arg_info);
    } else {
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CRWIWBuildReplication(node *fundef, info *arg_info)
 *
 *   @brief
 *
 *   @param node a N_fundef
 *   @param info the traversal INFO-struct
 *   @return
 *
 *****************************************************************************/
node *
CRWIWBuildReplication (node *fundef, info *arg_info)
{
    node *result;
    DBUG_ENTER ("CRWIWBuildReplication");

    /* first: do we really need to build a replication of this function? */

    if (((FUNDEF_COMPANION (fundef) == NULL)
         && (INFO_CRWIW_WITHINMULTI (arg_info) == FALSE))
        || ((FUNDEF_COMPANION (fundef) == fundef)
            && (((FUNDEF_EXECMODE (fundef) != MUTH_MULTI_SPECIALIZED)
                 && (INFO_CRWIW_WITHINMULTI (arg_info) == FALSE))
                || ((FUNDEF_EXECMODE (fundef) == MUTH_MULTI_SPECIALIZED)
                    && (INFO_CRWIW_WITHINMULTI (arg_info) == TRUE))))) {
        /* no replication needed */
        result = fundef;
    } else if (FUNDEF_COMPANION (fundef) != NULL) {
        /* no replication needed, too, but we have to figure out, which of these
         * both functions fit */
        if (((FUNDEF_EXECMODE (fundef) == MUTH_MULTI_SPECIALIZED)
             && (INFO_CRWIW_WITHINMULTI (arg_info) == TRUE))
            || ((FUNDEF_EXECMODE (fundef) != MUTH_MULTI_SPECIALIZED)
                && (INFO_CRWIW_WITHINMULTI (arg_info) == FALSE))) {
            result = fundef;
        } else {
            result = FUNDEF_COMPANION (fundef);
        }
    } else {
        /* 1. companion== NULL   && WITHINMULTI == TRUE
         * 2. companion== fundef && WITHINMULTI == TRUE  && EXECMODE != SPECIALIZED
         * 3. companion== fundef && WITHINMULTI == FALSE && EXECMODE == SPECIALIZED
         */

        if (FUNDEF_COMPANION (fundef) == NULL) {
            /* 1. */
            result = fundef;
            FUNDEF_COMPANION (result) = result;
        } else {
            /* 2. & 3. */
            result = DupNode (fundef);
            FUNDEF_COMPANION (result) = fundef;
            FUNDEF_COMPANION (fundef) = result;
        }

        if (INFO_CRWIW_WITHINMULTI (arg_info) == TRUE) {
            /* 1. & 2. */
            FUNDEF_EXECMODE (result) = MUTH_MULTI_SPECIALIZED;
            INFO_CRWIW_CREATESPECIALIZED (arg_info) = TRUE;
            result = MUTHExpandFundefName (result, "__MULTI__");
        } else {
            /* 3. */
            FUNDEF_EXECMODE (result) = MUTH_ANY;
            INFO_CRWIW_CREATESPECIALIZED (arg_info) = FALSE;
            result = MUTHExpandFundefName (result, "__SINGLE__");
        }

        FUNDEF_NEXT (result) = FUNDEF_NEXT (fundef);
        FUNDEF_NEXT (fundef) = result;

        INFO_CRWIW_DUPLICATEMODE (arg_info) = TRUE;
        /*
        DBUG_PRINT("CRWIW",("Duplicate: trav into function-body"));
        FUNDEF_BODY(result) = Trav(FUNDEF_BODY(result), arg_info);
        DBUG_PRINT("CRWIW",("Duplicate: trav from function-body"));
        */
        INFO_CRWIW_DUPLICATEMODE (arg_info) = FALSE;
    }

    DBUG_RETURN (result);
}
