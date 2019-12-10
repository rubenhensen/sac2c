/**
 * @file
 * @defgroup apm
 *
 * Introduces explicit instructions for allocating memory.
 * This effectively converts functional SAC programs into imperative programs
 * that perform state changes.
 *
 * @ingroup mm
 *
 * @{
 */

#include "annotate_pagelocked_memory.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "memory.h"
#include "free.h"
#include "ctinfo.h"
#include "print.h"
#include "globals.h"

#define DBUG_PREFIX "EMAPM"
#include "debug.h"

/**
 * INFO structure
 */
struct INFO {
    node * lhs;
    node * fundef;
    bool lhs_pinned;
    bool in_funcond;
    bool in_ap;
    bool topdown;
};

/**
 * INFO macros
 */
#define INFO_LHS(n) ((n)->lhs)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_LHS_PINNED(n) ((n)->lhs_pinned)
#define INFO_IN_FUNCOND(n) ((n)->in_funcond)
#define INFO_IN_AP(n) ((n)->in_ap)
#define INFO_TOPDOWN(n) ((n)->topdown)


/**
 * @name INFO functions
 *
 * @{
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_LHS (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_LHS_PINNED (result) = FALSE;
    INFO_IN_FUNCOND (result) = FALSE;
    INFO_IN_AP (result) = FALSE;
    INFO_TOPDOWN (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** @} */

/**
 *
 */
node *
EMAPMprintPreFun (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    switch (NODE_TYPE (arg_node)) {
    case N_avis:
        if (AVIS_ISCUDAPINNED (arg_node)) {
            fprintf (global.outfile, "/* CUDA Pinned */");
        }
        /* fall-through */
    default:
        break;
    }

    DBUG_RETURN (arg_node);
}
/**
 *
 */
node *
EMAPMfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("inspecting %s...", FUNDEF_NAME (arg_node));
    INFO_FUNDEF (arg_info) = arg_node;

    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    INFO_TOPDOWN (arg_info) = TRUE;
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    INFO_TOPDOWN (arg_info) = FALSE;

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 *
 */
node *
EMAPMassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* bottom-up */
    if (!INFO_TOPDOWN (arg_info)) {
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);
    } else {
    ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

static bool
IsPinnedInIDS (node * ids)
{
    bool ret = FALSE;

    DBUG_ENTER ();

    while (ids) {
        if (AVIS_ISCUDAPINNED (IDS_AVIS (ids))) {
            ret = TRUE;
            break;
        }
        ids = IDS_NEXT (ids);
    }

    DBUG_RETURN (ret);
}

/**
 *
 */
node *
EMAPMlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    INFO_LHS_PINNED (arg_info) = IsPinnedInIDS (LET_IDS (arg_node));

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 *
 */
node *
EMAPMap (node *arg_node, info *arg_info)
{
    node *lhs, *rets, *args, *params, *old_fundef;

    DBUG_ENTER ();

    if (INFO_FUNDEF (arg_info) != AP_FUNDEF (arg_node)
        && !INFO_IN_AP (arg_info)
        //&& INFO_LHS_PINNED (arg_info)
        && FUNDEF_RETURN (AP_FUNDEF (arg_node)) != NULL) {
        DBUG_PRINT ("at application of %s...", FUNDEF_NAME (AP_FUNDEF (arg_node)));

        /* we need to mark the N_avis in the N_return as pinned, to match the LHS of the N_ap */
        lhs = INFO_LHS (arg_info);
        rets = RETURN_EXPRS (FUNDEF_RETURN (AP_FUNDEF (arg_node)));
        while (lhs && rets) {
            if (!INFO_TOPDOWN (arg_info)) {
                AVIS_ISCUDAPINNED (ID_AVIS (EXPRS_EXPR (rets))) = AVIS_ISCUDAPINNED (IDS_AVIS (lhs));
            } else {
                AVIS_ISCUDAPINNED (IDS_AVIS (lhs)) = AVIS_ISCUDAPINNED (ID_AVIS (EXPRS_EXPR (rets)));
            }
            lhs = IDS_NEXT (lhs);
            rets = EXPRS_NEXT (rets);
        }

        if (!INFO_TOPDOWN (arg_info)) {
            old_fundef = INFO_FUNDEF (arg_info);
            INFO_IN_AP (arg_info) = TRUE;
            AP_FUNDEF (arg_node) = TRAVopt (AP_FUNDEF (arg_node), arg_info);
            INFO_IN_AP (arg_info) = FALSE;
            INFO_FUNDEF (arg_info) = old_fundef;

            /* we do this for the arguments as well */
            args = AP_ARGS (arg_node);
            params = FUNDEF_ARGS (AP_FUNDEF (arg_node));
            while (args && params) {
                DBUG_PRINT ("  looking at %s...", ARG_NAME (params));
                args = EXPRS_NEXT (args);
                params = ARG_NEXT (params);
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/**
 *
 */
node *
EMAPMfuncond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    //if (INFO_LHS_PINNED (arg_info)) {
        INFO_IN_FUNCOND (arg_info) = TRUE;
        if (FUNCOND_THEN (arg_node) && NODE_TYPE (FUNCOND_THEN (arg_node)) == N_id) {
            FUNCOND_THEN (arg_node) = TRAVdo (FUNCOND_THEN (arg_node), arg_info);
        }
        if (FUNCOND_ELSE (arg_node) && NODE_TYPE (FUNCOND_ELSE (arg_node)) == N_id) {
            FUNCOND_ELSE (arg_node) = TRAVdo (FUNCOND_ELSE (arg_node), arg_info);
        }
        INFO_IN_FUNCOND (arg_info) = FALSE;
    //}

    DBUG_RETURN (arg_node);
}

/**
 *
 */
node *
EMAPMid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_IN_FUNCOND (arg_info)) {
        if (!INFO_TOPDOWN (arg_info)) {
            DBUG_PRINT ("Marking %s as pinned...", AVIS_NAME (ID_AVIS (arg_node)));
            AVIS_ISCUDAPINNED (ID_AVIS (arg_node)) = AVIS_ISCUDAPINNED (IDS_AVIS (INFO_LHS (arg_info)));
        } else {
            DBUG_PRINT ("Marking %s as pinned...", AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));
            AVIS_ISCUDAPINNED (IDS_AVIS (INFO_LHS (arg_info))) = AVIS_ISCUDAPINNED (ID_AVIS (arg_node));
        }
    }

    DBUG_RETURN (arg_node);
}

/**
 *
 */
node *
EMAPMprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (!INFO_TOPDOWN (arg_info)) {

    switch (PRF_PRF (arg_node)) {
    case F_host2device:
        DBUG_PRINT ("Found h2d!");
        /* we elide the host2device in alloc.c, making this moot */
        if (!AVIS_ISALLOCLIFT (ID_AVIS (PRF_ARG1 (arg_node)))) {
            AVIS_ISCUDAPINNED (ID_AVIS (PRF_ARG1 (arg_node))) = TRUE;
        }
        break;

    case F_device2host:
        DBUG_PRINT ("Found d2h!");
        AVIS_ISCUDAPINNED (IDS_AVIS (INFO_LHS (arg_info))) = TRUE;
        break;

    case F_copy:
        DBUG_PRINT ("Found copy!");
        AVIS_ISCUDAPINNED (ID_AVIS (PRF_ARG1 (arg_node)))
            = AVIS_ISCUDAPINNED (IDS_AVIS (INFO_LHS (arg_info)));
        break;

    case F_modarray_AxVxS:
    case F_modarray_AxVxA:
    case F_idx_modarray_AxSxS:
    case F_idx_modarray_AxSxA:
        DBUG_PRINT ("Found modarray!");
        AVIS_ISCUDAPINNED (ID_AVIS (PRF_ARG1 (arg_node)))
            = AVIS_ISCUDAPINNED (IDS_AVIS (INFO_LHS (arg_info)));
        break;

    default:
        /* do nothing */
        break;
    }

    }

    DBUG_RETURN (arg_node);
}

/**
 *
 */
node *
EMAPMdoAnnotatePagelockedMem (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_emapm);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** @} */
#undef DBUG_PREFIX
