/* $Id: convert_type_representation.c 15841 2008-11-03 17:53:13Z cg $ */

/*****************************************************************************
 *
 * file:   tag_argument.c
 *
 * prefix: TA
 *
 * description:
 *
 *   This module tags all arguments ready for the mutc (SL) backend.
 *
 *****************************************************************************/

#include "dbug.h"
#include "tag_argument.h"
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"
#include "traverse.h"
#include "memory.h"
#include "free.h"

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
 * @fn node *TAarg( node *arg_node, node *arg_info)
 *
 *   @brief Set AVIS of all args
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
TAarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("TAarg");

    AVIS_ISFUNARGUMENT (ARG_AVIS (arg_node)) = TRUE;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TAdoTagArguments( node *arg_node)
 *
 *   @brief Tag all function arguments as the mutc SL back-end needs to know
 *          this
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
TAdoTagArguments (node *syntax_tree)
{
    info *arg_info;

    DBUG_ENTER ("TAdoTagArguments");

    TRAVpush (TR_ta);

    arg_info = MakeInfo ();
    syntax_tree = TRAVdo (syntax_tree, arg_info);
    arg_info = FreeInfo (arg_info);

    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}
