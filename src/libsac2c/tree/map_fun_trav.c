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

typedef node *(*trav_p) (node *);

/**
 * INFO structure
 */
struct INFO {
    trav_p maptrav;
};

/**
 * INFO macros
 */
#define INFO_MAPTRAV(n) ((n)->maptrav)

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

    arg_node = INFO_MAPTRAV (arg_info) (arg_node);

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**
 * traversal start function
 */
node *
MFTdoMapFunTrav (node *arg_node, trav_p maptrav)
{
    info *localinfo;

    DBUG_ENTER ("MFTdoMapFunTrav");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_module),
                 "MLFdoMapFunTrav called on non module node");

    localinfo = MakeInfo ();

    INFO_MAPTRAV (localinfo) = maptrav;

    TRAVpush (TR_mft);

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), localinfo);
    }

    TRAVpop ();

    localinfo = FreeInfo (localinfo);

    DBUG_RETURN (arg_node);
}
