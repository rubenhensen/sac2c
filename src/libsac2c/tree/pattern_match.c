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
 * Currently (2009-07-19), we have:
 * - PMmatchExact( pat, expr) which does not follow any definitions at all.
 *                            Using this matching function in the second
 *                            example above would NOT match!
 * - PMmatchFlat( pat, expr)  is the standard matcher. It follows variable
 *                            definitions and simple variable assignments.
 * - PMmatch...
 *
 * Note here, that the chasing of N_id nodes to their definitions happens
 * implicitly for all patterns. The only exception is PMvar which ALWAYS
 * matches an N_id node directly. However, PMvar enables further inspection
 * of its definition by an optional sub-pattern. For more details, see below.
 *
 * Creating Patterns:
 * -----------------
 *
 * We have three categories of pattern:
 * - singleton patterns that match expressions without subexpressions,
 *   such as N_num.
 * - nested pattern that match constructor-expressions containing
 *   subexpressions ,such as N_prf, N_ap, or N_array.
 * - iteration pattern that enable repeated matching with back-tracking
 *
 * All pattern constructing functions of the first 2 categories adhere to
 * the following structure:
 *
 * pattern *PM<xyz>( int num_attribs, attr_1, ..., attr_{num_attribs},
 *                   int num_subpats, pat_1, ..., pat_{num_subpats} )
 *
 * However, singleton patterns do not expect any of the arguments in the
 * second line.
 *
 * Each pattern can come with an abitrary number of attributes which
 * refine their matching behaviour. For details on attributes see
 * below the section on "Attributes".
 * All nested patterns come with a list of subpatterns which need to match
 * all subexpressions of the matched node. Note here that we have patterns
 * that match more than one expression, so there is no need to have a
 * one-to-one correspondence between subpatterns and subexpressions!
 *
 * At the time being (2009-07-18) we support the following patterns:
 *
 * SINGLETON PATTERN:
 * - PMconst   matches any constant expression (could be N_num, N_array, etc)
 * - PMint     matches an N_num
 * - PMskip    matches ALL expressions left to be matched ( '...'-pattern)
 * - PMskipN   matches n expressions
 * - PMany     matches any node      (BUT follows N_id nodes!)
 * - PMparam   matches an N_id       (BUT follows N_id nodes!)
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
 * The main question is how to represent patterns and their attributes in a
 * way that eases the matching process. The central idea is to implement all
 * pattern as well as all attributes as partial function applications, i.e.,
 * whenever a pattern function PMxxx or an pattern attribute function PMAyyy
 * is applied to arguments a1, ..., an, we create a record containing the
 * arguments seen and a pointer to the matching function. The matching
 * function then takes this record as well as an expression to be matched
 * against as parameters.
 * NB: using FP terminology, we create thunks here which are evaluated as
 * soon as the expression(s) to be matched against become available.
 *
 * These records are implemented by the types    pattern *, and attrib *,
 * for pattern and their attributes, respectively.
 * All matching functions, i.e., the functions that do the real matching
 * work have to be of types
 *
 * typedef node *matchFun( pattern*, node *)     and
 * typedef bool attribFun( attrib*, node *)   , respectively.
 *
 * both obtain their record containing the pattern arguments specified at
 * pattern construction time as first argument and the expression to match
 * against as second argument. In both cases, the expression to be matched
 * against can in fact be an entire stack of expressions. However, while
 * the pattern variant consumes expressions from the stack, the attribute
 * version does not. This is reflected in the return values:
 * the pattern variant returns the stack as well as boolean value which is
 * encoded in the return value (for details see below), whereas the attrib
 * variant only returns the boolean value.
 *
 * Their are two main difficulties concerning the stack of expressions to
 * be matched:
 * a) how to maintain a stack of expressions WITHOUT fiddling
 *    around with the existing exprs chains in the AST?
 * b) how to return a modified stack AND a boolean matching flag
 *    without using a global variable and an "out-argument"?
 *
 * We solve a) by introducing a spine of N_set-nodes whenever more than one
 * expr or exprs chain is needed in the stack. This is done dynamically by
 * inserting these whenever we hit a constructor-match (pushArgs) and by
 * freeing these when pulling out the last topmost expression of an N_set
 * node (extractOneArg). If the match goes through we expect all exprs to be
 * consumed and, therefore, all N_set nodes to be freed. In case the match
 * fails, failMatch( stack)  is called which explicitly frees any remaining
 * N_set nodes.
 *
 * b) is solved by creating a special static "node" FAIL which we return
 * instead of a proper stack whenever a match failed. All matching functions
 * therefore need to check agains a FAIL as stack and they need to pass-on
 * that FAIL.
 */

/** <!--*********************************************************************-->
 *
 * includes, typedefs, and static variables:
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
#include "LookUpTable.h"

static char *FAIL = "";
static int matching_level;

static pm_mode_t mode;

static lut_t *follow_lut = NULL;

typedef node *matchFun (pattern *, node *);
typedef bool prfMatchFun (prf);

/** <!--*********************************************************************-->
 *
 * basic pattern handling functionality:
 */

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
    pattern **pattern_link;
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
#define PAT_PAT_LINK(p) (p->pattern_link)

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
    PAT_PAT_LINK (res) = NULL;

    return (res);
}

/** <!--*******************************************************************-->
 *
 * @fn pattern *genericFillPattern( pattern *res, bool nested,
 *                                  int num_attribs, va_list arg_p)
 *
 * @brief fills the given pattern with the attribs and (if nested==TRUE)
 *        the subsequent subpattern contained in va_list.
 * @return the filled pattern
 *
 *****************************************************************************/
static pattern *
genericFillPattern (pattern *res, bool nested, int num_attribs, va_list arg_p)
{
    va_list arg_p_copy;
    int i;

    DBUG_ENTER ("genericFillPattern");
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

    DBUG_RETURN (res);
}

/** <!--*******************************************************************-->
 *
 * @fn pattern *genericFillPatternNoAttribs( pattern *res,
 *                                           int num_pats, va_list arg_p)
 *
 * @brief fills the given pattern with the subpattern contained in va_list.
 * @return the filled pattern
 *
 *****************************************************************************/
static pattern *
genericFillPatternNoAttribs (pattern *res, int num_pats, va_list arg_p)
{
    va_list arg_p_copy;
    int i;

    DBUG_ENTER ("genericFillPatternNoAttribs");
    va_copy (arg_p_copy, arg_p);

    PAT_NA (res) = 0;
    PAT_NP (res) = num_pats;
    PAT_PATS (res) = (pattern **)MEMmalloc (PAT_NP (res) * sizeof (pattern *));
    for (i = 0; i < PAT_NP (res); i++) {
        PAT_PATS (res)[i] = va_arg (arg_p_copy, pattern *);
    }

    va_end (arg_p_copy);

    DBUG_RETURN (res);
}

/** <!--*********************************************************************-->
 *
 * stack handling functionality:
 */

/** <!--*******************************************************************-->
 *
 * @fn node *copyStack( node *stack)
 *
 * @brief creates a copy of the "spine" of the stack
 * @param stack
 * @return copied stack
 *****************************************************************************/
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

/** <!--*******************************************************************-->
 *
 * @fn node *freeStack( node *stack)
 *
 * @brief frees the "spine" of the stack
 * @param stack
 * @return NULL pointer
 *****************************************************************************/
static node *
freeStack (node *stack)
{
    DBUG_ENTER ("freeStack");
    if ((stack != NULL) && (NODE_TYPE (stack) == N_set)) {
        stack = FREEdoFreeTree (stack);
    } else {
        stack = NULL;
    }
    DBUG_RETURN (stack);
}

/** <!--*******************************************************************-->
 *
 * @fn node *failMatch( node *stack)
 *
 * @brief cleans up the remaining stack and creates a FAIL node
 * @param stack: stack of exprs
 * @return FAIL node
 *****************************************************************************/
static node *
failMatch (node *stack)
{
    DBUG_ENTER ("failMatch");
    DBUG_PRINT ("PM", (PMINDENT "match failed!"));
    stack = freeStack (stack);

    DBUG_RETURN ((node *)FAIL);
}

/** <!--*******************************************************************-->
 *
 * @fn node *extractOneArg( node *stack, node **arg)
 *
 * @brief extracts the first argument from the exprs stack.
 *        Note here, that stack can either be N_set (stack), an N_exprs or
 *        any other expression! N_set nodes that become redundant
 *        are being freed!
 * @param stack: stack of exprs
 * @return via arg the first expression in the chain and the rest of the
 *         stack via the normal return value.
 *****************************************************************************/
#define REF_SET(n, val)                                                                  \
    {                                                                                    \
        if (n != NULL)                                                                   \
            *n = val;                                                                    \
    }

static node *
extractOneArg (node *stack, node **arg)
{
    node *next;

    DBUG_ENTER ("extractOneArg");

    if (stack != NULL) {
        if (NODE_TYPE (stack) == N_set) {
            next = extractOneArg (SET_MEMBER (stack), arg);
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
            DBUG_PRINT ("PM", (PMINDENT "argument found:"));
            DBUG_EXECUTE ("PM", PRTdoPrintNodeFile (stderr, *arg););
        }
    } else {
        *arg = NULL;
        DBUG_PRINT ("PM", (PMINDENT "trying to match against empty stack"));
        stack = failMatch (stack);
    }
    DBUG_RETURN (stack);
}

/** <!--*******************************************************************-->
 *
 * @fn node *extractTopFrame( node *stack, node **top)
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
extractTopFrame (node *stack, node **top)
{
    DBUG_ENTER ("extractTopFrame");

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
        DBUG_PRINT ("PM", (PMINDENT "frame found:"));
        DBUG_EXECUTE ("PM", PRTdoPrintNodeFile (stderr, *top););
    }
#endif

    DBUG_RETURN (stack);
}

/** <!--*******************************************************************-->
 *
 * @fn node *pushArgs( node *stack, node * args)
 *
 * @brief if stack is non-empty, it pushes the args on top of the existing
 *        args in stack by means of N_set nodes. Note here, that stack
 *        can either be N_set (existing stack) N_exprs or any other expression!
 * @param stack: stack of exprs
 *        args: exprs to be stacked on top
 * @return stacked exprs
 *****************************************************************************/
static node *
pushArgs (node *stack, node *args)
{
    DBUG_ENTER ("pushArgs");
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
 * @fn node *range2Set( node * range)
 *
 * @brief convert a list of ranges into a list of sets
 * @return chain of sets
 *****************************************************************************/
static node *
range2Set (node *range)
{
    node *set = NULL;
    DBUG_ENTER ("range2Set");

    if (RANGE_NEXT (range) != NULL) {
        set = range2Set (RANGE_NEXT (range));
    }

    set = TBmakeSet (range, set);

    DBUG_RETURN (set);
}

/** <!--*********************************************************************-->
 *
 * local helper functions:
 */

/** <!--*******************************************************************-->
 *
 * @fn static bool isInExtrema( node *expr)
 *
 * @brief Predicate for determining that an N_prf is an extrema note.
 *
 * @param N_prf
 * @return True if N_prf is member of the set below.
 *
 *****************************************************************************/
static bool
isInExtrema (prf prfun)
{
    DBUG_ENTER ("isInExtrema");
    DBUG_RETURN ((prfun == F_noteminval) || (prfun == F_notemaxval)
                 || (prfun == F_noteintersect));
}

/** <!--*******************************************************************-->
 *
 * @fn static bool isInGuards( node *expr)
 *
 * @brief Predicate for determining that an N_prf is a guard
 *
 * @param N_prf
 * @return True if N_prf is member of the set below.
 *
 *****************************************************************************/
static bool
isInGuards (prf prfun)
{
    DBUG_ENTER ("isInGuards");
    DBUG_RETURN ((prfun == F_guard) || (prfun == F_afterguard)
                 || (prfun == F_type_constraint) || (prfun == F_same_shape_AxA)
                 || (prfun == F_shape_matches_dim_VxA) || (prfun == F_non_neg_val_V)
                 || (prfun == F_val_lt_shape_VxA) || (prfun == F_val_le_val_VxV)
                 || (prfun == F_prod_matches_prod_shape_VxA));
}

/** <!--*******************************************************************-->
 *
 * @fn static bool isAfterguard( node *expr)
 *
 * @brief Predicate for determining that an N_prf is a guard
 *
 * @param N_prf
 * @return True if N_prf is member of the set below.
 *
 *****************************************************************************/
static bool
isAfterguard (prf prfun)
{
    DBUG_ENTER ("isInGuards");
    DBUG_RETURN ((prfun == F_afterguard));
}

/** <!--*******************************************************************-->
 *
 * @fn node *getInner( node *arg_node)
 *
 * @brief returns pointer to expr / N_exprs chain that constitutes a
 *        "subexpression" of the given arg_node
 * @return pointer to expr / N_exprs node
 *
 *****************************************************************************/
static node *
getInner (node *arg_node)
{
    node *inner;

    DBUG_ENTER ("getInner");
    switch (NODE_TYPE (arg_node)) {
    case N_array:
        inner = ARRAY_AELEMS (arg_node);
        break;
    case N_id:
    case N_with:
    case N_with2:
    case N_with3:
        inner = arg_node; /* needed for PMvar and selecting sub chains*/
        break;
    case N_range:
        inner = RANGE_RESULTS (arg_node);
    case N_prf:
        inner = PRF_ARGS (arg_node);
        break;
    default:
        inner = arg_node;
        DBUG_ASSERT (FALSE, "getInner applied to unexpected NODE_TYPE!");
        break;
    }

    DBUG_RETURN (inner);
}

/** <!--*******************************************************************-->
 *
 * @fn node *followId( node *expr)
 *
 * @brief follows Variables to their definitions using SSAASSIGN and the
 *        (potentially empty) LUT
 * @param starting expression
 * @return RHS of definition for the given Id node or the node itself.
 *****************************************************************************/
static node *
followId (node *expr)
{
    node *new_id;
    DBUG_ENTER ("followId");

    if (NODE_TYPE (expr) == N_id) {
        if (AVIS_SSAASSIGN (ID_AVIS (expr)) != NULL) {
            expr = LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (expr))));
        } else if (follow_lut != NULL) {
            new_id = (node *)LUTsearchInLutP (follow_lut, ID_AVIS (expr));
            expr = (new_id != NULL ? new_id : expr);
        }
    }
    DBUG_RETURN (expr);
}

/** <!--*******************************************************************-->
 *
 * @fn node *followPrf( prfMatchFun prfInspectFun, node *expr)
 *
 * @brief follows first arg of given prf if the prfInspectFun yields TRUE.
 * @param starting expression
 * @return first arg of given prf node if in expected set;
 *         otherwise the expr itself.
 *****************************************************************************/
static node *
followPrf (prfMatchFun prfInspectFun, node *expr)
{
    DBUG_ENTER ("followPrf");

    if ((NODE_TYPE (expr) == N_prf) && (prfInspectFun (PRF_PRF (expr)))) {
        expr = PRF_ARG1 (expr);
    }
    DBUG_RETURN (expr);
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
    node *old_expr;

    if (expr != NULL) {
        switch (mode) {
        case PM_exact:
            break;
        case PM_flat:
            do {
                old_expr = expr;
                expr = followId (old_expr);
            } while (expr != old_expr);
            break;

        case PM_flatSkipExtrema:
            do {
                old_expr = expr;
                expr = followId (old_expr);
                expr = followPrf (isInExtrema, expr);
            } while (expr != old_expr);
            break;

        case PM_flatSkipGuards:
            do {
                old_expr = expr;
                expr = followId (old_expr);
                expr = followPrf (isInGuards, expr);
            } while (expr != old_expr);
            break;

        case PM_flatWith:
            do {
                old_expr = expr;
                expr = followId (old_expr);
                expr = followPrf (isAfterguard, expr);
            } while (expr != old_expr);
            break;
        }
    }

    DBUG_RETURN (expr);
}

/** <!--*********************************************************************-->
 *
 * local generic matching functions:
 */
/** <!--*******************************************************************-->
 *
 * @fn node *genericAtribMatcher( pattern *pat, node *arg, node *stack)
 *
 * @brief matches all attribs contained in pat against the node arg.
 *        in case of failure, the stack is changed accordingly
 * @return potentially modified stack
 *****************************************************************************/
static node *
genericAtribMatcher (pattern *pat, node *arg, node *stack)
{
    int i;
    attrib *attr;
    DBUG_ENTER ("genericAtribMatcher");

    DBUG_PRINT ("PM", (PMINDENT "checking attributes"));
    for (i = 0; i < PAT_NA (pat); i++) {
        attr = PAT_PATTRS (pat)[i];
        if (!PMAmatch (attr, arg)) {
            stack = failMatch (stack);
            i = PAT_NA (pat);
        }
    }
    DBUG_RETURN (stack);
}

/** <!--*******************************************************************-->
 *
 * @fn bool genericSubPatternMatcher( pattern *pat, node *inner_stack)
 *
 * @brief matches all subpattern contained in pat against the nodes on stack.
 *        stack is freed in this process.
 * @return either NULL or FAIL
 *****************************************************************************/
static node *
genericSubPatternMatcher (pattern *pat, node *inner_stack)
{
    pattern *inner_pat;
    int i;
    DBUG_ENTER ("genericSubPatternMatcher");

    if (PAT_NESTED (pat)) {
        DBUG_PRINT ("PM", (PMINDENT "checking inner pattern"));
        matching_level++;
        for (i = 0; i < PAT_NP (pat); i++) {
            inner_pat = PAT_PATS (pat)[i];
            inner_stack = PAT_FUN (inner_pat) (inner_pat, inner_stack);
            if (inner_stack == (node *)FAIL) {
                i = PAT_NP (pat);
            }
        }
        matching_level--;
        if (inner_stack != NULL) {
            DBUG_PRINT ("PM", (PMINDENT "inner match %s",
                               (inner_stack == (node *)FAIL ? "failed"
                                                            : "left unmatched item(s)")));
            inner_stack = freeStack (inner_stack);
            inner_stack = (node *)FAIL;
        }
    }
    DBUG_RETURN (inner_stack);
}

/** <!--*******************************************************************-->
 *
 * @fn node *genericPatternMatcher( pattern *pat, node *stack)
 *
 * @brief matches the top entry of stack against the node specified in pat;
 *        Upon success, it checks attribs and subpattern against sub expressions
 *        as well.
 * @return potentially modified stack
 *****************************************************************************/
static node *
genericPatternMatcher (pattern *pat, node *stack)
{
    node *arg;

    DBUG_ENTER ("genericPatternMatcher");
    DBUG_PRINT ("PM", (PMSTART "matching %s:", matching_level,
                       global.mdb_nodetype[PAT_NT (pat)]));

    stack = extractOneArg (stack, &arg);
    if (PAT_DOFOLLOW (pat)) {
        arg = skipVarDefs (arg);
    }

    if ((arg != NULL) && (NODE_TYPE (arg) == PAT_NT (pat))) {

        DBUG_PRINT ("PM",
                    (PMINDENT "found %s %s%s%s", global.mdb_nodetype[NODE_TYPE (arg)],
                     (NODE_TYPE (arg) == N_id ? "\"" : ""),
                     (NODE_TYPE (arg) == N_id ? ID_NAME (arg) : ""),
                     (NODE_TYPE (arg) == N_id ? "\"" : "")));

        stack = genericAtribMatcher (pat, arg, stack);
        if ((stack != (node *)FAIL) && PAT_NESTED (pat)
            && (genericSubPatternMatcher (pat, getInner (arg)) == (node *)FAIL)) {
            stack = failMatch (stack);
        }

    } else {
        DBUG_PRINT ("PM", (PMINDENT "%s not found!", global.mdb_nodetype[PAT_NT (pat)]));
        stack = failMatch (stack);
    }
    DBUG_PRINT ("PM", (PMEND, matching_level));

    DBUG_RETURN (stack);
}

/** <!--*********************************************************************-->
 *
 * external PM functions:
 */

/** <!-- ****************************************************************** -->
 *
 * pattern *PMmulti( int num_pats, ...)
 *
 * @brief matches num_pats inner pattern
 *
 ******************************************************************************/
static node *
multiMatcher (pattern *pat, node *stack)
{
    DBUG_PRINT ("PM", (PMSTART "multi match:", matching_level));

    stack = genericSubPatternMatcher (pat, stack);

    DBUG_PRINT ("PM", (PMEND, matching_level));

    return (stack);
}

pattern *
PMmulti (int num_pats, ...)
{
    va_list ap;
    pattern *res;

    va_start (ap, num_pats);
    res
      = genericFillPatternNoAttribs (makePattern (N_module, multiMatcher), num_pats, ap);
    va_end (ap);

    return (res);
}
/** <!-- ****************************************************************** -->
 *
 * pattern *PMHlink( 0, 1, pattern **pat)
 *
 * @brief Provides a link out of the current pattern to a new pattern.
 *        When freeing does not free linked pattern
 *        Pointer to a pointer so you can pass a pattern before it is defined
 *
 ******************************************************************************/
static node *
linkMatcher (pattern *pat, node *stack)
{
    DBUG_PRINT ("PM", (PMSTART "link match:", matching_level));

    stack = genericPatternMatcher (*PAT_PAT_LINK (pat), stack);

    DBUG_PRINT ("PM", (PMEND, matching_level));

    return (stack);
}

pattern *
PMlink (int num_pats, ...)
{
    va_list ap;
    pattern *res;
    int pats;

    res = makePattern (N_module, linkMatcher);

    va_start (ap, num_pats);

    DBUG_ASSERT (num_pats == 0, "PMlink takes no args");
    pats = va_arg (ap, int);
    DBUG_ASSERT (pats == 1, "PMlink takes exactly 1 sub pattern");

    PAT_PAT_LINK (res) = va_arg (ap, pattern **);

    va_end (ap);

    return (res);
}

/** <!-- ****************************************************************** -->
 *
 * pattern *PMvar( int num_attribs, ... , num_pats, ...);
 *
 * @brief matches an IMMEDIATE identifier (N_id)
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
 * pattern *PMparam( int num_attribs, ... );
 *
 * @brief matches an identifyer (N_id) that has no SSA_ASSIGN and, thus,
 *        relates to either an N_arg or to a WL-index-variable!
 *        - no inner pattern
 *        - does depend on matching mode!
 *
 ******************************************************************************/
pattern *
PMparam (int num_attribs, ...)
{
    va_list ap;
    pattern *res;

    va_start (ap, num_attribs);
    res = genericFillPattern (makePattern (N_id, genericPatternMatcher), FALSE,
                              num_attribs, ap);
    va_end (ap);

    return (res);
}

/** <!-- ****************************************************************** -->
 *
 * pattern *PMany( int num_attribs, ... );
 *
 * @brief matches any node ...
 *        - no inner pattern
 *        - does depend on matching mode!
 *
 ******************************************************************************/
static node *
anyMatcher (pattern *pat, node *stack)
{
    node *arg;

    DBUG_ENTER ("anyMatcher");
    DBUG_PRINT ("PM", (PMSTART "matching any node:", matching_level));

    stack = extractOneArg (stack, &arg);
    arg = skipVarDefs (arg);

    if (arg != NULL) {

        DBUG_PRINT ("PM",
                    (PMINDENT "found %s %s%s%s", global.mdb_nodetype[NODE_TYPE (arg)],
                     (NODE_TYPE (arg) == N_id ? "\"" : ""),
                     (NODE_TYPE (arg) == N_id ? ID_NAME (arg) : ""),
                     (NODE_TYPE (arg) == N_id ? "\"" : "")));

        stack = genericAtribMatcher (pat, arg, stack);

    } else {
        DBUG_PRINT ("PM", (PMINDENT "nothing found!"));
        stack = failMatch (stack);
    }
    DBUG_PRINT ("PM", (PMEND, matching_level));

    DBUG_RETURN (stack);
}

pattern *
PMany (int num_attribs, ...)
{
    va_list ap;
    pattern *res;

    va_start (ap, num_attribs);
    res = genericFillPattern (makePattern (N_module, anyMatcher), FALSE, num_attribs, ap);
    va_end (ap);

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
    node *arg;
#ifndef DBUG_OFF
    constant *c;
    char *tmp_str;
#endif

    DBUG_PRINT ("PM", (PMSTART "matching a constant:", matching_level));

    stack = extractOneArg (stack, &arg);
    if (PAT_DOFOLLOW (pat)) {
        arg = skipVarDefs (arg);
    }

    if ((arg != NULL) && COisConstant (arg)) {
        DBUG_EXECUTE ("PM", c = COaST2Constant (arg); tmp_str = COconstant2String (c););
        DBUG_PRINT ("PM", (PMINDENT "constant %s found!", tmp_str));
        DBUG_EXECUTE ("PM", tmp_str = MEMfree (tmp_str); c = COfreeConstant (c););
        stack = genericAtribMatcher (pat, arg, stack);
    } else {
        DBUG_PRINT ("PM", (PMINDENT "no constant found!"));
        stack = failMatch (stack);
    }
    DBUG_PRINT ("PM", (PMEND, matching_level));

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

/** <!-- ****************************************************************** -->
 *
 * pattern *PMwith( int num_attribs, ..., int num_pats, ...)
 *
 * @brief
 *
 *****************************************************************************/
pattern *
PMwith (int num_attribs, ...)
{
    va_list ap;
    pattern *res;

    va_start (ap, num_attribs);
    res = genericFillPattern (makePattern (N_with, genericPatternMatcher), TRUE,
                              num_attribs, ap);
    va_end (ap);

    return (res);
}
/** <!-- ****************************************************************** -->
 *
 * pattern *PMwith3( int num_attribs, ..., 0)
 *
 * @brief
 *
 *****************************************************************************/
pattern *
PMwith3 (int num_attribs, ...)
{
    va_list ap;
    pattern *res;

    va_start (ap, num_attribs);
    res = genericFillPattern (makePattern (N_with3, genericPatternMatcher), TRUE,
                              num_attribs, ap);
    va_end (ap);

    return (res);
}
/** <!-- ****************************************************************** -->
 *
 * pattern *PMrange( int num_attribs, ...,)
 *
 * @brief
 *
 *****************************************************************************/
pattern *
PMrange (int num_attribs, ...)
{
    va_list ap;
    pattern *res;

    va_start (ap, num_attribs);
    res = genericFillPattern (makePattern (N_range, genericPatternMatcher), TRUE,
                              num_attribs, ap);
    va_end (ap);

    return (res);
}
/** <!-- ****************************************************************** -->
 *
 * pattern *PMSrange( int num_attribs, ...)
 *
 * @brief range of current with3
 *
 *****************************************************************************/
static node *
rangeSelector (pattern *pat, node *stack)
{
    node *arg;

    DBUG_ENTER ("rangeSelector");
    DBUG_PRINT ("PM", (PMSTART "matching range"));

    stack = extractOneArg (stack, &arg);

    if (NODE_TYPE (arg) == N_with3) {

        if ((genericSubPatternMatcher (pat, range2Set (WITH3_RANGES (arg)))
             == (node *)FAIL)) {
            stack = failMatch (stack);
        }

    } else {
        DBUG_PRINT ("PM", (PMINDENT "No with3 => no range"));
        stack = failMatch (stack);
    }

    DBUG_PRINT ("PM", (PMEND, matching_level));
    DBUG_RETURN (stack);
}

pattern *
PMSrange (int num_attribs, ...)
{
    va_list ap;
    pattern *res;

    va_start (ap, num_attribs);
    res
      = genericFillPattern (makePattern (N_with3, rangeSelector), TRUE, num_attribs, ap);
    va_end (ap);

    return (res);
}

/** <!--*********************************************************************-->
 *
 * @fn pattern *PMretryAny( int *i, int *l, int num_pats, ...);
 *
 * @brief
 ******************************************************************************/
static node *
retryAnyMatcher (pattern *pat, node *stack)
{
    *PAT_I1 (pat) = 0;
    bool match;

    DBUG_PRINT ("PM", (PMSTART "retry any matcher start", matching_level));
    do {
        DBUG_PRINT ("PM", (PMINDENT "trying i = %d:", *PAT_I1 (pat)));
        match = (genericSubPatternMatcher (pat, copyStack (stack)) == NULL);
        *PAT_I1 (pat) = *PAT_I1 (pat) + 1;
    } while ((*PAT_I1 (pat) < *PAT_I2 (pat)) && (!match));

    if (!match) {
        stack = failMatch (stack);
    } else {
        DBUG_PRINT ("PM", (PMINDENT "success with i = %d!", (*PAT_I1 (pat)) - 1));
        stack = freeStack (stack);
    }

    DBUG_PRINT ("PM", (PMEND, matching_level));

    return (stack);
}

pattern *
PMretryAny (int *i, int *l, int num_pats, ...)
{
    va_list ap;
    pattern *res;

    va_start (ap, num_pats);
    res = genericFillPatternNoAttribs (makePattern (N_module, retryAnyMatcher), num_pats,
                                       ap);
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
 ******************************************************************************/
static node *
retryAllMatcher (pattern *pat, node *stack)
{
    *PAT_I1 (pat) = 0;
    bool match;

    DBUG_PRINT ("PM", (PMSTART "retry all matcher start", matching_level));
    do {
        DBUG_PRINT ("PM", (PMINDENT "trying i = %d:", *PAT_I1 (pat)));
        match = (genericSubPatternMatcher (pat, copyStack (stack)) == NULL);
        *PAT_I1 (pat) = *PAT_I1 (pat) + 1;
    } while ((*PAT_I1 (pat) < *PAT_I2 (pat)) && match);

    if (!match) {
        stack = failMatch (stack);
    } else {
        DBUG_PRINT ("PM",
                    (PMINDENT "success for all i in {o, ..., %d)!", (*PAT_I2 (pat)) - 1));
        stack = freeStack (stack);
    }

    DBUG_PRINT ("PM", (PMEND, matching_level));

    return (stack);
}

pattern *
PMretryAll (int *i, int *l, int num_pats, ...)
{
    va_list ap;
    pattern *res;

    va_start (ap, num_pats);
    res = genericFillPatternNoAttribs (makePattern (N_module, retryAllMatcher), num_pats,
                                       ap);
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
    node *match;

    DBUG_PRINT ("PM", (PMSTART "skipping remaining elements!", matching_level));

    stack = extractTopFrame (stack, &match);

    stack = genericAtribMatcher (pat, match, stack);

    if (stack != (node *)FAIL) {
        stack = freeStack (stack);
    }

    DBUG_PRINT ("PM", (PMEND, matching_level));
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

/** <!--*********************************************************************-->
 *
 * @fn pattern *PMskipN(  int * n, int num_attribs, ...)
 *
 * @brief skipping pattern:   ...
 *        consumes n entries from the stack
 */

static node *
skipNMatcher (pattern *pat, node *stack)
{
    node *arg;
    int i;

    DBUG_PRINT ("PM", (PMSTART "skipping %d elements!", matching_level, *PAT_I1 (pat)));

    stack = genericAtribMatcher (pat, stack, stack);

    if (stack != (node *)FAIL) {
        for (i = 0; i < *PAT_I1 (pat); i++) {
            stack = extractOneArg (stack, &arg);
            DBUG_PRINT ("PM", (PMINDENT "deleting that argument"));
        }
    }

    DBUG_PRINT ("PM", (PMEND, matching_level));
    return (stack);
}

pattern *
PMskipN (int *n, int num_attribs, ...)
{
    va_list ap;
    pattern *res;

    va_start (ap, num_attribs);
    res
      = genericFillPattern (makePattern (N_exprs, skipNMatcher), FALSE, num_attribs, ap);
    va_end (ap);
    PAT_I1 (res) = n;

    return (res);
}

/** <!--*********************************************************************-->
 *
 * @fn pattern *PMfree( pattern *p)
 *
 * @brief frees the given pattern
 * @return NULL pointer
 *****************************************************************************/
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

/** <!--*********************************************************************-->
 *
 * the matching functions:
 */

/** <!--*********************************************************************-->
 *
 * @fn bool PMmatch( pattern *pat, pm_mode_t pm_mode, lut_t *f_lut, node *expr)
 *
 * @brief matches pat against expr in the specified mode using the giving
 *        lut when following N_oid nodes (can be NULL).
 * @return success
 *
 *****************************************************************************/
bool
PMmatch (pattern *pat, pm_mode_t pm_mode, lut_t *f_lut, node *expr)
{
    mode = pm_mode;
    follow_lut = f_lut;
    matching_level = 0;
    bool res;
    DBUG_ENTER ("PMmatch");

    DBUG_PRINT ("PM", ("starting match in mode %d", pm_mode));
    res = (PAT_FUN (pat) (pat, expr) != (node *)FAIL);
    DBUG_PRINT ("PM", ("match %s!", (res ? "succeeded" : "failed")));

    DBUG_RETURN (res);
}

/** <!--*********************************************************************-->
 *
 * @fn bool PMmatchExact( pattern *pat, node *expr)
 *
 * @brief matches pat against expr in mode PM_exact
 * @return success
 *
 *****************************************************************************/
bool
PMmatchExact (pattern *pat, node *expr)
{
    mode = PM_exact;
    follow_lut = NULL;

    matching_level = 0;
    bool res;
    DBUG_ENTER ("PMmatchExact");

    DBUG_PRINT ("PM", ("starting exact match"));
    res = (PAT_FUN (pat) (pat, expr) != (node *)FAIL);
    DBUG_PRINT ("PM", ("exact match %s!", (res ? "succeeded" : "failed")));

    DBUG_RETURN (res);
}

/** <!--*********************************************************************-->
 *
 * @fn bool PMmatchFlat( pattern *pat, node *expr)
 *
 * @brief matches pat against expr in mode PM_flat
 * @return success
 *
 *****************************************************************************/
bool
PMmatchFlat (pattern *pat, node *expr)
{
    mode = PM_flat;
    follow_lut = NULL;

    matching_level = 0;
    bool res;

    DBUG_PRINT ("PM", ("starting flat match"));
    res = (PAT_FUN (pat) (pat, expr) != (node *)FAIL);
    DBUG_PRINT ("PM", ("flat match %s!", (res ? "succeeded" : "failed")));

    return (res);
}

/** <!--*********************************************************************-->
 *
 * @fn bool PMmatchFlatSkipExtrema( pattern *pat, node *expr)
 *
 * @brief matches pat against expr in mode PM_flatSkipExtrema
 * @return success
 *
 *****************************************************************************/
bool
PMmatchFlatSkipExtrema (pattern *pat, node *expr)
{
    mode = PM_flatSkipExtrema;
    follow_lut = NULL;

    matching_level = 0;
    bool res;

    DBUG_PRINT ("PM", ("starting flatSkipExtrema match"));
    res = (PAT_FUN (pat) (pat, expr) != (node *)FAIL);
    DBUG_PRINT ("PM", ("flatSkipExtrema match %s!", (res ? "succeeded" : "failed")));

    return (res);
}

/** <!--*********************************************************************-->
 *
 * @fn bool PMmatchFlatSkipGuards( pattern *pat, node *expr)
 *
 * @brief matches pat against expr in mode PM_flatSkipGuards
 * @return success
 *
 *****************************************************************************/
bool
PMmatchFlatSkipGuards (pattern *pat, node *expr)
{
    mode = PM_flatSkipGuards;
    follow_lut = NULL;

    matching_level = 0;
    bool res;

    DBUG_PRINT ("PM", ("starting flatSkipGuards match"));
    res = (PAT_FUN (pat) (pat, expr) != (node *)FAIL);
    DBUG_PRINT ("PM", ("flatSkipGuards match %s!", (res ? "succeeded" : "failed")));

    return (res);
}

/** <!--*********************************************************************-->
 *
 * @fn bool PMmatchFlatWith( pattern *pat, node *expr)
 *
 * @brief matches pat against expr in mode PM_flatWith
 * @return success
 *
 *****************************************************************************/
bool
PMmatchFlatWith (pattern *pat, node *expr)
{
    mode = PM_flatWith;
    follow_lut = NULL;

    matching_level = 0;
    bool res;

    DBUG_PRINT ("PM", ("starting flatWith match"));
    res = (PAT_FUN (pat) (pat, expr) != (node *)FAIL);
    DBUG_PRINT ("PM", ("flatWith match %s!", (res ? "succeeded" : "failed")));

    return (res);
}

/** <!--*********************************************************************-->
 *
 * @fn node *PMmultiExprs( int num_nodes, ...)
 *
 * @brief enables multiple expressions to be matched against at the same time
 * @return stack suitable for the matching functions
 *
 *****************************************************************************/
node *
PMmultiExprs (int num_nodes, ...)
{
    va_list ap;
    node *stack = NULL;
    int i;

    va_start (ap, num_nodes);
    for (i = 0; i < num_nodes; i++) {
        stack = pushArgs (stack, va_arg (ap, node *));
    }
    va_end (ap);

    return (stack);
}
