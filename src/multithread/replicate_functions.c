/*
 * $Log$
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

#include "types.h"
#include "tree_basic.h"
#include "DupTree.h"
#include "traverse.h"
#include "print.h"
#include "replicate_functions.h"
#include "multithread_lib.h"

/*
 * INFO structure
 */
struct INFO {
    bool withinmulti;
    node *actassign;
    node *modul;
    mtexecmode_t execmode;
};

/*
 * INFO macros
 *    bool    REPFUN_WITHINMULTI
 *    node    REPFUN_ACTASSIGN
 *    node    REPFUN_MODUL
 *    node    REPFUN_EXECMODE
 */
#define INFO_REPFUN_WITHINMULTI(n) (n->withinmulti)
#define INFO_REPFUN_ACTASSIGN(n) (n->actassign)
#define INFO_REPFUN_MODUL(n) (n->modul)
#define INFO_REPFUN_EXECMODE(n) (n->execmode)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_REPFUN_WITHINMULTI (result) = FALSE;
    INFO_REPFUN_ACTASSIGN (result) = NULL;
    INFO_REPFUN_MODUL (result) = NULL;
    INFO_REPFUN_EXECMODE (result) = MUTH_ANY;

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
 * @fn node *ReplicateFunctions(node *arg_node)
 *
 *   @brief  Inits the traversal for this phase
 *
 *   @param arg_node a N_modul
 *   @return the N_modul with replicated, correct assigned functions
 *
 *****************************************************************************/
node *
ReplicateFunctions (node *arg_node)
{
    funtab *old_tab;
    info *arg_info;
    DBUG_ENTER ("ReplicateFunctions");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_modul),
                 "ReplicateFunctions expects a N_modul as arg_node");

    arg_info = MakeInfo ();
    /* push info ... */
    old_tab = act_tab;
    act_tab = repfun_tab;

    INFO_REPFUN_MODUL (arg_info) = arg_node;

    DBUG_PRINT ("REPFUN", ("trav into modul-funs"));
    Trav (MODUL_FUNS (arg_node), arg_info);
    DBUG_PRINT ("REPFUN", ("trav from modul-funs"));

    arg_node = INFO_REPFUN_MODUL (arg_info);

    /* pop info ... */
    act_tab = old_tab;

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

node *
REPFUNfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("REPFUNfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        /*FUNDEF_NEXT(arg_node) = */
        DBUG_PRINT ("REPFUN", ("trav into fundef-next"));
        Trav (FUNDEF_NEXT (arg_node), arg_info);
        /* (the FUNDEF_NEXT could change during the traversal - the pointer
         * is handled correct during REPFUNap)
         */
        DBUG_PRINT ("REPFUN", ("trav from fundef-next"));
    }

    DBUG_RETURN (arg_node);
}

node *
REPFUNex (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("REPFUNex");

    INFO_REPFUN_EXECMODE (arg_info) = MUTH_EXCLUSIVE;

    DBUG_PRINT ("REPFUN", ("trav into ex-region"));
    EX_REGION (arg_node) = Trav (EX_REGION (arg_node), arg_info);
    DBUG_PRINT ("REPFUN", ("trav from ex-region"));

    DBUG_RETURN (arg_node);
}

node *
REPFUNst (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("REPFUNmst");

    INFO_REPFUN_EXECMODE (arg_info) = MUTH_SINGLE;

    DBUG_PRINT ("REPFUN", ("trav into st-region"));
    ST_REGION (arg_node) = Trav (ST_REGION (arg_node), arg_info);
    DBUG_PRINT ("REPFUN", ("trav from st-region"));

    DBUG_RETURN (arg_node);
}

node *
REPFUNmt (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("REPFUNmt");

    INFO_REPFUN_EXECMODE (arg_info) = MUTH_MULTI;

    DBUG_PRINT ("REPFUN", ("trav into mt-region"));
    MT_REGION (arg_node) = Trav (MT_REGION (arg_node), arg_info);
    DBUG_PRINT ("REPFUN", ("trav from mt-region"));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *REPFUNassign(node *arg_node, info *arg_info)
 *
 *   @brief
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

    old_actassign = INFO_REPFUN_ACTASSIGN (arg_info);
    INFO_REPFUN_ACTASSIGN (arg_info) = arg_node;

    DBUG_PRINT ("REPFUN", ("trav into instruction"));
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    DBUG_PRINT ("REPFUN", ("trav from instruction"));

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
 *   @brief
 *
 *   @param arg_node a N_ap
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
REPFUNap (node *arg_node, info *arg_info)
{
    node *my_fundef;
    node *tmp_1, *tmp_2;
    DBUG_ENTER ("REPFUNap");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_ap), "arg_node is no a N_ap");

    if (ASSIGN_EXECMODE (INFO_REPFUN_ACTASSIGN (arg_info)) == MUTH_ANY) {
        ASSIGN_EXECMODE (INFO_REPFUN_ACTASSIGN (arg_info))
          = INFO_REPFUN_EXECMODE (arg_info);
        my_fundef = AP_FUNDEF (arg_node);

        /* no companions available - replicate function */
        if (FUNDEF_COMPANION (my_fundef) == NULL) {

            /* LaC-functions are handled seperatly */
            if (((FUNDEF_STATUS (my_fundef) != ST_dofun)
                 && (FUNDEF_STATUS (my_fundef) != ST_whilefun)
                 && (FUNDEF_STATUS (my_fundef) != ST_condfun))) {
                tmp_1 = DupNode (my_fundef);
                tmp_2 = DupNode (my_fundef);
                FUNDEF_NEXT (tmp_1) = tmp_2;
                FUNDEF_NEXT (tmp_2) = FUNDEF_NEXT (my_fundef);
                FUNDEF_NEXT (my_fundef) = tmp_1;

                FUNDEF_EXECMODE (my_fundef) = MUTH_EXCLUSIVE;
                FUNDEF_EXECMODE (tmp_1) = MUTH_SINGLE;
                FUNDEF_EXECMODE (tmp_2) = MUTH_MULTI;

                FUNDEF_COMPANION (my_fundef) = tmp_1;
                FUNDEF_COMPANION (tmp_1) = tmp_2;
                FUNDEF_COMPANION (tmp_2) = my_fundef;

                my_fundef = MUTHExpandFundefName (my_fundef, "__EX_");
                tmp_1 = MUTHExpandFundefName (tmp_1, "__ST_");
                tmp_2 = MUTHExpandFundefName (tmp_2, "__MT_");
            }
            /*   else {
              INFO_REPFUN_MODUL(arg_info)
                = CheckAndDupSpecialFundef(INFO_REPFUN_MODUL(arg_info),
                                           my_fundef,
                                           INFO_REPFUN_ACTASSIGN(arg_info));
              tmp = AP_FUNDEF(LET_EXPR(ASSIGN_INSTR(INFO_REPFUN_ACTASSIGN(arg_info))));
              }*/
        }

        /* search for the fundef with correct executionmode */
        while (FUNDEF_EXECMODE (my_fundef) != INFO_REPFUN_EXECMODE (arg_info)) {
            my_fundef = FUNDEF_COMPANION (my_fundef);
        }

        AP_FUNDEF (arg_node) = my_fundef;

        /* time to check the body of the function - perhaps we have to duplicate
         * somebody within it */
        DBUG_PRINT ("REPFUN", ("Duplicate: trav into function-body"));
        FUNDEF_BODY (AP_FUNDEF (arg_node))
          = Trav (FUNDEF_BODY (AP_FUNDEF (arg_node)), arg_info);
        DBUG_PRINT ("REPFUN", ("Duplicate: trav from function-body"));
    }

    DBUG_RETURN (arg_node);
}
