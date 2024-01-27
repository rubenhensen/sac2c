#include "DupTree.h"
#include "free.h"
#include "memory.h"
#include "str.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "tree_utils.h"

#define DBUG_PREFIX "RERA"
#include "debug.h"

#include "restore_reference_args.h"


/******************************************************************************
 *
 * @struct INFO
 *
 * @param INFO_LHS
 * @param INFO_ARGS
 * @param INFO_PREASSIGNS
 * @param INFO_DELETE
 *
 ******************************************************************************/
struct INFO {
    node *lhs;
    node *args;
    node *preassigns;
    bool delete;
};

#define INFO_LHS(n) ((n)->lhs)
#define INFO_ARGS(n) ((n)->args)
#define INFO_PREASSIGNS(n) ((n)->preassigns)
#define INFO_DELETE(n) ((n)->delete)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_LHS (result) = NULL;
    INFO_ARGS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_DELETE (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

static node *
ReintroduceReferenceArgs (node *args)
{
    DBUG_ENTER ();

    if (args != NULL) {
        ARG_NEXT (args) = ReintroduceReferenceArgs (ARG_NEXT (args));

        if (ARG_WASREFERENCE (args)) {
            ARG_ISREFERENCE (args) = TRUE;
            ARG_WASREFERENCE (args) = FALSE;
        }
    }

    DBUG_RETURN (args);
}

static void
SubstituteReferenceArg (node *lhs, node *arg)
{
    DBUG_ENTER ();

    if (NODE_TYPE (arg) == N_arg) {
        if (ARG_ISREFERENCE (arg) || ARG_WASREFERENCE (arg)) {
            AVIS_SUBST (IDS_AVIS (lhs)) = ARG_AVIS (arg);
        }
    }

    DBUG_RETURN ();
}

static node *
RemoveArtificialRets (node *rets)
{
    DBUG_ENTER ();

    if (rets != NULL) {
        RET_NEXT (rets) = RemoveArtificialRets (RET_NEXT (rets));

        if (RET_ISARTIFICIAL (rets)) {
            rets = FREEdoFreeNode (rets);
        }
    }

    DBUG_RETURN (rets);
}

static node *
RemoveArtificialReturnValues (node *form_args, node *act_args, node *ids)
{
    DBUG_ENTER ();

    if (form_args != NULL) {
        DBUG_ASSERT (act_args != NULL, "formal and actual args do not match");

        if (ARG_WASREFERENCE (form_args)) {
            /**
             * Prior to freeing the ids node, we have to make sure that its lhs
             * appearances are replaced with the reference arg.
             */
            AVIS_SUBST (IDS_AVIS (ids)) = ID_AVIS (EXPRS_EXPR (act_args));

            ids = FREEdoFreeNode (ids);
        }

        ids = RemoveArtificialReturnValues (ARG_NEXT (form_args),
                                            EXPRS_NEXT (act_args),
                                            ids);
    }

    DBUG_RETURN (ids);
}

static void
RemoveArtificialWithloopReturns (node *withops, node *withexprs, node *letlhs)
{
    DBUG_ENTER ();

    while (withops != NULL) {
        /**
         * If the withop is of type N_propagate, then the LHS is artificial and
         * we should replace the LHS N_id by the with N_expr.
         */
        if (NODE_TYPE (withops) == N_propagate) {
            AVIS_SUBST (IDS_AVIS (letlhs)) = ID_AVIS (EXPRS_EXPR (withexprs));
        }

        letlhs = IDS_NEXT (letlhs);
        withexprs = EXPRS_NEXT (withexprs);
        withops = WITHOP_NEXT (withops);
    }

    DBUG_RETURN ();
}

static node *
TransformArtificialReturnExprsIntoAssignments (node *args, node *exprs, node **assigns)
{
    DBUG_ENTER ();

    if (args != NULL) {
        if (ARG_WASREFERENCE (args)) {
            /**
             * Create an assignment of the expression to the original reference
             * argument if they are not the same anyways.
             */
            if (ID_AVIS (EXPRS_EXPR (exprs)) != ARG_AVIS (args)) {
                *assigns = TBmakeAssign (
                    TBmakeLet (TBmakeIds (ARG_AVIS (args), NULL),
                               DUPdoDupTree (EXPRS_EXPR (exprs))),
                    *assigns);
            }

            // Remove the return expression
            exprs = FREEdoFreeNode (exprs);
        }

        exprs = TransformArtificialReturnExprsIntoAssignments (ARG_NEXT (args),
                                                               exprs,
                                                               assigns);
    }

    DBUG_RETURN (exprs);
}

static node *
InitialiseVardecs (node *vardecs)
{
    DBUG_ENTER ();

    if (vardecs != NULL) {
        VARDEC_NEXT (vardecs) = InitialiseVardecs (VARDEC_NEXT (vardecs));

        AVIS_SUBST (VARDEC_AVIS (vardecs)) = NULL;
    }

    DBUG_RETURN (vardecs);
}

static node *
RemoveSubstitutedVardecs (node *vardecs)
{
    DBUG_ENTER ();

    if (vardecs != NULL) {
        VARDEC_NEXT (vardecs) = RemoveSubstitutedVardecs (VARDEC_NEXT (vardecs));

        if (AVIS_SUBST (VARDEC_AVIS (vardecs)) != NULL) {
            vardecs = FREEdoFreeNode (vardecs);
        }
    }

    DBUG_RETURN (vardecs);
}

node *
RERAassign (node *arg_node, info *arg_info)
{
    bool xdelete;
    DBUG_ENTER ();

    // Traverse the instruction to check for substitutions
    INFO_DELETE (arg_info) = FALSE;
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
    xdelete = INFO_DELETE (arg_info);
    INFO_DELETE (arg_info) = FALSE;

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    // If this assignment is superflouus, delete it
    if (xdelete) {
        arg_node = FREEdoFreeNode (arg_node);
    }

    /**
     * Insert any preassignments coming up from the traversal.
     *
     * It is save to do it like this as only the return statement creates
     * new assignments to be inserted and thus if PREASSIGNS != NULL then
     * ASSIGN_NEXT == NULL!
     */
    if (INFO_PREASSIGNS (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
        INFO_PREASSIGNS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

node *
RERAlet (node *arg_node, info *arg_info)
{
    node *oldlhs;
    node *arg;

    DBUG_ENTER ();

    oldlhs = INFO_LHS (arg_info);
    INFO_LHS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    LET_IDS (arg_node) = INFO_LHS (arg_info);
    INFO_LHS (arg_info) = oldlhs;

    /**
     * Check whether rhs is a reference arg and in that case substitute
     * the lhs by the rhs.
     */
    if (NODE_TYPE (LET_EXPR (arg_node)) == N_id) {
        arg = AVIS_DECL (ID_AVIS (LET_EXPR (arg_node)));
        SubstituteReferenceArg (LET_IDS (arg_node), arg);
    }

    // Substitute LHS ids
    LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);

    // Check whether this let is of form <id> = <id> and thus superflous
    if (NODE_TYPE (LET_EXPR (arg_node)) == N_id
        && IDS_AVIS (LET_IDS (arg_node)) == ID_AVIS (LET_EXPR (arg_node))) {
        INFO_DELETE (arg_info) = TRUE;
    }

    DBUG_RETURN (arg_node);
}

node *
RERAap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LHS (arg_info)
      = RemoveArtificialReturnValues (FUNDEF_ARGS (AP_FUNDEF (arg_node)),
                                      AP_ARGS (arg_node), INFO_LHS (arg_info));

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
RERAid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    while (AVIS_SUBST (ID_AVIS (arg_node)) != NULL) {
        ID_AVIS (arg_node) = AVIS_SUBST (ID_AVIS (arg_node));
    }

    DBUG_RETURN (arg_node);
}

node *
RERAids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    while (AVIS_SUBST (IDS_AVIS (arg_node)) != NULL) {
        IDS_AVIS (arg_node) = AVIS_SUBST (IDS_AVIS (arg_node));
    }

    DBUG_RETURN (arg_node);
}

node *
RERAreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    // First ensure that all substitutions have taken place
    arg_node = TRAVcont (arg_node, arg_info);

    RETURN_EXPRS (arg_node)
      = TransformArtificialReturnExprsIntoAssignments (INFO_ARGS (arg_info),
                                                       RETURN_EXPRS (arg_node),
                                                       &INFO_PREASSIGNS (arg_info));

    DBUG_RETURN (arg_node);
}

node *
RERAfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    // Clean up body first
    if (FUNDEF_BODY (arg_node) != NULL) {
        // Initialise AVIS_SUBST
        FUNDEF_VARDECS (arg_node) = InitialiseVardecs (FUNDEF_VARDECS (arg_node));

        // Remove references to artificials in body
        INFO_ARGS (arg_info) = FUNDEF_ARGS (arg_node);
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        INFO_ARGS (arg_info) = NULL;

        // Remove superflous vardecs
        FUNDEF_VARDECS (arg_node) = RemoveSubstitutedVardecs (FUNDEF_VARDECS (arg_node));
    }

    // Continue with other fundefs
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    // Clean up signature
    FUNDEF_ARGS (arg_node) = ReintroduceReferenceArgs (FUNDEF_ARGS (arg_node));
    FUNDEF_RETS (arg_node) = RemoveArtificialRets (FUNDEF_RETS (arg_node));

    DBUG_RETURN (arg_node);
}

node *
RERAprf (node *arg_node, info *arg_info)
{
    node *lhs, *args;

    DBUG_ENTER ();

    arg_node = TRAVcont (arg_node, arg_info);

    lhs = INFO_LHS (arg_info);

    switch (PRF_PRF (arg_node)) {
    /**
     * For these prfs we have to remove all LHS return vars that correspond to a
     * reference argument in the corresponding function call that this prf
     * resembles. However, we don't know which result corresponds to a reference
     * arg, as these prfs don't have args anymore. Even worse, we might not have
     * a function call at all anymore that would provide this information
     * anymore via the AVIS_SUBST flag.
     * Luckily, we are so late in compilation that we can just get rid of all
     * results. These two prfs terminate execution and in reality don't have a
     * result anyway.
     */
    case F_type_error:
    case F_dispatch_error:
        INFO_LHS (arg_info) = FREEdoFreeTree (lhs);
        break;

    /**
     * check whether rhs is a prop_obj and in that case substitute lhs of any
     * reference arguments by rhs.
     */
    case F_prop_obj_out:
    case F_prop_obj_in:
        args = PRF_ARGS (arg_node);

        if (PRF_PRF (arg_node) == F_prop_obj_in) {
            // Skip iv arg
            args = EXPRS_NEXT (args);
        }

        while (args != NULL) {
            SubstituteReferenceArg (lhs, ID_DECL (EXPRS_EXPR (args)));
            args = EXPRS_NEXT (args);
            lhs = IDS_NEXT (lhs);
        }
        break;

    /**
     * x1', .., xn' = guard (x1, .., xn, t1, .., tn, p1, .., pm)
     */
    case F_guard:
        args = PRF_ARGS (arg_node);
        while (lhs != NULL) {
            SubstituteReferenceArg (lhs, ID_DECL (EXPRS_EXPR (args)));
            lhs = IDS_NEXT (lhs);
            args = EXPRS_NEXT (args);
        }
        break;

    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

node *
RERAmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /**
     * We have to traverse the FUNS first here, as we have to remove artificials
     * from the bodies first. As this is done top-down, FUNS has to be first.
     */
    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    MODULE_FUNDECS (arg_node) = TRAVopt (MODULE_FUNDECS (arg_node), arg_info);

    MODULE_FUNSPECS (arg_node) = TRAVopt (MODULE_FUNSPECS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
RERAwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), arg_info);

    WITH_WITHOP (arg_node) = TRAVopt (WITH_WITHOP (arg_node), arg_info);

    RemoveArtificialWithloopReturns (WITH_WITHOP (arg_node),
                                     CODE_CEXPRS (WITH_CODE (arg_node)),
                                     INFO_LHS (arg_info));

    DBUG_RETURN (arg_node);
}

node *
RERAwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    WITH2_CODE (arg_node) = TRAVopt (WITH2_CODE (arg_node), arg_info);

    WITH2_WITHOP (arg_node) = TRAVopt (WITH2_WITHOP (arg_node), arg_info);

    RemoveArtificialWithloopReturns (WITH2_WITHOP (arg_node),
                                     CODE_CEXPRS (WITH2_CODE (arg_node)),
                                     INFO_LHS (arg_info));

    DBUG_RETURN (arg_node);
}

node *
RERAblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
RERAdoRestoreReferenceArguments (node *arg_node)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_rera);
    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
