#include "wl_lock_optimization_shifting.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "memory.h"
#include "dbug.h"
#include "traverse.h"
#include "DupTree.h"
#include "free.h"

/**
 ** INFO structure
 **/
struct INFO {
    int wllevel;
};

/**
 ** INFO macros
 **/
#define INFO_WLLEVEL(n) ((n)->wllevel)

/**
 ** INFO functions
 **/
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_WLLEVEL (result) = 0;
    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

node *
WLLOSprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLLOSprf");
    arg_node = TRAVcont (arg_node, arg_info);
    DBUG_RETURN (arg_node);
}

node *
WLLOSassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLLOSassign");
    arg_node = TRAVcont (arg_node, arg_info);
    DBUG_RETURN (arg_node);
}

node *
WLLOSlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLLOSlet");
    arg_node = TRAVcont (arg_node, arg_info);
    DBUG_RETURN (arg_node);
}

node *
WLLOSwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLLOSwith");
    INFO_WLLEVEL (arg_info) = INFO_WLLEVEL (arg_info) + 1;
    arg_node = TRAVcont (arg_node, arg_info);
    INFO_WLLEVEL (arg_info) = INFO_WLLEVEL (arg_info) - 1;
    DBUG_RETURN (arg_node);
}

node *
WLLOSdoLockOptimizationShifting (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("WLLOSdoLockOptimizationShifting");

    DBUG_ASSERT ((NODE_TYPE (syntax_tree) == N_module),
                 "WLLOSdoLockOptimizationShifting is intended to run on the entire tree");

    info = MakeInfo ();
    TRAVpush (TR_wllos);

    syntax_tree = TRAVdo (syntax_tree, info);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}
