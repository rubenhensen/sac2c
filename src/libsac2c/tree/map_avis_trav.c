/*
 * $Id$
 *
 */

#include "map_avis_trav.h"

#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "dbug.h"
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
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_MAPTRAV (result) = NULL;
    INFO_EXTINFO (result) = NULL;
    INFO_CONT (result) = FALSE;

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
 * traversal functions
 */

node *
MATfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MATfundef");

    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    if (INFO_CONT (arg_info) && (FUNDEF_NEXT (arg_node) != NULL)) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
MATavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MATavis");

    arg_node = INFO_MAPTRAV (arg_info) (arg_node, INFO_EXTINFO (arg_info));

    DBUG_RETURN (arg_node);
}

node *
MATblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MATblock");

    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = TRAVdo (BLOCK_VARDEC (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**
 * traversal start function
 */
static node *
MapAvisTrav (node *arg_node, info *extinfo, travfun_p maptrav, bool cont)
{
    info *localinfo;

    DBUG_ENTER ("MapAvisTrav");

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
    DBUG_ENTER ("MATdoMapAvisTrav");

    arg_node = MapAvisTrav (arg_node, extinfo, maptrav, TRUE);

    DBUG_RETURN (arg_node);
}

node *
MATdoMapAvisTravOneFundef (node *arg_node, info *extinfo, travfun_p maptrav)
{
    DBUG_ENTER ("MATdoMapAvisTravOneFundef");

    arg_node = MapAvisTrav (arg_node, extinfo, maptrav, FALSE);

    DBUG_RETURN (arg_node);
}
