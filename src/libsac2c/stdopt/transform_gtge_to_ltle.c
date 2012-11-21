/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup TGTL Operation Trasnformation
 *
 *    Module transform_gtge_to_ltle.c transform all gt and ge operators to lt and
 *    le operators.
 *
 *    For example, we have the following code
 *
 *    a = 3;
 *    b = 5;
 *    c = [3,5];
 *    d = [3,5];
 *
 *    _gt_SxS(a,b);                              _lt_SxS(b,a);
 *    _gt_VxV(c,d);   will be transformed into   _lt_VxV(d,c);
 *    _gt_SxV(a,d);                              _lt_VxS(d,a);
 *
 *
 *    Motivation:
 *      We employ this optimization to simpilfy the relationship operator in our
 *      SaC code. After code transformation, there are only equal or less or less
 *      than operators left. In the future, we want to build a grpahic for all
 *      static known knowledge to statically reslove other relation operations.
 *
 *
 * @ingroup opt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file transform_gtge_to_ltle.c
 *
 * Prefix: TGTL
 *
 *****************************************************************************/
#include "transform_gtge_to_ltle.h"

#define DBUG_PREFIX "TGTL"
#include "debug.h"

#include "free.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "memory.h"
#include "traverse.h"
#include "new_types.h"
#include "new_typecheck.h"
#include "constants.h"
#include "type_utils.h"
#include "globals.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/

struct INFO {
    node *lhs;
};

#define INFO_LHS(n) ((n)->lhs)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));
    INFO_LHS (result) = NULL;

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
 * @}  <!-- INFO structure -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper funcions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn static bool IsGegtOperator( prf op)
 *
 * @brief Returns whether or not the given operator is a gt or ge operator
 *
 * @param op primitive operator
 *
 * @return is given operator a gt/ge operator
 *
 *****************************************************************************/
static bool
IsGtgeOperator (prf op)
{
    bool result;

    DBUG_ENTER ();

    DBUG_PRINT ("Looking for gt or ge operator");

    result = (op == F_ge_SxS || op == F_ge_SxV || op == F_ge_VxS || op == F_ge_VxV
              || op == F_gt_SxS || op == F_gt_SxV || op == F_gt_VxS || op == F_gt_VxV);

    if (result) {
        DBUG_PRINT ("Comparison operator found");
    } else {
        DBUG_PRINT ("Comparison operator NOT found");
    }

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn static prf GetContraryOperator( prf op)
 *
 * @brief Returns contrary operator
 *
 * @param op primitive operator
 *
 * @return is given contrary operator
 *
 *****************************************************************************/
static prf
GetContraryOperator (prf op)
{
    prf result;

    DBUG_ENTER ();

    DBUG_PRINT ("Looking for comparison operator");

    switch (op) {
    case F_gt_SxS:
        result = F_lt_SxS;
        break;
    case F_gt_SxV:
        result = F_lt_VxS;
        break;
    case F_gt_VxS:
        result = F_lt_SxV;
        break;
    case F_gt_VxV:
        result = F_lt_VxV;
        break;
    case F_ge_SxS:
        result = F_le_SxS;
        break;
    case F_ge_SxV:
        result = F_le_VxS;
        break;
    case F_ge_VxS:
        result = F_le_SxV;
        break;
    case F_ge_VxV:
        result = F_le_VxV;
        break;
    default:
        DBUG_ASSERT (0, "Illegal argument, must be a gt/ge operator");
        result = F_unknown;
    }

    DBUG_RETURN (result);
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
 * @fn node *TGTLmodule(node *arg_node, info *arg_info)
 *
 * @brief Traverses only functions of the module, skipping all the rest for
 *        performance reasons.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
TGTLmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TGTLfundef(node *arg_node, info *arg_info)
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
TGTLfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("traversing body of (%s) %s",
                (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef"),
                FUNDEF_NAME (arg_node));

    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TGTLblock(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
TGTLblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TGTLassign(node *arg_node, info *arg_info)
 *
 * @brief Traverses into instructions and inserts new assignments.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
TGTLassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TGTLlet(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
TGTLlet (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);

    INFO_LHS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TGTLprf(node *arg_node, info *arg_info)
 *
 * @brief This function looks for suitable operator, and transform all _gt_ or
 *        _ge_ to _lt_ and _le_.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
TGTLprf (node *arg_node, info *arg_info)
{
    node *first_argu;
    node *second_argu;

    DBUG_ENTER ();

    DBUG_PRINT ("Looking at prf for %s", AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));

    // Check for is the current operator are _gt_ or _ge_
    if (IsGtgeOperator (PRF_PRF (arg_node))) {
        first_argu = EXPRS_EXPR (PRF_ARGS (arg_node));
        second_argu = EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node)));

        EXPRS_EXPR (PRF_ARGS (arg_node)) = second_argu;
        EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))) = first_argu;

        PRF_PRF (arg_node) = GetContraryOperator (PRF_PRF (arg_node));

    } // end IsComparisonOperator...

    DBUG_PRINT ("Leaving prf");
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Conditional Zero Comparison -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *TGTLdoComparisonToZero( node *argnode)
 *
 *****************************************************************************/

node *
TGTLdoTransformGtgeToLtle (node *argnode)
{
    info *info;
    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_tgtl);
    argnode = TRAVdo (argnode, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (argnode);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

#undef DBUG_PREFIX
