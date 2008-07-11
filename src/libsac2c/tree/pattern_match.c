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
 * A few examples:
 * a) we want to match expressions of the form _sub_SxS_( x, x)
 *    where both x's denote the same variable.
 *    Using this module, we can analyse a given expression "expr" by
 *
 *    node *x=NULL;
 *
 *    if(  PM( PMvar( &x, PMvar( &x, PMprf( _sub_SxS_, expr))))  ) {
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
 *    if( PM( PMvar( &val, PMvar( &iv, PMvar( &A, PMprf( F_modarray_AxVxS,
 *             PMvar( &iv, PMprf( F_sel_VxA, expr)))))))  ) {
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
 *  node *PM<xyz>( <match-specific-args>, node *stack)
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
 *  PMvar( node **var, ...) which matches N-id nodes, and
 *  PMprf( prf fun, ...)    which matches anything that is defined as
 *                          an application of the prf fun.
 *
 * Note here, that all variables that you want to match need to be
 * initialised to NULL prior to matching!
 *
 * The nesting of match functions needs to be surrounded by a call to
 * the function
 *   bool PM( node *stack)
 * which interprets the result of the match and yields true or false
 * accordingly.
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

#include "dbug.h"

#include "print.h"
#include "globals.h"
#include "free.h"
#include "traverse.h"
#include "tree_basic.h"
#include "new_types.h"
#include "tree_compound.h"
#include "constants.h"

static char *FAIL = "";

typedef union {
    prf *a_prf;
    node **a_node;
    constant **a_const;
} attrib_t;

#define REF_PRF(n) ((n).a_prf)
#define REF_NODE(n) ((n).a_node)
#define REF_CONST(n) ((n).a_const)

typedef bool (*checkFun_ptr) (node *, int, attrib_t *);

/** <!--*********************************************************************-->
 *
 * local helper functions:
 */

/** <!--*******************************************************************-->
 *
 * 1;5A
 * @fn node *ExtractOneArg( node *stack, node * args)
 *
 * @brief extracts the first argument from the exprs stack.
 *        Note here, that stack can either be N_set (stack), an N_exprs or
 *        any other expression! N_set nodes that become redundant
 *        are being freed!
 * @param stack: stack of exprs
 * @return via args the first expression in the chain and the rest of the
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
            *arg = EXPRS_EXPR (stack);
            stack = EXPRS_NEXT (stack);
        } else {
            *arg = stack;
            stack = NULL;
        }
        DBUG_PRINT ("PM", ("argument found:"));
        DBUG_EXECUTE ("PM", PRTdoPrintFile (stderr, *arg););
    }
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
 * @fn static node *lastId( node * arg_node)
 *
 * @brief looks behind the definitions of N_id nodes, if possible
 *        to find the last N_id node in a chain.
 *          f = [5,6];
 *          b = f;
 *          c = b;
 *          d = c;
 *
 *        If we do lastId(d), the result is f.
 *
 * @param arg: potential N_id node to be followed after
 * @return first N_id in chain iff available; unmodified arg otherwise
 *****************************************************************************/
static node *
lastId (node *arg_node)
{
    node *res;
    DBUG_ENTER ("lastId");

    DBUG_PRINT ("PM", ("lastId trying to look up the variable definition "));
    res = arg_node;
    while ((NODE_TYPE (arg_node) == N_id)
           && (AVIS_SSAASSIGN (ID_AVIS (arg_node)) != NULL)) {
        res = arg_node;
        DBUG_PRINT ("PM", ("lastId looking up definition of the variable"));
        arg_node = LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (arg_node))));
        DBUG_PRINT ("PM", ("lastId definition found:"));
        DBUG_EXECUTE ("PM", PRTdoPrintFile (stderr, arg_node););
    }
    DBUG_RETURN (res);
}

/** <!--*******************************************************************-->
 *
 * @fn static node *followId( node * arg_node)
 *
 * @brief looks behind the definitions of N_id nodes, if possible
 *        to find the definition point of a value. For example,
 *          f = [5,6];
 *          a = f;
 *          b = a;
 *          c = b;
 *          d = c;
 *
 *        If we do followId(d), the result is the N_array [5,6].
 *
 * @param arg: potential N_id node to be followed after
 * @return defining expression iff available;
 *         unmodified arg otherwise
 *****************************************************************************/
static node *
followId (node *arg_node)
{
    node *res;

    DBUG_ENTER ("followId");
    DBUG_PRINT ("PM", ("followId trying to look up the variable definition "));
    res = lastId (arg_node);
    if ((N_id == NODE_TYPE (res)) && (NULL != AVIS_SSAASSIGN (ID_AVIS (res)))
        && (NULL != ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (res))))) {
        arg_node = LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (res))));
    }
    DBUG_RETURN (arg_node);
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
    if ((stack != NULL) && (NODE_TYPE (stack) == N_set)) {
        stack = FREEdoFreeTree (stack);
    }
    DBUG_RETURN ((node *)FAIL);
}

/** <!--*******************************************************************-->
 *
 * @fn node *MatchNode( nodetype nt, checkFun_ptr matchAttribsFun,
 *                      int numAttribs, attrib_t *attribRefs,
 *                      node **matched_node,
 *                      bool pushSons, node *stack)
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
           attrib_t *attribRefs, node **matched_node, bool pushSons, node *stack)
{
    DBUG_ENTER ("MatchNode");

    DBUG_PRINT ("PM", ("MatchNode trying to match node of type \"%s\"...",
                       global.mdb_nodetype[nt]));

    if (stack != (node *)FAIL) {
        stack = ExtractOneArg (stack, matched_node);
        *matched_node = followId (*matched_node);
        if ((NODE_TYPE (*matched_node) == nt)
            && ((numAttribs == 0)
                || matchAttribsFun (*matched_node, numAttribs, attribRefs))) {
            DBUG_PRINT ("PM", ("MatchNode( %s, _, %d, _, _, %d, _) matched!",
                               global.mdb_nodetype[nt], numAttribs, pushSons));
            if (pushSons) {
                switch (nt) {
                case N_prf:
                    stack = PushArgs (stack, PRF_ARGS (*matched_node));
                    break;
                case N_array:
                    stack = PushArgs (stack, ARRAY_AELEMS (*matched_node));
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
        } else {
            stack = FailMatch (stack);
            DBUG_PRINT ("PM", ("failed!"));
        }
    } else {
        DBUG_PRINT ("PM", ("MatchNode passing on FAIL!"));
    }
    DBUG_RETURN (stack);
}

/** <!--*********************************************************************-->
 *
 * Exported functions for pattern matching:
 */

/** <!--*******************************************************************-->
 *
 * @fn bool PM( node *stack)
 *
 * @brief checks the result of a pattern match for failure
 * @param stack: resulting stack of a match
 * @return success
 *****************************************************************************/
bool
PM (node *stack)
{
    DBUG_ENTER ("PM");
    DBUG_RETURN (stack != (node *)FAIL);
}

/** <!--*******************************************************************-->
 *
 * @fn static node *pmvar( node **var, node *stack, bool lastid)
 *
 * @brief tries to match against a variable. If *var is NULL, the top of
 *        the stack is bound to it (provided it is an N_id).
 *        If *var is bound already, it only matches if both N_id nodes
 *        point to the same N_avis.
 *
 * @param *var: bound N_id (if any)
 *        stack: "stack" of exprs.
 *        getlastid: if TRUE, chase the list of N_id nodes back to its
 *                origin.
 * @return shortened stack.
 *****************************************************************************/
static node *
pmvar (node **var, node *stack, bool getlastid)
{
    node *arg;

    DBUG_ENTER ("pmvar");
    if (*var == NULL) {
        DBUG_PRINT ("PM", ("pmvar trying to match unbound variable..."));
    } else {
        DBUG_PRINT ("PM", ("pmvar trying to match bound variable..."));
    }

    if (stack != (node *)FAIL) {
        stack = ExtractOneArg (stack, &arg);
        if (getlastid) {
            arg = lastId (arg);
        }
        if (NODE_TYPE (arg) == N_id) {
            if (*var == NULL) {
                DBUG_PRINT ("PM", ("pmvar binding variable!"));
                *var = arg;
            } else if (ID_AVIS (*var) == ID_AVIS (arg)) {
                DBUG_PRINT ("PM", ("pmvar found variable matches bound one!"));
            } else {
                stack = FailMatch (stack);
            }
        } else {
            stack = FailMatch (stack);
        }
    } else {
        DBUG_PRINT ("PM", ("pmvar ...passing-on FAIL!"));
    }
    DBUG_RETURN (stack);
}

/** <!--*******************************************************************-->
 *
 * @fn node *PMvar( node **var, node *stack)
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
PMvar (node **var, node *stack)
{
    DBUG_ENTER ("PMvar");
    stack = pmvar (var, stack, FALSE);
    DBUG_RETURN (stack);
}

/** <!--*******************************************************************-->
 *
 * @fn node *PMlastVar( node **var, node *stack)
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
PMlastVar (node **var, node *stack)
{
    DBUG_ENTER ("PMlastVar");
    stack = pmvar (var, stack, TRUE);
    DBUG_RETURN (stack);
}

/** <!--*******************************************************************-->
 *
 * @fn node *PMprf( prf fun, node *stack)
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
PMprf (prf fun, node *stack)
{
    node *prf_node;
    attrib_t arefs[1];
    DBUG_ENTER ("PMprf");

    REF_PRF (arefs[0]) = &fun;
    stack = MatchNode (N_prf, MatchPrfAttribs, 1, arefs, &prf_node, TRUE, stack);

    DBUG_RETURN (stack);
}

/** <!--*******************************************************************-->
 *
 * @fn node *PMarray( constant ** frameshape, node **array, node *stack)
 * @fn node *PMarrayConstructor( constant ** frameshape, node **array, node *stack)
 *
 * @brief tries to match against an N_array. If *frameshape is NULL, any
 *        array on the top of the stack matches, its AST representation is bound
 *        to array and the frameshape found is converted into a constant which
 *        is bound to *frameshape.
 *        If *frameshape is not NULL, it only matches if the top of the stack is
 *        an N_array with the given frameshape. In that case only array is bound
 *        to the respective part of the AST.
 * @return shortened stack. In PMarray, the ARRAY_AELEMS are pushed onto the
 *        stackbefore returning, in PMarrayConstructor NOT!
 *****************************************************************************/
static bool
MatchArrayAttribs (node *array_node, int num, attrib_t *arefs)
{
    bool match;
    constant *shpfound;

    shpfound = COmakeConstantFromShape (ARRAY_FRAMESHAPE (array_node));
    if (*REF_CONST (arefs[0]) == NULL) {
        *REF_CONST (arefs[0]) = shpfound;
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
PMarray (constant **frameshape, node **array, node *stack)
{
    attrib_t arefs[1];
    DBUG_ENTER ("PMarray");

    REF_CONST (arefs[0]) = frameshape;
    stack = MatchNode (N_array, MatchArrayAttribs, 1, arefs, array, TRUE, stack);

    DBUG_RETURN (stack);
}

node *
PMarrayConstructor (constant **frameshape, node **array, node *stack)
{
    attrib_t arefs[1];
    DBUG_ENTER ("PMarray");

    REF_CONST (arefs[0]) = frameshape;
    stack = MatchNode (N_array, MatchArrayAttribs, 1, arefs, array, FALSE, stack);

    DBUG_RETURN (stack);
}

/** <!--*******************************************************************-->
 *
 * @fn node *PMconst( constant ** co, node **conode, node *stack)
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
PMconst (constant **co, node **conode, node *stack)
{
    node *arg;
    ntype *type;
    constant *cofound = NULL;

    DBUG_ENTER ("PMconst");

    if (stack != (node *)FAIL) {
        stack = ExtractOneArg (stack, &arg);
        if (NODE_TYPE (arg) == N_id) {
            type = AVIS_TYPE (ID_AVIS (arg));
            if (TYisAKV (type)) {
                cofound = COcopyConstant (TYgetValue (type));
                arg = followId (arg); /* needed for conode! */
            }
        } else {
            cofound = COaST2Constant (arg);
        }
        if (cofound != NULL) {
            DBUG_PRINT ("PM", ("PMconst matched constant!"));
            if (*co == NULL) {
                *co = cofound;
                *conode = arg;
            } else {
                if (COcompareConstants (*co, cofound)) {
                    DBUG_PRINT ("PM", ("PMconst matched value!"));
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
        DBUG_PRINT ("PM", ("PMconst passing on FAIL!"));
    }

    DBUG_RETURN (stack);
}

/** <!--*******************************************************************-->
 *
 * @fn node *PMintConst( constant ** co, node **conode, node *stack)
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
PMintConst (constant **co, node **conode, node *stack)
{
    constant *co_in;
    DBUG_ENTER ("PMintConst");

    if (stack != (node *)FAIL) {
        co_in = *co;
        stack = PMconst (co, conode, stack);
        if (stack != (node *)FAIL) {
            if (COgetType (*co) != T_int) {
                stack = FailMatch (stack);
                if (co_in == NULL) {
                    *co = COfreeConstant (*co);
                }
            }
        }
    } else {
        DBUG_PRINT ("PM", ("PMintConst passing on FAIL!"));
    }

    DBUG_RETURN (stack);
}
