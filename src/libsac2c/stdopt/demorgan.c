/*
 * $Id$
 */

#include "tree_basic.h"
#include "tree_compound.h"
#include "globals.h"
#include "traverse.h"
#include "dbug.h"
#include "memory.h"
#include "free.h"
#include "DupTree.h"
#include "demorgan.h"

/*
 * INFO structure
 */
struct INFO {
    node *preassign;
};

/*
 * INFO macros
 */
#define INFO_PREASSIGN(n) ((n)->preassign)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_PREASSIGN (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn node *DMLdoDeMorgan( node *arg_node)
 *
 * @brief starting point of deMorgan law optimization
 *
 * @param arg_node
 *
 * @return
 *
 *****************************************************************************/
node *
DMLdoDeMorgan (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("DMLdoDeMorgan");

    info = MakeInfo ();

    TRAVpush (TR_dml);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

node *
DMLassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DMLassign");

    /*
     * Bottom-up traversal
     */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_PREASSIGN (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_PREASSIGN (arg_info), arg_node);
        INFO_PREASSIGN (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

node *
DMLprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DMLprf");

    DBUG_RETURN (arg_node);
}
