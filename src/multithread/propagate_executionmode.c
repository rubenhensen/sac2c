/*
 * $Log$
 * Revision 1.2  2004/07/15 21:45:37  skt
 * some debug-information added (and fixed a compiler warning)
 *
 * Revision 1.1  2004/07/06 12:31:30  skt
 * Initial revision
 *
 */

/**
 *
 * @defgroup pem Propagate Executionmode
 * @ingroup muth
 *
 * @brief propagates the mode of execution on an assignment
 * @{
 **/

/** <!--********************************************************************-->
 *
 * @file propagate_executionmode.c
 *
 * prefix: PEM
 *
 * description:
 *   propagates the executionmode
 *   it concerns only applications
 *
 *
 *****************************************************************************/

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "print.h"
#include "propagate_executionmode.h"
#include "multithread.h"

/** <!--********************************************************************-->
 *
 * @fn node *PropagateExecutionmode(node *arg_node, node *arg_info)
 *
 *   @brief  Inits the traversal for this phase
 *
 *   @param arg_node a N_Modul
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
PropagateExecutionmode (node *arg_node, node *arg_info)
{
    funtab *old_tab;

    DBUG_ENTER ("PropagateExecutionmode");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_modul),
                 "PropagateExecutionmode expects a N_modul as arg_node");

    /* push info ... */
    old_tab = act_tab;
    act_tab = pem_tab;

    /* some initialisation */
    INFO_PEM_FIRSTTRAV (arg_info) = 1;

    do {
        /* some more initialisation */
        INFO_PEM_ANYCHANGE (arg_info) = 0;

        DBUG_PRINT ("PEM", ("trav into modul-funs"));
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
        DBUG_PRINT ("PEM", ("trav from modul-funs"));

        /* even more initialisation */
        INFO_PEM_FIRSTTRAV (arg_info) = 0;

    } while (INFO_PEM_ANYCHANGE (arg_info));

    /* pop info ... */
    act_tab = old_tab;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PEMfundef(node *arg_node, node *arg_info)
 *
 *   @brief
 *
 *   @param arg_node a N_fundef
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
PEMfundef (node *arg_node, node *arg_info)
{
    int old_execmode;
    DBUG_ENTER ("PEMfundef");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef),
                 "PEMfundef expects a N_fundef as arg_node");

    /* initialize the execmode of the fundef */
    if (INFO_PEM_FIRSTTRAV (arg_info)) {
        FUNDEF_EXECMODE (arg_node) = MUTH_ANY;
    }
    old_execmode = FUNDEF_EXECMODE (arg_node);
    INFO_PEM_FUNEXECMODE (arg_info) = FUNDEF_EXECMODE (arg_node);

    if (FUNDEF_BODY (arg_node) != NULL) {
        DBUG_PRINT ("PEM", ("trav into function-body"));
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
        DBUG_PRINT ("PEM", ("trav from function-body"));
    }

#if PEM_DEBUG
    fprintf (stdout, "current function:\n");
    PrintNode (arg_info);
    fprintf (stdout, "Executionmode changed from %s into %s\n",
             DecodeExecmode (old_execmode),
             DecodeExecmode (INFO_PEM_FUNEXECMODE (arg_info)));
#endif
    /* set any changing if appeared */
    if (old_execmode != INFO_PEM_FUNEXECMODE (arg_info)) {
        FUNDEF_EXECMODE (arg_node) = INFO_PEM_FUNEXECMODE (arg_info);

        /* set the corresponding flag */
        INFO_PEM_ANYCHANGE (arg_info) = 1;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PEMassign(node *arg_node, node *arg_info)
 *
 *   @brief propagates the N_assign with its executionmode
 *
 *   @param arg_node a N_assign
 *   @param arg_info
 *   @return N_assign
 *
 *****************************************************************************/
node *
PEMassign (node *arg_node, node *arg_info)
{
    int old_execmode;
    DBUG_ENTER ("PEMassign");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_assign),
                 "PEMassign expects a N_assign as arg_node");

    old_execmode = ASSIGN_EXECMODE (arg_node);
    INFO_PEM_ASSIGN (arg_info) = arg_node;

    DBUG_PRINT ("TEM", ("trav into instruction"));
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    DBUG_PRINT ("TEM", ("trav from instruction"));

    /* if the executionmode of the assignment had changed, update the
       executionmode of the function */
    if (old_execmode != ASSIGN_EXECMODE (arg_node)) {
        INFO_PEM_FUNEXECMODE (arg_info)
          = UpdateFunexecmode (INFO_PEM_FUNEXECMODE (arg_info),
                               ASSIGN_EXECMODE (arg_info));
        /* set the change-flag */
        INFO_PEM_ANYCHANGE (arg_info) = 1;
    }

    if (ASSIGN_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("TEM", ("trav into next"));
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
        DBUG_PRINT ("TEM", ("trav from next"));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PEMap(node *arg_node, node *arg_info)
 *
 *   @brief
 *
 *   @param arg_node a N_ap
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
PEMap (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PEMap");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_ap), "PEMap expects a N_ap as argument");

    /*ASSIGN_EXECMODE(INFO_PEM_ASSIGN(arg_info)) =
      FUNDEF_EXECMODE(AP_FUNDEF(arg_node));*/

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn int UpdateFunexecmode(int fun_execmode, int assign_execmode)
 *
 *   @brief updates the executionmode of a function
 *
 *   @param fun_executionmode the current executionmode of the function
 *   @param assign_execmode the executionmode of the current assignment
 *   @return the new executionmode of the function
 *
 *****************************************************************************/
int
UpdateFunexecmode (int fun_execmode, int assign_execmode)
{
    int result;
    DBUG_ENTER ("UpdateFunexecmode");

    result = MUTH_EXCLUSIVE;

    switch (assign_execmode) {
    case MUTH_EXCLUSIVE:
        result = MUTH_EXCLUSIVE;
        break;
    case MUTH_MULTI:
        result = MUTH_EXCLUSIVE;
        break;
    }

    DBUG_RETURN (result);
}

#if PEM_DEBUG
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
