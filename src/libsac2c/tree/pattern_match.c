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
 *    pattern *pat;
 *
 *    pat = PMprf( 1, PMAisPrf( F__sub_SxS),
 *                 2, PMvar( 1, PMAgetNode( &x), 0),
 *                    PMvar( 1, PMAisVar( &X), 0));
 *
 *    if( PMmatchFlat( pat, expr) ) {
 *      ...
 *    }
 *
 *    pat = PMfree( pat);
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
 *    form  _sel_VxA_( iv, _modarray_AxVxS_( ., iv, val))
 *    where both iv's again denote the same variable.
 *    This can be matched by:
 *
 *    node *iv=NULL:
 *    node *val=NULL:
 *    pattern *pat;
 *
 *    pat = PMprf( 1, PMAisPrf( F_sel_VxA),
 *                 2, PMvar( 1, PMAgetNode( &iv), 0),
 *                    PMprf( 1, PMAisPrf( F_modarray_AxVxS),
 *                           3, PMvar( 0, 0),
 *                              PMvar( 1, PMAisVar( &iv), 0),
 *                              PMvar( 1, PMAgetNode( &val), 0)));
 *
 *    if( PMmatchFlat( pat, expr) {
 *       ...
 *    }
 *
 *    pat = PMfree( pat);
 *
 *    Again, upon sucessfull match, iv, and val will point to
 *    the respective identifiers. A matching context for a variable "a"
 *    would be:
 *
 *    C = _modarray_AxVxS_( A, iv, val);
 *    ....
 *    B = C;
 *    ....
 *    a = _sel_VxA_( iv, B);
 *
 * c) sometimes, we want to match something that is a constant, but for
 *    creating some new code, we actually need a variable that is defined
 *    by that constant. Assume that we look for _sel_VxA_( const, .), but
 *    need a variable containing const.
 *    Since all SAC code is flattened, we know that such a variable actually
 *    does exist! We can accomplish this by:
 *
 *    node *x;
 *    pattern *pat;
 *
 *    pat = PMprf( 1, PMAisPrf( F__sel_VxA),
 *                 2, PMvar( 1, PMAgetNode( &x),
 *                           1, PMconst( 0)),
 *                    PMvar( 0, 0))
 *
 *    if( PMmatchFlat( pat, expr) {
 *       ...
 *    }
 *
 *    pat = PMfree( pat);
 *
 *    Upon successfull match, x will point to an N_id node that has a
 *    constant definition. For example, if matched against a in the following
 *    context, x would point to B:
 *
 *    C = [ 2, 4];
 *    ...
 *    B = C;
 *    ...
 *    a = _sel_VxA_( B, E);
 *
 *
 *
 *
 *
 * How to use it:
 * ==============
 *
 * As you can see from the examples above, matching requires
 * 1) creating a pattern
 * 2) matching the pattern against expressions (once or several times!)
 * 3) freeing the pattern
 *
 * Matching Modes for Pattern-Matching:
 * ------------------------------------
 * There are several matching functions available. They differ in the
 * way they chase up N_id arguments such as "B" in the example b) above.
 *
 * Currently (19.07.2009), we have:
 * - PMmatchExact( pat, expr) which does not follow any definitions at all.
 *                            Using this matching function in the second
 *                            example above would NOT match!
 * - PMmatchFlat( pat, expr)  is the standard matcher. It follows variable
 *                            definitions and simple variable assignments.
 * - PMmatch...
 *
 * Note here, that the chasing of N_id nodes to their definitions happens
 * implicitly for all pattern. The only exception is PMvar which ALWAYS
 * matches an N_id node directly. However, PMvar enables further inspection
 * of its definition by an optional sub-pattern. For more details see below.
 *
 * Creating Pattern:
 * -----------------
 *
 * We have three categories of pattern:
 * - singleton pattern that match expressions without subexpressions
 *   such as N_num.
 * - nested pattern that match constructor-expressions containing
 *   subexpressions such as N_prf, N_ap, or N_array.
 * - iteration pattern that enable repeated matching with back-tracking
 *
 * All pattern constructing functions of the first 2 categories adhere to
 * the following structure:
 *
 * pattern *PM<xyz>( int num_attribs, attr_1, ..., attr_{num_attribs},
 *                   int num_subpats, pat_1, ..., pat_{num_subpats} )
 *
 * However, singleton pattern do not expect any of the arguments in the
 * second line.
 *
 * Each pattern can come with an abitrary number of attributes which
 * refine their matching behaviour. For details on attributes see
 * below the section on "Attributes".
 * All nested pattern come with a list of subpattern which need to match
 * all subexpressions of the matched node. Note here, that we have pattern
 * that match more than one expression; so there is no need to have a
 * one-to-one correspondence between subpattern and subexpressions!
 *
 * At the time being (18.7.09) we support the following pattern:
 *
 * SINGLETON PATTERN:
 * - PMconst   matches any constant expression (could be N_num, N_array, etc)
 * - PMint     matches an N_num
 * - PMskip    matches ALL expressions left to be matched ( '...'-pattern)
 *
 * NESTED PATTERN:
 * - PMvar     matches an N_id        WITHOUT following N_id nodes
 *             NB: this is the only nested pattern with an OPTIONAL nested
 *                 pattern(!) only when num_subpats == 1, the matched
 *                 N_id is put back on the matching stack for further
 *                 inspection (see example c from above)
 * - PMarray   matches an N_array
 * - PMprf     matches an N_prf node
 *
 * Iteration Pattern:
 * ------------------
 *
 * Attributes:
 * -----------
 *
 *
 * Debugging:
 * ==========
 *
 * You may want to use -#d,PM to observe how your matches perform:-)
 *
 *
 * Some implementation issues:
 * ===========================
 *
 * The main question is how to represent patterns and
 * their attributes in a way that eases the matching
 * process. The central idea is to implement all
 * pattern as well as all attributes as partial function
 * applications, i.e., whenever a pattern function
 * PMxxx or an pattern attribute function PMAyyy is
 * applied to arguments a1, ..., an, we create a
 * record containing the arguments seen and a pointer
 * to the matching function. The matching function
 * then takes this record as well as an expression
 * to be matched against as parameters.
 * NB: using FP terminology, we create thunks here
 * which are evaluated as soon as the expression(s)
 * to be matched against become available.
 *
 * These records are implemented by the types
 *  pattern *, and attrib *, for pattern and their
 * attributes, respectively.
 * All matching functions, i.e., the functions that
 * do the real matching work have to be of types
 *
 * typedef node *matchFun( pattern*, node *)     and
 * typedef bool attribFun( attrib*, node *)   , respectively.
 *
 * both obtain their record containing the pattern arguments
 * specified at pattern construction time as first argument
 * and the expression to match against as second argument.
 * In both cases, the expression to be matched against can
 * in fact be an entire stack of expressions.
 * However, while the pattern variant consumes expressions
 * from the stack, the attribute version does not.
 * This is reflected in the return values:
 * the pattern variant returns the stack as well as
 * boolean value which is encoded in the return value
 * (for details see below), whereas the attrib variant only
 * returns the boolean value.
 *
 * Their are two main difficulties concerning the stack of
 * expressions to be matched:
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
#include "DupTree.h"
#include "constants.h"
#include "shape.h"
#include "compare_tree.h"

static char *FAIL = "";

static enum pm_mode { PM_exact, PM_flat, PM_flatPseudo } mode;

typedef node *matchFun (pattern *, node *);

struct PAT {
    nodetype nt;
    bool follow;
    int *i1;
    int *i2;
    int num_attr;
    attrib **attr;
    int num_pats;
    pattern **pats;
    matchFun *matcher;
};

#define PAT_NT(p) (p->nt)
#define PAT_NESTED(p) ((p->num_pats) != 0)
#define PAT_I1(p) (p->i1)
#define PAT_I2(p) (p->i2)
#define PAT_DOFOLLOW(p) (p->follow)
#define PAT_NA(p) (p->num_attr)
#define PAT_PATTRS(p) (p->attr)
#define PAT_NP(p) (p->num_pats)
#define PAT_PATS(p) (p->pats)
#define PAT_FUN(p) (p->matcher)

static pattern *
makePattern (nodetype nt, matchFun f)
{
    pattern *res;

    res = (pattern *)MEMmalloc (sizeof (pattern));

    PAT_NT (res) = nt;
    PAT_I1 (res) = NULL;
    PAT_I2 (res) = NULL;
    PAT_FUN (res) = f;
    PAT_DOFOLLOW (res) = TRUE;
    PAT_NA (res) = 0;
    PAT_PATTRS (res) = NULL;
    PAT_NP (res) = 0;
    PAT_PATS (res) = NULL;

    return (res);
}

/*******************************************************************************
 */

static pattern *
genericFillPattern (pattern *res, bool nested, int num_attribs, va_list arg_p)
{
    va_list arg_p_copy;
    int i;

    va_copy (arg_p_copy, arg_p);

    PAT_NA (res) = num_attribs;
    PAT_PATTRS (res) = (attrib **)MEMmalloc (num_attribs * sizeof (attrib *));

    for (i = 0; i < num_attribs; i++) {
        PAT_PATTRS (res)[i] = va_arg (arg_p_copy, attrib *);
    }
    if (nested) {
        PAT_NP (res) = va_arg (arg_p_copy, int);
        PAT_PATS (res) = (pattern **)MEMmalloc (PAT_NP (res) * sizeof (pattern *));
        for (i = 0; i < PAT_NP (res); i++) {
            PAT_PATS (res)[i] = va_arg (arg_p_copy, pattern *);
        }
    } else {
        PAT_NP (res) = 0;
    }
    va_end (arg_p_copy);

    return (res);
}

/*******************************************************************************
 */

static node *
getInner (node *arg_node)
{
    node *inner;

    switch (NODE_TYPE (arg_node)) {
    case N_array:
        inner = ARRAY_AELEMS (arg_node);
        break;
    case N_id:
        inner = arg_node; /* needed for PMvar */
        break;
    case N_prf:
        inner = PRF_ARGS (arg_node);
        break;
    default:
        inner = arg_node;
        DBUG_ASSERT (FALSE, "getInner applied to unexpected NODE_TYPE!");
        break;
    }

    return (inner);
}
/*******************************************************************************
 */

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

static node *
copyStack (node *stack)
{
    node *stack2;
    DBUG_ENTER ("copyStack");
    if ((stack != NULL) && (NODE_TYPE (stack) == N_set)) {
        stack2 = DUPdoDupTree (stack);
    } else {
        stack2 = stack;
    }
    DBUG_RETURN (stack2);
}

static node *
freeStack (node *stack)
{
    DBUG_ENTER ("freeStack");
    if ((stack != NULL) && (NODE_TYPE (stack) == N_set)) {
        stack = FREEdoFreeTree (stack);
    }
    DBUG_RETURN (stack);
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
    DBUG_PRINT ("PM", ("match failed!"));
    stack = freeStack (stack);

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

static node *
genericPatternMatcher (pattern *pat, node *stack)
{
    attrib *attr;
    pattern *inner_pat;
    node *inner_stack;
    node *arg;
    int i;

    DBUG_PRINT ("PM", ("trying to match %s:", global.mdb_nodetype[PAT_NT (pat)]));

    stack = ExtractOneArg (stack, &arg);
    if (PAT_DOFOLLOW (pat)) {
        arg = skipVarDefs (arg);
    }

    if ((arg != NULL) && (NODE_TYPE (arg) == PAT_NT (pat))) {

        for (i = 0; i < PAT_NA (pat); i++) {
            attr = PAT_PATTRS (pat)[i];
            if (!PMAmatch (attr, arg)) {
                stack = FailMatch (stack);
                i = PAT_NA (pat);
            }
        }

        if (PAT_NESTED (pat) && (stack != (node *)FAIL)) {
            inner_stack = getInner (arg);
            for (i = 0; i < PAT_NP (pat); i++) {
                inner_pat = PAT_PATS (pat)[i];
                inner_stack = PAT_FUN (inner_pat) (inner_pat, inner_stack);
                if (inner_stack == (node *)FAIL) {
                    i = PAT_NP (pat);
                }
            }
            stack = checkInnerMatchResult (inner_stack, stack);
        }

    } else {
        DBUG_PRINT ("PM", ("%s not found!", global.mdb_nodetype[PAT_NT (pat)]));
        stack = FailMatch (stack);
    }

    return (stack);
}

/** <!-- ****************************************************************** -->
 *
 * pattern *PMvar( int num_attribs, ... , num_pats, ...);
 *
 * @brief matches an IMMEDIATE identifyer (N_id)
 *        - up to one inner pattern
 *
 ******************************************************************************/
pattern *
PMvar (int num_attribs, ...)
{
    va_list ap;
    pattern *res;

    va_start (ap, num_attribs);
    res = genericFillPattern (makePattern (N_id, genericPatternMatcher), TRUE,
                              num_attribs, ap);
    va_end (ap);

    PAT_DOFOLLOW (res) = FALSE;

    return (res);
}

/** <!-- ****************************************************************** -->
 *
 * pattern *PMconst( int num_attribs, ...);
 *
 * @brief matches constant nodes...
 *        - no inner pattern
 *        - does depend on matching mode!
 *
 ******************************************************************************/
static node *
constMatcher (pattern *pat, node *stack)
{
    attrib *attr;
    node *arg;
    int i;

    DBUG_PRINT ("PM", ("trying to match a constant:"));

    stack = ExtractOneArg (stack, &arg);
    if (PAT_DOFOLLOW (pat)) {
        arg = skipVarDefs (arg);
    }

    if ((arg != NULL) && COisConstant (arg)) {
        DBUG_PRINT ("PM", ("matching constant "));
        for (i = 0; i < PAT_NA (pat); i++) {
            attr = PAT_PATTRS (pat)[i];
            if (!PMAmatch (attr, arg)) {
                stack = FailMatch (stack);
                i = PAT_NA (pat);
            }
        }
    } else {
        DBUG_PRINT ("PM", ("no constant found!"));
        stack = FailMatch (stack);
    }

    return (stack);
}

pattern *
PMconst (int num_attribs, ...)
{
    va_list ap;
    pattern *res;

    va_start (ap, num_attribs);
    res
      = genericFillPattern (makePattern (N_module, constMatcher), FALSE, num_attribs, ap);
    va_end (ap);

    return (res);
}

/** <!-- ****************************************************************** -->
 *
 * pattern *PMint( int num_attribs, ...)
 *
 * @brief matching N_num
 *        - no inner pattern
 *        - does depend on matching mode!
 *
 ******************************************************************************/
pattern *
PMint (int num_attribs, ...)
{
    va_list ap;
    pattern *res;

    va_start (ap, num_attribs);
    res = genericFillPattern (makePattern (N_num, genericPatternMatcher), FALSE,
                              num_attribs, ap);
    va_end (ap);

    return (res);
}

/** <!-- ****************************************************************** -->
 *
 * pattern *PMarray( int num_attribs, ..., int num_pats, ...)
 *
 * @brief array pattern:  [ sub_pat_1, ..., sub_pat_{num_pats}]
 *        - num_pats inner pattern
 *        - does depend on matching mode!
 *
 ******************************************************************************/
pattern *
PMarray (int num_attribs, ...)
{
    va_list ap;
    pattern *res;

    va_start (ap, num_attribs);
    res = genericFillPattern (makePattern (N_array, genericPatternMatcher), TRUE,
                              num_attribs, ap);
    va_end (ap);

    return (res);
}

/** <!-- ****************************************************************** -->
 *
 * pattern *PMprf( int num_attribs, ..., int num_pats, ...)
 *
 * @brief prf pattern:  prf( sub_pat_1, ..., sub_pat_{num_pats})
 *        - num_pats inner pattern
 *        - does depend on matching mode!
 *
 ******************************************************************************/
pattern *
PMprf (int num_attribs, ...)
{
    va_list ap;
    pattern *res;

    va_start (ap, num_attribs);
    res = genericFillPattern (makePattern (N_prf, genericPatternMatcher), TRUE,
                              num_attribs, ap);
    va_end (ap);

    return (res);
}

/** <!--*********************************************************************-->
 *
 * @fn pattern *PMretryAny( int *i, int *l, int num_pats, ...);
 *
 * @brief
 */
static node *
retryAnyMatcher (pattern *pat, node *stack)
{
    node *stack2;

    *PAT_I1 (pat) = 0;

    do {
        stack2 = copyStack (stack);
        stack2 = PAT_FUN (PAT_PATS (pat)[0]) (PAT_PATS (pat)[0], stack2);
    } while ((*PAT_I1 (pat) < *PAT_I2 (pat)) && (stack2 == (node *)FAIL));

    stack = freeStack (stack);

    return (stack2);
}

pattern *
PMretryAny (int *i, int *l, ...)
{
    va_list ap;
    pattern *res;

    va_start (ap, l);
    res = genericFillPattern (makePattern (N_module, retryAnyMatcher), TRUE, 0, ap);
    va_end (ap);

    PAT_I1 (res) = i;
    PAT_I2 (res) = l;

    return (res);
}

/** <!--*********************************************************************-->
 *
 * @fn pattern *PMretryAll( int *i, int *l, int num_pats, ...);
 *
 * @brief
 */
static node *
retryAllMatcher (pattern *pat, node *stack)
{
    node *stack2;

    *PAT_I1 (pat) = 0;

    do {
        stack2 = copyStack (stack);
        stack2 = PAT_FUN (PAT_PATS (pat)[0]) (PAT_PATS (pat)[0], stack2);
    } while ((*PAT_I1 (pat) < *PAT_I2 (pat)) && (stack2 != (node *)FAIL));

    stack = freeStack (stack);

    return (stack2);
}

pattern *
PMretryAll (int *i, int *l, ...)
{
    va_list ap;
    pattern *res;

    va_start (ap, l);
    res = genericFillPattern (makePattern (N_module, retryAllMatcher), TRUE, 0, ap);
    va_end (ap);

    PAT_I1 (res) = i;
    PAT_I2 (res) = l;

    return (res);
}

/** <!--*********************************************************************-->
 *
 * @fn pattern *PMskip(  int num_attribs, ...)
 *
 * @brief skipping pattern:   ...
 *        deletes stack and returnd NULL unless it contains FAIL which is
 *        propagated.
 */

static node *
skipMatcher (pattern *pat, node *stack)
{
    attrib *attr;
    node *match;
    int i;

    DBUG_PRINT ("PM", ("skipping remaining elements!"));

    stack = ExtractTopFrame (stack, &match);

    for (i = 0; i < PAT_NA (pat); i++) {
        attr = PAT_PATTRS (pat)[i];
        if (!PMAmatch (attr, match)) {
            stack = FailMatch (stack);
            i = PAT_NA (pat);
        }
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
PMskip (int num_attribs, ...)
{
    va_list ap;
    pattern *res;

    va_start (ap, num_attribs);
    res = genericFillPattern (makePattern (N_exprs, skipMatcher), FALSE, num_attribs, ap);
    va_end (ap);

    return (res);
}

/*******************************************************************************
 *
 */

pattern *
PMfree (pattern *p)
{
    int i;

    DBUG_ENTER ("PMfree");
    if (p != NULL) {
        for (i = 0; i < PAT_NA (p); i++) {
            PAT_PATTRS (p)[i] = PMAfree (PAT_PATTRS (p)[i]);
        }
        if (PAT_NA (p) > 0) {
            PAT_PATTRS (p) = (attrib **)MEMfree (PAT_PATTRS (p));
        }
        for (i = 0; i < PAT_NP (p); i++) {
            PAT_PATS (p)[i] = PMfree (PAT_PATS (p)[i]);
        }
        if (PAT_NP (p) > 0) {
            PAT_PATS (p) = (pattern **)MEMfree (PAT_PATS (p));
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

    return (PAT_FUN (pat) (pat, expr) != (node *)FAIL);
}

int
PMmatchFlat (pattern *pat, node *expr)
{
    mode = PM_flat;

    return (PAT_FUN (pat) (pat, expr) != (node *)FAIL);
}

int
PMmatchFlatPseudo (pattern *pat, node *expr)
{
    mode = PM_flatPseudo;

    return (PAT_FUN (pat) (pat, expr) != (node *)FAIL);
}
