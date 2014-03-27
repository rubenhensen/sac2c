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

#include "tree_basic.h"
#include "traverse.h"
#include "propagate_executionmode.h"
#include "multithread_lib.h"
#include "str.h"
#include "memory.h"
#include "print.h"

#define DBUG_PREFIX "PEM"
#include "debug.h"

/*
 * INFO structure
 */
struct INFO {
    bool changeflag;
    bool firstflag;
    node *actualfundef;
    node *myassign;
    node *lastconditionalassignment;
    node *lastwithloopassignment;
    node *lastvisitedassign;
    node *myreturn;
};

/*
 * INFO macros
 *   bool       ANYCHANGE     (is 1, if there was a change of the executionmode
 *                             somewhere in the current traversal)
 *   bool       FIRSTTRAV     (holds the information, wheter this is the first
 *                             traversal (=>1) or not (=>0)
 *   node*      MYASSIGN      (the current assignment, the traversal.mechanism
 *                             is in)
 */
#define INFO_PEM_ANYCHANGE(n) (n->changeflag)
#define INFO_PEM_FIRSTTRAV(n) (n->firstflag)
#define INFO_PEM_ACTFUNDEF(n) (n->actualfundef)
#define INFO_PEM_MYASSIGN(n) (n->myassign)
#define INFO_PEM_LASTCONDASSIGN(n) (n->lastconditionalassignment)
#define INFO_PEM_LASTWITHASSIGN(n) (n->lastwithloopassignment)
#define INFO_PEM_RETURN(n) (n->myreturn)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_PEM_ANYCHANGE (result) = FALSE;
    INFO_PEM_FIRSTTRAV (result) = TRUE;
    INFO_PEM_ACTFUNDEF (result) = NULL;
    INFO_PEM_MYASSIGN (result) = NULL;
    INFO_PEM_LASTCONDASSIGN (result) = NULL;
    INFO_PEM_LASTWITHASSIGN (result) = NULL;
    INFO_PEM_RETURN (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

#define PEM_DEBUG 0

/* dome declarations */
static void UpdateExecmodes (node *assign, info *arg_info);

static void UpdateFundefExecmode (node *fundef, mtexecmode_t execmode);

static void UpdateCondExecmode (node *condassign, mtexecmode_t execmode);

static void UpdateWithExecmode (node *withloop_assign, mtexecmode_t execmode);

/** <!--********************************************************************-->
 *
 * @fn node *PEMdoPropagateExecutionmode(node *arg_node)
 *
 *   @brief  Inits the traversal for this phase
 *
 *   @param arg_node a N_Modul
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
PEMdoPropagateExecutionmode (node *arg_node)
{
    info *arg_info;
    trav_t traversaltable;
#if PEM_DEBUG
    int counter;
    counter = 1;
#endif
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_module,
                 "PEMdoPropagateExecutionmode expects a N_module as arg_node");

    arg_info = MakeInfo ();

    TRAVpush (TR_pem);

    do {
#if PEM_DEBUG
        fprintf (stdout, "Do another iteration.\n");
#endif

        /* some more initialisation */
        INFO_PEM_ANYCHANGE (arg_info) = FALSE;

        DBUG_PRINT ("trav into module-funs");
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
        DBUG_PRINT ("trav from module-funs");

        /* even more initialisation */
        INFO_PEM_FIRSTTRAV (arg_info) = FALSE;

#if PEM_DEBUG
        fprintf (stdout, "Done iteration no. %i\n", counter++);
#endif
    } while (INFO_PEM_ANYCHANGE (arg_info));

    traversaltable = TRAVpop ();
    DBUG_ASSERT (traversaltable == TR_pem, "Popped incorrect traversal table");

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
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef,
                 "PEMfundef expects a N_fundef as arg_node");

    INFO_PEM_ACTFUNDEF (arg_info) = arg_node;
#if PEM_DEBUG
    fprintf (stdout, "current function:\n");
    PRTdoPrintNode (arg_node);
    fprintf (stdout, "Executionmode was %s.\n",
             MUTHLIBdecodeExecmode (FUNDEF_EXECMODE (arg_node)));
#endif
    if (FUNDEF_BODY (arg_node) != NULL) {
        DBUG_PRINT ("trav into function-body");
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        DBUG_PRINT ("trav from function-body");
    }

    /* set the execmode of the return statement */
    DBUG_ASSERT (NODE_TYPE (ASSIGN_STMT (INFO_PEM_RETURN (arg_info))) == N_return,
                 "N_return as last assignment expected");
    ASSIGN_EXECMODE (INFO_PEM_RETURN (arg_info)) = FUNDEF_EXECMODE (arg_node);

    INFO_PEM_ACTFUNDEF (arg_info) = NULL;

#if PEM_DEBUG
    fprintf (stdout, "Executionmode is %s.\n",
             MUTHLIBdecodeExecmode (FUNDEF_EXECMODE (arg_node)));
#endif

    if (FUNDEF_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("trav into function-next");
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        DBUG_PRINT ("trav from function-next");
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
    mtexecmode_t my_old_execmode;
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_assign,
                 "PEMassign expects a N_assign as arg_node");

    /* push_info */
    old_assign = INFO_PEM_MYASSIGN (arg_info);
    INFO_PEM_MYASSIGN (arg_info) = arg_node;

    my_old_execmode = ASSIGN_EXECMODE (arg_node);

    DBUG_PRINT ("trav into instruction");
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
    DBUG_PRINT ("trav from instruction");

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
        DBUG_PRINT ("trav into next");
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
        DBUG_PRINT ("trav from next");
    } else {
        if (NODE_TYPE (ASSIGN_STMT (arg_node)) == N_return) {
            /* store this assignment - it's the last of a function's N_block,
             * PEMfundef will set its executionmode */
            INFO_PEM_RETURN (arg_info) = arg_node;
        } else {
            INFO_PEM_RETURN (arg_info) = NULL;
        }
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
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_ap, "PEMap expects a N_ap as argument");

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
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_cond, "PEMcond expects a N_cond as argument");

    old_lastcond = INFO_PEM_LASTCONDASSIGN (arg_info);
    INFO_PEM_LASTCONDASSIGN (arg_info) = INFO_PEM_MYASSIGN (arg_info);

    if (COND_THEN (arg_node) != NULL) {
        DBUG_PRINT ("trav into then-branch");
        COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
        DBUG_PRINT ("trav from then-branch");
    }
    if (COND_ELSE (arg_node) != NULL) {
        DBUG_PRINT ("trav into else-branch");
        COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);
        DBUG_PRINT ("trav from else-branch");
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
    mtexecmode_t old_executionmode;
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_with2,
                 "PEMwith2 expects a N_with2 as argument");

    old_lastwith2 = INFO_PEM_LASTWITHASSIGN (arg_info);
    INFO_PEM_LASTWITHASSIGN (arg_info) = INFO_PEM_MYASSIGN (arg_info);
    old_executionmode = ASSIGN_EXECMODE (INFO_PEM_MYASSIGN (arg_info));

    if (WITH2_SEGS (arg_node) != NULL) {
        DBUG_PRINT ("trav into segments");
        WITH2_SEGS (arg_node) = TRAVdo (WITH2_SEGS (arg_node), arg_info);
        DBUG_PRINT ("trav from segments");
    }
    if (WITH2_CODE (arg_node) != NULL) {
        DBUG_PRINT ("trav into with-loop-code");
        WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
        DBUG_PRINT ("trav from with-loop-code");
    }
    if (WITH2_WITHOP (arg_node) != NULL) {
        DBUG_PRINT ("trav into withops");
        WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);
        DBUG_PRINT ("trav from withops");
    }

    /* perhaps some changes appeared
       - update the allocations of the with-loop */
    if (old_executionmode != ASSIGN_EXECMODE (INFO_PEM_MYASSIGN (arg_info))) {
        MUTHLIBtagAllocs (arg_node, ASSIGN_EXECMODE (INFO_PEM_MYASSIGN (arg_info)));
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
static void
UpdateExecmodes (node *assign, info *arg_info)
{
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (assign) == N_assign,
                 "UpdateExecmodes expects a N_assign as argument");

    UpdateFundefExecmode (INFO_PEM_ACTFUNDEF (arg_info), ASSIGN_EXECMODE (assign));

    UpdateCondExecmode (INFO_PEM_LASTCONDASSIGN (arg_info), ASSIGN_EXECMODE (assign));

    UpdateWithExecmode (INFO_PEM_LASTWITHASSIGN (arg_info), ASSIGN_EXECMODE (assign));
    DBUG_RETURN ();
}

static void
UpdateFundefExecmode (node *fundef, mtexecmode_t execmode)
{
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (fundef) == N_fundef,
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
        case MUTH_MULTI_SPECIALIZED:
            DBUG_UNREACHABLE ("execmode must not be MUTH_MULTI_SPECIALIZED");
            break;
        case MUTH_ANY:
            FUNDEF_EXECMODE (fundef) = MUTH_SINGLE;
            break;

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
        case MUTH_MULTI_SPECIALIZED:
            DBUG_UNREACHABLE ("execmode must not be MUTH_MULTI_SPECIALIZED");
            break;
        case MUTH_ANY:
            FUNDEF_EXECMODE (fundef) = MUTH_MULTI;
            break;
        }
        break;
    case MUTH_MULTI_SPECIALIZED:
        break;
    default:
        DBUG_UNREACHABLE ("UpdateFundefExecmode expects a valid executionmode");
        break;
    }
    DBUG_RETURN ();
}

static void
UpdateCondExecmode (node *condassign, mtexecmode_t execmode)
{
    DBUG_ENTER ();

    if (condassign != NULL) {
        DBUG_ASSERT (NODE_TYPE (condassign) == N_assign,
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
            case MUTH_MULTI_SPECIALIZED:
                DBUG_UNREACHABLE ("execmode must not be MUTH_MULTI_SPECIALIZED");
                break;
            case MUTH_ANY:
                ASSIGN_EXECMODE (condassign) = MUTH_SINGLE;
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
            case MUTH_MULTI_SPECIALIZED:
                DBUG_UNREACHABLE ("execmode must not be MUTH_MULTI_SPECIALIZED");
                break;
            case MUTH_ANY:
                ASSIGN_EXECMODE (condassign) = MUTH_SINGLE;
                break;
            }
            break;
        case MUTH_MULTI_SPECIALIZED:
            break;
        }
    } /* if (condassign != NULL) */
    DBUG_RETURN ();
}

static void
UpdateWithExecmode (node *withloop_assign, mtexecmode_t execmode)
{
    DBUG_ENTER ();
    if (withloop_assign != NULL) {
        DBUG_ASSERT (NODE_TYPE (withloop_assign) == N_assign,
                     "UpdateWithExecmode expects a N_assign as argument");

        if (execmode == MUTH_EXCLUSIVE) {
            ASSIGN_EXECMODE (withloop_assign) = MUTH_EXCLUSIVE;
        }
    }
    DBUG_RETURN ();
}

/**
 * @}
 **/

#undef DBUG_PREFIX
