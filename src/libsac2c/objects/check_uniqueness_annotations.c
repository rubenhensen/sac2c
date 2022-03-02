#include "check_uniqueness_annotations.h"

#include "traverse.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "ctinfo.h"
#include "tree_basic.h"
#include "new_types.h"
#include "type_utils.h"

/*
 * traversal functions
 */
node *
CUAobjdef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (!TUisUniqueUserType (TYgetScalar (OBJDEF_TYPE (arg_node)))) {
        CTIerror (NODE_LOCATION (arg_node),
                  "Objects can only be declared on unique types!");
    }

    DBUG_RETURN (arg_node);
}

node *
CUAarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (TYisArray (AVIS_TYPE (ARG_AVIS (arg_node)))
        && TUisUniqueUserType (TYgetScalar (AVIS_TYPE (ARG_AVIS (arg_node))))) {
        ARG_ISUNIQUE (arg_node) = TRUE;
    }

    if (ARG_WASREFERENCE (arg_node) && !ARG_ISUNIQUE (arg_node)) {
        CTIerror (NODE_LOCATION (arg_node),
                  "Reference args can only be declared on unique types!");
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
CUAret (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (TYisArray (RET_TYPE (arg_node))
        && TUisUniqueUserType (TYgetScalar (RET_TYPE (arg_node)))) {
        RET_ISUNIQUE (arg_node) = TRUE;
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/*
 * traversal start functions
 */
node *
CUAdoCheckUniquenessAnnotations (node *syntax_tree)
{
    DBUG_ENTER ();

    TRAVpush (TR_cua);

    syntax_tree = TRAVdo (syntax_tree, NULL);

    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
