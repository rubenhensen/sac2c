/******************************************************************************
 *
 * @defgroup SCC Strip Conformity Checks
 *
 * Removes all conformity checks from the dataflow graph so that they can be
 * safely removed by dead code removal.
 *
 * This phase also removes extrema and dataflow guards, as they should never
 * reach anywhere beyond the optimization phase.
 *
 ******************************************************************************/
#include "DupTree.h"
#include "free.h"
#include "globals.h"
#include "memory.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "tree_utils.h"

#define DBUG_PREFIX "SCC"
#include "debug.h"

#include "strip_conformity_checks.h"

/******************************************************************************
 *
 * @struct INFO
 *
 * @param INFO_LHS
 * @param INFO_PREASSIGNS
 * @param INFO_SCRAPASSIGN
 *
 ******************************************************************************/
struct INFO {
    node *lhs;
    node *preassigns;
    bool scrapassign;
};

#define INFO_LHS(n) ((n)->lhs)
#define INFO_PREASSIGNS(n) ((n)->preassigns)
#define INFO_SCRAPASSIGN(n) ((n)->scrapassign)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_LHS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_SCRAPASSIGN (result) = FALSE;

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
 * @fn node *SCCdoStripConformityChecks (node *arg_node)
 *
 ******************************************************************************/
node *
SCCdoStripConformityChecks (node *arg_node)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    DBUG_PRINT ("Starting strip conformity checks traversal.");

    TRAVpush (TR_scc);
    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();

    DBUG_PRINT ("Strip conformity checks traversal complete.");

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *RenameOrReplaceRets (int skip, int no, node *ids, node *args,
 *                                node **assigns)
 *
 * @brief Replaces the first no LHS IDS_AVIS with the corresponding RHS ID_AVIS
 * by setting AVIS_SUBST accordingly. The first skip arguments are ignored.
 *
 * @param skip Number of arguments to skip.
 * @param no Number of arguments to alias.
 * @param ids Lhs ids to be aliased.
 * @param args The arguments.
 * @param assigns Used to append aliasing assignments to.
 *
 * @returns The new lhs ids.
 *
 ******************************************************************************/
static node *
RenameOrReplaceRets (int skip, size_t no, node *ids, node *args, node **assigns)
{
    node *tmp;

    DBUG_ENTER ();

    if (skip != 0) {
        ids = RenameOrReplaceRets (skip - 1, no, ids,
                                   EXPRS_NEXT (args), assigns);
    } else if (no != 0) {
        IDS_NEXT (ids) = RenameOrReplaceRets (skip, no - 1, IDS_NEXT (ids),
                                              EXPRS_NEXT (args), assigns);

        if (NODE_TYPE (EXPRS_EXPR (args)) == N_id) {
            // We mark the avis for substitution and deletion
            DBUG_PRINT ("Aliasing %s ...", IDS_NAME (ids));

            DBUG_ASSERT (AVIS_SUBST (IDS_AVIS (ids)) == NULL,
                         "AVIS_SUBST already set!");

            AVIS_SUBST (IDS_AVIS (ids)) = ID_AVIS (EXPRS_EXPR (args));
        } else {
            // Insert a substitution assign
            DBUG_PRINT ("Inserting substitution for %s ...", IDS_NAME (ids));

            tmp = ids;
            ids = IDS_NEXT (ids);
            IDS_NEXT (tmp) = NULL;

            *assigns = TBmakeAssign (
                TBmakeLet (tmp, DUPdoDupTree (EXPRS_EXPR (args))),
                *assigns);
            AVIS_SSAASSIGN (IDS_AVIS (tmp)) = *assigns;
        }
    } else if (ids != NULL) {
        IDS_NEXT (ids) = RenameOrReplaceRets (skip, no, IDS_NEXT (ids),
                                              args, assigns);

        /**
         * The remaining lhs identifiers must be guards,
         * so we emit ids = TRUE assigns.
         */
        DBUG_PRINT ("Setting %s to TRUE...", IDS_NAME (ids));

        tmp = ids;
        ids = IDS_NEXT (ids);
        IDS_NEXT (tmp) = NULL;

        *assigns = TBmakeAssign (TBmakeLet (tmp, TBmakeBool (TRUE)), *assigns);
        AVIS_SSAASSIGN (IDS_AVIS (tmp)) = *assigns;
    }

    DBUG_RETURN (ids);
}

/******************************************************************************
 *
 * @fn node *SCCblock (node *arg_node, info *arg_info)
 *
 * @brief Traverses first the code in the block and then the vardec chain.
 * Furthermore, the INFO_LHS and INFO_PREASSIGNS is stacked.
 *
 ******************************************************************************/
node *
SCCblock (node *arg_node, info *arg_info)
{
    node *oldlhs, *preassigns;

    DBUG_ENTER ();

    oldlhs = INFO_LHS (arg_info);
    INFO_LHS (arg_info) = NULL;
    preassigns = INFO_PREASSIGNS (arg_info);
    INFO_PREASSIGNS (arg_info) = NULL;

    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);
    INFO_LHS (arg_info) = oldlhs;
    INFO_PREASSIGNS (arg_info) = preassigns;

    BLOCK_VARDECS (arg_node) = TRAVopt (BLOCK_VARDECS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *SCCassign (node *arg_node, info *arg_info)
 *
 * @brief Inserts preassignments from INFO_PREASSIGN.
 *
 ******************************************************************************/
node *
SCCassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_ASSERT (!INFO_SCRAPASSIGN (arg_info), "SCRAPASSIGN already set!");

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_SCRAPASSIGN (arg_info)) {
        DBUG_PRINT ("Scrapping assignment...");

        arg_node = FREEdoFreeNode (arg_node);
    }

    if (INFO_PREASSIGNS (arg_info) != NULL) {
        DBUG_PRINT ("Inserting preassigns...");

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

/******************************************************************************
 *
 * @fn node *SCClet (node *arg_node, info *arg_info)
 *
 ******************************************************************************/
node *
SCClet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    LET_IDS (arg_node) = INFO_LHS (arg_info);
    INFO_LHS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *SCCprf (node *arg_node, info *arg_info)
 *
 * @brief Marks conformity check prfs as identity on their first n arguments
 * by setting AVIS_SUBST.
 *
 ******************************************************************************/
node *
SCCprf (node *arg_node, info *arg_info)
{
    size_t num_rets;
    node *lhs, *args;

    DBUG_ENTER ();

    PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);

    DBUG_PRINT ("Looking at prf %s", PRF_NAME (PRF_PRF (arg_node)));

    lhs = INFO_LHS (arg_info);
    args = PRF_ARGS (arg_node);

    switch (PRF_PRF (arg_node)) {
    /**
     * Dataflow and extrema guards never reach code generator.
     */
    case F_noteminval:
    case F_notemaxval:
    case F_noteintersect:
        INFO_LHS (arg_info) = RenameOrReplaceRets (0, 1, lhs, args,
                                &INFO_PREASSIGNS (arg_info));
        INFO_SCRAPASSIGN (arg_info) = TRUE;
        break;

    /**
     * Prfs with one identity on first arg.
     */
    case F_non_neg_val_V:
    case F_non_neg_val_S:
    case F_val_le_val_VxV:
    case F_val_le_val_SxS:
    case F_val_lt_val_SxS:
    case F_shape_matches_dim_VxA:
    case F_val_lt_shape_VxA:
    case F_prod_matches_prod_shape_VxA:
    case F_conditional_error:
        if (!global.runtimecheck.conformity && global.insertconformitychecks) {
            INFO_LHS (arg_info) = RenameOrReplaceRets (0, 1, lhs, args,
                                    &INFO_PREASSIGNS (arg_info));
            INFO_SCRAPASSIGN (arg_info) = TRUE;
        }
        break;

    /**
     * Prfs with two identities on first args.
     */
    case F_same_shape_AxA:
        if (!global.runtimecheck.conformity && global.insertconformitychecks) {
            INFO_LHS (arg_info) = RenameOrReplaceRets (0, 2, lhs, args,
                                    &INFO_PREASSIGNS (arg_info));
            INFO_SCRAPASSIGN (arg_info) = TRUE;
        }
        break;

    /**
     * Prfs with one identity on second arg.
     */
    case F_type_constraint:
        if (!global.runtimecheck.conformity && global.insertconformitychecks) {
            INFO_LHS (arg_info) = RenameOrReplaceRets (1, 1, lhs, args,
                                    &INFO_PREASSIGNS (arg_info));
            INFO_SCRAPASSIGN (arg_info) = TRUE;
        }
        break;

    /**
     * Prfs with a variable number of identities.
     * x1', .., xn' = guard (x1, .., xn, p1, .., pm)
     */
    case F_guard:
        if (!global.runtimecheck.conformity && global.insertconformitychecks) {
            num_rets = PRF_NUMVARIABLERETS (arg_node);
            DBUG_ASSERT (num_rets > 0, "guard has no return values");
            INFO_LHS (arg_info) = RenameOrReplaceRets (0, num_rets, lhs, args,
                                    &INFO_PREASSIGNS (arg_info));
            INFO_SCRAPASSIGN (arg_info) = TRUE;
        }
        break;

    /**
     * Do nothing for the rest.
     */
    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *SCCid (node *arg_node, info *arg_info)
 *
 * @brief Substitutes the AVIS if needed.
 *
 ******************************************************************************/
node *
SCCid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    while (AVIS_SUBST (ID_AVIS (arg_node)) != NULL) {
        ID_AVIS (arg_node) = AVIS_SUBST (ID_AVIS (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *SCCvardec (node *arg_node, info *arg_info)
 *
 * @brief Removes unused vardecs.
 *
 ******************************************************************************/
node *
SCCvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (AVIS_SUBST (VARDEC_AVIS (arg_node)) != NULL) {
        DBUG_PRINT ("Removing variable %s...", AVIS_NAME (VARDEC_AVIS (arg_node)));

        arg_node = FREEdoFreeNode (arg_node);
        arg_node = TRAVopt(arg_node, arg_info);
    } else {
        VARDEC_NEXT (arg_node) = TRAVopt (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
