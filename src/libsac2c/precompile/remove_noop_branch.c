
/**
 *
 * @defgroup rnb Remove noop conditional branch
 *
 * @ingroup mm
 *
 * @{
 */

/**
 *
 * @file remove_noop_branch.c
 *
 * Prefix: RWO
 */
#include "remove_noop_branch.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"

#define DBUG_PREFIX "RNB"
#include "debug.h"

#include "print.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "DupTree.h"
#include "new_types.h"

/*
 * INFO structure
 */
struct INFO {
    bool inwl;
    bool noop_branch;
    node *wlasslet;
    node *resavis;
    node *postassign;
};

/*
 * INFO macros
 */
#define INFO_INWL(n) (n->inwl)
#define INFO_NOOP_BRANCH(n) (n->noop_branch)
#define INFO_WLASSLET(n) (n->wlasslet)
#define INFO_RESAVIS(n) (n->resavis)
#define INFO_POSTASSIGN(n) (n->postassign)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_INWL (result) = FALSE;
    INFO_NOOP_BRANCH (result) = FALSE;
    INFO_WLASSLET (result) = NULL;
    INFO_RESAVIS (result) = NULL;
    INFO_POSTASSIGN (result) = NULL;

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
 * @fn node *RNBdoRemoveNoopBranch( node *syntax_tree)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
RNBdoRemoveNoopBranch (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_rnb);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *RNBwith( node *arg_node, info* arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
RNBwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (!INFO_INWL (arg_info)) {
        INFO_INWL (arg_info) = TRUE;
        WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), arg_info);
        INFO_INWL (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *RNBwith2( node *arg_node, info* arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
RNBwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (!INFO_INWL (arg_info)) {
        INFO_INWL (arg_info) = TRUE;
        WITH2_CODE (arg_node) = TRAVopt (WITH2_CODE (arg_node), arg_info);
        INFO_INWL (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *RNBcode( node *arg_node, info* arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
RNBcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_RESAVIS (arg_info) = ID_AVIS (EXPRS_EXPR (CODE_CEXPRS (arg_node)));
    INFO_NOOP_BRANCH (arg_info) = FALSE;
    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);
    INFO_RESAVIS (arg_info) = NULL;

    if (INFO_NOOP_BRANCH (arg_info)) {
        DBUG_ASSERT (INFO_WLASSLET (arg_info) != NULL, "WITH-loop assign is NULL!");
        PRF_PRF (LET_EXPR (INFO_WLASSLET (arg_info))) = F_noop;
        INFO_NOOP_BRANCH (arg_info) = TRUE;
    }

    CODE_NEXT (arg_node) = TRAVopt (CODE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *RNBassign( node *arg_node, info* arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
RNBassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_POSTASSIGN (arg_info) != NULL) {
        ASSIGN_NEXT (INFO_POSTASSIGN (arg_info)) = ASSIGN_NEXT (arg_node);
        ASSIGN_NEXT (arg_node) = INFO_POSTASSIGN (arg_info);
        INFO_POSTASSIGN (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *RNBlet( node *arg_node, info* arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
RNBlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_TYPE (LET_EXPR (arg_node)) == N_prf
        && PRF_PRF (LET_EXPR (arg_node)) == F_wl_assign) {
        INFO_WLASSLET (arg_info) = arg_node;
    } else if (LET_IDS (arg_node) != NULL
               && IDS_AVIS (LET_IDS (arg_node)) == INFO_RESAVIS (arg_info)
               && INFO_NOOP_BRANCH (arg_info)) {
        DBUG_ASSERT (INFO_WLASSLET (arg_info) != NULL, "WITH-loop assign is NULL!");
        INFO_POSTASSIGN (arg_info)
          = TBmakeAssign (DUPdoDupNode (INFO_WLASSLET (arg_info)), NULL);
    } else {
        LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *RNBcond( node *arg_node, info* arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
RNBcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_ASSERT (!(COND_ISTHENNOOP (arg_node) && COND_ISELSENOOP (arg_node)),
                 "A conditional cannot be noop in both branches!");

    if (COND_ISTHENNOOP (arg_node)) {
        INFO_NOOP_BRANCH (arg_info) = TRUE;
        COND_THENASSIGNS (arg_node) = FREEdoFreeTree (COND_THENASSIGNS (arg_node));
        COND_THENASSIGNS (arg_node) = NULL;

        COND_ELSEASSIGNS (arg_node) = TRAVopt (COND_ELSEASSIGNS (arg_node), arg_info);
    } else if (COND_ISELSENOOP (arg_node)) {
        INFO_NOOP_BRANCH (arg_info) = TRUE;
        COND_ELSEASSIGNS (arg_node) = FREEdoFreeTree (COND_ELSEASSIGNS (arg_node));
        COND_ELSEASSIGNS (arg_node) = NULL;

        COND_THENASSIGNS (arg_node) = TRAVopt (COND_THENASSIGNS (arg_node), arg_info);
    } else {
        COND_THENASSIGNS (arg_node) = TRAVopt (COND_THENASSIGNS (arg_node), arg_info);
        COND_ELSEASSIGNS (arg_node) = TRAVopt (COND_ELSEASSIGNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
