/**
 *
 * @file sortassociativefunctionarguments.c
 *
 * @brief This function ensures that the arguments of dyadic* associative
 *        functions are in a canonical order. [It sort them today,
 *        but any canonical order would do.]
 *
 *        * If you need triadic, or worse, functions handled here, help
 *          yourself.
 *
 * We do the following transformation. Given expressions:
 *
 *    z1 = _add_SxS_( x, y);
 *    z2 = _add_SxS_( y, x);
 *
 *    z3 = _max_SxS_( x, y);
 *    z4 = _max_SxS_( y, x);
 *
 *  we produce these expressions:
 *
 *    z1 = _add_SxS_( x, y);
 *    z2 = _add_SxS_( x, y);
 *
 *    z3 = _max_SxS_( x, y);
 *    z4 = _max_SxS_( x, y);
 *
 *  Eventually, CSE will eliminate the 2nd expression in each group:
 *
 *    z1 = _add_SxS_( x, y);
 *    z2 = z1;
 *
 *    z3 = _max_SxS_( x, y);
 *    z4 = z3;
 *
 * The rationale for this transformation is that it allows CSE to remove
 * matching, but differently represented, expressions. CF bug1082.sac
 * had the following expressions for a PWL and CWL array bound elements:
 *
 *  colsx__SSA0_1 = _max_SxS_( _isaa_2820_colsx, _flat_40);
 *  _al_13163 = _max_SxS_( _flat_40, _isaa_2820_colsx);
 *
 *  The different ordering prevented AWLF from observing that the
 *  two values match.
 *
 */

#include "sortassociativefunctionarguments.h"

#include "new_types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "associative_law.h"

#define DBUG_PREFIX "SAFA"
#include "debug.h"

#include "memory.h"
#include "new_typecheck.h"
#include "globals.h"
#include "str.h"

/*
 * INFO structure
 */
struct INFO {
    bool onefundef;
    node *fundef;
    node *lhs;
};

/*
 * INFO macros
 */
#define INFO_ONEFUNDEF(n) ((n)->onefundef)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_LHS(n) ((n)->lhs)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_ONEFUNDEF (result) = FALSE;
    INFO_FUNDEF (result) = NULL;
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
 *
 * @fn node *SAFAmodule(node *arg_node, info *arg_info)
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
SAFAmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--**************************************************************-->
 *
 * @fn node *SAFAdoSortAssociativeFunctionArguments( node *arg_node)
 *
 * @brief start function for traversal at function level
 *
 * @param fundef fundef node
 *
 * @return
 *
 **********************************************************************/
node *
SAFAdoSortAssociativeFunctionArguments (node *arg_node)
{
    info *info;
    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_safa);
    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

/** <!--**************************************************************-->
 *
 * @fn node *SAFAfundef(node arg_node, info *arg_info)
 *
 * @brief Traverses into fundef local LAC fuctions, then function
 *        bodies and finally function next pointers. When traversing
 *        into a body a pointer in the info struct is maintained to
 *        the inner fundef.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 **********************************************************************/
node *
SAFAfundef (node *arg_node, info *arg_info)
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

/** <!--***************************************************************-->
 *
 * @fn SAFAblock(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 **********************************************************************/
node *
SAFAblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--***************************************************************-->
 *
 * @fn SAFAassign(node *arg_node, info *arg_info)
 *
 * @brief traverses instructions
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 **********************************************************************/
node *
SAFAassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--*************************************************************-->
 *
 * @fn SAFAlet(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 **********************************************************************/
node *
SAFAlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);
    INFO_LHS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--**************************************************************-->
 *
 * @fn SAFAprf(node *arg_node, info *arg_info)
 *
 * @brief Ensures that primitive function arguments are
 *        in canonical order.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 **********************************************************************/
prf
SwapPrfName (prf fun)
{
    DBUG_ENTER ();

    switch (fun) {
    case F_add_SxV:
        fun = F_add_VxS;
        break;
    case F_add_VxS:
        fun = F_add_SxV;
        break;

    case F_and_SxV:
        fun = F_and_VxS;
        break;
    case F_and_VxS:
        fun = F_and_SxV;
        break;

    case F_or_SxV:
        fun = F_or_VxS;
        break;
    case F_or_VxS:
        fun = F_or_SxV;
        break;

    case F_max_SxV:
        fun = F_max_VxS;
        break;
    case F_max_VxS:
        fun = F_max_SxV;
        break;

    case F_min_SxV:
        fun = F_min_VxS;
        break;
    case F_min_VxS:
        fun = F_min_SxV;
        break;

    case F_mul_SxV:
        fun = F_mul_VxS;
        break;
    case F_mul_VxS:
        fun = F_mul_SxV;
        break;

    case F_eq_SxV:
        fun = F_eq_VxS;
        break;
    case F_eq_VxS:
        fun = F_eq_SxV;
        break;

    case F_neq_SxV:
        fun = F_neq_VxS;
        break;
    case F_neq_VxS:
        fun = F_neq_SxV;
        break;

    default:
        break;
    }

    DBUG_RETURN (fun);
}

node *
SAFAprf (node *arg_node, info *arg_info)
{
    node *swap;

    DBUG_ENTER ();

    DBUG_PRINT ("Looking at prf for %s", AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));

    // This checks for associative AND commutative, which is overkill,
    // but I'm lazy. Feel free to write a more specific filter.
    if ((ALisAssociativeAndCommutativePrf (PRF_PRF (arg_node)))
        && (STRgt (AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                   AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node)))))) {
        // Today, we have min, max, add, mul, and, or, which are all
        // dyadic, so we don't need any checking here.
        swap = PRF_ARG1 (arg_node);
        PRF_ARG1 (arg_node) = PRF_ARG2 (arg_node);
        PRF_ARG2 (arg_node) = swap;
        PRF_PRF (arg_node) = SwapPrfName (PRF_PRF (arg_node));
        ++global.optcounters.safa_expr;
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
