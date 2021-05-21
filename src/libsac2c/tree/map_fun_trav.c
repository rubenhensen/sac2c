#include "map_fun_trav.h"

#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "memory.h"

/**
 * INFO structure
 */
struct INFO {
    travfun_p maptrav;
    info *extinfo;
};

/**
 * INFO macros
 */
#define INFO_MAPTRAV(n) ((n)->maptrav)
#define INFO_EXTINFO(n) ((n)->extinfo)

/**
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_MAPTRAV (result) = NULL;
    INFO_EXTINFO (result) = NULL;

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
 * traversal functions
 */

node *
MFTfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    arg_node = INFO_MAPTRAV (arg_info) (arg_node, INFO_EXTINFO (arg_info));

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * traversal start function
 */
node *
MFTdoMapFunTrav (node *arg_node, info *extinfo, travfun_p maptrav)
{
    info *localinfo;

    DBUG_ENTER ();

    DBUG_ASSERT (((arg_node == NULL)
                  || ((NODE_TYPE (arg_node) == N_fundef)
                      || (NODE_TYPE (arg_node) == N_module))),
                 "MLFdoMapFunTrav called on non fundef/module node");

    localinfo = MakeInfo ();

    INFO_MAPTRAV (localinfo) = maptrav;
    INFO_EXTINFO (localinfo) = extinfo;

    TRAVpush (TR_mft);

    if (arg_node != NULL) {
        arg_node = TRAVdo (arg_node, localinfo);
    }

    TRAVpop ();

    localinfo = FreeInfo (localinfo);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
