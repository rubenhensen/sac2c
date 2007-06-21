/*
 *
 * $Id:$
 */

/**
 *
 * @file pattern_match.c
 *
 */

#include "dbug.h"

#include "free.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"

static char *FAIL = '\0';

static node *
ExtractOneArg (node *exprs, node **arg)
{
    node *next;

    DBUG_ENTER ("ExtractOneArg");
    DBUG_ASSERT (exprs != NULL, ("ExtractOneArg called with NULL exprs!"));

    if (NODE_TYPE (exprs) == N_set) {
        next = ExtractOneArg (SET_MEMBER (exprs), arg);
        if (next != NULL) {
            SET_MEMBER (exprs) = next;
        } else {
            exprs = FREEdoFreeNode (exprs);
        }
    } else if (NODE_TYPE (exprs) == N_exprs) {
        *arg = EXPRS_EXPR (exprs);
        exprs = EXPRS_NEXT (exprs);
    } else {
        *arg = exprs;
        exprs = NULL;
    }
    DBUG_RETURN (exprs);
}

static node *
AppendArgs (node *exprs, node *args)
{
    DBUG_ENTER ("ExtractOneArg");
    if (exprs == NULL) {
        exprs = args;
    } else if (NODE_TYPE (exprs) == N_set) {
        exprs = TBmakeSet (args, exprs);
    } else {
        exprs = TBmakeSet (args, TBmakeSet (exprs, NULL));
    }
    DBUG_RETURN (exprs);
}

static node *
FollowId (node *arg)
{
    DBUG_ENTER ("FollowId");
    while ((NODE_TYPE (arg) == N_id) && (AVIS_SSAASSIGN (ID_AVIS (arg)) != NULL)) {
        arg = LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (arg))));
    }
    DBUG_RETURN (arg);
}

bool
PM (node *res)
{
    DBUG_ENTER ("PM");
    DBUG_RETURN (res != (node *)FAIL);
}

node *
PMvar (node **var, node *arg_node)
{
    node *arg;

    DBUG_ENTER ("PMvar");
    if (arg_node != (node *)FAIL) {
        arg_node = ExtractOneArg (arg_node, &arg);
        if ((NODE_TYPE (arg) == N_id)
            && ((*var == NULL) || (ID_AVIS (*var) == ID_AVIS (arg)))) {
            *var = arg;
        } else {
            arg_node = (node *)FAIL;
        }
    }
    DBUG_RETURN (arg_node);
}

node *
PMprf (prf fun, node *arg_node)
{
    node *arg;

    DBUG_ENTER ("PMvar");
    if (arg_node != (node *)FAIL) {
        arg_node = ExtractOneArg (arg_node, &arg);
        arg = FollowId (arg);
        if ((NODE_TYPE (arg) == N_prf) && (PRF_PRF (arg) == fun)) {
            arg_node = AppendArgs (arg_node, PRF_ARGS (arg));
        } else {
            arg_node = (node *)FAIL;
        }
    }
    DBUG_RETURN (arg_node);
}
