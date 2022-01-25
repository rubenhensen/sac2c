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
 *   As issue #2284 exposes, this operation, in general is not semantics
 *   preserving for any integer types!
 *
 *   Consider unsigned integers.
 *     1 > 2   is false
 *     (1-2) == MAX_UINT > 0 is true!
 *
 *   So for unisgned numbers this transformation almost always is illegal!
 *   Consequently, we do not apply to unsigned numbers at all!
 *
 *   Unfortunately, a similar problem exists for signed integers.
 *     MIN_INT > 1  is flase
 *     MIN_INT-1 == MAX_INT > 0 is true!
 *
 *   Here, we may argue that this is a rather rare situation. This is
 *   similar to the way we treat associative law and distributive law....
 *   so we only supress CTZ in case enforceIEEE is set.
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
 * Prefix: CTZ
 *
 *****************************************************************************/
#include "comparison_to_zero.h"

#define DBUG_PREFIX "CTZ"
#include "debug.h"

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
    node *fundef;
    node *newassign;
    node *lhs;
};

#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_NEWASSIGN(n) ((n)->newassign)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_NEWASSIGN (result) = NULL;

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
    DBUG_ENTER ();

    if (!global.enforce_ieee) {
        info = MakeInfo ();

        TRAVpush (TR_ctz);
        argnode = TRAVdo (argnode, info);
        TRAVpop ();

        info = FreeInfo (info);
    }

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
    bool result;

    DBUG_ENTER ();

    DBUG_PRINT ("Looking for comparison operator");

    result = (op == F_eq_SxS || op == F_eq_SxV || op == F_eq_VxS || op == F_eq_VxV
              || op == F_neq_SxS || op == F_neq_SxV || op == F_neq_VxS || op == F_neq_VxV
              || op == F_le_SxS || op == F_le_SxV || op == F_le_VxS || op == F_le_VxV
              || op == F_lt_SxS || op == F_lt_SxV || op == F_lt_VxS || op == F_lt_VxV
              || op == F_ge_SxS || op == F_ge_SxV || op == F_ge_VxS || op == F_ge_VxV
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
    ntype *type;
    simpletype simple;
    bool result;

    DBUG_ENTER ();

    DBUG_PRINT ("Checking for suitable type");

    DBUG_ASSERT (NODE_TYPE (node) == N_id, "Node must be an N_id node");

    type = AVIS_TYPE (ID_AVIS (node));

    if (TYisArray (type)) {
        type = TYgetScalar (type);
    }

    simple = TYgetSimpleType (type);

    result = simple == T_int || simple == T_byte || simple == T_short || simple == T_long
             || simple == T_longlong 
             || simple == T_double || simple == T_float;

    if (result) {
        DBUG_PRINT ("Suitable type found");
    } else {
        DBUG_PRINT ("Suitable type not found");
    }

    DBUG_RETURN (result);
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
    DBUG_ENTER ();

    switch (op) {
    case F_eq_SxV:
        op = F_eq_VxS;
        break;
    case F_eq_VxV:
        op = F_eq_VxS;
        break;
    case F_neq_SxV:
        op = F_neq_VxS;
        break;
    case F_neq_VxV:
        op = F_neq_VxS;
        break;
    case F_le_SxV:
        op = F_le_VxS;
        break;
    case F_le_VxV:
        op = F_le_VxS;
        break;
    case F_lt_SxV:
        op = F_lt_VxS;
        break;
    case F_lt_VxV:
        op = F_lt_VxS;
        break;
    case F_ge_SxV:
        op = F_ge_VxS;
        break;
    case F_ge_VxV:
        op = F_ge_VxS;
        break;
    case F_gt_SxV:
        op = F_gt_VxS;
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

    DBUG_ENTER ();

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
        DBUG_UNREACHABLE ("Illegal argument, must be a comparison operator");
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
 * @fn node *CTZmodule(node *arg_node, info *arg_info)
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
CTZmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

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
    DBUG_ENTER ();

    INFO_FUNDEF (arg_info) = arg_node;

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
    DBUG_ENTER ();

    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

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
    DBUG_ENTER ();

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    INFO_NEWASSIGN (arg_info) = NULL;

    ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);

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
    DBUG_ENTER ();

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
 *        optimization and creates the new structure.
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
    ntype *type_zero;
    ntype *type_zero_mem;
    ntype *type_sub;

    node *f_sub;
    node *n_zero;
    node *avis_sub;
    node *avis_zero;

    DBUG_ENTER ();

    DBUG_PRINT ("Looking at prf for %s", AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));

    // Check for comparisons that don't already use a literal zero
    if (IsComparisonOperator (PRF_PRF (arg_node))
        && !IsNodeLiteralZero (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))))
        && HasSuitableType (EXPRS_EXPR (PRF_ARGS (arg_node)))) {
        DBUG_PRINT ("Found suitable comparison function");

        // Create the new subtraction assignment with same arguments as comparison
        f_sub = TBmakePrf (GetSubtractionOperator (PRF_PRF (arg_node)),
                           TBmakeExprs (EXPRS_EXPR (PRF_ARGS (arg_node)),
                                        TBmakeExprs (EXPRS_EXPR (
                                                       EXPRS_NEXT (PRF_ARGS (arg_node))),
                                                     NULL)));

        type_zero = NTCnewTypeCheck_Expr (f_sub);
        type_zero_mem = TYgetProductMember (type_zero, 0);
        type_zero = TYfreeTypeConstructor (type_zero);

        // Create avis node for subtraction
        avis_sub = TBmakeAvis (TRAVtmpVar (), type_zero_mem);

        // Create new zero node where type is based on type of the subtraction
        type_sub = AVIS_TYPE (avis_sub);

        if (TYisArray (type_sub)) {
            type_sub = TYgetScalar (type_sub);
        }

        n_zero = NULL;
        switch (TYgetSimpleType (type_sub)) {
        case T_byte:
            DBUG_PRINT ("Type is byte");
            n_zero = TBmakeNumbyte (0);
            break;
        case T_short:
            DBUG_PRINT ("Type is short");
            n_zero = TBmakeNumshort (0);
            break;
        case T_int:
            DBUG_PRINT ("Type is int");
            n_zero = TBmakeNum (0);
            break;
        case T_long:
            DBUG_PRINT ("Type is long");
            n_zero = TBmakeNumlong (0);
            break;
        case T_longlong:
            DBUG_PRINT ("Type is longlong");
            n_zero = TBmakeNumlonglong (0);
            break;
        case T_double:
            DBUG_PRINT ("Type is double");
            n_zero = TBmakeDouble (0);
            break;
        case T_float:
            DBUG_PRINT ("Type is float");
            n_zero = TBmakeFloat (0);
            break;

        default:
            DBUG_UNREACHABLE ("Type is unknown, must be int, double or float");
        }

        // Avis node for zero
        avis_zero = TBmakeAvis (TRAVtmpVar (), TYcopyType (type_zero_mem));

        // Add the nodes to the instruction list
        INFO_NEWASSIGN (arg_info)
          = TBmakeAssign (TBmakeLet (TBmakeIds (avis_sub, NULL), f_sub),
                          TBmakeAssign (TBmakeLet (TBmakeIds (avis_zero, NULL), n_zero),
                                        NULL));

        AVIS_SSAASSIGN (avis_sub) = INFO_NEWASSIGN (arg_info);
        AVIS_SSAASSIGN (avis_zero) = ASSIGN_NEXT (INFO_NEWASSIGN (arg_info));

        // Create the new vardec nodes
        FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
          = TBmakeVardec (avis_sub, TBmakeVardec (avis_zero, FUNDEF_VARDECS (
                                                               INFO_FUNDEF (arg_info))));

        // Change the current comparison function
        PRF_PRF (arg_node) = ToScalarComparison (PRF_PRF (arg_node));
        PRF_ARG1 (arg_node) = TBmakeId (avis_sub);
        PRF_ARG2 (arg_node) = TBmakeId (avis_zero);
        global.optcounters.ctz_expr += 1;
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

#undef DBUG_PREFIX
