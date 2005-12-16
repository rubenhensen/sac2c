/* $Id$ */

#include "infer_uniqueness.h"

#include "traverse.h"
#include "dbug.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *lhs;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_LHS(n) ((n)->lhs)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_LHS (result) = NULL;

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
TransferFromRetsToReturn (node *exprs, node *rets)
{
    DBUG_ENTER ("TransferFromRetsToReturn");

    if (rets != NULL) {
        DBUG_ASSERT ((exprs != NULL),
                     "no of return exprs does not match no of declared return values!");

        EXPRS_NEXT (exprs)
          = TransferFromRetsToReturn (EXPRS_NEXT (exprs), RET_NEXT (rets));

        AVIS_ISUNIQUE (ID_AVIS (EXPRS_EXPR (exprs))) = RET_ISUNIQUE (rets);
    }

    DBUG_RETURN (exprs);
}

static node *
TransferFromArgsToExprs (node *exprs, node *args)
{
    DBUG_ENTER ("TransferFromArgsToExprs");

    if (args != NULL) {
        DBUG_ASSERT ((exprs != NULL),
                     "no of argument exprs does not match no of declared arguments!");

        EXPRS_NEXT (exprs)
          = TransferFromArgsToExprs (EXPRS_NEXT (exprs), ARG_NEXT (args));

        AVIS_ISUNIQUE (ID_AVIS (EXPRS_EXPR (exprs))) = ARG_ISUNIQUE (args);
    }

    DBUG_RETURN (exprs);
}

/*
 * traversal functions
 */
node *
IUQvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IUQvardec");

    /*
     * reset uniqueness flag
     */
    AVIS_ISUNIQUE (VARDEC_AVIS (arg_node)) = FALSE;

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
IUQarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IUQarg");

    /*
     * preset uniqueness flag
     */
    AVIS_ISUNIQUE (ARG_AVIS (arg_node)) = ARG_ISUNIQUE (arg_node);

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
IUQfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IUQfundef");

    INFO_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    INFO_FUNDEF (arg_info) = NULL;

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
IUQblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IUQblock");

    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = TRAVdo (BLOCK_VARDEC (arg_node), arg_info);
    }

    BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
IUQassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IUQassign");

    /*
     * bottom up traversal
     */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
IUQreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IUQreturn");

    RETURN_EXPRS (arg_node)
      = TransferFromRetsToReturn (RETURN_EXPRS (arg_node),
                                  FUNDEF_RETS (INFO_FUNDEF (arg_info)));

    DBUG_RETURN (arg_node);
}

node *
IUQlet (node *arg_node, info *arg_info)
{
    node *lhs;

    DBUG_ENTER ("IUQlet");

    lhs = INFO_LHS (arg_info);
    INFO_LHS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    INFO_LHS (arg_info) = lhs;

    DBUG_RETURN (arg_node);
}

node *
IUQap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IUQap");

    AP_ARGS (arg_node)
      = TransferFromArgsToExprs (AP_ARGS (arg_node), FUNDEF_ARGS (AP_FUNDEF (arg_node)));

    DBUG_RETURN (arg_node);
}

node *
IUQprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IUQprf");

    switch (PRF_PRF (arg_node)) {
    case F_type_conv:
        AVIS_ISUNIQUE (ID_AVIS (PRF_ARG2 (arg_node)))
          = AVIS_ISUNIQUE (IDS_AVIS (INFO_LHS (arg_info)));
        break;

    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

node *
IUQid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IUQid");

    AVIS_ISUNIQUE (ID_AVIS (arg_node)) = AVIS_ISUNIQUE (IDS_AVIS (INFO_LHS (arg_info)));

    DBUG_RETURN (arg_node);
}

node *
IUQfuncond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IUQfuncond");

    AVIS_ISUNIQUE (ID_AVIS (FUNCOND_THEN (arg_node)))
      = AVIS_ISUNIQUE (IDS_AVIS (INFO_LHS (arg_info)));

    AVIS_ISUNIQUE (ID_AVIS (FUNCOND_ELSE (arg_node)))
      = AVIS_ISUNIQUE (IDS_AVIS (INFO_LHS (arg_info)));

    DBUG_RETURN (arg_node);
}

node *
IUQcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IUQcond");

    if (COND_THEN (arg_node) != NULL) {
        COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
    }

    if (COND_ELSE (arg_node) != NULL) {
        COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
IUQwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IUQwith");

    if (WITH_CODE (arg_node) != NULL) {
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 * traversal start function
 */
node *
IUQdoInferUniqueness (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("IUQdoInferUniqueness");

    info = MakeInfo ();
    TRAVpush (TR_iuq);

    syntax_tree = TRAVdo (syntax_tree, info);

    TRAVpop ();
    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}
