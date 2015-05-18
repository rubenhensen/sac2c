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
#include "tree_compound.h"

#include <uuid/uuid.h>

struct INFO {
    node *module;
    bool isgeneric;
};

#define INFO_MODULE(n) ((n)->module)
#define INFO_ISGENERIC(n) ((n)->isgeneric)

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
 * @fn UIDarg (node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param  arg_node  The current node of the syntax tree.
 * @param  arg_info  Info object, unused.
 *
 * @return  The updated arg node.
 *
 *****************************************************************************/
node *
UIDarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ntype *argtype = ARG_NTYPE (arg_node);

    if (!TYisAKS (argtype)) {
        INFO_ISGENERIC (arg_info) = TRUE;
    }

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
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

    uuid_t uuid;

    if (FUNDEF_ISLOCAL (arg_node) && !FUNDEF_ISWRAPPERFUN (arg_node)
        && !FUNDEF_ISCONDFUN (arg_node) && !FUNDEF_ISLOOPFUN (arg_node)) {
        INFO_ISGENERIC (arg_info) = FALSE;

        if (FUNDEF_ARGS (arg_node) != NULL) {
            FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
        }

        if (INFO_ISGENERIC (arg_info)) {
            FUNDEF_RTSPECID (arg_node) = (char *)malloc (sizeof (char) * 36);

            uuid_generate (uuid);
            uuid_unparse_lower (uuid, FUNDEF_RTSPECID (arg_node));
        }
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        /*
         * Traverse all the functions in the fundef chain.
         */
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn UIDmodule (node *arg_node, info *arg_info)
 *
 * @brief Go over modules and traverse only the ones that actually have functions
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
