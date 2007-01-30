/*
 * $Log$
 * Revision 1.2  2005/09/04 12:47:34  ktr
 * traversal might work better with correct trav_tab
 *
 * Revision 1.1  2005/09/04 12:10:48  ktr
 * Initial revision
 *
 */

#include "map_fun_trav.h"

#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"

typedef node *(*trav_p) (node *, info *);

/**
 * INFO structure
 */
struct INFO {
    trav_p maptrav;
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
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_MAPTRAV (result) = NULL;
    INFO_EXTINFO (result) = NULL;

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
MFTfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MFTfundef");

    arg_node = INFO_MAPTRAV (arg_info) (arg_node, INFO_EXTINFO (arg_info));

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**
 * traversal start function
 */
node *
MFTdoMapFunTrav (node *arg_node, info *extinfo, trav_p maptrav)
{
    info *localinfo;

    DBUG_ENTER ("MFTdoMapFunTrav");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef),
                 "MLFdoMapFunTrav called on non fundef node");

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
