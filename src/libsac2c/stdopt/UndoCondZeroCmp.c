/**
 *
 *
 * @file UndoCondZeroCmp.c
 *
 * @brief optimize the bool assignment
 *
 * We do the following transformations:
 * a = t1 - t2
 * if (a > 0)
 * =>
 * if (t1 > t2)
 *
 */

#include "UndoCondZeroCmp.h"

#include "new_types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"

#define DBUG_PREFIX "UCZC"
#include "debug.h"

#include "memory.h"
#include "new_typecheck.h"
#include "globals.h"
#include "pattern_match.h"
#include "constants.h"
#include "DupTree.h"
#include "free.h"

/*
 * INFO structure
 */
struct INFO {
    bool onefundef;
    node *fundef;
    node *newassign;
    node *lhs;
};

/*
 * INFO macros
 */
#define INFO_ONEFUNDEF(n) ((n)->onefundef)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_NEWASSIGN(n) ((n)->newassign)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

    INFO_ONEFUNDEF (result) = FALSE;
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
    DBUG_ENTER ();

    DBUG_RETURN (op == F_eq_SxS || op == F_eq_SxV || op == F_eq_VxS || op == F_eq_VxV
                 || op == F_neq_SxS || op == F_neq_SxV || op == F_neq_VxS
                 || op == F_neq_VxV || op == F_le_SxS || op == F_le_SxV || op == F_le_VxS
                 || op == F_le_VxV || op == F_lt_SxS || op == F_lt_SxV || op == F_lt_VxS
                 || op == F_lt_VxV || op == F_ge_SxS || op == F_ge_SxV || op == F_ge_VxS
                 || op == F_ge_VxV || op == F_gt_SxS || op == F_gt_SxV || op == F_gt_VxS
                 || op == F_gt_VxV);
}

/** <!--********************************************************************-->
 *
 * @fn static prf ToScalarComparison( prf op)
 *
 * @brief If the given operator is a comparison operator, change it into
 * corresponding sub operator.
 *
 * @param op primitive operator
 *
 * @return sub operator
 *
 *****************************************************************************/
static prf
CmpToSub (prf op)
{
    DBUG_ENTER ();

    switch (op) {
    // SxS
    case F_le_SxS:
        op = F_sub_SxS;
        break;
    case F_lt_SxS:
        op = F_sub_SxS;
        break;
    case F_ge_SxS:
        op = F_sub_SxS;
        break;
    case F_gt_SxS:
        op = F_sub_SxS;
        break;

    // VxS
    case F_le_VxS:
        op = F_sub_VxS;
        break;
    case F_lt_VxS:
        op = F_sub_VxS;
        break;
    case F_ge_VxS:
        op = F_sub_VxS;
        break;
    case F_gt_VxS:
        op = F_sub_VxS;
        break;

    // SxV
    case F_le_SxV:
        op = F_sub_SxV;
        break;
    case F_lt_SxV:
        op = F_sub_SxV;
        break;
    case F_ge_SxV:
        op = F_sub_SxV;
        break;
    case F_gt_SxV:
        op = F_sub_SxV;
        break;

    // VxV
    case F_le_VxV:
        op = F_sub_VxV;
        break;
    case F_lt_VxV:
        op = F_sub_VxV;
        break;
    case F_ge_VxV:
        op = F_sub_VxV;
        break;
    case F_gt_VxV:
        op = F_sub_VxV;
        break;

    default:
        op = op;
    }

    DBUG_RETURN (op);
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

    argconst = COaST2Constant (node);

    if (NULL != argconst) {
        res = COisZero (argconst, TRUE);
        argconst = COfreeConstant (argconst);
    }

    DBUG_RETURN (res);
}

/**<!--***************************************************************-->
 *
 * @fn UCZCblock(node *arg_node, info *arg_info)
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
UCZCblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *UCZCassign(node *arg_node, info *arg_info)
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
UCZCassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    INFO_NEWASSIGN (arg_info) = NULL;

    ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_NEWASSIGN (arg_info) != NULL) {
        // insert new assignment node
        ASSIGN_NEXT (INFO_NEWASSIGN (arg_info)) = arg_node;
        arg_node = INFO_NEWASSIGN (arg_info);

        INFO_NEWASSIGN (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *UCZClet(node *arg_node, info *arg_info)
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
UCZClet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);
    INFO_LHS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

node *
UCZCprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    DBUG_PRINT ("Looking at prf for %s", AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));

    // if we find a pattern like (a > 0)
    // then look in the previous assignments
    // to see whether we can get sth like
    // a = t1 - t2
    // if so we change (a > 0) into (t1 > t2)
    if (IsComparisonOperator (PRF_PRF (arg_node))
        && IsNodeLiteralZero (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))))) {
        DBUG_PRINT ("find pattern a > 0");

        // the key issue now is find the pattern a = t1 -t2
        node *t1 = NULL;
        node *t2 = NULL;

        pattern *pat;

        pat = PMprf (1, PMAisPrf (CmpToSub (PRF_PRF (arg_node))), 2,
                     PMvar (1, PMAgetNode (&t1), 0), PMvar (1, PMAgetNode (&t2), 0));

        if (PMmatchFlat (pat, EXPRS_EXPR (PRF_ARGS (arg_node)))) {
            DBUG_PRINT ("pattern matches a = t1 - t2");

            // free exprs node d in expression d > 0
            FREEdoFreeNode (EXPRS_EXPR (PRF_ARGS (arg_node)));

            // free exprs node 0 in expression d > 0
            FREEdoFreeNode (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))));

            // make exprs node t1 and t2 and let them be the sons of prf node >
            PRF_ARGS (arg_node)
              = TBmakeExprs (DUPdoDupNode (t1), TBmakeExprs (DUPdoDupNode (t2), NULL));
        } else {
            DBUG_PRINT ("pattern unmatches");
        }

        pat = PMfree (pat);
    }

    DBUG_RETURN (arg_node);
}

/**<!--**************************************************************-->
 *
 * @fn node *UCZCdoUndoCondZeroCmp(node *fundef)
 *
 * @brief start function for traversal
 *
 * @param fundef fundef node
 *
 * @return
 *
 **********************************************************************/
node *
UCZCdoUndoCondZeroCmp (node *arg_node)
{
    info *info;
    DBUG_ENTER ();

    info = MakeInfo ();

    INFO_ONEFUNDEF (info) = TRUE;

    TRAVpush (TR_uczc);
    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

/**<!--**************************************************************-->
 *
 * @fn node *UCZCfundef(node arg_node, info *arg_info)
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
UCZCfundef (node *arg_node, info *arg_info)
{
    bool old_onefundef;

    DBUG_ENTER ();

    INFO_FUNDEF (arg_info) = arg_node;

    DBUG_PRINT ("traversing body of (%s) %s",
                (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef"),
                FUNDEF_NAME (arg_node));
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

#undef DBUG_PREFIX
