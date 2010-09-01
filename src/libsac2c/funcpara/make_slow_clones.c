/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup msc clonse functions with spawn to create slow clones
 *
 * @ingroup pc
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file make_slow_clones.c
 *
 * Prefix: MSC
 *
 *****************************************************************************/
#include "make_slow_clones.h"

#include "dbug.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "memory.h"
#include "traverse.h"
#include "new_types.h"
#include "new_typecheck.h"
#include "constants.h"
#include "type_utils.h"
#include "shape.h"
#include "DupTree.h"
#include "str.h"

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *MSCdoMakeSlowClones( node *argnode)
 *
 *****************************************************************************/

node *
MSCdoMakeSlowClones (node *argnode)
{
    DBUG_ENTER ("MSCdoMakeSlowClones");
    DBUG_PRINT ("MSC", ("Making slow clones..."));

    TRAVpush (TR_msc);
    argnode = TRAVdo (argnode, NULL);
    TRAVpop ();

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
 * @fn node *MSCfundef(node *arg_node, info *arg_info)
 *
 * @brief If functions contains spawn, copy it to create the slow clone.
 *
 * TODO: Optimize clone, remove unneccesary statements until first spawn
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/
node *
MSCfundef (node *arg_node, info *arg_info)
{
    node *clone;
    char *name;

    DBUG_ENTER ("MSCfundef");
    DBUG_PRINT ("MSC", ("traversing function (%s) %s",
                        (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef"),
                        FUNDEF_NAME (arg_node)));

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    if (FUNDEF_CONTAINSSPAWN (arg_node)) {
        DBUG_PRINT ("MSC", ("Function contains spawn, creating clone"));

        clone = DUPdoDupNode (arg_node);

        name = STRcat ("__slow__", FUNDEF_NAME (clone));
        // free old name
        FUNDEF_NAME (clone) = name;
        FUNDEF_ISSLOWCLONE (clone) = TRUE;

        FUNDEF_NEXT (clone) = FUNDEF_NEXT (arg_node);
        FUNDEF_NEXT (arg_node) = clone;

        FUNDEF_SLOWCLONE (arg_node) = clone;

        // numspawnsync is not copied?
        FUNDEF_NUMSPAWNSYNC (clone) = FUNDEF_NUMSPAWNSYNC (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Remove Spawn -->
 *****************************************************************************/
