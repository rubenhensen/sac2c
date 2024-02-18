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
#include "free.h"
#include "DupTree.h"
#include "memory.h"
#include "namespaces.h"
#include "new_types.h"
#include "shape.h"
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
 * @fn node *MakeConditionGuards (char *pred, node *conditions)
 *
 * @brief Convert conditions, e.g. foo(int a) | a > 0, to checks for our pre-
 * and post-check functions. Namely, for this example, we generate
 *   pred = a > 0 ? pred : _|_ ("condition failed");
 *
 * @todo If we convert the condition to a string we can make our error message
 * more descriptive: "condition a > 0 failed".
 *
 ******************************************************************************/
static node *
MakeConditionGuards (char *pred, node *conditions)
{
    node *guard, *bottom;
    node *res = NULL;

    DBUG_ENTER ();

    while (conditions != NULL) {
        bottom = TBmakeType (TYmakeBottomType (STRcpy ("condition failed")));
        guard = TBmakeFuncond (EXPRS_EXPR (conditions),
                               TBmakeSpid (NULL, STRcpy (pred)),
                               TCmakePrf1 (F_guard_error, bottom));
        guard = TBmakeLet (TBmakeSpids (STRcpy (pred), NULL), guard);
        res = TCappendAssign (res, TBmakeAssign (guard, NULL));

        conditions = EXPRS_NEXT (conditions);
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @fn node *MakeGuardFundef (char *guard_str, node *fundef, node *args,
 *                            node *body)
 *
 * @brief Generates the pre- or post-check function, based on the original
 * function definition, the new arguments, and the new function body.
 * For example:
 *
 * inline
 * bool foo_post (int[*] a, int[*] b, int[+] res) {
 *     pred = true;
 *     assignments...
 *     checks...
 *     return pred;
 * }
 *
 * Note that whereas the pre-check function only requires the original
 * arguments. The post-check functions also requires the returned values, since
 * these can also contain type patterns.
 *
 ******************************************************************************/
static node *
MakeGuardFundef (char *guard_str, node *fundef, node *args, node *body)
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
 * @fn node *AddPred (char *pred, node *body)
 *
 * @brief Surrounds a function body with an assignment and return of the
 * predicate. This results in the following function:
 *
 * bool foo_pre (int[*] a, int[*] b) {
 *     pred = true;
 *     ...
 *     return pred;
 * }
 *
 ******************************************************************************/
static node *
AddPred (char *pred, node *body)
{
    node *let, *ret;

    DBUG_ENTER ();

    let = TBmakeLet (TBmakeSpids (STRcpy (pred), NULL), TBmakeBool (TRUE));
    ret = TBmakeReturn (TBmakeExprs (TBmakeSpid (NULL, STRcpy (pred)), NULL));
    body = TCappendAssign (TBmakeAssign (let, body), TBmakeAssign (ret, NULL));

    DBUG_RETURN (body);
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
GTPmakeValidFundefName (char *guard_str, char *fundef_name)
{
    char *res;

    DBUG_ENTER ();

    guard_str = TRAVtmpVarName (guard_str);
    res = IsValidFundefName (fundef_name)
            ? STRcatn (3, guard_str, "_", fundef_name)
            : STRcpy (guard_str);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @fn node *GTPmakePreCheck (node *fundef, char *pred,
 *                            node *assigns, node *checks)
 *
 * @brief Generates a function that containts the pre-conditions for a function
 * with type patterns. It returns true if all constraints are satisfied,
 * otherwise a bottom type is raised for all failing conditions. We make this
 * function inline as the call-site of this function more likely contains
 * information that can help in statically resolving these conditions.
 *
 ******************************************************************************/
node *
GTPmakePreCheck (node *fundef, char *pred, node *assigns, node *checks)
{
    node *args, *conds, *res;

    DBUG_ENTER ();

    args = DUPdoDupTree (FUNDEF_ARGS (fundef));
    conds = MakeConditionGuards (pred, FUNDEF_PRECONDS (fundef));
    FUNDEF_PRECONDS (fundef) = NULL;

    res = TCappendAssign (assigns, checks);
    res = TCappendAssign (res, conds);
    res = AddPred (pred, res);
    res = MakeGuardFundef ("pre", fundef, args, res);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @fn node *GTPmakePostCheck (node *fundef, char *pred, node *assigns,
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
GTPmakePostCheck (node *fundef, char *pred, node *assigns,
                  node *checks, node *return_ids)
{
    node *args, *conds, *post_lhs, *rets, *res;

    DBUG_ENTER ();

    DBUG_PRINT ("GTPmakePostCheck");

    args = DUPdoDupTree (FUNDEF_ARGS (fundef));
    post_lhs = return_ids;
    rets = FUNDEF_RETS (fundef);

    while (rets != NULL) {
        char *v = SPID_NAME (EXPRS_EXPR (post_lhs));
        node *avis = TBmakeAvis (STRcpy (v), TYcopyType (RET_TYPE (rets)));
        node *arg = TBmakeArg (avis, NULL);

        AVIS_DECL (avis) = arg;
        AVIS_DECLTYPE (avis) = TYcopyType (RET_TYPE (rets));
        args = TCappendArgs (args, arg);

        rets = RET_NEXT (rets);
        post_lhs = EXPRS_NEXT (post_lhs);
    }

    conds = MakeConditionGuards (pred, FUNDEF_POSTCONDS (fundef));
    FUNDEF_POSTCONDS (fundef) = NULL;

    res = TCappendAssign (assigns, checks);
    res = TCappendAssign (res, conds);

    res = AddPred (pred, res);
    res = MakeGuardFundef ("post", fundef, args, res);

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
 * @fn node *MakeGuardLet (node *ids, node *exprs, node* types, char *pred,
 *                         const char *context)
 *
 * @brief Creates a guard assignment:
 *   x1', ..., xn' = guard (x1, .., xn, t1, .., tn, pred)
 * That guards the given identifiers, based on the given predicate.
 *
 ******************************************************************************/
static node *
MakeGuardLet (node *ids, node *exprs, node* types, char *pred,
              const char *context)
{
    node *spid, *prf, *res;
    size_t num_rets;

    DBUG_ENTER ();

    num_rets = TCcountExprs (exprs);

    // Append types to arguments
    exprs = TCappendExprs (exprs, types);
    // Append predicate to arguments
    spid = TBmakeSpid (NULL, STRcpy (pred));
    exprs = TCappendExprs (exprs, TBmakeExprs (spid, NULL));

    // Create guard primitive
    prf = TBmakePrf (F_guard, exprs);

    PRF_NUMVARIABLERETS (prf) = num_rets;
    PRF_CONTEXTSTRING (prf) = STRcpy (context);
    DBUG_PRINT ("guard created with %lu return values", num_rets);

    res = TBmakeLet (ids, prf);

    DBUG_RETURN (res);
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
    char context[256];
    char *pred;

    DBUG_ENTER ();

    pred = TRAVtmpVarName ("pred");

    ConvertArgs (FUNDEF_ARGS (fundef), &pre_lhs, &pre_args);
    ConvertRets (FUNDEF_RETS (fundef), &post_lhs, &post_args);

    // return (x, y, z)
    body = TBmakeAssign (TBmakeReturn (DUPdoDupTree (post_args)), NULL);

    if (post != NULL) {
        // x, y, z = guard (x, y, z, pred)
        sprintf (context, "Type pattern post-condition of %s failed",
                 FUNDEF_NAME (fundef));
        let = MakeGuardLet (DUPdoDupTree (post_lhs),
                            DUPdoDupTree (post_args),
                            TUmakeTypeExprsFromRets (FUNDEF_RETS (fundef)),
                            pred,
                            context);
        body = TBmakeAssign (let, body);

        // pred = foo_post (a, b, x, y, z)
        ap = TBmakeAp (post, TCappendExprs (DUPdoDupTree (pre_args), post_args));
        let = TBmakeLet (TBmakeSpids (STRcpy (pred), NULL), ap);
        body = TBmakeAssign (let, body);
    }

    if (pre != NULL) {
        // x, y, z = guard (x, y, z, pred)
        sprintf (context, "Type pattern pre-condition of %s failed",
                 FUNDEF_NAME (fundef));
        let = MakeGuardLet (DUPdoDupTree (post_lhs),
                            DUPdoDupTree (post_args),
                            TUmakeTypeExprsFromRets (FUNDEF_RETS (fundef)),
                            pred,
                            context);
        body = TBmakeAssign (let, body);
    }

    // x, y, z = foo_impl (a, b)
    ap = TBmakeSpap (TBmakeSpid (NULL, STRcpy (FUNDEF_NAME (impl))),
                     DUPdoDupTree (pre_args));
    let = TBmakeLet (post_lhs, ap);
    body = TBmakeAssign (let, body);

    if (pre != NULL) {
        // a, b = guard (a, b, pred)
        sprintf (context, "Type pattern pre-condition of %s failed",
                 FUNDEF_NAME (fundef));
        let = MakeGuardLet (pre_lhs,
                            DUPdoDupTree (pre_args),
                            TUmakeTypeExprsFromArgs (FUNDEF_ARGS (fundef)),
                            pred,
                            context);
        body = TBmakeAssign (let, body);

        // pre_pred = foo_pre (a, b)
        ap = TBmakeAp (pre, pre_args);
        let = TBmakeLet (TBmakeSpids (STRcpy (pred), NULL), ap);
        body = TBmakeAssign (let, body);
    }

    FUNDEF_ASSIGNS (fundef) = FREEoptFreeTree (FUNDEF_ASSIGNS (fundef));
    FUNDEF_ASSIGNS (fundef) = body;
    FUNDEF_ISINLINE (fundef) = TRUE;
    FUNDEF_CHECKIMPLFUNDEF (fundef) = impl;

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
