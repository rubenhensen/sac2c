
/** <!--********************************************************************-->
 *
 * @file change_arrays_to_dist.c
 *
 * Changes the type of all arrays of known shape to corresponding distributed
 * type
 *
 *
 *****************************************************************************/
#include "change_arrays_to_dist.h"

#define DBUG_PREFIX "CADT"
#include "debug.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "new_types.h"
#include "type_utils.h"
#include "memory.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
};

/**
 * Info structure macros
 */

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

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
 * @fn node *CADTdoChangeArraysToDistributedType( node *syntax_tree)
 *
 *****************************************************************************/
node *
CADTdoChangeArraysToDistributedType (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_cadt);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *changeType(ntype *host_type)
 *
 * @brief Changes type of host_type to the equivalent distributed type, iff it
 * is a non-scalar simple type.
 *
 *****************************************************************************/

static ntype *
changeType (ntype *host_type)
{
    DBUG_ENTER ();
    ntype *scalar_type;
    simpletype sty;

    if (TUdimKnown (host_type) && TYgetDim (host_type) > 0
        && TYisSimple (TYgetScalar (host_type))) {
        scalar_type = TYgetScalar (host_type);
        /* Get the corresponding device simple type e.g. int_dev, float_dev...*/
        switch (TYgetSimpleType (scalar_type)) {
        case T_int:
            sty = T_int_dist;
            break;
        case T_float:
            sty = T_float_dist;
            break;
        case T_double:
            sty = T_double_dist;
            break;
        default:
            sty = TYgetSimpleType (scalar_type);
            break;
        };
        /* Set the device simple type */
        scalar_type = TYsetSimpleType (scalar_type, sty);
    }

    DBUG_RETURN (host_type);
}

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *CADTfundef(node *arg_node, info *arg_info)
 *
 * @brief Traverse the variable declarations, the function arguments and the
 *        return types. No need to go through the body.
 *
 *****************************************************************************/
node *
CADTfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node) != NULL)
        FUNDEF_VARDECS (arg_node) = TRAVopt (FUNDEF_VARDECS (arg_node), arg_info);

    FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);

    FUNDEF_RETS (arg_node) = TRAVopt (FUNDEF_RETS (arg_node), arg_info);

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CADTret(node *arg_node, info *arg_info)
 *
 * @brief Changes the type of the function returns.
 *
 *****************************************************************************/
node *
CADTret (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    RET_TYPE (arg_node) = changeType (RET_TYPE (arg_node));

    RET_NEXT (arg_node) = TRAVopt (RET_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CADTavis(node *arg_node, info *arg_info)
 *
 * @brief Changes the type of the avis.
 *
 *****************************************************************************/
node *
CADTavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    AVIS_TYPE (arg_node) = changeType (AVIS_TYPE (arg_node));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- ChangeArraysToDistributedType -->
 *****************************************************************************/
