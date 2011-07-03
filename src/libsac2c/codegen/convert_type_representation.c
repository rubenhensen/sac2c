/* $Id$ */

/*****************************************************************************
 *
 * file:   convert_type_representation.c
 *
 * prefix: CTR
 *
 * description:
 *
 *   This module restores all types-structures from ntype-structures.
 *   All ntype-structures will be removed.
 *
 *
 *****************************************************************************/

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "convert_type_representation.h"
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "new_types.h"
#include "node_basic.h"
#include "str.h"
#include "memory.h"
#include "traverse.h"
#include "free.h"
#include "type_utils.h"

/**
 * INFO structure
 */
struct INFO {
    types *oldtypes;
};

/**
 * INFO macros
 */
#define INFO_TYPES(n) (n->oldtypes)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

    INFO_TYPES (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn node *CTRvardec( node *arg_node, node *arg_info)
 *
 *   @brief traverse vardecs only!
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
CTRvardec (node *arg_node, info *arg_info)
{
    ntype *type;

    DBUG_ENTER ();

    type = AVIS_TYPE (VARDEC_AVIS (arg_node));
    DBUG_ASSERT (type != NULL, "missing ntype information");

    VARDEC_TYPE (arg_node) = TYtype2OldType (type);

    AVIS_TYPE (VARDEC_AVIS (arg_node)) = TYfreeType (type);

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CTRarg( node *arg_node, node *arg_info)
 *
 *   @brief traverse vardecs only!
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
CTRarg (node *arg_node, info *arg_info)
{
    ntype *type;

    DBUG_ENTER ();

    type = AVIS_TYPE (ARG_AVIS (arg_node));
    DBUG_ASSERT (type != NULL, "missing ntype information");

    if (ARG_TYPE (arg_node) != NULL)
        ARG_TYPE (arg_node) = FREEfreeAllTypes (ARG_TYPE (arg_node));

    ARG_TYPE (arg_node) = TYtype2OldType (type);

    AVIS_TYPE (ARG_AVIS (arg_node)) = TYfreeType (type);

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CTRblock( node *arg_node, node *arg_info)
 *
 *   @brief traverse vardecs only!
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
CTRblock (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = TRAVdo (BLOCK_VARDEC (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CTRfundef( node *arg_node, node *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
CTRfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_RETS (arg_node) != NULL) {
        FUNDEF_RETS (arg_node) = TRAVdo (FUNDEF_RETS (arg_node), arg_info);
    }

    FUNDEF_TYPES (arg_node) = INFO_TYPES (arg_info);
    INFO_TYPES (arg_info) = NULL;

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CTRret( node *arg_node, node *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
CTRret (node *arg_node, info *arg_info)
{
    ntype *type;
    types *old_type;

    DBUG_ENTER ();

    type = RET_TYPE (arg_node);
    DBUG_ASSERT (type != NULL, "missing ntype in N_ret!");

    if (RET_NEXT (arg_node) != NULL) {
        RET_NEXT (arg_node) = TRAVdo (RET_NEXT (arg_node), arg_info);
    }

    old_type = TYtype2OldType (type);
    TYPES_NEXT (old_type) = INFO_TYPES (arg_info);
    INFO_TYPES (arg_info) = old_type;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CTRdoConvertToOldTypes( node *arg_node)
 *
 *   @brief replaces "ntype" info by "types" info
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
CTRdoConvertToOldTypes (node *syntax_tree)
{
    info *arg_info;

    DBUG_ENTER ();

    TRAVpush (TR_ctr);

    arg_info = MakeInfo ();
    syntax_tree = TRAVdo (syntax_tree, arg_info);
    arg_info = FreeInfo (arg_info);

    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
