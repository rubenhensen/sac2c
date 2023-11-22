#include "resolve_reference_args.h"

#include "tree_basic.h"

#define DBUG_PREFIX "RRA"
#include "debug.h"

#include "traverse.h"
#include "str.h"
#include "memory.h"
#include "new_types.h"
#include "type_utils.h"
#include "globals.h"
#include "ctinfo.h"

/**
 * INFO structure
 */
struct INFO {
    node *args;
    node *rets;
    node *lhs;
};

/**
 * INFO macros
 */
#define INFO_ARGS(n) ((n)->args)
#define INFO_RETS(n) ((n)->rets)
#define INFO_LHS(n) ((n)->lhs)

/**
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_ARGS (result) = NULL;
    INFO_RETS (result) = NULL;
    INFO_LHS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/**
 * helper functions
 */
static node *
ExpandArgsToRets (node *rets, node *args)
{
    DBUG_ENTER ();

    if (ARG_NEXT (args) != NULL) {
        rets = ExpandArgsToRets (rets, ARG_NEXT (args));
    }

    if (ARG_ISREFERENCE (args)) {
        ARG_ISREFERENCE (args) = FALSE;
        ARG_WASREFERENCE (args) = TRUE;

        rets = TBmakeRet (TYcopyType (AVIS_TYPE (ARG_AVIS (args))), rets);
        RET_ISARTIFICIAL (rets) = TRUE;
        RET_ISUNIQUE (rets) = ARG_ISUNIQUE (args);
    }

    DBUG_RETURN (rets);
}

static node *
ExpandArgsToReturnExprs (node *exprs, node *args)
{
    DBUG_ENTER ();

    if (ARG_NEXT (args) != NULL) {
        exprs = ExpandArgsToReturnExprs (exprs, ARG_NEXT (args));
    }

    if (ARG_WASREFERENCE (args)) {
        exprs = TBmakeExprs (TBmakeId (ARG_AVIS (args)), exprs);
    }

    DBUG_RETURN (exprs);
}

static node *
ExpandApArgsToResult (node *ids, node *args, node *exprs)
{
    DBUG_ENTER ();

    DBUG_ASSERT (((args != NULL) && (exprs != NULL)),
                 "no of args and exprs does not match");

    if (ARG_NEXT (args) != NULL) {
        ids = ExpandApArgsToResult (ids, ARG_NEXT (args), EXPRS_NEXT (exprs));
    }

    if ((ARG_ISREFERENCE (args)) || (ARG_WASREFERENCE (args))) {
        DBUG_ASSERT (NODE_TYPE (EXPRS_EXPR (exprs)) == N_id,
                     "non N_id node at reference arg position!");

        DBUG_PRINT ("...expanding %s to ret", AVIS_NAME (ID_AVIS (EXPRS_EXPR (exprs))));

        ids = TBmakeIds (ID_AVIS (EXPRS_EXPR (exprs)), ids);
    }

    DBUG_RETURN (ids);
}

node *
RRAfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Entering fundef %s...", CTIitemName (arg_node));

    /*
     * expand reference parameters to parameters + return value
     */
    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_RETS (arg_node)
          = ExpandArgsToRets (FUNDEF_RETS (arg_node), FUNDEF_ARGS (arg_node));
    }

    /*
     * expand the return statement and function applications
     */
    if (FUNDEF_BODY (arg_node) != NULL) {
        DBUG_PRINT ("...processing body");

        INFO_ARGS (arg_info) = FUNDEF_ARGS (arg_node);
        INFO_RETS (arg_info) = FUNDEF_RETS (arg_node);

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        INFO_ARGS (arg_info) = NULL;
        INFO_RETS (arg_info) = NULL;
    }

    DBUG_PRINT ("Completed fundef %s...", CTIitemName (arg_node));

    FUNDEF_NEXT (arg_node) = TRAVopt(FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
RRAreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_ARGS (arg_info) != NULL) {
        RETURN_EXPRS (arg_node)
          = ExpandArgsToReturnExprs (RETURN_EXPRS (arg_node), INFO_ARGS (arg_info));
    }

    DBUG_RETURN (arg_node);
}

node *
RRAlet (node *arg_node, info *arg_info)
{
    node *oldlhs;

    DBUG_ENTER ();

    oldlhs = INFO_LHS (arg_info);
    INFO_LHS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    LET_IDS (arg_node) = INFO_LHS (arg_info);
    INFO_LHS (arg_info) = oldlhs;

    DBUG_RETURN (arg_node);
}

node *
RRAap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (AP_ARGS (arg_node) != NULL) {
        INFO_LHS (arg_info)
          = ExpandApArgsToResult (INFO_LHS (arg_info), FUNDEF_ARGS (AP_FUNDEF (arg_node)),
                                  AP_ARGS (arg_node));
    }

    DBUG_RETURN (arg_node);
}

node *
RRAmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt(MODULE_FUNS (arg_node), arg_info);

    MODULE_FUNDECS (arg_node) = TRAVopt(MODULE_FUNDECS (arg_node), arg_info);

    MODULE_FUNSPECS (arg_node) = TRAVopt(MODULE_FUNSPECS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
RRAdoResolveReferenceArgs (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();
    TRAVpush (TR_rra);

    syntax_tree = TRAVdo (syntax_tree, info);

    TRAVpop ();
    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
