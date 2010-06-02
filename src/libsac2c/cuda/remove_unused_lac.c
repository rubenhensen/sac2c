
#include "remove_unused_lac.h"

#include <stdlib.h>

#include "tree_basic.h"
#include "tree_compound.h"
#include "globals.h"
#include "memory.h"
#include "dbug.h"
#include "ctinfo.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "print.h"
#include "new_types.h"
#include "shape.h"
#include "types.h"
#include "constants.h"

typedef enum {
    trav_collect,
    trav_remove,
} travmode_t;

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    travmode_t mode;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_TRAVMODE(n) (n->mode)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;

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
RLACdoRemoveUnusedLac (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("RLACdoRemoveUnusedLac");

    info = MakeInfo ();
    TRAVpush (TR_rlac);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();
    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

node *
RLACmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RLACmodule");

    INFO_TRAVMODE (arg_info) = trav_collect;
    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    INFO_TRAVMODE (arg_info) = trav_remove;
    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
RLACfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCUFfundef");

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    if (INFO_TRAVMODE (arg_info) == trav_collect) {
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    } else if (INFO_TRAVMODE (arg_info) == trav_remove) {
        if (FUNDEF_ISLACFUN (arg_node)) {
            if (!FUNDEF_ISNEEDED (arg_node)) {
                arg_node = FREEdoFreeNode (arg_node);
            } else {
                FUNDEF_ISNEEDED (arg_node) = FALSE;
            }
        }
    } else {
        DBUG_ASSERT ((0), "Wrong traverse mode in RLACfundef!");
    }

    DBUG_RETURN (arg_node);
}

node *
RLACap (node *arg_node, info *arg_info)
{
    node *fundef;

    DBUG_ENTER ("RLACap");

    fundef = AP_FUNDEF (arg_node);

    if (fundef != NULL && FUNDEF_ISLACFUN (fundef) && INFO_FUNDEF (arg_info) != fundef) {
        FUNDEF_ISNEEDED (fundef) = TRUE;
    }

    DBUG_RETURN (arg_node);
}
