/*
 *
 * $Id$
 */

/**
 *
 * @file pattern_match.c
 *
 */

#include "dbug.h"

#include "print.h"
#include "globals.h"
#include "free.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"

static char *FAIL = "";

/** <!--*********************************************************************-->
 *
 * local helper functions:
 */

/** <!--*******************************************************************-->
 *
 * @fn node *ExtractOneArg( node *stack, node * args)
 *
 * @brief extracts the first argument from the exprs stack.
 *        Note here, that stack can either be N_set (stack), an N_exprs or
 *        any other expression! N_set nodes that become redundant
 *        are being freed!
 * @param stack: stack of exprs
 * @return via args the first expression in the chain and the rest of the
 *         stack via the normal return value.
 *****************************************************************************/
static node *
ExtractOneArg (node *stack, node **arg)
{
    node *next;
#ifndef DBUG_OFF
    FILE *mem_outfile;
#endif

    DBUG_ENTER ("ExtractOneArg");
    DBUG_ASSERT (stack != NULL, ("ExtractOneArg called with NULL stack!"));

    if (NODE_TYPE (stack) == N_set) {
        next = ExtractOneArg (SET_MEMBER (stack), arg);
        if (next != NULL) {
            SET_MEMBER (stack) = next;
        } else {
            stack = FREEdoFreeNode (stack);
        }
    } else {
        if (NODE_TYPE (stack) == N_exprs) {
            *arg = EXPRS_EXPR (stack);
            stack = EXPRS_NEXT (stack);
        } else {
            *arg = stack;
            stack = NULL;
        }
        DBUG_PRINT ("PM", ("argument found:"));
        DBUG_EXECUTE ("PM", mem_outfile = global.outfile; global.outfile = stderr;
                      PRTdoPrint (*arg); global.outfile = mem_outfile;);
    }
    DBUG_RETURN (stack);
}

/** <!--*******************************************************************-->
 *
 * @fn node *PushArgs( node *stack, node * args)
 *
 * @brief if stack is non-empty, it pushes the args on top of the existing
 *        args in stack by means of N_set nodes. Note here, that stack
 *        can either be N_set (existing stack) N_exprs or any other expression!
 * @param stack: stack of exprs
 *        args: exprs to be stacked on top
 * @return stacked exprs
 *****************************************************************************/
static node *
PushArgs (node *stack, node *args)
{
    DBUG_ENTER ("PushArgs");
    if (stack == NULL) {
        stack = args;
    } else if (NODE_TYPE (stack) == N_set) {
        stack = TBmakeSet (args, stack);
    } else {
        stack = TBmakeSet (args, TBmakeSet (stack, NULL));
    }
    DBUG_RETURN (stack);
}

/** <!--*******************************************************************-->
 *
 * @fn node *FollowId( node * arg)
 *
 * @brief looks behind the definitions of N_id nodes if possible
 * @param arg: potential N_id node to be followed after
 * @return defining expression iff available; unmodified arg otherwise
 *****************************************************************************/
static node *
FollowId (node *arg)
{
#ifndef DBUG_OFF
    FILE *mem_outfile;
#endif

    DBUG_ENTER ("FollowId");

    DBUG_PRINT ("PM", ("trying to look up the variable definition "));
    while ((NODE_TYPE (arg) == N_id) && (AVIS_SSAASSIGN (ID_AVIS (arg)) != NULL)) {
        DBUG_PRINT ("PM", ("looking up definition of the variable"));
        arg = LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (arg))));
        DBUG_PRINT ("PM", ("definition found:"));
        DBUG_EXECUTE ("PM", mem_outfile = global.outfile; global.outfile = stderr;
                      PRTdoPrint (arg); global.outfile = mem_outfile;);
    }
    DBUG_RETURN (arg);
}

/** <!--*******************************************************************-->
 *
 * @fn node *FailMatch( node *stack)
 *
 * @brief cleans up the remaining stack and creates a FAIL node
 * @param stack: stack of exprs
 * @return FAIL node
 *****************************************************************************/
static node *
FailMatch (node *stack)
{
    DBUG_ENTER ("FailMatch");
    DBUG_PRINT ("PM", ("match failed!"));
    if ((stack != NULL) && (NODE_TYPE (stack) == N_set)) {
        stack = FREEdoFreeTree (stack);
    }
    DBUG_RETURN ((node *)FAIL);
}

/** <!--*********************************************************************-->
 *
 * Exported functions for pattern matching:
 */

/** <!--*******************************************************************-->
 *
 * @fn bool PM( node *stack)
 *
 * @brief checks the result of a pattern match for failure
 * @param stack: resulting stack of a match
 * @return success
 *****************************************************************************/
bool
PM (node *stack)
{
    DBUG_ENTER ("PM");
    DBUG_RETURN (stack != (node *)FAIL);
}

/** <!--*******************************************************************-->
 *
 * @fn node *PMvar( node **var, node *stack)
 *
 * @brief tries to match against a variable. If *var is NULL, the top of
 *        the stack is bound to it (provided it is an N_id).
 *        If *var is bound already, it only matches if both N_id nodes
 *        point top the same N_avis.
 * @param *var: bound N_id (if any)
 *        stack: "stack" of exprs.
 * @return shortened stack.
 *****************************************************************************/
node *
PMvar (node **var, node *stack)
{
    node *arg;

    DBUG_ENTER ("PMvar");
    if (*var == NULL) {
        DBUG_PRINT ("PM", ("trying to match unbound variable..."));
    } else {
        DBUG_PRINT ("PM", ("trying to match bound variable..."));
    }

    if (stack != (node *)FAIL) {
        stack = ExtractOneArg (stack, &arg);
        if (NODE_TYPE (arg) == N_id) {
            if (*var == NULL) {
                DBUG_PRINT ("PM", ("binding variable!"));
                *var = arg;
            } else if (ID_AVIS (*var) == ID_AVIS (arg)) {
                DBUG_PRINT ("PM", ("found variable matches bound one!"));
            } else {
                stack = FailMatch (stack);
            }
        } else {
            stack = FailMatch (stack);
        }
    } else {
        DBUG_PRINT ("PM", ("...passing-on FAIL!"));
    }
    DBUG_RETURN (stack);
}

/** <!--*******************************************************************-->
 *
 * @fn node *PMprf( prf fun, node *stack)
 *
 * @brief tries to match against an N_prf with the given fun. If successfull,
 *        the actual arguments are pushed on top of the stack.
 * @param fun: prf to match
 *        stack: stack of exprs
 * @return potentially extended stack
 *****************************************************************************/
node *
PMprf (prf fun, node *stack)
{
    node *arg;

    DBUG_ENTER ("PMprf");

    DBUG_PRINT ("PM", ("trying to match prf \"%s\"...", global.mdb_prf[fun]));

    if (stack != (node *)FAIL) {
        stack = ExtractOneArg (stack, &arg);
        arg = FollowId (arg);
        if ((NODE_TYPE (arg) == N_prf) && (PRF_PRF (arg) == fun)) {
            DBUG_PRINT ("PM", ("matched!"));
            stack = PushArgs (stack, PRF_ARGS (arg));
        } else {
            stack = FailMatch (stack);
        }
    } else {
        DBUG_PRINT ("PM", ("passing on FAIL!"));
    }
    DBUG_RETURN (stack);
}
