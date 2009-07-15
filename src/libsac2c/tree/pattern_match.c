/*
 *
 * $Id$
 */

/**
 *
 * @file pattern_match.c
 *
 * Overview:
 * =========
 *
 * This module implements some basic functionality for writing
 * patten-matching-style expression analyses.
 *
 *
 * A few examples:
 * a) we want to match expressions of the form _sub_SxS_( x, x)
 *    where both x's denote the same variable.
 *    Using this module, we can analyse a given expression "expr" by
 *
 *    node *x=NULL;
 *
 *    if(  PMO( PMOvar( &x, PMOvar( &x, PMOprf( _sub_SxS_, expr))))  ) {
 *      ...
 *    }
 *
 *    If the match is sucessfull, "x" will point to the variable
 *    that matched.
 *    Note here, that we assume flattened SSA form, i.e., it will
 *    trace variable definitions as far as possible and necessary.
 *    Therefore, the above code returns true even if "expr" points to
 *    some variable "a" within a context of the form:
 *
 *    k = ....
 *    ...
 *    b = _sub_SxS_( k, k);
 *    ...
 *    a = b;
 *
 * b) we want to match a more complex constellation of the
 *    form  _sel_VxA_( iv, _modarray_AxVxS_( A, iv, val))
 *    where both iv's again denote the same variable.
 *    This can be matched by:
 *
 *    node *iv=NULL:
 *    node *A=NULL:
 *    node *val=NULL:
 *
 *    if( PMO( PMOvar( &val, PMOvar( &iv, PMOvar( &A, PMOprf( F_modarray_AxVxS,
 *             PMOvar( &iv, PMOprf( F_sel_VxA, expr)))))))  ) {
 *       ...
 *    }
 *
 *    Again, upon sucessfull match, iv, A, and val will point to
 *    the respective identifiers. A matching context for a variable "a"
 *    would be:
 *
 *    B = _modarray_AxVxS_( A, iv, val);
 *    ....
 *    a = _sel_VxA_( iv, B);
 *
 *
 * How to use it:
 * ==============
 *
 * You can use an arbitrary nesting of the matching functions. They
 * all adhere to the following structure:
 *
 *  node *PMO<xyz>( <match-specific-args>, node *stack)
 *
 * The argument stack expects the expression to be matched against.
 * This can either be an expression node (N_num, N_array, N_prf, ...)
 * or even an expression chain (N_exprs). The remaining expressions are
 * returned so that they can be passed on to the next matching function.
 * Matching on "constructors" such as N_prf, N_ap, or N_array extends
 * these chains. NB: this is done internally by using N_set nodes so that
 * the AST remains untouched!
 *
 * At the time being (22.6.07) we support the following matching functions:
 *
 *  PMOvar( node **var, ...) which matches N-id nodes, and
 *  PMOprf( prf fun, ...)    which matches anything that is defined as
 *                          an application of the prf fun.
 *
 * Note here, that all variables that you want to match need to be
 * initialised to NULL prior to matching!
 *
 * The nesting of match functions needs to be surrounded by a call to
 * the function
 *   bool PMO( node *stack)
 * which interprets the result of the match and yields true or false
 * accordingly.
 *
 * Debugging:
 * ==========
 *
 * You may want to use -#d,PMO to observe how your matches perform:-)
 *
 *
 * Some implementation issues:
 * ===========================
 *
 * Their are two main difficulties in this implementation:
 * a) how to maintain a stack of expressions WITHOUT fiddling
 *    around with the existing exprs chains in the AST?
 * b) how to return a modified stack AND a boolean matching flag
 *    without using a global variable and an "out-argument"?
 *
 * We solve a) by introducing a spine of N_set-nodes whenever
 * more than one expr or exprs chain is needed in the stack.
 * This is done dynamically by inserting these whenever we hit
 * a constructor-match (PushArgs) and by freeing these when pulling
 * out the last topmost expression of an N_set node (ExtractOneArg).
 * If the match goes through we expect all exprs to be consumed
 * and, therefore, all N_set nodes to be freed.
 * In case the match fails, FailMatch( stack)  is called which
 * explicitly frees any remaining N_set nodes.
 *
 * b) is solved by creating a special static "node" FAIL which we
 * return instead of a proper stack whenever a match failed. All
 * matching functions therefore need to check agains a FAIL as stack
 * and they need to pass-on that FAIL.
 *
 */

#include <stdarg.h>
#include "pattern_match.h"

#include "dbug.h"
#include "memory.h"
#include "print.h"
#include "globals.h"
#include "free.h"
#include "traverse.h"
#include "tree_basic.h"
#include "new_types.h"
#include "tree_compound.h"
#include "constants.h"
#include "shape.h"
#include "compare_tree.h"

static char *FAIL = "";

static enum pm_mode { PM_exact, PM_flat, PM_flatPseudo } mode;

typedef node *matchFun (pattern *, node *, node **);

struct PAT {
    node **n_arg1;
    int *i_arg1;
    int *i_arg2;
    pattern *p_arg1;
    int num_p_dyn;
    pattern **p_dyn;
    matchFun *matcher;
};

#define PAT_I1(p) (p->i_arg1)
#define PAT_I2(p) (p->i_arg2)
#define PAT_N1(p) (p->n_arg1)
#define PAT_P1(p) (p->p_arg1)
#define PAT_NP(p) (p->num_p_dyn)
#define PAT_PD(p) (p->p_dyn)
#define PAT_FUN(p) (p->matcher)

static pattern *
makePattern (matchFun f, int num_pats)
{
    pattern *res;

    res = (pattern *)MEMmalloc (sizeof (pattern));
    PAT_I1 (res) = NULL;
    PAT_I2 (res) = NULL;
    PAT_N1 (res) = NULL;
    PAT_P1 (res) = NULL;
    PAT_NP (res) = num_pats;
    PAT_PD (res) = (pattern **)MEMmalloc (num_pats * sizeof (pattern *));
    PAT_FUN (res) = f;

    return (res);
}

typedef union {
    prf *a_prf;
    node **a_node;
    constant **a_const;
    int a_num;
    bool a_bool;
    double a_double;
    float a_float;
    char a_char;
} attrib_t;

#define REF_ISUNDEFINED(n) ((n == NULL) || (*n == NULL))
#define REF_SET(n, val)                                                                  \
    {                                                                                    \
        if (n != NULL)                                                                   \
            *n = val;                                                                    \
    }

#define REF_PRF(n) ((n).a_prf)
#define REF_NODE(n) ((n).a_node)
#define REF_CONST(n) ((n).a_const)
#define REF_NUM(n) ((n).a_num)
#define REF_BOOL(n) ((n).a_bool)
#define REF_DOUBLE(n) ((n).a_double)
#define REF_FLOAT(n) ((n).a_float)
#define REF_CHAR(n) ((n).a_char)

typedef bool (*checkFun_ptr) (node *, int, attrib_t *);

/** <!--*********************************************************************-->
 *
 * local helper functions:
 */

/** <!--*******************************************************************-->
 *
 * @fn node *ExtractOneArg( node *stack, node **arg)
 *
 * @brief extracts the first argument from the exprs stack.
 *        Note here, that stack can either be N_set (stack), an N_exprs or
 *        any other expression! N_set nodes that become redundant
 *        are being freed!
 * @param stack: stack of exprs
 * @return via arg the first expression in the chain and the rest of the
 *         stack via the normal return value.
 *****************************************************************************/
static node *
ExtractOneArg (node *stack, node **arg)
{
    node *next;

    DBUG_ENTER ("ExtractOneArg");
    DBUG_ASSERT (stack != NULL, ("ExtractOneArg called with NULL stack!"));

    if (NODE_TYPE (stack) == N_set) {
        next = ExtractOneArg (SET_MEMBER (stack), arg);
        if (next != NULL) {
            SET_MEMBER (stack) = next;
        } else {
            stack = FREEdoFreeNode (stack);
        }
    } else {
        if (NODE_TYPE (stack) == N_exprs) {
            REF_SET (arg, EXPRS_EXPR (stack));
            stack = EXPRS_NEXT (stack);
        } else {
            REF_SET (arg, stack);
            stack = NULL;
        }
        DBUG_PRINT ("PM", ("argument found:"));
        DBUG_EXECUTE ("PM", PRTdoPrintFile (stderr, *arg););
    }
    DBUG_RETURN (stack);
}

/** <!--*******************************************************************-->
 *
 * @fn node *ExtractTopFrame( node *stack, node **top)
 *
 * @brief extracts the top frame from the stack, i.e., the topmost
 *        N_exprs chain. If none such exists, NULL is returned.
 *        Note here, that stack can either be N_set (stack), an N_exprs or
 *        any other expression! N_set nodes that become redundant
 *        are being freed!
 * @param stack: stack of exprs
 * @return via top the top frame of the chain and the rest of the
 *         stack via the normal return value.
 *****************************************************************************/
static node *
ExtractTopFrame (node *stack, node **top)
{
    DBUG_ENTER ("ExtractTopFrame");

    if ((stack != NULL) && (NODE_TYPE (stack) == N_set)
        && (NODE_TYPE (SET_MEMBER (stack)) = N_exprs)) {
        *top = SET_MEMBER (stack);
        stack = FREEdoFreeNode (stack);
    } else {
        DBUG_ASSERT (((stack == NULL) || (NODE_TYPE (stack) == N_exprs)),
                     "unexpected element on stack!");

        *top = stack;
        stack = NULL;
    }

#ifndef DBUG_OFF
    if (*top != NULL) {
        DBUG_PRINT ("PM", ("frame found:"));
        DBUG_EXECUTE ("PM", PRTdoPrintFile (stderr, *top););
    }
#endif

    DBUG_RETURN (stack);
}

/** <!--*******************************************************************-->
 *
 * @fn node *PushArgs( node *stack, node * args)
 *
 * @brief if stack is non-empty, it pushes the args on top of the existing
 *        args in stack by means of N_set nodes. Note here, that stack
 *        can either be N_set (existing stack) N_exprs or any other expression!
 * @param stack: stack of exprs
 *        args: exprs to be stacked on top
 * @return stacked exprs
 *****************************************************************************/
static node *
PushArgs (node *stack, node *args)
{
    DBUG_ENTER ("PushArgs");
    if (stack == NULL) {
        stack = args;
    } else if (NODE_TYPE (stack) == N_set) {
        stack = TBmakeSet (args, stack);
    } else {
        stack = TBmakeSet (args, TBmakeSet (stack, NULL));
    }
    DBUG_RETURN (stack);
}

/** <!--*******************************************************************-->
 *
 * @fn bool isPrfGuard(node *arg_node)
 *
 * @brief If arg_node is an N_prf and is a guard with PRF_ARG1 as
 *        its primary result, return TRUE; else FALSE.
 * @param
 * @return
 *****************************************************************************/
static bool
isPrfGuard (node *arg_node)
{
    bool z;

    DBUG_ENTER ("isPrfGuard");
    z = (N_prf == NODE_TYPE (arg_node));
    if (z) {
        switch (PRF_PRF (arg_node)) {
        default:
            z = FALSE;
            break;
        case F_guard:
        case F_attachextrema:
        case F_attachintersect:
        case F_non_neg_val_V:
        case F_val_lt_shape_VxA:
        case F_val_le_val_VxV:
        case F_shape_matches_dim_VxA:
            z = TRUE;
            break;
        }
    }

    DBUG_RETURN (z);
}

/** <!--*******************************************************************-->
 *
 * @fn static node *lastId( node *arg_node, bool ignoreguards)
 *
 * @brief looks behind the definitions of N_id nodes, if possible
 *        to find the last N_id node in a chain.
 *        Guards are treated as if they are just another
 *        N_id in the chain, if ignoreguards is TRUE.
 *
 *          f = [5,6];
 *          b = f;
 *          c = _attachintersect(b, min, max);
 *          d = c;
 *
 *        If we do lastId(d, TRUE), the result is f.
 *        If we do lastId(d, FALSE), the result is c.
 *
 * @param arg: potential N_id node to be followed after
 * @return first N_id in chain iff available; unmodified arg otherwise
 * `
 *****************************************************************************/
static node *
lastId (node *arg_node, bool ignoreguards)
{
    node *res;
    node *newres;
    node *assgn;
    DBUG_ENTER ("lastId");

    res = arg_node;
    newres = arg_node;
    while ((arg_node != NULL) && (NULL != newres)) {
        newres = NULL;
        /* Find precursor to this node, if it exists */
        if ((NODE_TYPE (arg_node) == N_id)
            && (AVIS_SSAASSIGN (ID_AVIS (arg_node)) != NULL)) {
            DBUG_PRINT ("PMO", ("lastId looking up variable definition for %s.",
                                AVIS_NAME (ID_AVIS (arg_node))));
            newres = arg_node;
            arg_node = LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (arg_node))));
        } else {
            if (ignoreguards && isPrfGuard (arg_node)) {
                newres = PRF_ARG1 (arg_node);
                DBUG_PRINT ("PMO", ("lastId looking past guard, at %s.",
                                    AVIS_NAME (ID_AVIS (newres))));
                assgn = AVIS_SSAASSIGN (ID_AVIS (newres));
                if (NULL != assgn) {
                    arg_node = LET_EXPR (ASSIGN_INSTR (assgn));
                } else {
                    arg_node = NULL;
                }
            }
        }

        if (NULL != newres) {
            res = newres;
            DBUG_PRINT ("PMO", ("lastId definition is: %s.", AVIS_NAME (ID_AVIS (res))));
        }
    }
    DBUG_RETURN (res);
}

/** <!--*******************************************************************-->
 *
 * @fn static node *followId( node * arg_node, bool ignoreguards)
 *
 * @brief looks behind the definitions of N_id nodes, if possible
 *        to find the definition point of a value. For example,
 *          f = [5,6];
 *          a = f;
 *          b = _attachextrema( a);
 *          c = b;
 *          d = c;
 *
 *        If we do followId(d, TRUE), the result is the N_array [5,6].
 *        If we do followId(d, FALSE), the result is FAIL.
 *
 * @param arg: potential N_id node to be followed after
 * @return defining expression iff available;
 *         unmodified arg otherwise
 *****************************************************************************/
static node *
followId (node *arg_node, bool ignoreguards)
{
    node *res;

    DBUG_ENTER ("followId");
    DBUG_PRINT ("PMO", ("followId trying to look up the variable definition "));
    res = lastId (arg_node, ignoreguards);
    if ((NULL != arg_node) && (N_id == NODE_TYPE (res))
        && (NULL != AVIS_SSAASSIGN (ID_AVIS (res)))
        && (NULL != ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (res))))) {
        arg_node = LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (res))));
    }
    DBUG_RETURN (arg_node);
}

bool
isInPseudo (prf prfun)
{
    return ((prfun == F_guard) || (prfun == F_attachextrema)
            || (prfun == F_attachintersect) || (prfun == F_non_neg_val_V)
            || (prfun == F_val_lt_shape_VxA) || (prfun == F_shape_matches_dim_VxA)
            || (prfun == F_val_le_val_VxV));
}

/** <!--*******************************************************************-->
 *
 * @fn node *skipVarDefs( node *expr)
 *
 * @brief follows Variables to their definitions according to
 *        the overall matching mode
 * @param starting expression
 * @return first non-N_id RHS or N_id in case it refers to a function arg.
 *****************************************************************************/
static node *
skipVarDefs (node *expr)
{
    DBUG_ENTER ("skipVarDefs");

    if (expr != NULL) {
        switch (mode) {
        case PM_exact:
            break;
        case PM_flat:
            while ((NODE_TYPE (expr) == N_id)
                   && (AVIS_SSAASSIGN (ID_AVIS (expr)) != NULL)) {
                expr = LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (expr))));
            }
            break;
        case PM_flatPseudo:
            while (
              ((NODE_TYPE (expr) == N_id) && (AVIS_SSAASSIGN (ID_AVIS (expr)) != NULL))
              || ((NODE_TYPE (expr) == N_prf) && (isInPseudo (PRF_PRF (expr))))) {
                if (NODE_TYPE (expr) == N_id) {
                    expr = LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (expr))));
                } else {
                    expr = PRF_ARG1 (expr);
                }
            }
            break;
        }
    }

    DBUG_RETURN (expr);
}

/** <!--*******************************************************************-->
 *
 * @fn node *FailMatch( node *stack)
 *
 * @brief cleans up the remaining stack and creates a FAIL node
 * @param stack: stack of exprs
 * @return FAIL node
 *****************************************************************************/
static node *
FailMatch (node *stack)
{
    DBUG_ENTER ("FailMatch");
    DBUG_PRINT ("PMO", ("match failed!"));
    if ((stack != NULL) && (NODE_TYPE (stack) == N_set)) {
        stack = FREEdoFreeTree (stack);
    }
    DBUG_RETURN ((node *)FAIL);
}

/** <!--*******************************************************************-->
 *
 * @fn node *checkInnerMatchResult( node *inner_stack, node *stack)
 *
 * @brief produces Fail in case the inner match left unmatched items
 *
 *****************************************************************************/
static node *
checkInnerMatchResult (node *inner_stack, node *stack)
{
    if (inner_stack != NULL) {
        DBUG_PRINT ("PM", ("inner match either failed or left unmatched items!"));
        inner_stack = FailMatch (inner_stack);
        stack = FailMatch (stack);
    }
    return (stack);
}

/** <!--*******************************************************************-->
 *
 * @fn node *MatchNode( nodetype nt, checkFun_ptr matchAttribsFun,
 *                      int numAttribs, attrib_t *attribRefs,
 *                      node **matched_node,
 *                      bool pushSons, node *stack, bool ignoreguards)
 *
 * @brief generic matching function. It follows the top of the stack
 *        expression through variable definitions until the specified
 *        node type is found.
 *        If so, matched_node is being set appropriately.
 *        Then, it matches the attributes by means of  matchAttribsFun
 *        and the presented arguments in attribRefs. Generally, this
 *        function should match in case the given attributes are NULL
 *        and compare if they are not.
 *        The parameter pushSons decides whether or not the son nodes
 *        are being pushed on the stack
 * @return potentially extended stack
 *****************************************************************************/
static node *
MatchNode (nodetype nt, checkFun_ptr matchAttribsFun, int numAttribs,
           attrib_t *attribRefs, node **matched_node, bool pushSons, node *stack,
           bool ignoreguards)
{
    node *match = NULL;

    DBUG_ENTER ("MatchNode");

    DBUG_PRINT ("PMO", ("MatchNode trying to match node of type \"%s\"...",
                        global.mdb_nodetype[nt]));

    if (stack != (node *)FAIL) {
        stack = ExtractOneArg (stack, &match);

        match = followId (match, ignoreguards);
        if ((NODE_TYPE (match) == nt)
            && ((numAttribs == 0) || matchAttribsFun (match, numAttribs, attribRefs))) {
            DBUG_PRINT ("PMO", ("MatchNode( %s, _, %d, _, _, %d, _) matched!",
                                global.mdb_nodetype[nt], numAttribs, pushSons));
            if (pushSons) {
                switch (nt) {
                case N_prf:
                    stack = PushArgs (stack, PRF_ARGS (match));
                    break;
                case N_array:
                    stack = PushArgs (stack, ARRAY_AELEMS (match));
                    break;
                case N_id:
                case N_num:
                case N_char:
                case N_bool:
                    break;
                default:
                    DBUG_ASSERT ((FALSE), "pushSons not yet fully implemented!");
                    break;
                }
            }

            REF_SET (matched_node, match);
        } else {
            stack = FailMatch (stack);
            DBUG_PRINT ("PMO", ("failed!"));
        }
    } else {
        DBUG_PRINT ("PMO", ("MatchNode passing on FAIL!"));
    }
    DBUG_RETURN (stack);
}

/** <!--*********************************************************************-->
 *
 * Exported functions for pattern matching:
 */

/** <!--*******************************************************************-->
 *
 * @fn bool PMO( node *stack)
 *
 * @brief checks the result of a pattern match for failure
 * @param stack: resulting stack of a match
 * @return success
 *****************************************************************************/
bool
PMO (node *stack)
{
    DBUG_ENTER ("PMO");
    DBUG_RETURN (stack != (node *)FAIL);
}

/** <!--*******************************************************************-->
 *
 * @fn static node *pmvar( node **var, node *stack,
 *                         bool lastid, bool ignoreguards)
 *
 * @brief tries to match against a variable. If *var is NULL, the top of
 *        the stack is bound to it (provided it is an N_id).
 *        If *var is bound already, it only matches if both N_id nodes
 *        point to the same N_avis.
 *        If ignoreguards is true, search will continue through guard
 *        nodes.
 *
 * @param *var: bound N_id (if any)
 *        stack: "stack" of exprs.
 *        getlastid: if TRUE, chase the list of N_id nodes back to its
 *                origin.
 * @return shortened stack.
 *****************************************************************************/
static node *
pmvar (node **var, node *stack, bool getlastid, bool ignoreguards)
{
    node *arg;

    DBUG_ENTER ("pmvar");
    if (*var == NULL) {
        DBUG_PRINT ("PMO", ("pmvar trying to match unbound variable..."));
    } else {
        DBUG_PRINT ("PMO", ("pmvar trying to match bound variable..."));
    }

    if (stack != (node *)FAIL) {
        stack = ExtractOneArg (stack, &arg);
        if (getlastid) {
            arg = lastId (arg, ignoreguards);
        }
        if (NODE_TYPE (arg) == N_id) {
            if (*var == NULL) {
                DBUG_PRINT ("PMO", ("pmvar binding variable!"));
                *var = arg;
            } else if (ID_AVIS (*var) == ID_AVIS (arg)) {
                DBUG_PRINT ("PMO", ("pmvar found variable matches bound one!"));
            } else {
                stack = FailMatch (stack);
            }
        } else {
            stack = FailMatch (stack);
        }
    } else {
        DBUG_PRINT ("PMO", ("pmvar ...passing-on FAIL!"));
    }
    DBUG_RETURN (stack);
}

/** <!--*******************************************************************-->
 *
 * @fn node *PMOvar( node **var, node *stack)
 *
 * @brief tries to match against a variable. If *var is NULL, the top of
 *        the stack is bound to it (provided it is an N_id).
 *        If *var is bound already, it only matches if both N_id nodes
 *        point to the same N_avis.
 * @param *var: bound N_id (if any)
 *        stack: "stack" of exprs.
 * @return shortened stack.
 *****************************************************************************/
node *
PMOvar (node **var, node *stack)
{
    DBUG_ENTER ("PMOvar");
    stack = pmvar (var, stack, FALSE, FALSE);
    DBUG_RETURN (stack);
}

/** <!--*******************************************************************-->
 *
 * @fn node *PMOlastVar( node **var, node *stack)
 *
 * @brief tries to match against the last variable in a
 *        chain. If *var is NULL, the top of
 *        the stack is bound to it (provided it is an N_id).
 *        If *var is bound already, it only matches if both N_id nodes
 *        point to the same N_avis.
 * @param *var: bound N_id (if any)
 *        stack: "stack" of exprs.
 * @return shortened stack.
 *****************************************************************************/
node *
PMOlastVar (node **var, node *stack)
{
    DBUG_ENTER ("PMOlastVar");
    stack = pmvar (var, stack, TRUE, FALSE);
    DBUG_RETURN (stack);
}

/** <!--*******************************************************************-->
 *
 * @fn node *PMOlastVarGuards( node **var, node *stack)
 *
 * @brief tries to match against the last variable in a
 *        chain, while treating guards as if they were not in
 *        the chain.
 *        E.g., the call PMOlastVarGuards(d...) in this chain:
 *
 *          a = id([3]);
 *          b = a;
 *          c = F_dataflowguard(b, save1, save2);
 *          d = c;
 *
 *        will result in  pointer to <a>.
 *
 *        If *var is NULL, the top of
 *        the stack is bound to it (provided it is an N_id).
 *        If *var is bound already, it only matches if both N_id nodes
 *        point to the same N_avis.
 * @param *var: bound N_id (if any)
 *        stack: "stack" of exprs.
 * @return shortened stack.
 *****************************************************************************/
node *
PMOlastVarGuards (node **var, node *stack)
{
    DBUG_ENTER ("PMOlastVarGuards");
    stack = pmvar (var, stack, TRUE, TRUE);
    DBUG_RETURN (stack);
}

/** <!--*******************************************************************-->
 *
 * @fn node *PMObool( node *stack)
 * @fn node *PMOchar( node *stack)
 * @fn node *PMOnum( node *stack)
 * @fn node *PMOfloat( node *stack)
 * @fn node *PMOdouble( node *stack)
 *
 *****************************************************************************/
#define MATCH_SCALAR_CONST(kind)                                                         \
    node *PMO##kind (node *stack)                                                        \
    {                                                                                    \
        node *kind##_node;                                                               \
        DBUG_ENTER ("PMO##kind");                                                        \
                                                                                         \
        stack = MatchNode (N_##kind, NULL, 0, NULL, &kind##_node, FALSE, stack, FALSE);  \
                                                                                         \
        DBUG_RETURN (stack);                                                             \
    }

MATCH_SCALAR_CONST (bool)
MATCH_SCALAR_CONST (char)
MATCH_SCALAR_CONST (num)
MATCH_SCALAR_CONST (float)
MATCH_SCALAR_CONST (double)

/** <!--*******************************************************************-->
 *
 * @fn node *PMOboolVal( node *stack)
 * @fn node *PMOcharVal( node *stack)
 * @fn node *PMOnumVal( node *stack)
 * @fn node *PMOfloatVal( node *stack)
 * @fn node *PMOdoubleVal( node *stack)
 *
 *****************************************************************************/
#define MATCH_SCALAR_VALUE(kind, typ, accessor)                                          \
    static bool Match##kind##Value (node *arg, int noa, attrib_t *attrs)                 \
    {                                                                                    \
        return (accessor##_VAL (arg) == REF_##accessor (attrs[0]));                      \
    }                                                                                    \
                                                                                         \
    node *PMO##kind##Val (typ val, node *stack)                                          \
    {                                                                                    \
        attrib_t attribs[1];                                                             \
        node *kind##_node = NULL;                                                        \
        DBUG_ENTER ("PMO##kind##Val");                                                   \
                                                                                         \
        REF_##accessor (attribs[0]) = val;                                               \
                                                                                         \
        stack = MatchNode (N_##kind, Match##kind##Value, 1, attribs, &kind##_node,       \
                           FALSE, stack, FALSE);                                         \
                                                                                         \
        DBUG_RETURN (stack);                                                             \
    }

MATCH_SCALAR_VALUE (bool, bool, BOOL)
MATCH_SCALAR_VALUE (char, char, CHAR)
MATCH_SCALAR_VALUE (num, int, NUM)
MATCH_SCALAR_VALUE (float, float, FLOAT)
MATCH_SCALAR_VALUE (double, double, DOUBLE)

/** <!--*******************************************************************-->
 *
 * @fn node *PMOprf( prf fun, node *stack)
 *
 * @brief tries to match against an N_prf with the given fun. If successfull,
 *        the actual arguments are pushed on top of the stack.
 * @param fun: prf to match
 *        stack: stack of exprs
 * @return potentially extended stack
 *****************************************************************************/
static bool
MatchPrfAttribs (node *prf_node, int num, attrib_t *arefs)
{
    return (PRF_PRF (prf_node) == *REF_PRF (arefs[0]));
}

node *
PMOprf (prf fun, node *stack)
{
    node *prf_node;
    attrib_t arefs[1];
    DBUG_ENTER ("PMOprf");

    REF_PRF (arefs[0]) = &fun;
    stack = MatchNode (N_prf, MatchPrfAttribs, 1, arefs, &prf_node, TRUE, stack, FALSE);

    DBUG_RETURN (stack);
}

/** <!--*******************************************************************-->
 *
 * @fn node *PMOarray( constant ** frameshape, node **array, node *stack)
 * @fn node *PMOarrayConstructor( constant ** frameshape, node **array, node *stack)
 *
 * @brief tries to match against an N_array. If *frameshape is NULL, any
 *        array on the top of the stack matches, its AST representation is bound
 *        to array and the frameshape found is converted into a constant which
 *        is bound to *frameshape.
 *        If *frameshape is not NULL, it only matches if the top of the stack is
 *        an N_array with the given frameshape. In that case only array is bound
 *        to the respective part of the AST.
 * @return shortened stack. In PMOarray, the ARRAY_AELEMS are pushed onto the
 *        stackbefore returning, in PMOarrayConstructor NOT!
 *****************************************************************************/
static bool
MatchArrayAttribs (node *array_node, int num, attrib_t *arefs)
{
    bool match;
    constant *shpfound;

    shpfound = COmakeConstantFromShape (ARRAY_FRAMESHAPE (array_node));
    if (REF_ISUNDEFINED (REF_CONST (arefs[0]))) {
        REF_SET (REF_CONST (arefs[0]), shpfound);
        /* still undefied -> must have passed a NULL pointer */
        if (REF_ISUNDEFINED (REF_CONST (arefs[0]))) {
            shpfound = COfreeConstant (shpfound);
        }
        match = TRUE;
    } else if (COcompareConstants (shpfound, *REF_CONST (arefs[0]))) {
        shpfound = COfreeConstant (shpfound);
        match = TRUE;
    } else {
        match = FALSE;
    }
    return (match);
}

node *
PMOarray (constant **frameshape, node **array, node *stack)
{
    attrib_t arefs[1];
    DBUG_ENTER ("PMOarray");

    REF_CONST (arefs[0]) = frameshape;
    stack = MatchNode (N_array, MatchArrayAttribs, 1, arefs, array, TRUE, stack, FALSE);

    DBUG_RETURN (stack);
}

node *
PMOarrayConstructor (constant **frameshape, node **array, node *stack)
{
    attrib_t arefs[1];
    DBUG_ENTER ("PMOarray");

    REF_CONST (arefs[0]) = frameshape;
    stack = MatchNode (N_array, MatchArrayAttribs, 1, arefs, array, FALSE, stack, FALSE);

    DBUG_RETURN (stack);
}

node *
PMOarrayConstructorGuards (constant **frameshape, node **array, node *stack)
{
    attrib_t arefs[1];
    DBUG_ENTER ("PMOarray");

    REF_CONST (arefs[0]) = frameshape;
    stack = MatchNode (N_array, MatchArrayAttribs, 1, arefs, array, FALSE, stack, TRUE);

    DBUG_RETURN (stack);
}

/** <!--*******************************************************************-->
 *
 * @fn node *PMOconst( constant ** co, node **conode, node *stack)
 *
 * @brief tries to match against a constant. If *co is NULL, any constant
 *        on the top of the stack matches, its AST representation is bound
 *        to conode and (a copy of (!)) the constant value is bound to co.
 *        If *co is not NULL, it only matches if the top of the stack is
 *        a constant of the same value. In that case only conode is bound
 *        to the respective part of the AST.
 * @return shortened stack.
 *****************************************************************************/
node *
PMOconst (constant **co, node **conode, node *stack)
{
    node *arg;
    ntype *type;
    constant *cofound = NULL;

    DBUG_ENTER ("PMOconst");

    if (stack != (node *)FAIL) {
        stack = ExtractOneArg (stack, &arg);
        if (NODE_TYPE (arg) == N_id) {
            type = AVIS_TYPE (ID_AVIS (arg));
            if (TYisAKV (type)) {
                cofound = COcopyConstant (TYgetValue (type));
                arg = followId (arg, FALSE); /* needed for conode! */
            }
        } else {
            cofound = COaST2Constant (arg);
        }
        if (cofound != NULL) {
            DBUG_PRINT ("PMO", ("PMOconst matched constant!"));
            if (*co == NULL) {
                *co = cofound;
                *conode = arg;
            } else {
                if (COcompareConstants (*co, cofound)) {
                    DBUG_PRINT ("PMO", ("PMOconst matched value!"));
                    *conode = arg;
                } else {
                    stack = FailMatch (stack);
                }
                cofound = COfreeConstant (cofound);
            }
        } else {
            stack = FailMatch (stack);
        }
    } else {
        DBUG_PRINT ("PMO", ("PMOconst passing on FAIL!"));
    }

    DBUG_RETURN (stack);
}

/** <!--*******************************************************************-->
 *
 * @fn node *PMOintConst( constant ** co, node **conode, node *stack)
 *
 * @brief tries to match against an integer constant. If *co is NULL, any
 *        constant on the top of the stack matches, its AST representation
 *        is bound
 *        to conode and (a copy of (!)) the constant value is bound to co.
 *        If *co is not NULL, it only matches if the top of the stack is
 *        a constant of the same value. In that case only co node is bound
 *        to the respective pasrt of the AST.
 * @return shortened stack.
 *****************************************************************************/
node *
PMOintConst (constant **co, node **conode, node *stack)
{
    constant *co_in;
    DBUG_ENTER ("PMOintConst");

    if (stack != (node *)FAIL) {
        co_in = *co;
        stack = PMOconst (co, conode, stack);
        if (stack != (node *)FAIL) {
            if (COgetType (*co) != T_int) {
                stack = FailMatch (stack);
                if (co_in == NULL) {
                    *co = COfreeConstant (*co);
                }
            }
        }
    } else {
        DBUG_PRINT ("PMO", ("PMOintConst passing on FAIL!"));
    }

    DBUG_RETURN (stack);
}

/** <!--*******************************************************************-->
 *
 * @fn node *PMOshapePrimogenitor( node *stack)
 *
 * @brief This is not really a PMO function, but it has
 *        attributes that sort of want to be one.
 *
 *        This has to work in non-saa mode, so we do not rely
 *        on AVIS_SHAPE.
 *
 *        We define the "shape primogenitor" of an array, id, as:
 *           1. the earliest, in a dataflow sense,
 *              array that has the same shape as id, and
 *           2. that has a direct lineage to id.
 *
 *        The idea here is to trace a chain of N_id and
 *        N_with/modarray nodes from id to the
 *        shape primogenitor of id. Basically,
 *        it does lastId(id, TRUE), but also has some tricks to go
 *        further when that result comes from a modarray WL
 *        or (someday) a genarray WL.
 *
 *        Hence, if we have:
 *
 *          a = iota(shp);
 *          b = a + 1;
 *          c = b * 2;
 *          s0 = idx_shape_sel(0, c);
 *          s1 = idx_shape_sel(1, c);
 *          sv = [s0, s1];
 *          d = with { ([0] <= iv < sv ) : c[iv] + 1;
 *                   } : modarray(c);
 *
 *          CFidx_shape_sel will invoke PMOshapePrimogenitor to produce:
 *
 *          s0 = idx_shape_sel(0, a);
 *          s1 = idx_shape_sel(1, a);
 *
 *        Please excuse the sloppy parameters - I'm not sure how this
 *        would be used in a proper PMO environment. Feel free to
 *        fix it.
 *
 * @param id is
 *
 * @return the uppermost N_id with the same shape.
 *****************************************************************************/
node *
PMOshapePrimogenitor (node *arg)
{
    node *modarr;
    node *res;
    node *defaultcell;

    DBUG_ENTER ("PMOshapePrimogenitor");

    DBUG_PRINT ("PMO", ("PMOshapePrimogenitor trying to find primogenitor for: %s.",
                        AVIS_NAME (ID_AVIS (arg))));

    arg = lastId (arg, TRUE);
    res = arg;

    /* Chase possible modarray WL */
    /* FIXME: probably can do something similar with genarray,
     * if we can find its shape
     */
    modarr = AVIS_SSAASSIGN (ID_AVIS (arg));
    if (NULL != modarr) {
        modarr = LET_EXPR (ASSIGN_INSTR (modarr));
        if ((N_with == NODE_TYPE (modarr))) {
            switch (NODE_TYPE (WITH_WITHOP (modarr))) {
            case N_modarray:
                arg = MODARRAY_ARRAY (WITH_WITHOP (modarr));
                break;
            case N_genarray:
                /* If the default cell is scalar, and the genarray shape is
                 * shape(x), we can replace arg with x.
                 */
                defaultcell = GENARRAY_DEFAULT (WITH_WITHOP (modarr));
                if ((NULL == defaultcell)
                    || ((N_id == NODE_TYPE (defaultcell))
                        && TYisScalar (AVIS_TYPE (ID_AVIS (defaultcell))))) {
                    /*
                     * The default cell is, indeed, scalar.
                     * Result shape is genarray_shape. If that comes from shape(x),
                     * we can replace arg with x.
                     */
                    DBUG_PRINT ("PMO",
                                ("PMOshapePrimogenitor found scalar default cell"));
                }
                break;
            default:
                break;
            }
            /* Recurse to continue up the assign chain. */
            if (arg != res) {
                arg = PMOshapePrimogenitor (arg);
            }
        }
    }
    DBUG_RETURN (arg);
}

/** <!--*******************************************************************-->
 *
 * @fn node *PMOsaashape( node **shp, node **arg, node *stack)
 *
 * @brief tries to match against an AVIS_SHAPE.
 *        If *shp is NULL, the AVIS_SHAPE(*array) is bound to shp.
 *        If *shp is bound already, it only matches if both N_id nodes
 *        have the same AVIS_SHAPE.
 * @param shp AVIS_SHAPE( ID_AVIS(*array), if any
 * @param stack "stack" of exprs.
 *
 * @return stack is unchanged.
 *****************************************************************************/
node *
PMOsaashape (node **shp, node **array, node *stack)
{
    node *arg;
    DBUG_ENTER ("PMOsaashape");
    if (*shp == NULL) {
        DBUG_PRINT ("PMO", ("PMOsaashape trying to match unbound variable."));
    } else {
        DBUG_PRINT ("PMO", ("PMOsaashape trying to match bound variable."));
    }

    if (stack != (node *)FAIL) {
        arg = AVIS_SHAPE (ID_AVIS (*array));
        if (NULL != arg) {
            arg = lastId (arg, TRUE); /* Not sure about ignoreguards value here... */
        }
        if ((NULL != arg) && (N_id == NODE_TYPE (arg))) {
            if (REF_ISUNDEFINED (shp)) {
                DBUG_PRINT ("PMO", ("PMOsaashape binding AVIS_SHAPE"));
                REF_SET (shp, AVIS_SHAPE (ID_AVIS (arg)));
            } else if (*shp == AVIS_SHAPE (ID_AVIS (arg))) {
                DBUG_PRINT ("PMO", ("PMOsaashape found matching AVIS_SHAPE"));
            } else {
                stack = FailMatch (stack);
            }
        } else {
            stack = FailMatch (stack);
        }
    } else {
        DBUG_PRINT ("PMO", ("PMOsaashape passing-on FAIL."));
    }
    DBUG_RETURN (stack);
}

/** <!-- ****************************************************************** -->
 * @brief Matches the given pattern against each subexpression of the
 *        top-most stack frame, i.e., N_exprs chain. For each match,
 *        additionally to the expression to match on, the position
 *        within the exprs chain starting with 0 is passed to the
 *
 * @param pattern pattern matching function used for each element
 * @param stack the current stack
 * @return shortened stack
 ******************************************************************************/
node *
PMOforEachI (node *(*pattern) (int, node *stack), node *stack)
{
    node *exprs;
    bool success = TRUE;
    int pos = 0;

    DBUG_ENTER ("PMOforEachI");

    if (stack != (node *)FAIL) {
        stack = ExtractTopFrame (stack, &exprs);

        DBUG_ASSERT ((exprs != NULL), "No exprs on top of stack");

        do {
            success = PMO (pattern (pos, EXPRS_EXPR (exprs)));

            exprs = EXPRS_NEXT (exprs);
            pos++;
        } while ((exprs != NULL) && success);

        if (!success) {
            stack = FailMatch (stack);
        }
    }

    DBUG_RETURN (stack);
}

/** <!-- ****************************************************************** -->
 * @brief Matches any single expression. If any is non-NULL, it fails if the
 *        expressions are non-equal. If any points to an empty  memory
 *        location, the matched expression is stored.
 *
 * @param any   expression to match against or NULL
 * @param stack the stack
 *
 * @return modified stack
 ******************************************************************************/
node *
PMOany (node **any, node *stack)
{
    node *actual;

    DBUG_ENTER ("PMOany");

    if (stack != (node *)FAIL) {
        stack = ExtractOneArg (stack, &actual);

        if (REF_ISUNDEFINED (any)) {
            REF_SET (any, actual);
        } else if (CMPTdoCompareTree (actual, *any) == CMPT_NEQ) {
            stack = FailMatch (stack);
        }
    }

    DBUG_RETURN (stack);
}

/** <!-- ****************************************************************** -->
 * @brief Matches any chain of expression. If exprs is non-NULL, it fails if
 *        the expressions are non-equal. If exprs points to an empty memory
 *        location, the matched expression is stored.
 *
 * @param any   expressions to match against or NULL
 * @param stack the stack
 *
 * @return modified stack
 ******************************************************************************/
node *
PMOexprs (node **exprs, node *stack)
{
    node *top;

    DBUG_ENTER ("PMOexprs");

    if (stack != (node *)FAIL) {
        stack = ExtractTopFrame (stack, &top);

        if (top == NULL) {
            if (!REF_ISUNDEFINED (exprs)) {
                /* NULL matches an empty stack frame */
                stack = FailMatch (stack);
            }
        } else {
            if (REF_ISUNDEFINED (exprs)) {
                REF_SET (exprs, top);
            } else if (CMPTdoCompareTree (top, *exprs) == CMPT_NEQ) {
                stack = FailMatch (stack);
            }
        }
    }

    DBUG_RETURN (stack);
}

/** <!-- ****************************************************************** -->
 * @brief Matches the given chain of expressions against the top
 *        frame on the stack.
 *
 * @param exprs   expressions to match against
 * @param stack   the stack
 *
 * @return modified stack
 ******************************************************************************/
node *
PMOpartExprs (node *exprs, node *stack)
{
    node *top;

    DBUG_ENTER ("PMOpartExprs");

    if (stack != (node *)FAIL) {
        stack = ExtractTopFrame (stack, &top);

        if (top == NULL) {
            stack = FailMatch (stack);
        } else {
            while ((exprs != NULL) && (top != NULL)) {
                if (CMPTdoCompareTree (EXPRS_EXPR (top), EXPRS_EXPR (exprs))
                    == CMPT_NEQ) {
                    stack = FailMatch (stack);
                    break;
                }

                exprs = EXPRS_NEXT (exprs);
                top = EXPRS_NEXT (top);
            }
            if (exprs != NULL) {
                stack = FailMatch (stack);
            } else if (top != NULL) {
                stack = PushArgs (stack, top);
            }
        }
    }

    DBUG_RETURN (stack);
}

/** <!-- ****************************************************************** -->
 *
 * pattern *PMvar( node ** var)
 *
 * @brief identifyer patter:  < *var>
 *        - no inner pattern
 *        - does NOT depend on matching mode!
 *
 *        tri-state argument:
 *        var == NULL   => ignore what you match
 *        *var == NULL  => bring back link to the avis of the var matched
 *        *var != NULL  => match iff the variable matched points to THAT avis
 *
 ******************************************************************************/
static node *
patternVar (pattern *pat, node *stack, node **match)
{
    node **var;
    node *arg;

    if (match == NULL) {
        DBUG_PRINT ("PM", ("PMvar trying to match N_id"));
    } else if (*match == NULL) {
        DBUG_PRINT ("PM", ("PMvar trying to match and fetch N_id."));
    } else {
        DBUG_PRINT ("PM", ("PMvar trying to match N_id and compare against \"%s\".",
                           AVIS_NAME (*match)));
    }
    var = PAT_N1 (pat);

    stack = ExtractOneArg (stack, &arg);

    if ((arg != NULL) && (NODE_TYPE (arg) == N_id)) {
        DBUG_PRINT ("PM", ("matching variable \"%s\"", ID_NAME (arg)));

        if (var == NULL) {
            DBUG_PRINT ("PM", ("ignoring matched variable!"));
        } else if (*var == NULL) {
            DBUG_PRINT ("PM", ("binding matched variable!"));
            *var = arg;
        } else if (ID_AVIS (*var) == ID_AVIS (arg)) {
            DBUG_PRINT ("PM", ("variable matches bound one!"));
        } else {
            stack = FailMatch (stack);
        }
    }

    if (match != NULL) {
        *match = arg;
    }

    return (stack);
}

pattern *
PMvar (node **var)
{
    pattern *res;

    res = makePattern (patternVar, 0);
    PAT_N1 (res) = var;

    return (res);
}

/** <!-- ****************************************************************** -->
 *
 * pattern *PMint( int * v)
 *
 * @brief scalar value pattern:  < *v>
 *        - no inner pattern
 *        - does depend on matching mode!
 *
 *        bi-state argument:
 *        v == NULL   => ignore what you match
 *        v != NULL  => bring back int value of the N_num node matched.
 *
 ******************************************************************************/
static node *
patternInt (pattern *pat, node *stack, node **match)
{
    node *arg;

    DBUG_PRINT ("PM", ("trying to match N_num"));

    stack = ExtractOneArg (stack, &arg);
    arg = skipVarDefs (arg);

    if ((arg != NULL) && (NODE_TYPE (arg) == N_num)) {
        DBUG_PRINT ("PM", ("N_num with value %d found!", NUM_VAL (arg)));
        *PAT_I1 (pat) = NUM_VAL (arg);
        if (match != NULL) {
            *match = arg;
        }
    } else {
        stack = FailMatch (stack);
    }

    return (stack);
}

pattern *
PMint (int *v)
{
    pattern *res;

    res = makePattern (patternInt, 0);
    PAT_I1 (res) = v;

    return (res);
}

/** <!-- ****************************************************************** -->
 *
 * pattern *PMintLE( int * v1, int *v2)
 *
 * @brief scalar value pattern:  < *v1>   |  *v1 <= *v2
 *        - no inner pattern
 *        - does depend on matching mode!
 *
 *        we expect ( v1 != NULL) and (v2!= NULL)!!!
 *
 *        *v1 will bring back int value of the N_num node matched.
 *
 ******************************************************************************/
static node *
patternIntLE (pattern *pat, node *stack, node **match)
{
    node *arg;

    DBUG_PRINT ("PM", ("trying to match N_num that is <= %d", *PAT_I2 (pat)));

    stack = ExtractOneArg (stack, &arg);
    arg = skipVarDefs (arg);

    if ((arg != NULL) && (NODE_TYPE (arg) == N_num) && (NUM_VAL (arg) <= *PAT_I2 (pat))) {
        DBUG_PRINT ("PM", ("N_num with value %d found!", NUM_VAL (arg)));
        *PAT_I1 (pat) = NUM_VAL (arg);
        if (match != NULL) {
            *match = arg;
        }
    } else {
        stack = FailMatch (stack);
    }

    return (stack);
}

pattern *
PMintLE (int *v1, int *v2)
{
    pattern *res;

    res = makePattern (patternIntLE, 0);
    DBUG_ASSERT ((v1 != NULL), "PMintLE expects non-NULL first argument");
    DBUG_ASSERT ((v2 != NULL), "PMintLE expects non-NULL second argument");
    PAT_I1 (res) = v1;
    PAT_I2 (res) = v2;

    return (res);
}

/** <!-- ****************************************************************** -->
 *
 * pattern *PMarray( int num_pats, va_list arg_p)
 *
 * @brief array pattern:  [ sub_pat_1, ..., sub_pat_{num_pats}]
 *        - num_pats inner pattern
 *        - does depend on matching mode!
 *
 ******************************************************************************/
static node *
patternArray (pattern *pat, node *stack, node **match)
{
    node *inner_stack;
    node *arg;
    int i;

    DBUG_PRINT ("PM", ("trying to match N_array"));

    stack = ExtractOneArg (stack, &arg);
    arg = skipVarDefs (arg);

    if ((arg != NULL) && (NODE_TYPE (arg) == N_array)) {
        DBUG_PRINT ("PM", ("N_array found!"));
        if (match != NULL) {
            *match = arg;
        }
        inner_stack = ARRAY_AELEMS (arg);

        for (i = 0; i < PAT_NP (pat); i++) {
            inner_stack = PAT_FUN (PAT_PD (pat)[i]) (PAT_PD (pat)[i], inner_stack, NULL);
            if (inner_stack == (node *)FAIL) {
                i = PAT_NP (pat);
            }
        }
        stack = checkInnerMatchResult (inner_stack, stack);
    } else {
        stack = FailMatch (stack);
    }

    return (stack);
}

pattern *
PMarray (int num_pats, ...)
{
    va_list ap;
    pattern *res;
    int i;

    res = makePattern (patternArray, num_pats);

    va_start (ap, num_pats);
    for (i = 0; i < num_pats; i++) {
        PAT_PD (res)[i] = va_arg (ap, pattern *);
    }
    va_end (ap);

    return (res);
}

/** <!-- ****************************************************************** -->
 *
 * pattern *PMarrayLen( int * l, int num_pats, va_list arg_p)
 *
 * @brief array pattern:  [ sub_pat_1, ..., sub_pat_{num_pats}]
 *        - num_pats inner pattern
 *        - does depend on matching mode!
 *
 *        bi-state argument:
 *        *l == -1  => bring back ravel-length of N_array matched
 *        *l >= 0   => match iff the ravel-length of N_array == *l

 *
 ******************************************************************************/
static node *
patternArrayLen (pattern *pat, node *stack, node **match)
{
    node *arg, *inner_stack;
    int i;
    int *l;

    l = PAT_I1 (pat);
    DBUG_PRINT ("PM",
                ("trying to match N_array of frame-length %d (-1 == arbitrary)!", *l));

    stack = ExtractOneArg (stack, &arg);
    arg = skipVarDefs (arg);

    if ((arg != NULL) && (NODE_TYPE (arg) == N_array)
        && ((*l == -1) || (*l == SHgetUnrLen (ARRAY_FRAMESHAPE (arg))))) {
        DBUG_PRINT ("PM", ("N_array with frame-length %d found!",
                           SHgetUnrLen (ARRAY_FRAMESHAPE (arg))));

        *l = SHgetUnrLen (ARRAY_FRAMESHAPE (arg));

        if (match != NULL) {
            *match = arg;
        }
        inner_stack = ARRAY_AELEMS (arg);

        for (i = 0; i < PAT_NP (pat); i++) {
            inner_stack = PAT_FUN (PAT_PD (pat)[i]) (PAT_PD (pat)[i], inner_stack, NULL);
            if (inner_stack == (node *)FAIL) {
                i = PAT_NP (pat);
            }
        }
        stack = checkInnerMatchResult (inner_stack, stack);
    } else {
        stack = FailMatch (stack);
    }

    return (stack);
}

pattern *
PMarrayLen (int *l, int num_pats, ...)
{
    va_list ap;
    pattern *res;
    int i;

    res = makePattern (patternArrayLen, num_pats);

    PAT_I1 (res) = l;

    va_start (ap, num_pats);
    for (i = 0; i < num_pats; i++) {
        PAT_PD (res)[i] = va_arg (ap, pattern *);
    }
    va_end (ap);

    return (res);
}

#if 0
extern pattern *PMprf( prf fun, int num_pats, ...);
#endif

/** <!--*********************************************************************-->
 *
 * @fn pattern *PMfetch( node **hook, pattern *what)
 *
 * @brief fetch pattern:   <what> as <*hook>
 *        - has one inner pattern <what>
 *        - in case <what> matches, the corresponding pointer in the AST
 *          is placed into <*hook>.
 */

static node *
patternFetch (pattern *pat, node *stack, node **match)
{
    pattern *what_pat;

    what_pat = PAT_P1 (pat);
    stack = PAT_FUN (what_pat) (what_pat, stack, PAT_N1 (pat));

    return (stack);
}

pattern *
PMfetch (node **hook, pattern *what)
{
    pattern *res;

    /* DBUG_ASSERT( hook != NULL) */

    res = makePattern (patternFetch, 0);
    PAT_N1 (res) = hook;
    PAT_P1 (res) = what;
    return (res);
}

/** <!--*********************************************************************-->
 *
 * @fn pattern *PMfetchAsVar( node **hook, pattern *what)
 *
 * @brief fetch pattern:   <id> = <what> as <*hook>
 *        - has one inner pattern <what>
 *        - the corresponding expression needs to be an N_id node which
 *          is defined by something that matches the inner pattern <what>.
 *          If that holds, <id> is placed into <*hook>.
 */

static node *
patternFetchAsVar (pattern *pat, node *stack, node **match)
{
    node *inner_stack, *arg;
    pattern *what_pat;

    what_pat = PAT_P1 (pat);

    stack = ExtractOneArg (stack, &arg);

    if ((arg != NULL) && (NODE_TYPE (arg) == N_id)) {
        DBUG_PRINT ("PM", ("matching variable \"%s\"", ID_NAME (arg)));
        if (match != NULL) {
            *match = arg;
        }
        inner_stack = PAT_FUN (what_pat) (what_pat, arg, NULL);
        stack = checkInnerMatchResult (inner_stack, stack);
    } else {
        stack = FailMatch (stack);
    }

    return (stack);
}

pattern *
PMfetchAsVar (node **hook, pattern *what)
{
    pattern *res;

    /* DBUG_ASSERT( hook != NULL) */

    res = makePattern (patternFetchAsVar, 0);
    PAT_N1 (res) = hook;
    PAT_P1 (res) = what;
    return (res);
}

#if 0
extern pattern *PMretryAny( int *i, int *l, int num_pats, ...);

/** <!--*********************************************************************-->
 *
 * @fn pattern *PMretryAll( int *i, int *l, int num_pats, ...);
 *
 * @brief fetch pattern:   <what> as <*hook>
 *        - has one inner pattern <what>
 *        - in case <what> matches, the corresponding pointer in the AST
 *          is placed into <*hook>.
 */

static
node *patternAll( pattern *pat, node *stack, node **match)
{
  node *rest;
  pattern *elem_pat;

  rest = PMarray( NULL, match, stack);
  elem_pat = PAT_P1( pat);

  return( PAT_FUN(elem_pat)( elem_pat, rest, NULL));
}

pattern *Pall( pattern *what)
{
  pattern *res;

  res = makePattern( patternAll);
  PAT_P1( res) = what;
  return( res);
}

#endif

/** <!--*********************************************************************-->
 *
 * @fn pattern *PMskip( );
 *
 * @brief skipping pattern:   ...
 *        deletes stack and returnd NULL unless it contains FAIL which is
 *        propagated.
 */

static node *
patternSkip (pattern *pat, node *stack, node **match)
{
    DBUG_PRINT ("PM", ("skipping remaining elements!"));

    if (match != NULL) {
        stack = ExtractTopFrame (stack, match);
    }
    if (stack != (node *)FAIL) {
        if ((stack != NULL) && (NODE_TYPE (stack) == N_set)) {
            stack = FREEdoFreeTree (stack);
        } else {
            stack = NULL;
        }
    }

    return (stack);
}

pattern *
PMskip ()
{
    pattern *res;

    res = makePattern (patternSkip, 0);
    return (res);
}

/*******************************************************************************
 *
 */

pattern *
PMfree (pattern *p)
{
    DBUG_ENTER ("PMfree");
    if (p != NULL) {
        PAT_P1 (p) = PMfree (PAT_P1 (p));
        if (PAT_NP (p) > 0) {
            PAT_PD (p) = (pattern **)MEMfree (PAT_PD (p));
        }
        p = (pattern *)MEMfree (p);
    }
    DBUG_RETURN (p);
}

/*******************************************************************************
 *
 */

int
PMmatchExact (pattern *pat, node *expr)
{
    mode = PM_exact;

    return (PAT_FUN (pat) (pat, expr, NULL) != (node *)FAIL);
}

int
PMmatchFlat (pattern *pat, node *expr)
{
    mode = PM_flat;

    return (PAT_FUN (pat) (pat, expr, NULL) != (node *)FAIL);
}

int
PMmatchFlatPseudo (pattern *pat, node *expr)
{
    mode = PM_flatPseudo;

    return (PAT_FUN (pat) (pat, expr, NULL) != (node *)FAIL);
}
