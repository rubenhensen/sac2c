/******************************************************************************
 *
 * This file contains methods for generating check functions, for functions with
 * type patterns. Additionally, it contains methods for modifying the original
 * definition of this function with type patterns to make use of these check
 * functions.
 *
 * These methods are used by type_pattern_resolve.
 *
 * Given a function with type patterns, we generate:
 *   - A function that checks pre-conditions imposed by type patterns.
 *   - A function that checks post-conditions imposed by type patterns.
 *   - A function, using the same name as the original function, that calls the
 *     original implementation and these generated check functions.
 *
 * Additionally we rename the original function definition, to ensure that
 * applications of this functions instead use the newly generated function that
 * makes use of the genrated checks.
 *
 ******************************************************************************/
#include "ctype.h"
#include "DupTree.h"
#include "free.h"
#include "globals.h"
#include "memory.h"
#include "namespaces.h"
#include "new_types.h"
#include "shape.h"
#include "str_buffer.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "type_utils.h"

#define DBUG_PREFIX "GTP"
#include "debug.h"

#include "type_pattern_guard.h"

/******************************************************************************
 *
 * @fn bool IsValidFundefName (char *name)
 *
 * @brief Checks whether the function name contains only alpha-numeric
 * characters or underscores. Otherwise, this function name is likely an
 * operator (e.g. +, -, *, /), which we cannot include in our pre- and post-
 * check function names.
 *
 ******************************************************************************/
static bool
IsValidFundefName (char *name)
{
    char *c;
    bool valid = TRUE;

    DBUG_ENTER ();

    c = name;
    while (valid && *c != '\0') {
        valid = isalnum(*c) || *c == '_';
        c += 1;
    }

    DBUG_RETURN (valid);
}

/******************************************************************************
 *
 * @fn char *GTPmakeValidFundefName (char *guard_str, char *fundef_name)
 *
 * @brief Generates a unique function name, based on an existing function name.
 * If this function name contains no special characters, we prepend a unique
 * identifier. Otherwise, in cases such as an overloaded operator (+, -, *, /),
 * we only return this unique identifier.
 *
 ******************************************************************************/
char *
GTPmakeValidFundefName (const char *guard_str, char *fundef_name)
{
    char *name, *res;

    DBUG_ENTER ();

    name = TRAVtmpVarName (guard_str);
    res = IsValidFundefName (fundef_name)
            ? STRcatn (3, name, "_", fundef_name)
            : STRcpy (name);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @fn node *MakeConditionGuards (const char *guard_str, node *fundef,
 *                                node *conditions)
 *
 * @brief Convert conditions, e.g. foo(int a) | a > 0, to checks for our pre-
 * and post-check functions. Namely, for this example, we generate
 *   acc' = conditional_error (acc, a > 0, "condition failed");
 *
 * @todo If we convert the condition to a string we can make our error message
 * more descriptive: "condition a > 0 failed".
 *
 ******************************************************************************/
static node *
MakeConditionGuards (const char *guard_str, node *fundef, node *conditions)
{
    node *guard, *res = NULL;
    str_buf *buf;

    DBUG_ENTER ();

    buf = SBUFcreate (256);

    while (conditions != NULL) {
        buf = SBUFprintf (buf, "Type pattern %s-condition of %s failed",
                               guard_str, FUNDEF_NAME (fundef));

        // acc' = conditional_error (acc, pred, "msg");
        guard = TCmakePrf3 (F_conditional_error,
                            TBmakeSpid (NULL, STRcpy (GTP_PRED_NAME)),
                            EXPRS_EXPR (conditions),
                            TBmakeStr (SBUF2str (buf)));
        guard = TBmakeLet (TBmakeSpids (STRcpy (GTP_PRED_NAME), NULL), guard);
        res = TCappendAssign (res, TBmakeAssign (guard, NULL));

        EXPRS_EXPR (conditions) = NULL;
        conditions = FREEdoFreeNode (conditions);
    }

    buf = SBUFfree (buf);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @fn node *MakeGuardFundef (const char *guard_str, node *fundef, node *args,
 *                            node *body)
 *
 * @brief Generates the pre- or post-check function, based on the original
 * function definition, the new arguments, and the new function body.
 *
 * @example
 * inline
 * bool foo_post (int[*] a, int[*] b, int[+] res) {
 *     pred = true;
 *     assignments...
 *     checks...
 *     return pred;
 * }
 *
 * @note Whereas the pre-check function only requires the original arguments.
 * The post-check functions also requires the returned values, since these can
 * also contain type patterns.
 *
 ******************************************************************************/
static node *
MakeGuardFundef (const char *guard_str, node *fundef, node *args, node *body)
{
    char *fundef_name;
    ntype *typ;
    node *res;

    DBUG_ENTER ();

    fundef_name = GTPmakeValidFundefName (guard_str, FUNDEF_NAME (fundef));
    typ = TYmakeAKS (TYmakeSimpleType (T_bool), SHmakeShape (0));
    res = TBmakeFundef (fundef_name,
                        NSdupNamespace (FUNDEF_NS (fundef)),
                        TBmakeRet (typ, NULL),
                        args,
                        TBmakeBlock (body, NULL),
                        NULL);

    FUNDEF_ISINLINE (res) = TRUE;
    FUNDEF_ISGUARDFUN (res) = TRUE;

    DBUG_PRINT ("created %s function %s for %s",
                guard_str, fundef_name, FUNDEF_NAME (fundef));

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @fn node *GTPmakePreCheck (node *fundef, node *assigns, node *checks)
 *
 * @brief Generates a function that containts the pre-conditions for a function
 * with type patterns. It returns true if all constraints are satisfied,
 * otherwise a bottom type is raised for all failing conditions. We make this
 * function inline as the call-site of this function more likely contains
 * information that can help in statically resolving these conditions.
 *
 ******************************************************************************/
node *
GTPmakePreCheck (node *fundef, node *assigns, node *checks)
{
    node *conditions, *pred_let, *pred_ret;
    node *formal_args, *res;

    DBUG_ENTER ();

    // Build function body bottom-up
    pred_ret = TBmakeSpid (NULL, STRcpy (GTP_PRED_NAME));
    pred_ret = TBmakeReturn (TBmakeExprs (pred_ret, NULL));
    res = TBmakeAssign (pred_ret, NULL);

    conditions = MakeConditionGuards ("pre", fundef, FUNDEF_PRECONDS (fundef));
    res = TCappendAssign (conditions, res);
    FUNDEF_PRECONDS (fundef) = NULL;

    res = TCappendAssign (checks, res);
    res = TCappendAssign (assigns, res);

    pred_let = TBmakeLet (TBmakeSpids (STRcpy (GTP_PRED_NAME), NULL),
                          TBmakeBool (TRUE));
    res = TBmakeAssign (pred_let, res);

    // Build guard function
    formal_args = DUPdoDupTree (FUNDEF_ARGS (fundef));
    res = MakeGuardFundef ("pre", fundef, formal_args, res);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @fn node *GTPmakePostCheck (node *fundef, node *assigns,
 *                             node *checks, node *return_ids)
 *
 * @brief Generates a function that containts the post-conditions for a function
 * with type patterns. It returns true if all constraints are satisfied,
 * otherwise a bottom type is raised for all failing conditions. We make this
 * function inline as the call-site of this function more likely contains
 * information that can help in statically resolving these conditions.
 *
 ******************************************************************************/
node *
GTPmakePostCheck (node *fundef, node *assigns,
                  node *checks, node *return_ids)
{
    node *conditions, *pred_let, *pred_ret;
    node *formal_args, *rets, *post_lhs, *res;

    DBUG_ENTER ();

    formal_args = DUPdoDupTree (FUNDEF_ARGS (fundef));
    rets = FUNDEF_RETS (fundef);
    post_lhs = return_ids;

    while (rets != NULL) {
        char *v = SPID_NAME (EXPRS_EXPR (post_lhs));
        node *avis = TBmakeAvis (STRcpy (v), TYcopyType (RET_TYPE (rets)));
        node *arg = TBmakeArg (avis, NULL);

        AVIS_DECL (avis) = arg;
        AVIS_DECLTYPE (avis) = TYcopyType (RET_TYPE (rets));
        formal_args = TCappendArgs (formal_args, arg);

        rets = RET_NEXT (rets);
        post_lhs = EXPRS_NEXT (post_lhs);
    }

    // Build function body bottom-up
    pred_ret = TBmakeSpid (NULL, STRcpy (GTP_PRED_NAME));
    pred_ret = TBmakeReturn (TBmakeExprs (pred_ret, NULL));
    res = TBmakeAssign (pred_ret, NULL);

    conditions = MakeConditionGuards ("post", fundef, FUNDEF_POSTCONDS (fundef));
    res = TCappendAssign (conditions, res);
    FUNDEF_POSTCONDS (fundef) = NULL;

    res = TCappendAssign (checks, res);
    res = TCappendAssign (assigns, res);

    pred_let = TBmakeLet (TBmakeSpids (STRcpy (GTP_PRED_NAME), NULL),
                          TBmakeBool (TRUE));
    res = TBmakeAssign (pred_let, res);

    // Build guard function
    res = MakeGuardFundef ("post", fundef, formal_args, res);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @fn void ConvertArgs (node *args, node **spids, node **exprs)
 *
 * @brief Converts an N_arg chain to corresponding N_spids and N_exprs chains.
 *
 ******************************************************************************/
static void
ConvertArgs (node *args, node **spids, node **exprs)
{
    node *expr;

    DBUG_ENTER ();

    while (args != NULL) {
        char *id = ARG_NAME (args);
        *spids = TCappendSpids (*spids, TBmakeSpids (STRcpy (id), NULL));
        expr = TBmakeExprs (TBmakeSpid (NULL, STRcpy (id)), NULL);
        *exprs = TCappendExprs (*exprs, expr);
        args = ARG_NEXT (args);
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * @fn void ConvertRets (node *rets, node **spids, node **exprs)
 *
 * @brief Converts an N_ret chain to corresponding N_spids and N_exprs chains.
 *
 ******************************************************************************/
static void
ConvertRets (node *rets, node **spids, node **exprs)
{
    node *expr;

    DBUG_ENTER ();

    while (rets != NULL) {
        char *id = TRAVtmpVarName ("res");
        *spids = TCappendSpids (*spids, TBmakeSpids (STRcpy (id), NULL));
        expr = TBmakeExprs (TBmakeSpid (NULL, STRcpy (id)), NULL);
        *exprs = TCappendExprs (*exprs, expr);
        rets = RET_NEXT (rets);
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * @fn node *GTPmodifyFundef (node *fundef, node *impl, node *pre, node *post)
 *
 * @brief Surrounds a function with type patterns with the newly generated pre-
 * and post-condition functions. We make this function inline as the call-site
 * of this function more likely contains information that can help in statically
 * resolving these conditions.
 *
 ******************************************************************************/
node *
GTPmodifyFundef (node *fundef, node *impl, node *pre, node *post)
{
    node *pre_lhs = NULL, *pre_args = NULL;
    node *post_lhs = NULL, *post_args = NULL;
    node *ap, *let, *body;

    DBUG_ENTER ();

    ConvertArgs (FUNDEF_ARGS (fundef), &pre_lhs, &pre_args);
    ConvertRets (FUNDEF_RETS (fundef), &post_lhs, &post_args);

    // return (x, y, z)
    body = TBmakeAssign (TBmakeReturn (DUPdoDupTree (post_args)), NULL);

    if (post != NULL) {
        // x, y, z = conditionalAbort (x, y, z, pred)
        let = TCmakeSpap1 (NSgetNamespace (global.rterrorname),
                           STRcpy ("conditionalAbort"),
                           TBmakeSpid (NULL, STRcpy (GTP_PRED_NAME)));
        body = TBmakeAssign (TBmakeLet (NULL, let), body);

        // pred = foo_post (a, b, x, y, z)
        ap = TBmakeAp (post,
                       TCappendExprs (DUPdoDupTree (pre_args), post_args));
        let = TBmakeLet (TBmakeSpids (STRcpy (GTP_PRED_NAME), NULL), ap);
        body = TBmakeAssign (let, body);
    }

    // x, y, z = foo_impl (a, b)
    ap = TBmakeSpap (TBmakeSpid (NULL, STRcpy (FUNDEF_NAME (impl))),
                     DUPdoDupTree (pre_args));
    let = TBmakeLet (post_lhs, ap);
    body = TBmakeAssign (let, body);

    if (pre != NULL) {
        // a, b = conditionalAbort (a, b, pred)
        let = TCmakeSpap1 (NSgetNamespace (global.rterrorname),
                           STRcpy ("conditionalAbort"),
                           TBmakeSpid (NULL, STRcpy (GTP_PRED_NAME)));
        body = TBmakeAssign (TBmakeLet (NULL, let), body);

        // pre_pred = foo_pre (a, b)
        ap = TBmakeAp (pre, pre_args);
        let = TBmakeLet (TBmakeSpids (STRcpy (GTP_PRED_NAME), NULL), ap);
        body = TBmakeAssign (let, body);
    }

    FUNDEF_ASSIGNS (fundef) = FREEoptFreeTree (FUNDEF_ASSIGNS (fundef));
    FUNDEF_ASSIGNS (fundef) = body;
    FUNDEF_ISINLINE (fundef) = TRUE;

    DBUG_PRINT ("modified function %s", FUNDEF_NAME (fundef));

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * @fn node *GTPmakeImpl (node *fundef)
 *
 * @brief Moves the original body, with generated assignments, of a function
 * with type patterns to a separate definition. By doing so we make it possible
 * to inline the newly generated definition, whilst still allowing the original
 * definition to be no-inline.
 *
 ******************************************************************************/
node *
GTPmakeImpl (node *fundef)
{
    node *res;

    DBUG_ENTER ();

    res = DUPdoDupNode (fundef);

    FUNDEF_NAME (res) = MEMfree (FUNDEF_NAME (res));
    FUNDEF_NAME (res) = GTPmakeValidFundefName ("impl", FUNDEF_NAME (fundef));
    FUNDEF_ASSIGNS (res) = DUPdoDupTree (FUNDEF_ASSIGNS (fundef));
    FUNDEF_ISINLINE (res) = FUNDEF_ISINLINE (fundef);

    DBUG_PRINT ("created impl function %s for %s",
                FUNDEF_NAME (res), FUNDEF_NAME (fundef));

    DBUG_RETURN (res);
}
