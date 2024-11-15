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

#define DBUG_PREFIX "MSC"
#include "debug.h"

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
    DBUG_ENTER ();
    DBUG_PRINT ("Making slow clones...");

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

    DBUG_ENTER ();
    DBUG_PRINT ("traversing function (%s) %s",
                (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef"),
                FUNDEF_NAME (arg_node));

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    if (FUNDEF_CONTAINSSPAWN (arg_node)) {
        DBUG_PRINT ("Function contains spawn, creating clone");

        FUNDEF_FPFRAMENAME (arg_node) = STRcpy (FUNDEF_NAME (arg_node));
        clone = DUPdoDupNode (arg_node);

        // TODO: free( FUNDEF_NAME( clone));
        FUNDEF_NAME (clone) = STRcat ("__slow__", FUNDEF_NAME (clone));
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

#undef DBUG_PREFIX
