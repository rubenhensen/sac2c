#include "map_avis_trav.h"

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
    bool cont;
};

/**
 * INFO macros
 */
#define INFO_MAPTRAV(n) ((n)->maptrav)
#define INFO_EXTINFO(n) ((n)->extinfo)
#define INFO_CONT(n) ((n)->cont)

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
    INFO_CONT (result) = FALSE;

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
MATfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    FUNDEF_ARGS (arg_node) = TRAVopt(FUNDEF_ARGS (arg_node), arg_info);

    FUNDEF_BODY (arg_node) = TRAVopt(FUNDEF_BODY (arg_node), arg_info);

    if (INFO_CONT (arg_info) && (FUNDEF_NEXT (arg_node) != NULL)) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
MATavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    arg_node = INFO_MAPTRAV (arg_info) (arg_node, INFO_EXTINFO (arg_info));

    DBUG_RETURN (arg_node);
}

node *
MATblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    BLOCK_VARDECS (arg_node) = TRAVopt(BLOCK_VARDECS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * traversal start function
 */
static node *
MapAvisTrav (node *arg_node, info *extinfo, travfun_p maptrav, bool cont)
{
    info *localinfo;

    DBUG_ENTER ();

    localinfo = MakeInfo ();

    INFO_MAPTRAV (localinfo) = maptrav;
    INFO_EXTINFO (localinfo) = extinfo;
    INFO_CONT (localinfo) = cont;

    TRAVpush (TR_mat);

    if (arg_node != NULL) {
        arg_node = TRAVdo (arg_node, localinfo);
    }

    TRAVpop ();

    localinfo = FreeInfo (localinfo);

    DBUG_RETURN (arg_node);
}

node *
MATdoMapAvisTrav (node *arg_node, info *extinfo, travfun_p maptrav)
{
    DBUG_ENTER ();

    arg_node = MapAvisTrav (arg_node, extinfo, maptrav, TRUE);

    DBUG_RETURN (arg_node);
}

node *
MATdoMapAvisTravOneFundef (node *arg_node, info *extinfo, travfun_p maptrav)
{
    DBUG_ENTER ();

    arg_node = MapAvisTrav (arg_node, extinfo, maptrav, FALSE);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
