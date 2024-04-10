/*
 * WARNING: this fild contains old, deprecated stuff!!!!
 * As soon as all uses of PMOxxx are gone it should be deleted!
 *
 */

#include <stdarg.h>
#include "pattern_match.h"

#define DBUG_PREFIX "PMO"
#include "debug.h"

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

static node *FAIL = (node *)0xfa1afe1;

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

    DBUG_ENTER ();

    if (stack != NULL) {
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
            DBUG_PRINT ("argument found:");
            DBUG_EXECUTE (PRTdoPrintFile (stderr, *arg));
        }
    } else {
        *arg = NULL;
        stack = (node *)FAIL;
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
    DBUG_ENTER ();

    if ((stack != NULL) && (NODE_TYPE (stack) == N_set)
        && (NODE_TYPE (SET_MEMBER (stack)) == N_exprs)) {
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
        DBUG_PRINT ("frame found:");
        DBUG_EXECUTE (PRTdoPrintFile (stderr, *top));
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
    DBUG_ENTER ();
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

    DBUG_ENTER ();
    z = (N_prf == NODE_TYPE (arg_node));
    if (z) {
        switch (PRF_PRF (arg_node)) {
        default:
            z = FALSE;
            break;
        case F_noteminval:
        case F_notemaxval:
        case F_noteintersect:
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
 *          c = _noteintersect(b, min, max);
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
    DBUG_ENTER ();

    res = arg_node;
    newres = arg_node;
    while ((arg_node != NULL) && (NULL != newres)) {
        newres = NULL;
        /* Find precursor to this node, if it exists */
        if ((NODE_TYPE (arg_node) == N_id)
            && (AVIS_SSAASSIGN (ID_AVIS (arg_node)) != NULL)) {
            DBUG_PRINT ("lastId looking up variable definition for %s.",
                        AVIS_NAME (ID_AVIS (arg_node)));
            newres = arg_node;
            arg_node = LET_EXPR (ASSIGN_STMT (AVIS_SSAASSIGN (ID_AVIS (arg_node))));
        } else {
            if (ignoreguards && isPrfGuard (arg_node)) {
                newres = PRF_ARG1 (arg_node);
                DBUG_PRINT ("lastId looking past guard, at %s.",
                            AVIS_NAME (ID_AVIS (newres)));
                assgn = AVIS_SSAASSIGN (ID_AVIS (newres));
                if (NULL != assgn) {
                    arg_node = LET_EXPR (ASSIGN_STMT (assgn));
                } else {
                    arg_node = NULL;
                }
            }
        }

        if (NULL != newres) {
            res = newres;
            DBUG_PRINT ("lastId definition is: %s.", AVIS_NAME (ID_AVIS (res)));
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
 *          b = _noteminval( a);
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

    DBUG_ENTER ();
    DBUG_PRINT ("followId trying to look up the variable definition ");
    res = lastId (arg_node, ignoreguards);
    if ((NULL != arg_node) && (N_id == NODE_TYPE (res))
        && (NULL != AVIS_SSAASSIGN (ID_AVIS (res)))
        && (NULL != ASSIGN_STMT (AVIS_SSAASSIGN (ID_AVIS (res))))) {
        arg_node = LET_EXPR (ASSIGN_STMT (AVIS_SSAASSIGN (ID_AVIS (res))));
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
    DBUG_ENTER ();
    DBUG_PRINT ("match failed!");
    if ((stack != NULL) && (stack != FAIL) && (NODE_TYPE (stack) == N_set)) {
        stack = FREEdoFreeTree (stack);
    }
    DBUG_RETURN ((node *)FAIL);
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

    DBUG_ENTER ();

    DBUG_PRINT ("MatchNode trying to match node of type \"%s\"...",
                global.mdb_nodetype[nt]);

    if (stack != (node *)FAIL) {
        stack = ExtractOneArg (stack, &match);

        match = followId (match, ignoreguards);
        if ((NODE_TYPE (match) == nt)
            && ((numAttribs == 0) || matchAttribsFun (match, numAttribs, attribRefs))) {
            DBUG_PRINT ("MatchNode( %s, _, %d, _, _, %d, _) matched!",
                        global.mdb_nodetype[nt], numAttribs, pushSons);
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
                    DBUG_UNREACHABLE ("pushSons not yet fully implemented!");
                    break;
                }
            }

            REF_SET (matched_node, match);
        } else {
            stack = FailMatch (stack);
            DBUG_PRINT ("failed!");
        }
    } else {
        DBUG_PRINT ("MatchNode passing on FAIL!");
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
    DBUG_ENTER ();
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

    DBUG_ENTER ();
    if (*var == NULL) {
        DBUG_PRINT ("pmvar trying to match unbound variable...");
    } else {
        DBUG_PRINT ("pmvar trying to match bound variable...");
    }

    if (stack != (node *)FAIL) {
        stack = ExtractOneArg (stack, &arg);
        if (getlastid) {
            arg = lastId (arg, ignoreguards);
        }
        if (NODE_TYPE (arg) == N_id) {
            if (*var == NULL) {
                DBUG_PRINT ("pmvar binding variable!");
                *var = arg;
            } else if (ID_AVIS (*var) == ID_AVIS (arg)) {
                DBUG_PRINT ("pmvar found variable matches bound one!");
            } else {
                stack = FailMatch (stack);
            }
        } else {
            stack = FailMatch (stack);
        }
    } else {
        DBUG_PRINT ("pmvar ...passing-on FAIL!");
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
    DBUG_ENTER ();
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
    DBUG_ENTER ();
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
    DBUG_ENTER ();
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
 *  etc.
 *
 *****************************************************************************/
#define MATCH_SCALAR_CONST(kind)                                                         \
    node *PMO##kind (node *stack)                                                        \
    {                                                                                    \
        node *kind##_node;                                                               \
        DBUG_ENTER ();                                                                   \
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
MATCH_SCALAR_CONST (numbyte)
MATCH_SCALAR_CONST (numubyte)
MATCH_SCALAR_CONST (numint)
MATCH_SCALAR_CONST (numuint)
MATCH_SCALAR_CONST (numshort)
MATCH_SCALAR_CONST (numushort)
MATCH_SCALAR_CONST (numlong)
MATCH_SCALAR_CONST (numulong)
MATCH_SCALAR_CONST (numlonglong)
MATCH_SCALAR_CONST (numulonglong)

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
        DBUG_ENTER ();                                                                   \
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
    DBUG_ENTER ();

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
    DBUG_ENTER ();

    REF_CONST (arefs[0]) = frameshape;
    stack = MatchNode (N_array, MatchArrayAttribs, 1, arefs, array, TRUE, stack, FALSE);

    DBUG_RETURN (stack);
}

node *
PMOarrayConstructor (constant **frameshape, node **array, node *stack)
{
    attrib_t arefs[1];
    DBUG_ENTER ();

    REF_CONST (arefs[0]) = frameshape;
    stack = MatchNode (N_array, MatchArrayAttribs, 1, arefs, array, FALSE, stack, FALSE);

    DBUG_RETURN (stack);
}

node *
PMOarrayConstructorGuards (constant **frameshape, node **array, node *stack)
{
    attrib_t arefs[1];
    DBUG_ENTER ();

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

    DBUG_ENTER ();

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
            DBUG_PRINT ("PMOconst matched constant!");
            if (*co == NULL) {
                *co = cofound;
                *conode = arg;
            } else {
                if (COcompareConstants (*co, cofound)) {
                    DBUG_PRINT ("PMOconst matched value!");
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
        DBUG_PRINT ("PMOconst passing on FAIL!");
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
    DBUG_ENTER ();

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
        DBUG_PRINT ("PMOintConst passing on FAIL!");
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
 *        N_with genarray/modarray nodes from id to the
 *        shape primogenitor of id. Basically,
 *        it does lastId(id, TRUE), but also has some tricks to go
 *        further when that result comes from a modarray WL
 *        or a genarray WL.
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

    DBUG_ENTER ();

    DBUG_PRINT ("PMOshapePrimogenitor trying to find primogenitor for: %s.",
                AVIS_NAME (ID_AVIS (arg)));

    arg = lastId (arg, TRUE);
    res = arg;

    /* Chase possible modarray WL */
    /* FIXME: probably can do something similar with genarray,
     * if we can find its shape
     */
    modarr = AVIS_SSAASSIGN (ID_AVIS (arg));
    if (NULL != modarr) {
        modarr = LET_EXPR (ASSIGN_STMT (modarr));
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
                    DBUG_PRINT ("PMOshapePrimogenitor found scalar default cell");
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
    DBUG_ENTER ();
    if (*shp == NULL) {
        DBUG_PRINT ("PMOsaashape trying to match unbound variable.");
    } else {
        DBUG_PRINT ("PMOsaashape trying to match bound variable.");
    }

    if (stack != (node *)FAIL) {
        arg = AVIS_SHAPE (ID_AVIS (*array));
        if (NULL != arg) {
            arg = lastId (arg, TRUE); /* Not sure about ignoreguards value here... */
        }
        if ((NULL != arg) && (N_id == NODE_TYPE (arg))) {
            if (REF_ISUNDEFINED (shp)) {
                DBUG_PRINT ("PMOsaashape binding AVIS_SHAPE");
                REF_SET (shp, AVIS_SHAPE (ID_AVIS (arg)));
            } else if (*shp == AVIS_SHAPE (ID_AVIS (arg))) {
                DBUG_PRINT ("PMOsaashape found matching AVIS_SHAPE");
            } else {
                stack = FailMatch (stack);
            }
        } else {
            stack = FailMatch (stack);
        }
    } else {
        DBUG_PRINT ("PMOsaashape passing-on FAIL.");
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

    DBUG_ENTER ();

    if (stack != (node *)FAIL) {
        stack = ExtractTopFrame (stack, &exprs);

        DBUG_ASSERT (exprs != NULL, "No exprs on top of stack");

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

    DBUG_ENTER ();

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

    DBUG_ENTER ();

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

    DBUG_ENTER ();

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

#undef DBUG_PREFIX
