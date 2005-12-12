/* $Id$ */

#include "restore_reference_args.h"

#include "dbug.h"
#include "free.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"

/*
 * INFO structure
 */
struct INFO {
    node *lhs;
    node *args;
};

/*
 * INFO macros
 */
#define INFO_LHS(n) ((n)->lhs)
#define INFO_ARGS(n) ((n)->args)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_LHS (result) = NULL;
    INFO_ARGS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

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
             * mark avis for substitution and delete ids
             */
            AVIS_SUBST (IDS_AVIS (ids)) = ID_AVIS (EXPRS_EXPR (act_args));
            ids = FREEdoFreeNode (ids);
        }

        ids = RemoveArtificialReturnValues (ARG_NEXT (form_args), EXPRS_NEXT (act_args),
                                            ids);
    }

    DBUG_RETURN (ids);
}

static node *
RemoveArtificialReturnExprs (node *args, node *exprs)
{
    DBUG_ENTER ("RemoveArtificialReturnExprs");

    if (args != NULL) {
        if (ARG_WASREFERENCE (args)) {
            exprs = FREEdoFreeNode (exprs);

            exprs = RemoveArtificialReturnExprs (ARG_NEXT (args), exprs);
        }
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
RemoveArtificialVardecs (node *vardecs)
{
    DBUG_ENTER ("RemoveArtificialVardecs");

    if (vardecs != NULL) {
        VARDEC_NEXT (vardecs) = RemoveArtificialVardecs (VARDEC_NEXT (vardecs));

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
RERAblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RERAblock");

    arg_node = TRAVcont (arg_node, arg_info);

    /*
     * as we delete assignments in RERAassign, the
     * block may become empty. in that case we have
     * to add a special N_empty node
     */
    if (BLOCK_INSTR (arg_node) == NULL) {
        BLOCK_INSTR (arg_node) = TBmakeEmpty ();
    }

    DBUG_RETURN (arg_node);
}

node *
RERAassign (node *arg_node, info *arg_info)
{
    node *avis;
    node *new_node;

    DBUG_ENTER ("RERAassign");

    /*
     * first traverse rhs
     */
    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    /*
     * detect assignments of the form <ida> = <idb>
     * where idb is a reference arg
     */
    new_node = arg_node;

    if ((NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_let)
        && (NODE_TYPE (LET_EXPR (ASSIGN_INSTR (arg_node))) == N_id)) {

        avis = ID_AVIS (LET_EXPR (ASSIGN_INSTR (arg_node)));

        if ((NODE_TYPE (AVIS_DECL (avis)) == N_arg)
            && (ARG_WASREFERENCE (AVIS_DECL (avis)))) {
            AVIS_SUBST (IDS_AVIS (LET_IDS (ASSIGN_INSTR (arg_node)))) = avis;
            new_node = FREEdoFreeNode (arg_node);
        }
    }

    if (new_node == arg_node) {
        if (ASSIGN_NEXT (arg_node) != NULL) {
            ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
        }
    } else {
        if (new_node != NULL) {
            new_node = TRAVdo (new_node, arg_info);
        }
    }

    DBUG_RETURN (new_node);
}

node *
RERAlet (node *arg_node, info *arg_info)
{
    node *oldlhs;

    DBUG_ENTER ("RERAlet");

    oldlhs = INFO_LHS (arg_info);
    INFO_LHS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    LET_IDS (arg_node) = INFO_LHS (arg_info);
    INFO_LHS (arg_info) = oldlhs;

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
RERAreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RERAreturn");

    RETURN_EXPRS (arg_node)
      = RemoveArtificialReturnExprs (INFO_ARGS (arg_info), RETURN_EXPRS (arg_node));

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
RERAfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RERAfundef");

    /*
     * clean up body first
     */
    if (FUNDEF_BODY (arg_node) != NULL) {
        /*
         * reset AVIS_SUBST flag
         */
        FUNDEF_VARDEC (arg_node) = InitialiseVardecs (FUNDEF_VARDEC (arg_node));

        /*
         * remove references to artificials in body
         */
        INFO_ARGS (arg_info) = FUNDEF_ARGS (arg_node);
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        INFO_ARGS (arg_info) = NULL;
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

    /*
     * clean up vardecs
     */
    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_VARDEC (arg_node) = RemoveArtificialVardecs (FUNDEF_VARDEC (arg_node));
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
