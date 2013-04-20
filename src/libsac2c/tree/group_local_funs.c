/*****************************************************************************
 *
 * file:   group_local_funs.c
 *
 * prefix: GLF
 *
 * description:
 *
 *   This traversal integrates all local functions pertaining to a standard
 *   function in a local fundef spine. Currently, only loop and conditional
 *   functions are considered local in this sense, but later this should be
 *   extended to all functions that have a unique call site.
 *
 *   When this traversal completes, NO local function should have a
 *   non-NULL FUNDEF_NEXT.
 *
 *****************************************************************************/

/**
 * Includes
 */

#include "group_local_funs.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "traverse.h"
#include "memory.h"
#include "tree_basic.h"
#include "globals.h"

/**
 * INFO structure
 */

struct INFO {
    node *localfuns;
    bool spine;
};

/**
 * INFO macros
 */

#define INFO_LOCALFUNS(n) (n->localfuns)
#define INFO_SPINE(n) (n->spine)

/**
 * INFO functions
 */

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_LOCALFUNS (result) = NULL;
    INFO_SPINE (result) = TRUE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/**
 * Auxiliary functions
 */

/*
 * We export this function to have a common predicate here and in traverse.c
 */

bool
GLFisLocalFun (node *fundef)
{
    bool is_local_fun;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (fundef) == N_fundef,
                 "IsLocalFun called with illegal node type.");

    /**
     *  this is NOT a good solution!! We should have a separate flag
     *  here. The problem is, that we may deal with a function that
     *  is to be lacinlined. which still lives in the locals.
     *  Since traverse makes use of this function to decide whether
     *  hooked functions (zombies or duped ones) should be treated,
     *  we need to make sure these two are still considered "local".
     */
    is_local_fun = FUNDEF_ISCONDFUN (fundef) || FUNDEF_ISLOOPFUN (fundef)
                   || FUNDEF_ISLACINLINE (fundef) || FUNDEF_ISZOMBIE (fundef);

    DBUG_RETURN (is_local_fun);
}

/**
 * Traversal start function
 */

node *
GLFdoGroupLocalFuns (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (syntax_tree) == N_module, "Illegal argument node!");

    info = MakeInfo ();

    TRAVpush (TR_glf);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/**
 * Traversal functions
 */

node *
GLFmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);
    global.local_funs_grouped = TRUE;

    DBUG_RETURN (arg_node);
}

node *
GLFfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_SPINE (arg_info)) {
        if (GLFisLocalFun (arg_node)) {
            arg_node = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
        } else {
            FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

            INFO_SPINE (arg_info) = FALSE;
            FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
            INFO_SPINE (arg_info) = TRUE;

            FUNDEF_LOCALFUNS (arg_node) = INFO_LOCALFUNS (arg_info);
            INFO_LOCALFUNS (arg_info) = NULL;
        }
    } else {
        if (GLFisLocalFun (arg_node)) {
            FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
            FUNDEF_NEXT (arg_node) = INFO_LOCALFUNS (arg_info);
            INFO_LOCALFUNS (arg_info) = arg_node;
        }
    }

    DBUG_RETURN (arg_node);
}

node *
GLFap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (!AP_ISRECURSIVEDOFUNCALL (arg_node)) {
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
