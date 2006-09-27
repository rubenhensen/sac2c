/* $Id$ */

#include "dbug.h"
#include "free.h"
#include "internal_lib.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"

#include "spmdfun_fix.h"

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
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

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
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/*
 * Static helper functions
 */
static void
ScanWithops (node *withop, node *lhs, info *arg_info)
{
    DBUG_ENTER ("ScanWithops");

    DBUG_PRINT ("FSFS", ("with2"));

    if (withop != NULL) {
        DBUG_ASSERT (lhs != NULL, ("number of lhs exprs does not match withops"));

        if (NODE_TYPE (withop) == N_propagate) {
            /* a-ha! */
            DBUG_PRINT ("FSFS", ("Adding arg %s",
                                 AVIS_NAME (ID_AVIS (PROPAGATE_DEFAULT (withop)))));
            DBUG_PRINT ("FSFS", ("Adding ret %s", AVIS_NAME (IDS_AVIS (lhs))));
            INFO_OBJS_IN (arg_info)
              = TCappendIds (INFO_OBJS_IN (arg_info),
                             TBmakeIds (ID_AVIS (PROPAGATE_DEFAULT (withop)), NULL));
            INFO_OBJS_OUT (arg_info)
              = TCappendIds (INFO_OBJS_OUT (arg_info), TBmakeIds (IDS_AVIS (lhs), NULL));
        }

        ScanWithops (WITHOP_NEXT (withop), IDS_NEXT (lhs), arg_info);
    }

    DBUG_LEAVE;
}

static int
FindInArgs (node *args, node *obj, int lev)
{
    lev += 1;

    DBUG_PRINT ("FSFS", ("%d: %s %s", lev, AVIS_NAME (ARG_AVIS (args)),
                         AVIS_NAME (IDS_AVIS (obj))));
    if (args == NULL || obj == NULL)
        return -1;

    if (ARG_AVIS (args) == IDS_AVIS (obj)) {
        return lev;
    } else {
        return FindInArgs (ARG_NEXT (args), obj, lev);
    }
}

static int
FindInRets (node *args, node *obj, int lev)
{
    lev += 1;

    DBUG_PRINT ("FSFS", ("%d: %s %s", lev, AVIS_NAME (ID_AVIS (EXPRS_EXPR (args))),
                         AVIS_NAME (IDS_AVIS (obj))));
    if (args == NULL || obj == NULL)
        return -1;

    if (ID_AVIS (EXPRS_EXPR (args)) == IDS_AVIS (obj)) {
        return lev;
    } else {
        return FindInRets (EXPRS_NEXT (args), obj, lev);
    }
}

static node *
BubbleArgUp (node *fundef_args, int pos)
{
    node *iter = fundef_args;
    node *prev_iter = NULL;
    node *keep;
    int cnt = 0;

    while (iter != NULL) {
        cnt++;
        if (cnt == pos) {
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
    }

    DBUG_ASSERT (keep != NULL, "baaaad");
    ARG_WASREFERENCE (keep) = TRUE;
    ARG_NEXT (keep) = fundef_args;
    return (keep);
}

static node *
BubbleRetUp (node *fundef_rets, int pos)
{
    node *iter = fundef_rets;
    node *prev_iter = NULL;
    node *keep;
    int cnt = 0;

    while (iter != NULL) {
        cnt++;
        if (cnt == pos) {
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
    }

    DBUG_ASSERT (keep != NULL, "baaaad");
    RET_ISARTIFICIAL (keep) = TRUE;
    RET_NEXT (keep) = fundef_rets;
    return (keep);
}

static node *
BubbleExprUp (node *exprs, int pos)
{
    node *iter = exprs;
    node *prev_iter = NULL;
    node *keep;
    int cnt = 0;

    while (iter != NULL) {
        cnt++;
        if (cnt == pos) {
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
    }

    DBUG_ASSERT (keep != NULL, "baaaad");
    EXPRS_NEXT (keep) = exprs;
    return (keep);
}

static node *
BubbleIdsUp (node *ids, int pos)
{
    node *iter = ids;
    node *prev_iter = NULL;
    node *keep;
    int cnt = 0;

    while (iter != NULL) {
        cnt++;
        if (cnt == pos) {
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
    }

    DBUG_ASSERT (keep != NULL, "baaaad");
    IDS_NEXT (keep) = ids;
    return (keep);
}

static node *
ShuffleFundefArgs (node *fundef_args, info *arg_info)
{
    int pos;
    node *objs_iter;
    DBUG_ENTER ("ShuffleFundefArgs");

    objs_iter = INFO_OBJS_IN (arg_info);
    while (objs_iter != NULL) {
        pos = FindInArgs (fundef_args, objs_iter, 0);
        DBUG_ASSERT (pos != -1, "Couldn't find object in SPMD function sig");
        DBUG_PRINT ("FSFS", ("arg found at %d", pos));
        fundef_args = BubbleArgUp (fundef_args, pos);
        INFO_AP_ARGS (arg_info) = BubbleExprUp (INFO_AP_ARGS (arg_info), pos);
        objs_iter = FREEdoFreeNode (objs_iter);
    }

    DBUG_RETURN (fundef_args);
}

static node *
ShuffleReturnExprs (node *return_exprs, info *arg_info)
{
    int pos;
    node *objs_iter;
    DBUG_ENTER ("ShuffleReturnValues");

    objs_iter = INFO_OBJS_OUT (arg_info);
    while (objs_iter != NULL) {
        pos = FindInRets (return_exprs, objs_iter, 0);
        DBUG_ASSERT (pos != -1, "Couldn't find object in SPMD function sig");
        DBUG_PRINT ("FSFS", ("ret found at %d", pos));
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
    DBUG_ENTER ("FSFSap");

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
    DBUG_ENTER ("FSFSfundef");

    if (FUNDEF_ISSPMDFUN (arg_node) && INFO_ENTER_SPMD (arg_info)) {
        INFO_FUNDEF_RETS (arg_info) = FUNDEF_RETS (arg_node);
    }

    if ((!FUNDEF_ISSPMDFUN (arg_node))
        || (FUNDEF_ISSPMDFUN (arg_node) && INFO_ENTER_SPMD (arg_info))) {
        DBUG_PRINT ("FSFS", ("entered fundef %s!", FUNDEF_NAME (arg_node)));
        if (FUNDEF_BODY (arg_node) != NULL) {
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        }
    }

    if (FUNDEF_NEXT (arg_node) != NULL && !FUNDEF_ISSPMDFUN (arg_node)) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    /* shuffle function signature here */
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
    DBUG_ENTER ("FSFSlet");

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
    DBUG_ENTER ("FSFSreturn");

    if (INFO_ENTER_SPMD (arg_info)) {
        RETURN_EXPRS (arg_node) = ShuffleReturnExprs (RETURN_EXPRS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
FSFSwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("FSFSwith2");

    if (INFO_ENTER_SPMD (arg_info)) {
        /* scan through with-ops and with lhs at the same time */
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

    DBUG_ENTER ("FSFSdoFixSpmdFunctionSignatures");

    info = MakeInfo ();

    TRAVpush (TR_fsfs);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}
