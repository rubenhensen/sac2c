/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup tff tag fundef nodes if they contain a spawn statement
 *
 * @ingroup fp
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file tag_fp_fundefs.c
 *
 * Prefix: MSS
 *
 *****************************************************************************/
#include "tag_fp_fundefs.h"

#include "dbug.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "memory.h"
#include "traverse.h"
#include "new_types.h"
#include "new_typecheck.h"
#include "constants.h"
#include "type_utils.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/

struct INFO {
    bool spawnfound;
};

#define INFO_SPAWNFOUND(n) ((n)->spawnfound)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_SPAWNFOUND (result) = FALSE;

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
 * @}  <!-- INFO structure -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *TFFdoTagFPFundefs( node *argnode)
 *
 *****************************************************************************/

node *
TFFdoTagFPFundefs (node *argnode)
{
    info *info;
    DBUG_ENTER ("TFFdoTagFPFundefs");
    DBUG_PRINT ("TFF", ("Tagging FP Fundef nodes"));

    info = MakeInfo ();

    TRAVpush (TR_tff);
    argnode = TRAVdo (argnode, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (argnode);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *TFFfundef(node *arg_node, info *arg_info)
 *
 * @brief Traverses into fundef local LAC functions, then function
 *        bodies and finally function next pointers. When traversing
 *        into a body a pointer in the info struct is maintained to
 *        the inner fundef.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
TFFfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("TFFfundef");

    DBUG_PRINT ("TFF", ("traversing body of (%s) %s",
                        (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef"),
                        FUNDEF_NAME (arg_node)));

    INFO_SPAWNFOUND (arg_info) = FALSE;

    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    FUNDEF_CONTAINSSPAWN (arg_node) = INFO_SPAWNFOUND (arg_info);

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TFFap(node *arg_node, info *arg_info)
 *
 * @brief Check to see if AP is tagged as spawnable
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
TFFap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("TFFap");
    DBUG_PRINT ("TFF", ("Traversing Ap node"));

    INFO_SPAWNFOUND (arg_info) = INFO_SPAWNFOUND (arg_info) || AP_ISSPAWNED (arg_node);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Add Sync -->
 *****************************************************************************/
