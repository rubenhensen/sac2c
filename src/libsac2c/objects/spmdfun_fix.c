#define DBUG_PREFIX "FSFS"
#include "debug.h"

#include "free.h"
#include "str.h"
#include "memory.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"

#include "spmdfun_fix.h"

/*
 * This traversal fixes up any SPMD function's signature so that
 * by-reference arguments and return values come first. It also marks
 * such arguments as was-by-reference.
 *
 * BEFORE:
 *    a, b', c' = spmdfun( d, e, f, b, c);
 *    ... (and accompanying fundef)
 *
 * AFTER:
 *    b', c', a = spmdfun( b, c, d, e, f);
 *    ...
 */

/**
 * INFO structure
 */
struct INFO {
    node *ap_lhs;
    node *ap_args;
    node *fundef_rets;
    node *objs_in;
    node *objs_out;
    node *with_lhs;
    bool enter_spmd;
};

/**
 * INFO macros
 */
#define INFO_AP_LHS(n) (n->ap_lhs)
#define INFO_AP_ARGS(n) (n->ap_args)
#define INFO_FUNDEF_RETS(n) (n->fundef_rets)
#define INFO_OBJS_IN(n) (n->objs_in)
#define INFO_OBJS_OUT(n) (n->objs_out)
#define INFO_WITH_LHS(n) (n->with_lhs)
#define INFO_ENTER_SPMD(n) (n->enter_spmd)

/**
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_AP_LHS (result) = NULL;
    INFO_AP_ARGS (result) = NULL;
    INFO_FUNDEF_RETS (result) = NULL;
    INFO_OBJS_IN (result) = NULL;
    INFO_OBJS_OUT (result) = NULL;
    INFO_WITH_LHS (result) = NULL;
    INFO_ENTER_SPMD (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/*
 * Static helper functions
 */

/** <!-- ****************************************************************** -->
 * @brief Scans the with-ops for propagate ops. Adds the default element
 * of the propagate node to OBJS_IN and the corresponding with-loop lhs
 * into OBJS_OUT, because we know those will be the by-reference in and
 * out arguments of the spmd function.
 *
 * @param withop  Withops.
 * @param lhs  With-loop LHS.
 * @param arg_info Info.
 ******************************************************************************/
static void
ScanWithops (node *withop, node *lhs, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("with2");

    if (withop != NULL) {
        DBUG_ASSERT (lhs != NULL, "number of lhs exprs does not match withops");

        if (NODE_TYPE (withop) == N_propagate) {
            /*
             * Found a propagate node. Add it's argument to IN_OBJS
             * and it's LHS to OUT_OBJS.
             */
            DBUG_PRINT ("Adding arg %s to IN_OBJS",
                        AVIS_NAME (ID_AVIS (PROPAGATE_DEFAULT (withop))));
            DBUG_PRINT ("Adding ret %s to OUT_OBJS", AVIS_NAME (IDS_AVIS (lhs)));
            INFO_OBJS_IN (arg_info)
              = TCappendIds (INFO_OBJS_IN (arg_info),
                             TBmakeIds (ID_AVIS (PROPAGATE_DEFAULT (withop)), NULL));
            INFO_OBJS_OUT (arg_info)
              = TCappendIds (INFO_OBJS_OUT (arg_info), TBmakeIds (IDS_AVIS (lhs), NULL));
        }

        ScanWithops (WITHOP_NEXT (withop), IDS_NEXT (lhs), arg_info);
    }

    DBUG_RETURN ();
}

/** <!-- ****************************************************************** -->
 * @brief Finds the argument declaration of obj, returning it's position
 * in the chain of arguments given in args.
 *
 * @param args Argument chain.
 * @param obj Object to find.
 * @param lev Starting level (pass 0 here).
 *
 * @return Position of the object in the chain (starts at 1), or -1 if
 * not found.
 ******************************************************************************/
static int
FindInArgs (node *args, node *obj, int lev)
{
    if (args == NULL)
        return -1;

    if (ARG_AVIS (args) == IDS_AVIS (obj)) {
        return (lev + 1);
    } else {
        return FindInArgs (ARG_NEXT (args), obj, lev + 1);
    }
}

/** <!-- ****************************************************************** -->
 * @brief Finds the return expression containing obj, returning it's position
 * in the chain of expressions given in exprs.
 *
 * @param args N_exprs chain.
 * @param obj Object to find.
 * @param lev Starting level (pass 0 here).
 *
 * @return Position of the object in the chain (starts at 1), or -1 if
 * not found.
 ******************************************************************************/
static int
FindInExprs (node *args, node *obj, int lev)
{
    if (args == NULL)
        return -1;

    if (ID_AVIS (EXPRS_EXPR (args)) == IDS_AVIS (obj)) {
        return (lev + 1);
    } else {
        return FindInExprs (EXPRS_NEXT (args), obj, lev + 1);
    }
}

/** <!-- ****************************************************************** -->
 * @brief Bubbles the argument at position pos in the chain fundef_args to
 * the top of the chain.
 *
 * @param fundef_args Argument chain (N_arg nodes).
 * @param pos Position to bubble up.
 *
 * @return New start of the argument chain.
 ******************************************************************************/
static node *
BubbleArgUp (node *fundef_args, int pos)
{
    node *prev_iter;
    node *iter;
    node *keep;
    int count;

    prev_iter = NULL;
    iter = fundef_args;
    count = 1;
    keep = NULL;

    /* Iterate until we hit the argument we're looking for. */
    while (iter != NULL) {
        if (count == pos) {
            keep = iter;
            if (prev_iter != NULL) {
                ARG_NEXT (prev_iter) = ARG_NEXT (iter);
            } else {
                fundef_args = ARG_NEXT (iter);
                break;
            }
        }

        prev_iter = iter;
        iter = ARG_NEXT (iter);
        count++;
    }

    /* Keep should contain the arg node to bubble up here. */
    DBUG_ASSERT (keep != NULL, "specified argument position higher than"
                               "number of arguments in the chain");

    /* Mark it as at one point being a by-reference argument. */
    ARG_WASREFERENCE (keep) = TRUE;
    ARG_NEXT (keep) = fundef_args;

    return (keep);
}

/** <!-- ****************************************************************** -->
 * @brief Bubbles the N_ret at position pos in the chain fundef_rets to
 * the top of the chain.
 *
 * @param fundef_rets Ret chain (N_ret nodes).
 * @param pos Position to bubble up.
 *
 * @return New start of the ret chain.
 ******************************************************************************/
static node *
BubbleRetUp (node *fundef_rets, int pos)
{
    node *prev_iter;
    node *iter;
    node *keep;
    int count;

    prev_iter = NULL;
    iter = fundef_rets;
    count = 1;
    keep = NULL;

    /* Iterate until we hit the ret we're looking for. */
    while (iter != NULL) {
        if (count == pos) {
            keep = iter;
            if (prev_iter != NULL) {
                RET_NEXT (prev_iter) = RET_NEXT (iter);
            } else {
                fundef_rets = RET_NEXT (iter);
                break;
            }
        }

        prev_iter = iter;
        iter = RET_NEXT (iter);
        count++;
    }

    /* Keep should contain the ret node to bubble up here. */
    DBUG_ASSERT (keep != NULL, "specified ret position higher than"
                               "number of nodes in the chain");

    /* Mark it as being artificial (introduced by by-reference argument). */
    RET_ISARTIFICIAL (keep) = TRUE;
    RET_NEXT (keep) = fundef_rets;

    return (keep);
}

/** <!-- ****************************************************************** -->
 * @brief Bubbles the N_exprs at position pos in the chain exprs to
 * the top of the chain.
 *
 * @param exprs Exprs chain (N_exprs nodes).
 * @param pos Position to bubble up.
 *
 * @return New start of the exprs chain.
 ******************************************************************************/
static node *
BubbleExprUp (node *exprs, int pos)
{
    node *prev_iter;
    node *iter;
    node *keep;
    int count;

    prev_iter = NULL;
    iter = exprs;
    count = 1;
    keep = NULL;

    /* Iterate until we hit the expr we're looking for. */
    while (iter != NULL) {
        if (count == pos) {
            keep = iter;
            if (prev_iter != NULL) {
                EXPRS_NEXT (prev_iter) = EXPRS_NEXT (iter);
            } else {
                exprs = EXPRS_NEXT (iter);
                break;
            }
        }

        prev_iter = iter;
        iter = EXPRS_NEXT (iter);
        count++;
    }

    /* Keep should contain the expr node to bubble up here. */
    DBUG_ASSERT (keep != NULL, "specified expr position higher than"
                               "number of nodes in the chain");

    EXPRS_NEXT (keep) = exprs;
    return (keep);
}

/** <!-- ****************************************************************** -->
 * @brief Bubbles the N_ids at position pos in the chain ids to
 * the top of the chain.
 *
 * @param ids Ids chain (N_ids nodes).
 * @param pos Position to bubble up.
 *
 * @return New start of the ids chain.
 ******************************************************************************/
static node *
BubbleIdsUp (node *ids, int pos)
{
    node *prev_iter;
    node *iter;
    node *keep;
    int count;

    prev_iter = NULL;
    iter = ids;
    count = 1;
    keep = NULL;

    /* Iterate until we hit the expr we're looking for. */
    while (iter != NULL) {
        if (count == pos) {
            keep = iter;
            if (prev_iter != NULL) {
                IDS_NEXT (prev_iter) = IDS_NEXT (iter);
            } else {
                ids = IDS_NEXT (iter);
                break;
            }
        }

        prev_iter = iter;
        iter = IDS_NEXT (iter);
        count++;
    }

    /* Keep should contain the ids node to bubble up here. */
    DBUG_ASSERT (keep != NULL, "specified ids position higher than"
                               "number of nodes in the chain");

    IDS_NEXT (keep) = ids;
    return (keep);
}

/** <!-- ****************************************************************** -->
 * @brief Bubble up to the start of the function formal argument list and
 * the applied argument list, all objects contained in the OBJS_IN chain.
 * The applied argument list and the objects are in the info struct.
 *
 * @param fundef_args Formal SPMD function arguments.
 * @param arg_info Info.
 *
 * @return New start of the formal arguments.
 ******************************************************************************/
static node *
ShuffleFundefArgs (node *fundef_args, info *arg_info)
{
    node *objs_iter;
    int pos;

    DBUG_ENTER ();

    objs_iter = INFO_OBJS_IN (arg_info);

    /* Iterate through the objects, removing them as we bubble each one to
     * the top. */
    while (objs_iter != NULL) {

        /* Find the object in the formal argument chain. */
        pos = FindInArgs (fundef_args, objs_iter, 0);

        DBUG_ASSERT (pos != -1, "Couldn't find object in SPMD function sig");
        DBUG_PRINT ("arg found at %d", pos);

        /* Bubble it up in the formal argument chain, and because we don't
         * know the name of the applied argument, bubble up the same position
         * in the applied argument chain at the same time. */
        fundef_args = BubbleArgUp (fundef_args, pos);
        INFO_AP_ARGS (arg_info) = BubbleExprUp (INFO_AP_ARGS (arg_info), pos);

        objs_iter = FREEdoFreeNode (objs_iter);
    }

    DBUG_RETURN (fundef_args);
}

/** <!-- ****************************************************************** -->
 * @brief Bubble up to the start of the function formal return list, the lhs
 * of the application, and the actual return statement,
 * all objects contained in the OBJS_OUT chain.
 * The application lhs, fundef rets and the objects are in the info struct.
 *
 * @param fundef_args SPMD function return expression.
 * @param arg_info Info.
 *
 * @return New start of the function return expressions.
 ******************************************************************************/
static node *
ShuffleReturnExprs (node *return_exprs, info *arg_info)
{
    node *objs_iter;
    int pos;

    DBUG_ENTER ();

    objs_iter = INFO_OBJS_OUT (arg_info);

    /* Iterate through the objects, removing them as we bubble each one to
     * the top. */
    while (objs_iter != NULL) {

        /* Find the object in the return expressions. */
        pos = FindInExprs (return_exprs, objs_iter, 0);

        DBUG_ASSERT (pos != -1, "Couldn't find object in SPMD function return");
        DBUG_PRINT ("ret found at %d", pos);

        /* Bubble it up simultaneously in the actual return expression, the
         * formal return signature and the application lhs. */
        return_exprs = BubbleExprUp (return_exprs, pos);
        INFO_AP_LHS (arg_info) = BubbleIdsUp (INFO_AP_LHS (arg_info), pos);
        INFO_FUNDEF_RETS (arg_info) = BubbleRetUp (INFO_FUNDEF_RETS (arg_info), pos);

        objs_iter = FREEdoFreeNode (objs_iter);
    }

    DBUG_RETURN (return_exprs);
}

/*
 * Traversal functions
 */
node *
FSFSap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_ISSPMDFUN (AP_FUNDEF (arg_node))) {
        DBUG_ASSERT (INFO_ENTER_SPMD (arg_info) == FALSE,
                     "Nested SPMD functions not allowed!");

        INFO_ENTER_SPMD (arg_info) = TRUE;
        INFO_AP_ARGS (arg_info) = AP_ARGS (arg_node);
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
        AP_ARGS (arg_node) = INFO_AP_ARGS (arg_info);
        INFO_AP_ARGS (arg_info) = NULL;
        INFO_ENTER_SPMD (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

node *
FSFSfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_ISSPMDFUN (arg_node) && INFO_ENTER_SPMD (arg_info)) {
        INFO_FUNDEF_RETS (arg_info) = FUNDEF_RETS (arg_node);
    }

    if ((!FUNDEF_ISSPMDFUN (arg_node))
        || (FUNDEF_ISSPMDFUN (arg_node) && INFO_ENTER_SPMD (arg_info))) {
        DBUG_PRINT ("entered fundef %s!", FUNDEF_NAME (arg_node));
        FUNDEF_BODY (arg_node) = TRAVopt(FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL && !FUNDEF_ISSPMDFUN (arg_node)) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    /* Shuffle the SPMD function's signature. */
    if (FUNDEF_ISSPMDFUN (arg_node) && INFO_ENTER_SPMD (arg_info)) {
        FUNDEF_ARGS (arg_node) = ShuffleFundefArgs (FUNDEF_ARGS (arg_node), arg_info);
        FUNDEF_RETS (arg_node) = INFO_FUNDEF_RETS (arg_info);
        INFO_FUNDEF_RETS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

node *
FSFSlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* first mode: put lhs of spmd-ap in info struct */
    if (!INFO_ENTER_SPMD (arg_info)) {
        if (NODE_TYPE (LET_EXPR (arg_node)) == N_ap) {
            INFO_AP_LHS (arg_info) = LET_IDS (arg_node);
            LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
            LET_IDS (arg_node) = INFO_AP_LHS (arg_info);
            INFO_AP_LHS (arg_info) = NULL;
        }
    }

    /* second mode: put lhs of with-dingus in info struct */
    if (INFO_ENTER_SPMD (arg_info)) {
        if (NODE_TYPE (LET_EXPR (arg_node)) == N_with2) {
            INFO_WITH_LHS (arg_info) = LET_IDS (arg_node);
            LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
            INFO_WITH_LHS (arg_info) = NULL;
        }
    }

    DBUG_RETURN (arg_node);
}

node *
FSFSreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_ENTER_SPMD (arg_info)) {
        /* This is the return expression of the SPMD function. Shuffle it. */
        RETURN_EXPRS (arg_node) = ShuffleReturnExprs (RETURN_EXPRS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
FSFSwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_ENTER_SPMD (arg_info)) {
        /* Scan through with-ops and with lhs at the same time. */
        ScanWithops (WITH2_WITHOP (arg_node), INFO_WITH_LHS (arg_info), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 * Traversal start function
 */
node *
FSFSdoFixSpmdFunctionSignatures (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_fsfs);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
