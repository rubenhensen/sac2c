/* $Id$ */

#include "restore_reference_args.h"

#include "dbug.h"
#include "free.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#include "DupTree.h"

/*
 * INFO structure
 */
struct INFO {
    node *lhs;
    node *args;
    node *preassigns;
    bool delete;
};

/*
 * INFO macros
 */
#define INFO_LHS(n) ((n)->lhs)
#define INFO_ARGS(n) ((n)->args)
#define INFO_PREASSIGNS(n) ((n)->preassigns)
#define INFO_DELETE(n) ((n)->delete)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_LHS (result) = NULL;
    INFO_ARGS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_DELETE (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/*
 * helper functions
 */
static node *
ReintroduceReferenceArgs (node *args)
{
    DBUG_ENTER ("ReintroduceReferenceArgs");

    if (args != NULL) {
        ARG_NEXT (args) = ReintroduceReferenceArgs (ARG_NEXT (args));

        if (ARG_WASREFERENCE (args)) {
            ARG_ISREFERENCE (args) = TRUE;
            ARG_WASREFERENCE (args) = FALSE;
        }
    }

    DBUG_RETURN (args);
}

static node *
RemoveArtificialRets (node *rets)
{
    DBUG_ENTER ("RemoveArtificialRets");

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
    DBUG_ENTER ("RemoveArtificialReturnValues");

    if (form_args != NULL) {
        DBUG_ASSERT ((act_args != NULL), "formal and actual args do not match");

        if (ARG_WASREFERENCE (form_args)) {
            /*
             * prior to freeing the ids node, we have to make
             * sure that its lhs appearances are replaced with the
             * reference arg
             */
            AVIS_SUBST (IDS_AVIS (ids)) = ID_AVIS (EXPRS_EXPR (act_args));

            ids = FREEdoFreeNode (ids);
        }

        ids = RemoveArtificialReturnValues (ARG_NEXT (form_args), EXPRS_NEXT (act_args),
                                            ids);
    }

    DBUG_RETURN (ids);
}

static void
RemoveArtificialWithloopReturns (node *withops, node *withexprs, node *letlhs)
{
    DBUG_ENTER ("RemoveArtificialWithloopReturn");

    while (withops != NULL) {
        /* If the withop is of type N_propagate, then the LHS is artificial
         * and we should replace the LHS N_id by the with N_expr. */
        if (NODE_TYPE (withops) == N_propagate) {
            AVIS_SUBST (IDS_AVIS (letlhs)) = ID_AVIS (EXPRS_EXPR (withexprs));
        }

        letlhs = IDS_NEXT (letlhs);
        withexprs = EXPRS_NEXT (withexprs);
        withops = WITHOP_NEXT (withops);
    }

    DBUG_VOID_RETURN;
}

static node *
TransformArtificialReturnExprsIntoAssignments (node *args, node *exprs, node **assigns)
{
    DBUG_ENTER ("TransformArtificialReturnExprsIntoAssignments");

    if (args != NULL) {
        if (ARG_WASREFERENCE (args)) {
            /*
             * create an assignment of the expression to the original
             * reference argument if they are not the same anyways
             */
            if (ID_AVIS (EXPRS_EXPR (exprs)) != ARG_AVIS (args)) {
                *assigns = TBmakeAssign (TBmakeLet (TBmakeIds (ARG_AVIS (args), NULL),
                                                    DUPdoDupTree (EXPRS_EXPR (exprs))),
                                         *assigns);
            }

            /*
             * remove the return expression
             */
            exprs = FREEdoFreeNode (exprs);
        }

        exprs = TransformArtificialReturnExprsIntoAssignments (ARG_NEXT (args), exprs,
                                                               assigns);
    }

    DBUG_RETURN (exprs);
}

static node *
InitialiseVardecs (node *vardecs)
{
    DBUG_ENTER ("InitialiseVardecs");

    if (vardecs != NULL) {
        VARDEC_NEXT (vardecs) = InitialiseVardecs (VARDEC_NEXT (vardecs));

        AVIS_SUBST (VARDEC_AVIS (vardecs)) = NULL;
    }

    DBUG_RETURN (vardecs);
}

static node *
RemoveSubstitutedVardecs (node *vardecs)
{
    DBUG_ENTER ("RemoveSubstitutedVardecs");

    if (vardecs != NULL) {
        VARDEC_NEXT (vardecs) = RemoveSubstitutedVardecs (VARDEC_NEXT (vardecs));

        if (AVIS_SUBST (VARDEC_AVIS (vardecs)) != NULL) {
            vardecs = FREEdoFreeNode (vardecs);
        }
    }

    DBUG_RETURN (vardecs);
}

/*
 * traversal functions
 */

node *
RERAassign (node *arg_node, info *arg_info)
{
    bool delete;
    DBUG_ENTER ("RERAassign");

    /*
     * traverse the instruction to check for substitutions
     */
    INFO_DELETE (arg_info) = FALSE;
    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    delete = INFO_DELETE (arg_info);
    INFO_DELETE (arg_info) = FALSE;

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    /*
     * if this assignment is superflouus, delete it
     */
    if (delete) {
        arg_node = FREEdoFreeNode (arg_node);
    }

    /*
     * insert any preassignments coming up from the traversal
     *
     * it is save to do it like this as only the return statement
     * creates new assignments to be inserted and thus if PREASSIGNS != NULL
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

    DBUG_ENTER ("RERAlet");

    oldlhs = INFO_LHS (arg_info);
    INFO_LHS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    LET_IDS (arg_node) = INFO_LHS (arg_info);
    INFO_LHS (arg_info) = oldlhs;

    /*
     * check whether rhs is a reference arg and in that case substitute
     * the lhs by the rhs.
     */
    if ((NODE_TYPE (LET_EXPR (arg_node)) == N_id)
        && (NODE_TYPE (AVIS_DECL (ID_AVIS (LET_EXPR (arg_node)))) == N_arg)) {
        arg = AVIS_DECL (ID_AVIS (LET_EXPR (arg_node)));

        if (ARG_WASREFERENCE (arg) || ARG_ISREFERENCE (arg)) {
            AVIS_SUBST (IDS_AVIS (LET_IDS (arg_node))) = ARG_AVIS (arg);
        }
    }

    /*
     * substitute LHS ids
     */
    if (LET_IDS (arg_node) != NULL) {
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }

    /*
     * check whether this let is of form <id> = <id>
     * and thus superflouus
     */
    if ((NODE_TYPE (LET_EXPR (arg_node)) == N_id)
        && (IDS_AVIS (LET_IDS (arg_node)) == ID_AVIS (LET_EXPR (arg_node)))) {
        INFO_DELETE (arg_info) = TRUE;
    }

    DBUG_RETURN (arg_node);
}

node *
RERAap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RERAap");

    INFO_LHS (arg_info)
      = RemoveArtificialReturnValues (FUNDEF_ARGS (AP_FUNDEF (arg_node)),
                                      AP_ARGS (arg_node), INFO_LHS (arg_info));

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
RERAid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RERAid");

    while (AVIS_SUBST (ID_AVIS (arg_node)) != NULL) {
        ID_AVIS (arg_node) = AVIS_SUBST (ID_AVIS (arg_node));
    }

    DBUG_RETURN (arg_node);
}

node *
RERAids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RERAids");

    if (IDS_NEXT (arg_node) != NULL) {
        IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
    }

    while (AVIS_SUBST (IDS_AVIS (arg_node)) != NULL) {
        IDS_AVIS (arg_node) = AVIS_SUBST (IDS_AVIS (arg_node));
    }

    DBUG_RETURN (arg_node);
}

node *
RERAreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RERAreturn");

    /*
     * first ensure that all substitutions have taken place!
     */
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
    DBUG_ENTER ("RERAfundef");

    /*
     * clean up body first
     */
    if (FUNDEF_BODY (arg_node) != NULL) {
        /*
         * initialise AVIS_SUBST
         */
        FUNDEF_VARDEC (arg_node) = InitialiseVardecs (FUNDEF_VARDEC (arg_node));

        /*
         * remove references to artificials in body
         */
        INFO_ARGS (arg_info) = FUNDEF_ARGS (arg_node);
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        INFO_ARGS (arg_info) = NULL;

        /*
         * remove superflous vardecs
         */
        FUNDEF_VARDEC (arg_node) = RemoveSubstitutedVardecs (FUNDEF_VARDEC (arg_node));
    }

    /*
     * continue with other fundefs
     */
    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    /*
     * clean up signature
     */
    FUNDEF_ARGS (arg_node) = ReintroduceReferenceArgs (FUNDEF_ARGS (arg_node));
    FUNDEF_RETS (arg_node) = RemoveArtificialRets (FUNDEF_RETS (arg_node));

    DBUG_RETURN (arg_node);
}

node *
RERAprf (node *arg_node, info *arg_info)
{
    node *args;
    node *lhs;

    DBUG_ENTER ("RERAprf");

    arg_node = TRAVcont (arg_node, arg_info);

    switch (PRF_PRF (arg_node)) {
    case F_dispatch_error:
    case F_type_error:
        /*
         * for these prfs we have to remove all LHS return vars that
         * have a subst flag set (e.g. they are artificial return
         * values). In fact this is a bad hack, as it only works, if
         * the return value has been used in a regular function call.
         * At least for wrappers this is true...
         */
        while ((INFO_LHS (arg_info) != NULL)
               && (AVIS_SUBST (IDS_AVIS (INFO_LHS (arg_info))) != NULL)) {
            INFO_LHS (arg_info) = FREEdoFreeNode (INFO_LHS (arg_info));
        }
        break;

    case F_prop_obj_out:
    case F_prop_obj_in:
        /*
         * check whether rhs is a prop_obj() and in that case substitute
         * lhs of any reference args by rhs.
         */
        args = PRF_ARGS (arg_node);
        lhs = INFO_LHS (arg_info);

        if (PRF_PRF (arg_node) == F_prop_obj_in) {
            args = EXPRS_NEXT (args); /* skip iv arg */
        }

        while (args != NULL) {
            if (NODE_TYPE (AVIS_DECL (ID_AVIS (EXPRS_EXPR (args)))) == N_arg) {
                node *arg = AVIS_DECL (ID_AVIS (EXPRS_EXPR (args)));

                if (ARG_WASREFERENCE (arg) || ARG_ISREFERENCE (arg)) {
                    AVIS_SUBST (IDS_AVIS (lhs)) = ARG_AVIS (arg);
                }
            }

            /* iterate */
            args = EXPRS_NEXT (args);
            lhs = IDS_NEXT (lhs);
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
    DBUG_ENTER ("RERAmodule");

    /*
     * we have to traverse the FUNS first here, as we have
     * to remove artificials from the bodies first. As this is
     * done top-down, FUNS has to be first
     */
    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    if (MODULE_FUNDECS (arg_node) != NULL) {
        MODULE_FUNDECS (arg_node) = TRAVdo (MODULE_FUNDECS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
RERAwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RERAwith");

    if (WITH_CODE (arg_node) != NULL) {
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    }

    if (WITH_WITHOP (arg_node) != NULL) {
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
    }

    RemoveArtificialWithloopReturns (WITH_WITHOP (arg_node),
                                     CODE_CEXPRS (WITH_CODE (arg_node)),
                                     INFO_LHS (arg_info));

    DBUG_RETURN (arg_node);
}

node *
RERAwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RERAwith2");

    if (WITH2_CODE (arg_node) != NULL) {
        WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
    }

    if (WITH2_WITHOP (arg_node) != NULL) {
        WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);
    }

    RemoveArtificialWithloopReturns (WITH2_WITHOP (arg_node),
                                     CODE_CEXPRS (WITH2_CODE (arg_node)),
                                     INFO_LHS (arg_info));

    DBUG_RETURN (arg_node);
}

node *
RERAblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RERAblock");

    BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);

    if (BLOCK_INSTR (arg_node) == NULL) {
        BLOCK_INSTR (arg_node) = TBmakeEmpty ();
    }

    DBUG_RETURN (arg_node);
}

/*
 * traversal start function
 */
node *
RERAdoRestoreReferenceArguments (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("RERAdoRestoreReferenceArgs");

    info = MakeInfo ();

    TRAVpush (TR_rera);

    syntax_tree = TRAVdo (syntax_tree, info);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}
