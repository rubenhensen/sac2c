/******************************************************************************
 *
 * If the user enabled the -ecc option, the type_pattern_resolve traversal will
 * insert functions for checking whether the pre- and post-conditions imposed by
 * type patterns are satisfied. At this point, some constraints might have
 * already been resolved statically. Any remaining constraints will have to be
 * checked dynamically at run-time.
 *
 * If the user enabled -ecc, but not -check c, we use this traversal to strip
 * these constraints from the program. We do so by removing the inserted guard
 * functions, after which dead code removal is able to remove the inserted check
 * functions.
 *
 * E.g. the function
 *
 * inline int foo (int[*] a) {
 *   pred = foo_pre (a);
 *   a = guard (a, pred);
 *   res = foo_impl (a);
 *   pred = foo_post (a, res);
 *   res = guard (res, pred);
 *   return res;
 * }
 *
 * Is converted to
 *
 * inline int foo (int[*] a) {
 *   pred = foo_pre (a);
 *   res = foo_impl (a);
 *   pred = foo_post (a, res);
 *   return res;
 * }
 *
 * After which dead code removal finds that pred is not used, removing the pre-
 * and post-condition functions.
 *
 * inline int foo (int[*] a) {
 *   res = foo_impl (a);
 *   return res;
 * }
 *
 * Note that by enabling -ecc, guards are inserted, which might hinder
 * optimisation if some constraints could not be resolved statically. Thus we
 * inform the user about constraints that could not be resolved in the
 * type_pattern_statistics traversal.
 *
 ******************************************************************************/
#include "DupTree.h"
#include "free.h"
#include "globals.h"
#include "memory.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"

#define DBUG_PREFIX "STPC"
#include "debug.h"

#include "strip_type_pattern_checks.h"

struct INFO {
    node *lhs;
    node *pre_assigns;
    bool scrap_assign;
};

#define INFO_LHS(n) ((n)->lhs)
#define INFO_PREASSIGNS(n) ((n)->pre_assigns)
#define INFO_SCRAPASSIGN(n) ((n)->scrap_assign)

static info *
MakeInfo (void)
{
    info *res;

    DBUG_ENTER ();

    res = (info *)MEMmalloc (sizeof (info));

    INFO_LHS (res) = NULL;
    INFO_PREASSIGNS (res) = NULL;
    INFO_SCRAPASSIGN (res) = FALSE;

    DBUG_RETURN (res);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

static node *
RenameOrReplaceRets (size_t no, node *ids, node *args, node **assigns)
{
    node *tmp, *expr;

    DBUG_ENTER ();

    if (no > 0) {
        IDS_NEXT (ids) = RenameOrReplaceRets (no - 1,
                                              IDS_NEXT (ids),
                                              EXPRS_NEXT (args),
                                              assigns);

        if (NODE_TYPE (EXPRS_EXPR (args)) == N_id) {
            DBUG_ASSERT (AVIS_SUBST (IDS_AVIS (ids)) == NULL,
                         "AVIS_SUBST already set");

            DBUG_PRINT ("aliasing %s", IDS_NAME (ids));
            AVIS_SUBST (IDS_AVIS (ids)) = ID_AVIS (EXPRS_EXPR (args));
        } else {
            DBUG_PRINT ("inserting substitution for %s", IDS_NAME (ids));

            tmp = ids;
            ids = IDS_NEXT (ids);
            IDS_NEXT (tmp) = NULL;

            expr = DUPdoDupTree (EXPRS_EXPR (args));
            *assigns = TBmakeAssign (TBmakeLet (tmp, expr), *assigns);
            AVIS_SSAASSIGN (IDS_AVIS (tmp)) = *assigns;
        }
    } else if (ids != NULL) {
        IDS_NEXT (ids) = RenameOrReplaceRets (no,
                                              IDS_NEXT (ids),
                                              args,
                                              assigns);

        DBUG_PRINT ("setting %s to true", IDS_NAME (ids));

        tmp = ids;
        ids = IDS_NEXT (ids);
        IDS_NEXT (tmp) = NULL;

        expr = TBmakeBool (TRUE);
        *assigns = TBmakeAssign (TBmakeLet (tmp, expr), *assigns);
        AVIS_SSAASSIGN (IDS_AVIS (tmp)) = *assigns;
    }

    DBUG_RETURN (ids);
}

/******************************************************************************
 *
 * @fn node *STPCdoStripTypePatternChecks (node *arg_node)
 *
 * @brief Gets rid of inserted type pattern checks by removing all applications
 * of the primitive guard function. Only removing these guards is sufficient, as
 * dead code removal will then be able to remove applications of the pre- and
 * post-condition functions since their boolean result is only used by the
 * guards.
 *
 ******************************************************************************/
node *
STPCdoStripTypePatternChecks (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_module,
                 "called with non-module node");

    arg_info = MakeInfo ();

    TRAVpush (TR_stpc);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

node *
STPCblock (node *arg_node, info *arg_info)
{
    node *old_lhs, *pre_assigns;

    DBUG_ENTER ();

    old_lhs = INFO_LHS (arg_info);
    INFO_LHS (arg_info) = NULL;
    pre_assigns = INFO_PREASSIGNS (arg_info);
    INFO_PREASSIGNS (arg_info) = NULL;

    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

    INFO_LHS (arg_info) = old_lhs;
    INFO_PREASSIGNS (arg_info) = pre_assigns;

    BLOCK_VARDECS (arg_node) = TRAVopt (BLOCK_VARDECS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
STPCassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_ASSERT (!INFO_SCRAPASSIGN (arg_info), "SCRAPASSIGN already set");

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_SCRAPASSIGN (arg_info)) {
        DBUG_PRINT ("scrapping assignment");
        arg_node = FREEdoFreeNode (arg_node);
    }

    if (INFO_PREASSIGNS (arg_info) != NULL) {
        DBUG_PRINT ("inserting pre_assigns");
        arg_node = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
        INFO_PREASSIGNS (arg_info) = NULL;
    }

    if (INFO_SCRAPASSIGN (arg_info)) {
        INFO_SCRAPASSIGN (arg_info) = FALSE;
        arg_node = TRAVopt (arg_node, arg_info);
    } else {
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
STPClet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    LET_IDS (arg_node) = INFO_LHS (arg_info);
    INFO_LHS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

node *
STPCprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);

    switch (PRF_PRF (arg_node)) {
        case F_guard:
            if (!global.runtimecheck.conformity) {
                INFO_LHS (arg_info) =
                    RenameOrReplaceRets (TCcountExprs (PRF_ARGS (arg_node)) - 1,
                                         INFO_LHS (arg_info),
                                         PRF_ARGS (arg_node),
                                         &INFO_PREASSIGNS (arg_info));
                INFO_SCRAPASSIGN (arg_info) = TRUE;
            }

        default:
            break;
    }

    DBUG_RETURN (arg_node);
}

node *
STPCid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    while (AVIS_SUBST (ID_AVIS (arg_node)) != NULL) {
        ID_AVIS (arg_node) = AVIS_SUBST (ID_AVIS (arg_node));
    }

    DBUG_RETURN (arg_node);
}

node *
STPCvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (AVIS_SUBST (VARDEC_AVIS (arg_node)) != NULL) {
        DBUG_PRINT ("removing variable %s", VARDEC_NAME (arg_node));
        arg_node = FREEdoFreeNode (arg_node);
        arg_node = TRAVopt (arg_node, arg_info);
    } else {
        VARDEC_NEXT (arg_node) = TRAVopt (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
