/*
 * $Log$
 * Revision 1.1  2005/09/02 17:44:38  sah
 * Initial revision
 *
 *
 */

#include "map_lac_funs.h"

#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"

/**
 * INFO structure
 */
struct INFO {
    travfun_p mapfundown;
    travfun_p mapfunup;
    info *info;
    node *current;
};

/**
 * INFO macros
 */
#define INFO_MAPFUNDOWN(n) ((n)->mapfundown)
#define INFO_MAPFUNUP(n) ((n)->mapfunup)
#define INFO_INFO(n) ((n)->info)
#define INFO_CURRENT(n) ((n)->current)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_MAPFUNDOWN (result) = NULL;
    INFO_MAPFUNUP (result) = NULL;
    INFO_INFO (result) = NULL;
    INFO_CURRENT (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/**
 * traversal functions
 */

node *
MLFfundef (node *arg_node, info *arg_info)
{
    node *last;

    DBUG_ENTER ("MLFfundef");

    if (FUNDEF_ISLACFUN (arg_node)) {
        last = INFO_CURRENT (arg_info);
        INFO_CURRENT (arg_info) = arg_node;

        if (INFO_MAPFUNDOWN (arg_info) != NULL) {
            arg_node = INFO_MAPFUNDOWN (arg_info) (arg_node, INFO_INFO (arg_info));
        }

        if (FUNDEF_BODY (arg_node) != NULL) {
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        }

        if (INFO_MAPFUNUP (arg_info) != NULL) {
            arg_node = INFO_MAPFUNUP (arg_info) (arg_node, INFO_INFO (arg_info));
        }

        INFO_CURRENT (arg_info) = last;
    }

    DBUG_RETURN (arg_node);
}

node *
MLFap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MLFap");

    if ((FUNDEF_ISLACFUN (AP_FUNDEF (arg_node)))
        && (AP_FUNDEF (arg_node) != INFO_CURRENT (arg_info))) {
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**
 * traversal start function
 */
info *
MLFdoMapLacFuns (node *arg_node, travfun_p mapfundown, travfun_p mapfunup, info *arg_info)
{
    info *localinfo;

    DBUG_ENTER ("MLFdoMapLacFuns");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef),
                 "MLFdoMapLacFuns called on non fundef node");

    DBUG_ASSERT ((!FUNDEF_ISLACFUN (arg_node)), "MLFdoMapLacFuns called on lacfun");

    localinfo = MakeInfo ();

    INFO_MAPFUNDOWN (localinfo) = mapfundown;
    INFO_MAPFUNUP (localinfo) = mapfunup;
    INFO_INFO (localinfo) = arg_info;

    TRAVpush (TR_mlf);

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), localinfo);
    }

    TRAVpop ();

    localinfo = FreeInfo (localinfo);

    DBUG_RETURN (arg_info);
}
