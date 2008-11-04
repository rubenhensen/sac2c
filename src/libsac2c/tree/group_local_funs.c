/*****************************************************************************
 *
 * $Id$
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
 *****************************************************************************/

/**
 * Includes
 */

#include "group_local_funs.h"

#include "dbug.h"
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
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_LOCALFUNS (result) = NULL;
    INFO_SPINE (result) = TRUE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/**
 * Local functions
 */

static bool
IsLocalFun (node *fundef)
{
    bool is_local_fun;

    DBUG_ENTER ("IsLocalFun");

    DBUG_ASSERT (NODE_TYPE (fundef) == N_fundef,
                 "IsLocalFun called with illegal node type.");

    is_local_fun = FUNDEF_ISCONDFUN (fundef) || FUNDEF_ISDOFUN (fundef);

    DBUG_RETURN (is_local_fun);
}

/**
 * Traversal start function
 */

node *
GLFdoGroupLocalFuns (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("GLFdoGroupLocalFuns");

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
    DBUG_ENTER ("GLFmodule");

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);
    global.local_funs_grouped = TRUE;

    DBUG_RETURN (arg_node);
}

node *
GLFfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("GLFfundef");

    if (INFO_SPINE (arg_info)) {
        if (IsLocalFun (arg_node)) {
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
        if (IsLocalFun (arg_node)) {
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
    DBUG_ENTER ("GLFap");

    if (!AP_ISRECURSIVEDOFUNCALL (arg_node)) {
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
