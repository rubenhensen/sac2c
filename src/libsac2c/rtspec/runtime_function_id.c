/**<!--*********************************************************************-->
 *
 * @file runtime_function_id.c
 *
 * @brief Traversal for setting unique function ids to generic functions.
 *
 * @author hmw
 *
 *****************************************************************************/

#include "tree_basic.h"
#include "memory.h"

#define DBUG_PREFIX "UID"
#include "debug.h"

#include "traverse.h"
#include "new_types.h"

struct INFO {
    node *module;
};

#define INFO_MODULE(n) ((n)->module)

static info *
MakeInfo (info *arg_info)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_MODULE (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *arg_info)
{
    DBUG_ENTER ();

    arg_info = MEMfree (arg_info);

    DBUG_RETURN (arg_info);
}

/** <!--********************************************************************-->
 *
 * @fn UIDfundef (node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param  arg_node  The current node of the syntax tree.
 * @param  arg_info  Info object, unused.
 *
 * @return  The updated fundef node.
 *
 *****************************************************************************/
node *
UIDfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn UIDmodule (node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node  the syntax tree.
 *
 * @return  the updated and extended syntax tree.
 *
 * ***************************************************************************/
node *
UIDmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (MODULE_FUNS (arg_node) != NULL) {
        INFO_MODULE (arg_info) = arg_node;
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn UIDdoSetFunctionIDs( node *arg_node)
 *
 * @brief Start the traversal for setting unique function ids.
 *
 * @param arg_node  the syntax tree.
 *
 * @return  the updated and extended syntax tree.
 *
 * ***************************************************************************/
node *
UIDdoSetFunctionIDs (node *arg_node)
{
    DBUG_ENTER ();

    info *info = NULL;

    info = MakeInfo (info);

    TRAVpush (TR_uid);

    arg_node = TRAVdo (arg_node, info);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
