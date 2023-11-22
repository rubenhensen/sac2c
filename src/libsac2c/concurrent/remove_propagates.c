/*****************************************************************************
 *
 * file:   remove_propagates.c
 *
 * prefix: RMPR
 *
 * description:
 *
 * The purpose of this compiler transformation is to remove prop_obj_in and
 * prop_obj_out operations wherever they are not required. Given that they
 * turn into mutex lock/unlock operations eventually, superfluous propagates
 * are prone to cause deadlock.
 *
 * The following occurrences are removed:
 *
 * All occurrences in MT/ST functions. In sequential code we do not need any
 * locks, and in MT code we will already be within the dynamic scope of another
 * propagate pair.
 *
 * In SPMD functions we keep the top-most pair of propagates and removed all
 * propagates in nested with-loops.
 *
 * As a future optimisation we could check whether the outer pair of propagates
 * solely guards a single with-loop. In this case we could remove the outer
 * pair and continue our transformation on the inner nesting level.
 *
 *****************************************************************************/

#include "remove_propagates.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "memory.h"

/**
 * INFO structure
 */

struct INFO {
    bool remove;
    int level;
};

/**
 * INFO macros
 */

#define INFO_REMOVEASSIGN(n) ((n)->remove)
#define INFO_LEVEL(n) ((n)->level)

/**
 * INFO functions
 */

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_REMOVEASSIGN (result) = FALSE;
    INFO_LEVEL (result) = 0;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * @fn node *RMPRmodule( node *arg_node, info *arg_info)
 *
 *  @brief RMPR traversal function for N_module node
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 *****************************************************************************/

node *
RMPRmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt(MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *RMPRfundef( node *arg_node, info *arg_info)
 *
 *  @brief RMPR traversal function for N_fundef node
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 *****************************************************************************/

node *
RMPRfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (!FUNDEF_ISSPMDFUN (arg_node)) {
        INFO_LEVEL (arg_info) += 1;
    }

    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    if (!FUNDEF_ISSPMDFUN (arg_node)) {
        INFO_LEVEL (arg_info) -= 1;
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *RMPRassign( node *arg_node, info *arg_info)
 *
 *  @brief RMPR traversal function for N_assign node
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 *****************************************************************************/

node *
RMPRassign (node *arg_node, info *arg_info)
{
    bool remove;

    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    remove = INFO_REMOVEASSIGN (arg_info);
    INFO_REMOVEASSIGN (arg_info) = FALSE;

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    if (remove) {
        arg_node = FREEdoFreeNode (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *RMPRwith2( node *arg_node, info *arg_info)
 *
 *  @brief RMPR traversal function for N_with2 node
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 *****************************************************************************/

node *
RMPRwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LEVEL (arg_info) += 1;
    WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
    INFO_LEVEL (arg_info) -= 1;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *RMPRprf( node *arg_node, info *arg_info)
 *
 *  @brief RMPR traversal function for N_prf node
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 *****************************************************************************/

node *
RMPRprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    node *id = NULL;

    if (INFO_LEVEL (arg_info) > 1) {
        if (PRF_PRF (arg_node) == F_prop_obj_in) {
            switch (TCcountExprs (PRF_ARGS (arg_node))) {
            case 1:
                INFO_REMOVEASSIGN (arg_info) = TRUE;
                break;
            case 2:
                id = PRF_ARG2 (arg_node);
                PRF_ARG2 (arg_node) = NULL;
                break;
            default:
                DBUG_UNREACHABLE ("prop_obj_in with other than 1 or 2 args encountered");
            }
        } else if (PRF_PRF (arg_node) == F_prop_obj_out) {
            switch (TCcountExprs (PRF_ARGS (arg_node))) {
            case 0:
                INFO_REMOVEASSIGN (arg_info) = TRUE;
                break;
            case 1:
                id = PRF_ARG1 (arg_node);
                PRF_ARG1 (arg_node) = NULL;
                break;
            default:
                DBUG_UNREACHABLE ("prop_obj_out with more than 1 arg encountered");
            }
        }
    }

    if (id != NULL) {
        arg_node = FREEdoFreeNode (arg_node);
        arg_node = id;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *RMPRdoRemovePropagates( node *syntax_tree)
 *
 *  @brief initiates RMPR traversal
 *
 *  @param syntax_tree
 *
 *  @return syntax_tree
 *
 *****************************************************************************/

node *
RMPRdoRemovePropagates (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (syntax_tree) == N_module, "Illegal argument node!");

    info = MakeInfo ();

    TRAVpush (TR_rmpr);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
