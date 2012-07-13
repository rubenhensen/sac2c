/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup mc Move Constants
 *
 * <pre>
 * Property                                | should be | y/n |  who  |  when
 * =============================================================================
 * can be called on N_module               |   -----   |  y  |       |
 * can be called on N_fundef               |   -----   |  y  |       |
 * expects LaC funs                        |   -----   |     |       |
 * follows N_ap to LaC funs                |   -----   |  n  |       |
 * =============================================================================
 * deals with GLF properly                 |    yes    |  y  |       |
 * =============================================================================
 * is aware of potential SAA annotations   |    yes    |  y  |       |
 * utilises SAA annotations                |   -----   |  n  |       |
 * =============================================================================
 * tolerates flattened N_array             |    yes    |  y  |       |
 * tolerates flattened Generators          |    yes    |  y  |       |
 * tolerates flattened operation parts     |    yes    |  y  |       |
 * tolerates different generator variables
 *           in individual WL partitions   |    yes    |  y  |       |
 * =============================================================================
 * tolerates multi-operator WLs            |    yes    |  y  |       |
 * =============================================================================
 * </pre>
 *
 * Converts
 *
 * type{...} a;
 * ...
 * a = const;
 *
 * into
 *
 * type{...} a = const;
 *
 *
 * where type is any AKV scalar type
 * and const is a scalar constant
 * @ingroup tt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file move_const.c
 *
 * Prefix: MC
 *
 *****************************************************************************/
#include "move_const.h"

/*
 * Other includes go here
 */

#define DBUG_PREFIX "MC"
#include "debug.h"

#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "type_utils.h"
#include "new_types.h"
#include "memory.h"
#include "free.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *vardecs;
    bool dead_assign;
};

/**
 * INFO_VARDECS     vardec chain of current scope
 * INFO_DEAD_ASSIGN current assign is dead and should be removed
 */
#define INFO_VARDECS(n) (n->vardecs)
#define INFO_DEAD_ASSIGN(n) (n->dead_assign)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_VARDECS (result) = NULL;
    INFO_DEAD_ASSIGN (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    DBUG_ASSERT (INFO_VARDECS (info) == NULL,
                 "Unexpected link to vardecs still in info struct");

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

static node *
ATravIds (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    AVIS_COUNT (IDS_AVIS (arg_node))++;

    DBUG_RETURN (arg_node);
}

static node *
CountLhsUsage (node *syntax_tree)
{
    anontrav_t trav[2] = {{N_ids, &ATravIds}, {(nodetype)0, NULL}};
    DBUG_ENTER ();

    TRAVpushAnonymous (trav, &TRAVsons);

    syntax_tree = TRAVopt (syntax_tree, NULL);

    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *SetConst(node *arg_node)
 *
 * @brief save rhs into vardec of lhs
 *
 *****************************************************************************/
static void
SetConst (node *avis, node *rhs, node *vardecs)
{
    DBUG_ENTER ();

    DBUG_ASSERT (vardecs != NULL, "No more vardecs to check");

    if (avis == VARDEC_AVIS (vardecs)) {
        VARDEC_INIT (vardecs) = rhs;
    } else {
        SetConst (avis, rhs, VARDEC_NEXT (vardecs));
    }

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *MVdoMoveConst( node *syntax_tree)
 *
 *****************************************************************************/
node *
MCdoMoveConst (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    DBUG_PRINT ("Starting move const traversal.");

    syntax_tree = CountLhsUsage (syntax_tree);

    TRAVpush (TR_mc);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    DBUG_PRINT ("Move const traversal complete.");

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
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
 * @fn node *MCfundef(node *arg_node, info *arg_info)
 *
 * @brief Save the vardec chain in info struct
 *
 *****************************************************************************/
node *
MCfundef (node *arg_node, info *arg_info)
{
    node *stack = INFO_VARDECS (arg_info);
    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_VARDECS (arg_info) = BLOCK_VARDECS (FUNDEF_BODY (arg_node));
    }

    arg_node = TRAVcont (arg_node, arg_info);

    INFO_VARDECS (arg_info) = stack;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MCassign(node *arg_node, info *arg_info)
 *
 * @brief Remove this assign if no longer needed
 *
 *****************************************************************************/
node *
MCassign (node *arg_node, info *arg_info)
{
    bool stack = INFO_DEAD_ASSIGN (arg_info);
    DBUG_ENTER ();

    INFO_DEAD_ASSIGN (arg_info) = FALSE;

    arg_node = TRAVcont (arg_node, arg_info);

    if (INFO_DEAD_ASSIGN (arg_info)) {
        arg_node = FREEdoFreeNode (arg_node);
    }

    INFO_DEAD_ASSIGN (arg_info) = stack;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MClet(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
MClet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if ((LET_IDS (arg_node) != NULL)
        && TUisScalar (AVIS_TYPE (IDS_AVIS (LET_IDS (arg_node))))
        && (!TUisHidden (AVIS_TYPE (IDS_AVIS (LET_IDS (arg_node)))))) {

        if ((AVIS_COUNT (IDS_AVIS (LET_IDS (arg_node))) == 2)
            && (NODE_TYPE (LET_EXPR (arg_node)) == N_prf)
            && (PRF_PRF (LET_EXPR (arg_node)) == F_alloc)) {
            DBUG_ASSERT (IDS_NEXT (LET_IDS (arg_node)) == NULL,
                         "Expected const to be only var on lhs");
            AVIS_COUNT (IDS_AVIS (LET_IDS (arg_node)))--;
            INFO_DEAD_ASSIGN (arg_info) = TRUE;
        } else if ((AVIS_COUNT (IDS_AVIS (LET_IDS (arg_node))) == 1)
                   && TCisScalar (LET_EXPR (arg_node))) {
            DBUG_ASSERT (IDS_NEXT (LET_IDS (arg_node)) == NULL,
                         "Expected const to be only var on lhs");
            SetConst (IDS_AVIS (LET_IDS (arg_node)), LET_EXPR (arg_node),
                      INFO_VARDECS (arg_info));
            LET_EXPR (arg_node) = NULL; /* moved to vardec */
            AVIS_COUNT (IDS_AVIS (LET_IDS (arg_node)))--;
            INFO_DEAD_ASSIGN (arg_info) = TRUE;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/

#undef DBUG_PREFIX
