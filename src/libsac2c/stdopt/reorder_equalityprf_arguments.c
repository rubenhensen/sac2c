/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup REA Reorder arguments of primitive function _eq_XxX
 *
 *   For _eq_XxX(b,a), it will be transformed into _eq_XxX(a,b);
 *
 *
 * @ingroup opt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file reorder_equalityprf_arugment.c
 *
 * Prefix: CZC
 *
 *****************************************************************************/
#include "reorder_equalityprf_arguments.h"

#define DBUG_PREFIX "REA"
#include "debug.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "memory.h"
#include "traverse.h"
#include "new_types.h"
#include "constants.h"
#include "type_utils.h"
#include "globals.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "DupTree.h"
#include "pattern_match.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/

static enum equal_type {
    not_equal,
    equal_same,
    equal_diff
}

struct INFO {
    node *new_equal_prf;
    node *lhs;
};

#define INFO_NEWEQUALPRF(n) ((n)->new_equal_prf)
#define INFO_LHS(n) ((n)->lhs)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));
    INFO_NEWEQUALPRF (result) = NULL;
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
 * @fn static bool IsEqualityOperator( prf op)
 *
 * @brief Returns whether or not the given operator is a equality operator
 *
 * @param op primitive operator
 *
 * @return is given operator a comparison operator
 *
 *****************************************************************************/
static equal_type
IsEqualityOperator (prf op)
{
    bool result;

    DBUG_ENTER ();

    DBUG_PRINT ("Looking for comparison operator");

    result = (op == F_eq_SxS || op == F_eq_VxV || op == F_eq_SxV || F_eq_VxS);

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
    case F_eq_SxS:
        result = F_eq_SxS;
        break;
    case F_eq_VxV:
        result = F_eq_VxV;
        break;
    case F_eq_SxV:
        result = F_eq_VxS;
        break;
    case F_eq_VxS:
        result = F_eq_SxV;
        break;
    default:
        DBUG_ASSERT (0, "Illegal argument, must be a gt/ge operator");
        result = F_unknown;
    }

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn static bool AreId( prf op)
 *
 * @brief Returns whether or not the given arguments are ids
 *
 * @param arguments
 *
 * @return is true or false
 *
 *****************************************************************************/
static bool
AreId (node *first_argu, node *second_argu)
{
    bool result;

    DBUG_ENTER ();

    DBUG_PRINT ("Checking whether all arguments of equality operator are ids");

    result = (NODE_TYPE (first_argu) == N_id && NODE_TYPE (second_argu) == N_id);

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn static bool IsNodeLiteralZero( node *node)
 *
 * @brief This function checks if the given node is a literal 0.
 *
 * @param node
 *
 * @return is given node a 0
 *
 *****************************************************************************/
static bool
IsNodeLiteralZero (node *node)
{
    constant *argconst;
    bool res = FALSE;

    DBUG_ENTER ();
    DBUG_PRINT ("Comparing to zero");

    argconst = COaST2Constant (node);

    if (NULL != argconst) {
        res = COisZero (argconst, TRUE);
        argconst = COfreeConstant (argconst);
    }

    if (res) {
        DBUG_PRINT ("Zero found");
    } else {
        DBUG_PRINT ("Zero not found");
    }

    DBUG_RETURN (res);
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
 * @fn node *REAmodule(node *arg_node, info *arg_info)
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
REAmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *REAfundef(node *arg_node, info *arg_info)
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
REAfundef (node *arg_node, info *arg_info)
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
 * @fn node *REAblock(node *arg_node, info *arg_info)
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
REAblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *REAassign(node *arg_node, info *arg_info)
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
REAassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *REAlet(node *arg_node, info *arg_info)
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
REAlet (node *arg_node, info *arg_info)
{
    node *previous_prf;

    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    INFO_NEWEQUALPRF (arg_info) = NULL;

    LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);

    if (INFO_NEWEQUALPRF (arg_info) != NULL) {
        previous_prf = LET_EXPR (arg_node);
        previous_prf = FREEdoFreeNode (previous_prf);

        LET_EXPR (arg_node) = INFO_NEWEQUALPRF (arg_info);
        INFO_NEWEQUALPRF (arg_info) = NULL;
    }

    INFO_LHS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *REAprf(node *arg_node, info *arg_info)
 *
 * @brief This function looks for suitable comparisons, applies the
 *        optimization and creates the new structure.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
REAprf (node *arg_node, info *arg_info)
{
    node *first_argu;
    node *second_argu;
    char *first_id;
    char *second_id;
    node *new_equal_prf;

    DBUG_ENTER ();

    DBUG_PRINT ("Looking at prf for %s", AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));

    if (IsEqualityOperator (PRF_PRF (arg_node))
        && !IsNodeLiteralZero (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))))
        && AreId (EXPRS_EXPR (PRF_ARGS (arg_node)),
                  EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))))) {
        DBUG_PRINT ("Found equality function");

        first_id = AVIS_NAME (ID_AVIS (EXPRS_EXPR (PRF_ARGS (arg_node))));
        second_id = AVIS_NAME (ID_AVIS (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node)))));

        if (STRgt (first_id, second_id)) {
            first_argu = EXPRS_NEXT (PRF_ARGS (arg_node));
            second_argu = PRF_ARGS (arg_node);

            // reset the link
            EXPRS_NEXT (first_argu) = second_argu;
            EXPRS_NEXT (second_argu) = NULL;
            PRF_ARGS (arg_node) = NULL;
            // create new pritive operator
            new_equal_prf
              = TBmakePrf (GetContraryOperator (PRF_PRF (arg_node)), first_argu);

            PRF_ISNOTEINTERSECTPRESENT (new_equal_prf)
              = PRF_ISNOTEINTERSECTPRESENT (arg_node);

            PRF_ISINPLACESELECT (new_equal_prf) = PRF_ISINPLACESELECT (arg_node);

            PRF_ISNOP (new_equal_prf) = PRF_ISNOP (arg_node);

            INFO_NEWEQUALPRF (arg_info) = new_equal_prf;
        }

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
 * @fn node *REAdoComparisonToZero( node *argnode)
 *
 *****************************************************************************/

node *
REAdoReorderEqualityprfArguments (node *argnode)
{
    info *info;
    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_rea);
    argnode = TRAVdo (argnode, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (argnode);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

#undef DBUG_PREFIX
