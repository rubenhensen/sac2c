/*
 * $Log$
 * Revision 1.6  2004/08/05 17:42:19  skt
 * moved handling of the allocation around the withloop from create_cells
 *
 * Revision 1.5  2004/08/05 13:50:18  skt
 * welcome to the new INFO structure
 *
 * Revision 1.4  2004/07/29 13:25:59  skt
 * fixed the "doubled withop"-bug
 *
 * Revision 1.3  2004/07/23 10:05:46  skt
 * complete redesign
 *
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
 *   it concerns only applications and conditionals
 *
 *
 *****************************************************************************/

#define NEW_INFO

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "print.h"
#include "propagate_executionmode.h"
#include "multithread.h"
#include "multithread_lib.h"

/*
 * INFO structure
 */
struct INFO {
    int changeflag;
    int firstflag;
    node *actualfundef;
    node *myassign;
    node *lastconditionalassignment;
    node *lastwithloopassignment;
};

/*
 * INFO macros
 *   int        ANYCHANGE     (is 1, if there was a change of the executionmode
 *                             somewhere in the current traversal)
 *   int        FIRSTTRAV     (holds the information, wheter this is the first
 *                             traversal (=>1) or not (=>0)
 *   int        FUNEXECMODE   (holds the current executionmode of the function)
 *   node*      ASSIGN        (the current assignment, the traversal.mechanism
 *                             is in)
 */
#define INFO_PEM_ANYCHANGE(n) (n->changeflag)
#define INFO_PEM_FIRSTTRAV(n) (n->firstflag)
#define INFO_PEM_ACTFUNDEF(n) (n->actualfundef)
#define INFO_PEM_MYASSIGN(n) (n->myassign)
#define INFO_PEM_LASTCONDASSIGN(n) (n->lastconditionalassignment)
#define INFO_PEM_LASTWITHASSIGN(n) (n->lastwithloopassignment)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_PEM_ANYCHANGE (result) = FALSE;
    INFO_PEM_FIRSTTRAV (result) = TRUE;
    INFO_PEM_ACTFUNDEF (result) = NULL;
    INFO_PEM_MYASSIGN (result) = NULL;
    INFO_PEM_LASTCONDASSIGN (result) = NULL;
    INFO_PEM_LASTWITHASSIGN (result) = NULL;

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
 * @fn node *PropagateExecutionmode(node *arg_node)
 *
 *   @brief  Inits the traversal for this phase
 *
 *   @param arg_node a N_Modul
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
PropagateExecutionmode (node *arg_node)
{
    funtab *old_tab;
    info *arg_info;
#if PEM_DEBUG
    int counter;
    counter = 1;
#endif
    DBUG_ENTER ("PropagateExecutionmode");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_modul),
                 "PropagateExecutionmode expects a N_modul as arg_node");

    arg_info = MakeInfo ();

    /* push info ... */
    old_tab = act_tab;
    act_tab = pem_tab;

    do {
#if PEM_DEBUG
        fprintf (stdout, "Do another iteration.\n");
#endif

        /* some more initialisation */
        INFO_PEM_ANYCHANGE (arg_info) = FALSE;

        DBUG_PRINT ("PEM", ("trav into modul-funs"));
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
        DBUG_PRINT ("PEM", ("trav from modul-funs"));

        /* even more initialisation */
        INFO_PEM_FIRSTTRAV (arg_info) = FALSE;

#if PEM_DEBUG
        fprintf (stdout, "Done iteration no. %i\n", counter++);
#endif
    } while (INFO_PEM_ANYCHANGE (arg_info));

    /* pop info ... */
    act_tab = old_tab;
    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PEMfundef(node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param arg_node a N_fundef
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
PEMfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PEMfundef");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef),
                 "PEMfundef expects a N_fundef as arg_node");

    INFO_PEM_ACTFUNDEF (arg_info) = arg_node;
#if PEM_DEBUG
    fprintf (stdout, "current function:\n");
    PrintNode (arg_node);
    fprintf (stdout, "Executionmode was %s.\n",
             DecodeExecmode (FUNDEF_EXECMODE (arg_node)));
#endif
    if (FUNDEF_BODY (arg_node) != NULL) {
        DBUG_PRINT ("PEM", ("trav into function-body"));
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
        DBUG_PRINT ("PEM", ("trav from function-body"));
    }

    INFO_PEM_ACTFUNDEF (arg_info) = NULL;

#if PEM_DEBUG
    fprintf (stdout, "Executionmode is %s.\n",
             DecodeExecmode (FUNDEF_EXECMODE (arg_node)));
#endif

    if (FUNDEF_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("PEM", ("trav into function-next"));
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
        DBUG_PRINT ("PEM", ("trav from function-next"));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PEMassign(node *arg_node, info *arg_info)
 *
 *   @brief propagates the N_assign with its executionmode
 *
 *   @param arg_node a N_assign
 *   @param arg_info
 *   @return N_assign
 *
 *****************************************************************************/
node *
PEMassign (node *arg_node, info *arg_info)
{
    node *old_assign;
    int my_old_execmode;
    DBUG_ENTER ("PEMassign");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_assign),
                 "PEMassign expects a N_assign as arg_node");

    /* push_info */
    old_assign = INFO_PEM_MYASSIGN (arg_info);
    INFO_PEM_MYASSIGN (arg_info) = arg_node;

    my_old_execmode = ASSIGN_EXECMODE (arg_node);

    DBUG_PRINT ("PEM", ("trav into instruction"));
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    DBUG_PRINT ("PEM", ("trav from instruction"));

    /* if the executionmode of the assignment had changed or we are on our first
       traversal, update the executionmode of the function */
    if ((my_old_execmode != ASSIGN_EXECMODE (arg_node))
        || (INFO_PEM_FIRSTTRAV (arg_info) == TRUE)) {
        UpdateExecmodes (arg_node, arg_info);
        /* set the change-flag */
        INFO_PEM_ANYCHANGE (arg_info) = TRUE;
    }

    /* pop_info */
    INFO_PEM_MYASSIGN (arg_info) = old_assign;

    if (ASSIGN_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("PEM", ("trav into next"));
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
        DBUG_PRINT ("PEM", ("trav from next"));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PEMap(node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param arg_node a N_ap
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
PEMap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PEMap");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_ap), "PEMap expects a N_ap as argument");

    if (FUNDEF_BODY (AP_FUNDEF (arg_node)) == NULL) {
        ASSIGN_EXECMODE (INFO_PEM_MYASSIGN (arg_info)) = MUTH_EXCLUSIVE;
    } else {
        ASSIGN_EXECMODE (INFO_PEM_MYASSIGN (arg_info))
          = FUNDEF_EXECMODE (AP_FUNDEF (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PEMcond(node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param arg_node a N_cond
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
PEMcond (node *arg_node, info *arg_info)
{
    node *old_lastcond;
    DBUG_ENTER ("PEMcond");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_cond),
                 "PEMcond expects a N_cond as argument");

    old_lastcond = INFO_PEM_LASTCONDASSIGN (arg_info);
    INFO_PEM_LASTCONDASSIGN (arg_info) = INFO_PEM_MYASSIGN (arg_info);

    if (COND_THEN (arg_node) != NULL) {
        DBUG_PRINT ("PEM", ("trav into then-branch"));
        ASSIGN_NEXT (arg_node) = Trav (COND_THEN (arg_node), arg_info);
        DBUG_PRINT ("PEM", ("trav from then-branch"));
    }
    if (COND_ELSE (arg_node) != NULL) {
        DBUG_PRINT ("PEM", ("trav into else-branch"));
        ASSIGN_NEXT (arg_node) = Trav (COND_THEN (arg_node), arg_info);
        DBUG_PRINT ("PEM", ("trav from else-branch"));
    }

    INFO_PEM_LASTCONDASSIGN (arg_info) = old_lastcond;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PEMwith2(node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param arg_node a N_with2
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
PEMwith2 (node *arg_node, info *arg_info)
{
    node *old_lastwith2;
    int old_executionmode;
    DBUG_ENTER ("PEMwith2");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_Nwith2),
                 "PEMwith2 expects a N_with2 as argument");

    old_lastwith2 = INFO_PEM_LASTWITHASSIGN (arg_info);
    INFO_PEM_LASTWITHASSIGN (arg_info) = INFO_PEM_MYASSIGN (arg_info);
    old_executionmode = ASSIGN_EXECMODE (INFO_PEM_MYASSIGN (arg_info));

    if (NWITH2_SEGS (arg_node) != NULL) {
        DBUG_PRINT ("PEM", ("trav into segments"));
        NWITH2_SEGS (arg_node) = Trav (NWITH2_SEGS (arg_node), arg_info);
        DBUG_PRINT ("PEM", ("trav from segments"));
    }
    if (NWITH2_CODE (arg_node) != NULL) {
        DBUG_PRINT ("PEM", ("trav into with-loop-code"));
        NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);
        DBUG_PRINT ("PEM", ("trav from with-loop-code"));
    }

    if (NWITH2_WITHOP (arg_node) != NULL) {
        DBUG_PRINT ("PEM", ("trav into withops"));
        NWITH2_WITHOP (arg_node) = Trav (NWITH2_WITHOP (arg_node), arg_info);
        DBUG_PRINT ("PEM", ("trav from withops"));
    }

    /* perhaps some changes appeared
       - update the allocations of the with-loop */
    if (old_executionmode != ASSIGN_EXECMODE (INFO_PEM_MYASSIGN (arg_info))) {
        TagAllocs (arg_node, ASSIGN_EXECMODE (INFO_PEM_MYASSIGN (arg_info)));
    }

    INFO_PEM_LASTWITHASSIGN (arg_info) = old_lastwith2;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn void UpdateExecmodes(node *assign, info *arg_info)
 *
 *   @brief
 *
 *   @param arg_info
 *   @param assign
 *   @return nothing at all
 *
 *****************************************************************************/
void
UpdateExecmodes (node *assign, info *arg_info)
{
    DBUG_ENTER ("UpdateExecmodes");
    DBUG_ASSERT ((NODE_TYPE (assign) == N_assign),
                 "UpdateExecmodes expects a N_assign as argument");

    UpdateFundefExecmode (INFO_PEM_ACTFUNDEF (arg_info), ASSIGN_EXECMODE (assign));

    UpdateCondExecmode (INFO_PEM_LASTCONDASSIGN (arg_info), ASSIGN_EXECMODE (assign));

    UpdateWithExecmode (INFO_PEM_LASTWITHASSIGN (arg_info), ASSIGN_EXECMODE (assign));
    DBUG_VOID_RETURN;
}

void
UpdateFundefExecmode (node *fundef, int execmode)
{
    DBUG_ENTER ("UpdateFundefExecmode");
    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "UpdateFundefExecmode expects a N_fundef as argument");

    switch (execmode) {
    case MUTH_ANY:
        break;
    case MUTH_EXCLUSIVE:
        FUNDEF_EXECMODE (fundef) = MUTH_EXCLUSIVE;
        break;
    case MUTH_SINGLE:
        switch (FUNDEF_EXECMODE (fundef)) {
        case MUTH_EXCLUSIVE:
            break;
        case MUTH_SINGLE:
            break;
        case MUTH_MULTI:
            FUNDEF_EXECMODE (fundef) = MUTH_EXCLUSIVE;
            break;
        case MUTH_ANY:
            FUNDEF_EXECMODE (fundef) = MUTH_SINGLE;
            break;
        default:
            DBUG_ASSERT (0, "fundef has an invalid executionmode");
            break;
        }
        break;
    case MUTH_MULTI:
        switch (FUNDEF_EXECMODE (fundef)) {
        case MUTH_EXCLUSIVE:
            break;
        case MUTH_SINGLE:
            FUNDEF_EXECMODE (fundef) = MUTH_EXCLUSIVE;
            break;
        case MUTH_MULTI:
            break;
        case MUTH_ANY:
            FUNDEF_EXECMODE (fundef) = MUTH_MULTI;
            break;
        default:
            DBUG_ASSERT (0, "fundef has an invalid executionmode");
            break;
        }
        break;
    default:
        DBUG_ASSERT (0, "UpdateFundefExecmode expects a valid executionmode");
        break;
    }
    DBUG_VOID_RETURN;
}

void
UpdateCondExecmode (node *condassign, int execmode)
{
    DBUG_ENTER ("UpdateCondExecmode");

    if (condassign != NULL) {
        DBUG_ASSERT ((NODE_TYPE (condassign) == N_assign),
                     "UpdateCondExecmode expects a N_assign as argument");

        switch (execmode) {
        case MUTH_ANY:
            break;
        case MUTH_EXCLUSIVE:
            ASSIGN_EXECMODE (condassign) = MUTH_EXCLUSIVE;
            break;
        case MUTH_SINGLE:
            switch (ASSIGN_EXECMODE (condassign)) {
            case MUTH_EXCLUSIVE:
                break;
            case MUTH_SINGLE:
                break;
            case MUTH_MULTI:
                ASSIGN_EXECMODE (condassign) = MUTH_EXCLUSIVE;
                break;
            case MUTH_ANY:
                ASSIGN_EXECMODE (condassign) = MUTH_SINGLE;
                break;
            default:
                DBUG_ASSERT (0, "condassign has an invalid executionmode");
                break;
            }
            break;
        case MUTH_MULTI:
            switch (ASSIGN_EXECMODE (condassign)) {
            case MUTH_EXCLUSIVE:
                break;
            case MUTH_SINGLE:
                ASSIGN_EXECMODE (condassign) = MUTH_EXCLUSIVE;
                break;
            case MUTH_MULTI:
                break;
            case MUTH_ANY:
                ASSIGN_EXECMODE (condassign) = MUTH_SINGLE;
                break;
            default:
                DBUG_ASSERT (0, "condassign has an invalid executionmode");
                break;
            }
        default:
            DBUG_ASSERT (0, "UpdateCondExecmode expects a valid executionmode");
            break;
        }
    } /* if (condassign != NULL) */
    DBUG_VOID_RETURN;
}

void
UpdateWithExecmode (node *withloop_assign, int execmode)
{
    DBUG_ENTER ("UpdateWithExecmode");
    if (withloop_assign != NULL) {
        DBUG_ASSERT ((NODE_TYPE (withloop_assign) == N_assign),
                     "UpdateWithExecmode expects a N_assign as argument");

        if (execmode == MUTH_EXCLUSIVE) {
            ASSIGN_EXECMODE (withloop_assign) = MUTH_EXCLUSIVE;
        }
    }
    DBUG_VOID_RETURN;
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
        break;
    }
    return "NN";
}
#endif

/**
 * @}
 **/
