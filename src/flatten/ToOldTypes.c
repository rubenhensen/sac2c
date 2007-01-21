/* $Id$ */

/*****************************************************************************
 *
 * file:   ToOldTypes.h
 *
 * prefix: TOT
 *
 * description:
 *
 *   This module restores all types-structures from ntype-structures.
 *   All ntype-structures will be removed.
 *
 *
 *****************************************************************************/

#include "dbug.h"
#include "ToOldTypes.h"
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "new_types.h"
#include "node_basic.h"
#include "internal_lib.h"
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
#define INFO_TOT_TYPES(n) (n->oldtypes)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_TOT_TYPES (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn node *TOTvardec( node *arg_node, node *arg_info)
 *
 *   @brief traverse vardecs only!
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
TOTvardec (node *arg_node, info *arg_info)
{
    ntype *type;

    DBUG_ENTER ("TOTvardec");

    type = AVIS_TYPE (VARDEC_AVIS (arg_node));
    DBUG_ASSERT ((type != NULL), "missing ntype information");

    if (VARDEC_TYPE (arg_node) != NULL) {
        VARDEC_TYPE (arg_node) = FREEfreeAllTypes (VARDEC_TYPE (arg_node));
    }

    VARDEC_TYPE (arg_node) = TYtype2OldType (type);

    AVIS_TYPE (VARDEC_AVIS (arg_node)) = TYfreeType (type);

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TOTarg( node *arg_node, node *arg_info)
 *
 *   @brief traverse vardecs only!
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
TOTarg (node *arg_node, info *arg_info)
{
    ntype *type;

    DBUG_ENTER ("TOTarg");

    type = AVIS_TYPE (ARG_AVIS (arg_node));
    DBUG_ASSERT ((type != NULL), "missing ntype information");

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
 * @fn node *TOTblock( node *arg_node, node *arg_info)
 *
 *   @brief traverse vardecs only!
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
TOTblock (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("TOTblock");

    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = TRAVdo (BLOCK_VARDEC (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TOTfundef( node *arg_node, node *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
TOTfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("TOTfundef");

    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_RETS (arg_node) != NULL) {
        FUNDEF_RETS (arg_node) = TRAVdo (FUNDEF_RETS (arg_node), arg_info);
    }

    if (FUNDEF_TYPES (arg_node) != NULL) {
        FUNDEF_TYPES (arg_node) = FREEfreeAllTypes (FUNDEF_TYPES (arg_node));
    }
    FUNDEF_TYPES (arg_node) = INFO_TOT_TYPES (arg_info);
    INFO_TOT_TYPES (arg_info) = NULL;

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TOTret( node *arg_node, node *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
TOTret (node *arg_node, info *arg_info)
{
    ntype *type;
    types *old_type;

    DBUG_ENTER ("TOTret");

    type = RET_TYPE (arg_node);
    DBUG_ASSERT (type != NULL, "missing ntype in N_ret!");

    if (RET_NEXT (arg_node) != NULL) {
        RET_NEXT (arg_node) = TRAVdo (RET_NEXT (arg_node), arg_info);
    }

    old_type = TYtype2OldType (type);
    TYPES_NEXT (old_type) = INFO_TOT_TYPES (arg_info);
    INFO_TOT_TYPES (arg_info) = old_type;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TOTdoToOldTypes( node *arg_node)
 *
 *   @brief replaces "ntype" info by "types" info
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
TOTdoToOldTypes (node *syntax_tree)
{
    info *arg_info;

    DBUG_ENTER ("ToOldTypes");

    if (global.compiler_phase > PH_typecheck) {

        TRAVpush (TR_tot);

        arg_info = MakeInfo ();
        syntax_tree = TRAVdo (syntax_tree, arg_info);
        arg_info = FreeInfo (arg_info);

        TRAVpop ();
    }

    DBUG_RETURN (syntax_tree);
}
