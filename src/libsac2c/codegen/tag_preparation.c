/* $Id$ */

/*****************************************************************************
 *
 * file:   tag_preparation.c
 *
 * prefix: TP
 *
 * description:
 *
 *   This module add any tag information to the ntype that may not
 *   have been added to the ntype yet.
 *
 *
 *****************************************************************************/

#include "dbug.h"
#include "tag_preparation.h"
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "new_types.h"
#include "node_basic.h"
#include "memory.h"
#include "traverse.h"
#include "free.h"
#include "type_utils.h"

/**
 * INFO structure
 */
struct INFO {
};

/**
 * INFO macros
 */

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

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
 * @fn node *TParg( node *arg_node, node *arg_info)
 *
 *   @brief Arguments must be tagged as params
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
TParg (node *arg_node, info *arg_info)
{
    ntype *type;

    DBUG_ENTER ("TParg");

    type = AVIS_TYPE (ARG_AVIS (arg_node));
    DBUG_ASSERT ((type != NULL), "missing ntype information");

    arg_node = TRAVcont (arg_node, arg_info);

    type = TYsetMutcUsage (type, MUTC_US_PARAM);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TPdoTagPreparation( node *arg_node)
 *
 *   @brief Add information to the ntype needed for tags.
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
TPdoTagPreparation (node *syntax_tree)
{
    info *arg_info;

    DBUG_ENTER ("PTdoTagPreparation");

    TRAVpush (TR_tp);

    arg_info = MakeInfo ();
    syntax_tree = TRAVdo (syntax_tree, arg_info);
    arg_info = FreeInfo (arg_info);

    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}
