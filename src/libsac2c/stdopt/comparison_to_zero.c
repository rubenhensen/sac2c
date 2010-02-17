/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup ctz introduce a comparison to zero in compare statements
 *
 *   This module searches for comparisons and transforms them to a comparison
 *   with zero instead. This allows for further optimization of complex
 *   comparisons. For example:
 *
 *     a + c > b + c
 *
 *   Will not be optimized any further. This comparison can be transformed
 *   by adding a subtraction and comparing that to zero.
 *
 *     (a + c) - (b + c) > 0
 *
 *   This change is the only operation this module does.
 *   Other optimizations will optimize the left hand side to a - b > 0 and
 *   finally the undo comparison to zero optimization will later transform
 *   the statement back to a > b.
 *
 *
 * TODO: test support for vector comparisons
 *
 *
 * @ingroup opt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file comparison_to_zero.c
 *
 * Prefix: CZC
 *
 *****************************************************************************/
#include "comparison_to_zero.h"

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
    bool onefundef;
    node *fundef;
    node *newassign;
    node *lhs;
};

#define INFO_ONEFUNDEF(n) ((n)->onefundef)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_NEWASSIGN(n) ((n)->newassign)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_ONEFUNDEF (result) = TRUE;
    INFO_FUNDEF (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_NEWASSIGN (result) = NULL;

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
 * @fn node *CTZdoComparisonToZero( node *argnode)
 *
 *****************************************************************************/

node *
CTZdoComparisonToZero (node *argnode)
{
    info *info;
    DBUG_ENTER ("CTZdoComparisonToZero");

    info = MakeInfo ();

    INFO_ONEFUNDEF (info) = TRUE;

    TRAVpush (TR_ctz);
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
 * @name Static helper funcions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn static bool IsComparisonOperator( prf op)
 *
 * @brief Returns whether or not the given operator is a comparison operator
 *
 * @param op primitive operator
 *
 * @return is given operator a comparison operator
 *
 *****************************************************************************/
static bool
IsComparisonOperator (prf op)
{
    DBUG_ENTER ("IsComparisonOperator");

    DBUG_PRINT ("CTZ", ("Looking for comparison operator"));

    bool res
      = (op == F_eq_SxS || op == F_eq_SxV || op == F_eq_VxS || op == F_eq_VxV
         || op == F_neq_SxS || op == F_neq_SxV || op == F_neq_VxS || op == F_neq_VxV
         || op == F_le_SxS || op == F_le_SxV || op == F_le_VxS || op == F_le_VxV
         || op == F_lt_SxS || op == F_lt_SxV || op == F_lt_VxS || op == F_lt_VxV
         || op == F_ge_SxS || op == F_ge_SxV || op == F_ge_VxS || op == F_ge_VxV
         || op == F_gt_SxS || op == F_gt_SxV || op == F_gt_VxS || op == F_gt_VxV);

    if (res)
        DBUG_PRINT ("CTZ", ("Comp. op. found :)"));
    else
        DBUG_PRINT ("CTZ", ("Comp. op. not found :("));

    DBUG_RETURN (res);
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

    DBUG_ENTER ("IsNodeLiteralZero");
    DBUG_PRINT ("CTZ", ("Comparing to zero"));

    argconst = COaST2Constant (node);

    if (NULL != argconst) {
        res = COisZero (argconst, TRUE);
        argconst = COfreeConstant (argconst);
    }

    if (res)
        DBUG_PRINT ("CTZ", ("Zero found"));
    else
        DBUG_PRINT ("CTZ", ("Zero not found"));

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn static bool HasSuitableType(node *node)
 *
 * @brief This function checks to see of the given argument node has a
 *        suitable type. A suitable types is int, double or float.
 *
 * @param node *node must be an N_id node
 *
 * @return whether node is of type int, double or float
 *
 *****************************************************************************/
static bool
HasSuitableType (node *node)
{
    DBUG_ENTER ("IsSuitableType");

    DBUG_PRINT ("CTZ", ("Checking for type..."));

    DBUG_ASSERT (NODE_TYPE (node) == N_id, "Node must be an N_id node");

    ntype *type = AVIS_TYPE (ID_AVIS (node));

    if (TYisArray (type))
        type = TYgetScalar (type);

    simpletype simple = TYgetSimpleType (type);

    bool res = simple == T_int || simple == T_double || simple == T_float;

    if (res)
        DBUG_PRINT ("CTZ", ("Suitable type found"));
    else
        DBUG_PRINT ("CTZ", ("Suitable type not found"));

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn static prf ToScalarComparison( prf op)
 *
 * @brief If the given operator is a comparison operator in which the second
 *        argument is a vector, this function returns the same comparison
 *        operator, but the second argument is now a scalar.
 *
 * @param op primitive operator
 *
 * @return comparison operator with second argument a scalar
 *
 *****************************************************************************/
static prf
ToScalarComparison (prf op)
{
    DBUG_ENTER ("ToScalarComparison");

    switch (op) {
    case F_eq_SxV:
        op = F_eq_SxS;
        break;
    case F_eq_VxV:
        op = F_eq_VxS;
        break;
    case F_neq_SxV:
        op = F_neq_SxS;
        break;
    case F_neq_VxV:
        op = F_neq_VxS;
        break;
    case F_le_SxV:
        op = F_le_SxS;
        break;
    case F_le_VxV:
        op = F_le_VxS;
        break;
    case F_lt_SxV:
        op = F_lt_SxS;
        break;
    case F_lt_VxV:
        op = F_lt_VxS;
        break;
    case F_ge_SxV:
        op = F_ge_SxS;
        break;
    case F_ge_VxV:
        op = F_ge_VxS;
        break;
    case F_gt_SxV:
        op = F_gt_SxS;
        break;
    case F_gt_VxV:
        op = F_gt_VxS;
        break;
    default:
        break;
    }

    DBUG_RETURN (op);
}

/** <!--********************************************************************-->
 *
 * @fn static prf GetSubtractionOperator( prf op)
 *
 * @brief This function returns the subtraction operator with the same
 *        argument types as the given comparison operator.
 *
 * @param prf comparison operator
 *
 * @return subtraction operator
 *
 *****************************************************************************/
static prf
GetSubtractionOperator (prf op)
{
    prf result;
    DBUG_ENTER ("GetSubtractionOperator");

    switch (op) {
    case F_eq_SxS:
    case F_neq_SxS:
    case F_le_SxS:
    case F_lt_SxS:
    case F_ge_SxS:
    case F_gt_SxS:
        result = F_sub_SxS;
        break;

    case F_eq_SxV:
    case F_neq_SxV:
    case F_le_SxV:
    case F_lt_SxV:
    case F_ge_SxV:
    case F_gt_SxV:
        result = F_sub_SxV;
        break;

    case F_eq_VxS:
    case F_neq_VxS:
    case F_le_VxS:
    case F_lt_VxS:
    case F_ge_VxS:
    case F_gt_VxS:
        result = F_sub_VxS;
        break;

    case F_eq_VxV:
    case F_neq_VxV:
    case F_le_VxV:
    case F_lt_VxV:
    case F_ge_VxV:
    case F_gt_VxV:
        result = F_sub_VxV;
        break;

    default:
        DBUG_ASSERT ((0), "Illegal argument, must be a comparison operator");
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
 * @fn node *CTZfundef(node *arg_node, info *arg_info)
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
CTZfundef (node *arg_node, info *arg_info)
{
    bool old_onefundef;

    DBUG_ENTER ("CTZfundef");

    INFO_FUNDEF (arg_info) = arg_node;

    DBUG_PRINT ("CTZ", ("traversing body of (%s) %s",
                        (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef"),
                        FUNDEF_NAME (arg_node)));

    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    old_onefundef = INFO_ONEFUNDEF (arg_info);
    INFO_ONEFUNDEF (arg_info) = FALSE;
    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    INFO_ONEFUNDEF (arg_info) = old_onefundef;

    if (!INFO_ONEFUNDEF (arg_info)) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CTZblock(node *arg_node, info *arg_info)
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
CTZblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CTZblock");

    BLOCK_INSTR (arg_node) = TRAVopt (BLOCK_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CTZassign(node *arg_node, info *arg_info)
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
CTZassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CTZassign");

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    INFO_NEWASSIGN (arg_info) = NULL;

    ASSIGN_INSTR (arg_node) = TRAVopt (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_NEWASSIGN (arg_info) != NULL) {
        // insert 2 new assignment nodes
        ASSIGN_NEXT (ASSIGN_NEXT (INFO_NEWASSIGN (arg_info))) = arg_node;
        arg_node = INFO_NEWASSIGN (arg_info);

        INFO_NEWASSIGN (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CTZlet(node *arg_node, info *arg_info)
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
CTZlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CTZlet");

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);
    INFO_LHS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CTZprf(node *arg_node, info *arg_info)
 *
 * @brief This function looks for suitable comparisons, applies the
 *        ptimization and creates the new structure.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
CTZprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CTZprf");

    DBUG_PRINT ("CTZ",
                ("Looking at prf for %s", AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info)))));

    // Check for comparisons that don't already use a literal zero
    if (IsComparisonOperator (PRF_PRF (arg_node))
        && !IsNodeLiteralZero (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))))
        && HasSuitableType (EXPRS_EXPR (PRF_ARGS (arg_node)))) {
        DBUG_PRINT ("CTZ", ("Found suitable comparison function"));

        // Create the new subtraction assignment with same arguments as comparison
        node *f_sub = TBmakePrf (GetSubtractionOperator (PRF_PRF (arg_node)),
                                 TBmakeExprs (EXPRS_EXPR (PRF_ARGS (arg_node)),
                                              TBmakeExprs (EXPRS_EXPR (EXPRS_NEXT (
                                                             PRF_ARGS (arg_node))),
                                                           NULL)));

        ntype *pt = NTCnewTypeCheck_Expr (f_sub);
        ntype *ptype = TYgetProductMember (pt, 0);

        // Create avis node for subtraction
        node *avissub = TBmakeAvis (TRAVtmpVar (), TYcopyType (ptype));

        // Create new zero node where type is based on type of the comparison
        ntype *type = AVIS_TYPE (avissub);

        if (TYisArray (type))
            type = TYgetScalar (type);

        simpletype simple = TYgetSimpleType (type);

        node *n_zero = NULL;

        switch (simple) {
        case T_int:
            DBUG_PRINT ("CTZ", ("Type is int"));
            n_zero = TBmakeNum (0);
            break;
        case T_double:
            DBUG_PRINT ("CTZ", ("Type is double"));
            n_zero = TBmakeDouble (0);
            break;

        case T_float:
            DBUG_PRINT ("CTZ", ("Type is float"));
            n_zero = TBmakeFloat (0);
            break;

        default:
            DBUG_ASSERT ((0), "Type is unknown, must be int, double or float");
        }

        // Avis node for zero
        node *aviszero = TBmakeAvis (TRAVtmpVar (), TYcopyType (ptype));

        ptype = TYfreeType (ptype);

        // Add the nodes to the instruction list
        INFO_NEWASSIGN (arg_info)
          = TBmakeAssign (TBmakeLet (TBmakeIds (avissub, NULL), f_sub),
                          TBmakeAssign (TBmakeLet (TBmakeIds (aviszero, NULL), n_zero),
                                        NULL));

        AVIS_SSAASSIGN (avissub) = INFO_NEWASSIGN (arg_info);
        AVIS_SSAASSIGN (aviszero) = ASSIGN_NEXT (INFO_NEWASSIGN (arg_info));

        // Create the new vardec nodes
        FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
          = TBmakeVardec (avissub, TBmakeVardec (aviszero,
                                                 FUNDEF_VARDEC (INFO_FUNDEF (arg_info))));

        // Change the current comparison function
        PRF_PRF (arg_node) = ToScalarComparison (PRF_PRF (arg_node));
        PRF_ARG1 (arg_node) = TBmakeId (avissub);
        PRF_ARG2 (arg_node) = TBmakeId (aviszero);

    } // end IsComparisonOperator...

    DBUG_PRINT ("CTZ", ("Leaving prf"));
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Conditional Zero Comparison -->
 *****************************************************************************/
