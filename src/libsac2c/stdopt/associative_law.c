/** <!--********************************************************************-->
 *
 * @file associative_law.c
 *
 * Prefix: AL
 *
 * Conceptual view:
 *
 * This optimization attempts to reorder a chain of associative,
 * commutative operations. The idea of AL is to reorder complex
 * expressions such that they may be further optimized by other
 * optimisations, namely constant folding.
 *
 * Currently AL aims at the following:
 *  - moving constant operands closer together, so that a maximum partial
 *    evaluation can be realised;
 *  - pairing of operands and their inverse, such that the application of
 *    of the associative operation can be replaced by its neutral element.
 *  - separate vector operations from scalar operations in order to compute
 *    expressions as long as possible on the scalar level.
 *  - identifying operands that are loop or with-loop invariant and group
 *    them together to enable (with-)loop invariant removal.
 *
 * Example:
 *
 *   s = 1 + 2;
 *   r = s + a;
 *
 * may be optimized (by the typechecker) to r = 3 + a, while
 *
 *   s'= 1 + a;
 *   r'= s'+ 2;
 *
 * can't.
 *
 * AL tries to regroup such complex expressions into constant parts,
 * scalar identifiers, and vector identifiers.
 *
 * For example, let a: int, b:int, c:int[2],d:int[2]
 *
 * x = a _add_SxS_ 1;
 * y = c _add_VxS_ 1;
 * z = b _add_SxV_ d;
 *
 * r = x + y
 * s = r + z
 *
 * CollectExprs forms a chain with all identifiers:
 *
 * s = +(a,1,c,1,b,d)
 *
 * This is then split into constants, loop-invariant variables, non-constant scalars,
 * scalars and non-constant vectors. The latter distinction aims at doing computations
 * as long as possible on the scalar level before switching to vectors.
 *
 * Finally, these 4 sets of variables are transformed back into a piece of AST:
 *
 * const = 1 _add_SxS_ 1;
 * nonconst = a _add_SxS_ b;
 * vecs = c _add_VxV_ d;
 *
 * s' = const _add_SxS_ nonconst
 * s = s' _add_SxV_ vecs;
 *
 *
 * Implementation notes:
 *
 * The implementation of the tree traversal is controlled by four modes,
 * which realise two distinct pairs of top-down and bottom-up traversals.
 *
 * MODE_recurse: This is the first top-down traversal in each block. Here
 * we only take care of nested blocks (due to with-loops).
 *
 * MODE_noop: This is the corresponding bottom-up traversal where we do
 * effectively nothing.
 *
 * MODE_mark: This is the second top-down traversal. Here, we mark each
 * avis of a left-hand-side variable as active. As we do not (recursively)
 * traverse into right-hand-side expressions, this marking step allows
 * us to identify all assignments in the currently active block (aka
 * assignment chain).
 *
 * MODE_transform: This is the corresponding bottom-up traversal where we
 * actually do the interesting stuff: when we reach an N_prf node with an
 * associative function we start to recursively identify all operands of a
 * virtual multi-operand application of the same built-in function. Then
 * we analyse the list of operands for optimisation opportunities as sketched
 * out above. If so, we reorder the list accordingly and re-transform it into
 * a properly flattened tree of binary operator applications. If not, we
 * just give up and leave the code as is.
 *
 *
 * A note on guards:
 *
 * The collection of expressions transparently traverses through guards in
 * the data flow. This creates the largest possible optimisation context.
 * The guards nonetheless will stay within the code as long as they are
 * captured by the after guard.
 *
 *****************************************************************************/

#include "associative_law.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "globals.h"
#include "new_types.h"
#include "new_typecheck.h"
#include "traverse.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "DupTree.h"
#include "pattern_match.h"

#define DBUG_PREFIX "AL"
#include "debug.h"

/*
 * INFO structure
 */

typedef enum { MODE_recurse, MODE_mark, MODE_transform, MODE_noop } trav_mode_t;

struct INFO {
    trav_mode_t mode;
    node *fundef;
    node *preassign;
    bool isalcandidate;
    bool isloopbody;
    bool iswithloopbody;
    node *lhs;
    node *withid;
    node *recursiveapargs;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_PREASSIGN(n) ((n)->preassign)
#define INFO_MODE(n) ((n)->mode)
#define INFO_ISALCANDIDATE(n) ((n)->isalcandidate)
#define INFO_ISLOOPBODY(n) ((n)->isloopbody)
#define INFO_ISWITHLOOPBODY(n) ((n)->iswithloopbody)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_WITHID(n) ((n)->withid)
#define INFO_RECURSIVEAPARGS(n) ((n)->recursiveapargs)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_PREASSIGN (result) = NULL;
    INFO_MODE (result) = MODE_noop;
    INFO_ISALCANDIDATE (result) = FALSE;
    INFO_ISLOOPBODY (result) = FALSE;
    INFO_ISWITHLOOPBODY (result) = FALSE;
    INFO_LHS (result) = NULL;
    INFO_WITHID (result) = NULL;
    INFO_RECURSIVEAPARGS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn node *ALdoAssocLawOptimization( node *arg_node)
 *
 * @brief starting point of associativity optimization
 *
 * @param arg_node: N_module or N_fundef node
 *
 * @return
 *
 *****************************************************************************/

node *
ALdoAssocLawOptimization (node *arg_node)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_al);
    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Helper functions
 *
 *****************************************************************************/

#ifndef DBUG_OFF
static void
printOperands (node *exprs)
{
    node *tmp = exprs;

    DBUG_ENTER ();

    while (tmp != NULL) {
        DBUG_PRINT ("%s ", AVIS_NAME (ID_AVIS (EXPRS_EXPR (tmp))));
        tmp = EXPRS_NEXT (tmp);
    }

    DBUG_RETURN ();
}
#endif

/*
 * The classification of primitive functions should rather be
 * derived from corresponding information in prf_info.mac for
 * use here as well as in other compiler modules.
 * cg currently lacks the time to implement this.
 */
static bool
IsGuardPrf (prf op)
{
#if 0
    bool res;
#endif

    DBUG_ENTER ();

#if 0
    switch (op) {
        //  case F_guard:
        //  disbabled until the format of F_guard has been decided upon.
    case F_same_shape_AxA:
    case F_shape_matches_dim_VxA:
    case F_non_neg_val_S:
    case F_non_neg_val_V:
    case F_val_lt_shape_VxA:
    case F_val_lt_val_SxS:
    case F_val_le_val_SxS:
    case F_val_le_val_VxV:
    case F_prod_matches_prod_shape_VxA:
        res = TRUE;
        break;
    default:
        res = FALSE;
    }
#endif

    DBUG_RETURN (FALSE);
}

static prf
AltPrf (prf op)
{
    prf res;

    DBUG_ENTER ();

    switch (op) {
    case F_neg_S:
        res = F_neg_V;
        break;
    case F_reciproc_S:
        res = F_reciproc_V;
        break;
    default:
        res = F_noop;
        DBUG_UNREACHABLE ("We should never get here.");
    }

    DBUG_RETURN (res);
}

static node *
getInverse (prf prf, node *exprs)
{
    node *var, *res;
    pattern *pat;

    DBUG_ENTER ();

    var = NULL;
    res = NULL;

    pat = PMprf (1, PMAisPrf (prf), 1, PMvar (1, PMAgetNode (&var), 0));

    if (PMmatchFlat (pat, EXPRS_EXPR (exprs))) {
        DBUG_ASSERT ((var == NULL) || NODE_TYPE (var) == N_id,
                     "Result has wrong node type.");
        res = ID_AVIS (var);
    } else {
        pat = PMfree (pat);

        pat = PMprf (1, PMAisPrf (AltPrf (prf)), 1, PMvar (1, PMAgetNode (&var), 0));

        if (PMmatchFlat (pat, EXPRS_EXPR (exprs))) {
            DBUG_ASSERT ((var == NULL) || NODE_TYPE (var) == N_id,
                         "Result has wrong node type.");
            res = ID_AVIS (var);
        } else {
            res = NULL;
        }
    }

    pat = PMfree (pat);

    DBUG_RETURN (res);
}

static node *
getElement (node *exprs)
{
    node *res;

    DBUG_ENTER ();

    res = ID_AVIS (EXPRS_EXPR (exprs));

    DBUG_ASSERT (NODE_TYPE (res) == N_avis, "Result has wrong node type.");

    DBUG_RETURN (res);
}

static node *
identifyInverses (prf inverse_prf, node **head)
{
    node *left, *right, *res, *tmp, *left_last, *right_last;
    node *left_inv, *right_inv, *left_elem, *right_elem;

    DBUG_ENTER ();

    left = *head;
    left_last = NULL;

    res = NULL;

    while (left != NULL) {
        right = EXPRS_NEXT (left);
        right_last = left;

        while (right != NULL) {
            left_elem = getElement (left);
            left_inv = getInverse (inverse_prf, left);
            right_elem = getElement (right);
            right_inv = getInverse (inverse_prf, right);

            if ((left_elem == right_inv) || (left_inv == right_elem)) {
                EXPRS_NEXT (right_last) = EXPRS_NEXT (right);
                EXPRS_NEXT (right) = res;
                tmp = EXPRS_NEXT (left);
                EXPRS_NEXT (left) = right;
                res = left;
                if (left_last != NULL) {
                    EXPRS_NEXT (left_last) = tmp;
                } else {
                    *head = tmp;
                }

                left = tmp;
                break;
            } else {
                right_last = right;
                right = EXPRS_NEXT (right);
            }
        }

        if (right == NULL) {
            left_last = left;
            left = EXPRS_NEXT (left);
        }
    }

    DBUG_RETURN (res);
}

// Crude workaround to get AL to ignore eq/neq
static bool
isEqNeqPrf (prf fun)
{
    bool res;

    DBUG_ENTER ();

    switch (fun) {

    case F_eq_SxS:
    case F_eq_SxV:
    case F_eq_VxS:
    case F_eq_VxV:

    case F_neq_SxS:
    case F_neq_SxV:
    case F_neq_VxS:
    case F_neq_VxV:
        res = TRUE;
        break;

    default:
        res = FALSE;
    }

    DBUG_RETURN (res);
}

bool
ALisAssociativeAndCommutativePrf (prf fun)
{
    bool res;

    DBUG_ENTER ();

    switch (fun) {
    case F_add_SxS:
    case F_add_SxV:
    case F_add_VxS:
    case F_add_VxV:

    case F_mul_SxS:
    case F_mul_SxV:
    case F_mul_VxS:
    case F_mul_VxV:

    case F_max_SxS:
    case F_max_SxV:
    case F_max_VxS:
    case F_max_VxV:

    case F_min_SxS:
    case F_min_SxV:
    case F_min_VxS:
    case F_min_VxV:

    case F_and_SxS:
    case F_and_SxV:
    case F_and_VxS:
    case F_and_VxV:

    case F_or_SxS:
    case F_or_SxV:
    case F_or_VxS:
    case F_or_VxV:

    case F_eq_SxS:
    case F_eq_SxV:
    case F_eq_VxS:
    case F_eq_VxV:

    case F_neq_SxS:
    case F_neq_SxV:
    case F_neq_VxS:
    case F_neq_VxV:
        res = TRUE;
        break;

    default:
        res = FALSE;
    }

    DBUG_RETURN (res);
}

static bool
isPrfAdd (prf prf)
{
    bool res;

    DBUG_ENTER ();

    switch (prf) {
    case F_add_SxS:
    case F_add_SxV:
    case F_add_VxS:
    case F_add_VxV:

        res = TRUE;
        break;

    default:
        res = FALSE;
    }

    DBUG_RETURN (res);
}

static bool
isPrfMul (prf prf)
{
    bool res;

    DBUG_ENTER ();

    switch (prf) {
    case F_mul_SxS:
    case F_mul_SxV:
    case F_mul_VxS:
    case F_mul_VxV:

        res = TRUE;
        break;

    default:
        res = FALSE;
    }

    DBUG_RETURN (res);
}

static bool
isConst (node *n)
{
    bool res;

    DBUG_ENTER ();

    switch (NODE_TYPE (n)) {
    case N_char:
    case N_bool:
    case N_double:
    case N_float:
    case N_numbyte:
    case N_numshort:
    case N_numint:
    case N_numlong:
    case N_numlonglong:
    case N_numubyte:
    case N_numushort:
    case N_numuint:
    case N_numulong:
    case N_numulonglong:
    case N_num:
        res = TRUE;
        break;

    case N_array:
        res = TCfoldPredExprs (isConst, ARRAY_AELEMS (n));
        break;

    case N_id:
        res = TYisAKV (AVIS_TYPE (ID_AVIS (n)));
        break;

    default:
        res = FALSE;
    }

    DBUG_RETURN (res);
}

static bool
isNonConstScalar (node *n)
{
    bool res;

    DBUG_ENTER ();

    if (NODE_TYPE (n) == N_id) {
        res = ((TYisAKS (ID_NTYPE (n))) && (TYgetDim (ID_NTYPE (n)) == 0));
    } else {
        res = FALSE;
    }

    DBUG_RETURN (res);
}

static bool
isScalar (node *n)
{
    bool res;

    DBUG_ENTER ();

    switch (NODE_TYPE (n)) {
    case N_numbyte:
    case N_numshort:
    case N_numint:
    case N_numlong:
    case N_numlonglong:
    case N_numubyte:
    case N_numushort:
    case N_numuint:
    case N_numulong:
    case N_numulonglong:
    case N_num:
    case N_char:
    case N_bool:
    case N_double:
    case N_float:
        res = TRUE;
        break;

    case N_id:
        res = ID_ISSCLPRF (n);
        break;

    default:
        res = FALSE;
    }

    DBUG_RETURN (res);
}

static bool
isNonLocal (node *n)
{
    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (n) == N_id, "Illegal node type detected");

    DBUG_RETURN (!AVIS_ISDEFINEDINCURRENTBLOCK (ID_AVIS (n)));
}

static bool
isLoopInvariant (node *n)
{
    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (n) == N_id, "Illegal node type detected");

    DBUG_RETURN (AVIS_ISLOOPINVARIANT (ID_AVIS (n)));
}

static prf
normalizePrf (prf prf)
{
    DBUG_ENTER ();

    switch (prf) {
    case F_add_SxS:
    case F_add_SxV:
    case F_add_VxS:
    case F_add_VxV:
        prf = F_add_SxS;
        break;

    case F_mul_SxS:
    case F_mul_SxV:
    case F_mul_VxS:
    case F_mul_VxV:
        prf = F_mul_SxS;
        break;

    default:
        break;
    }

    DBUG_RETURN (prf);
}

static bool
compatiblePrf (prf p1, prf p2)
{
    bool res;

    DBUG_ENTER ();

    res = (normalizePrf (p1) == normalizePrf (p2));

    DBUG_RETURN (res);
}

static prf
getPrf (prf prf, node *e1, node *e2)
{
    bool s1, s2;

    DBUG_ENTER ();

    s1 = isScalar (e1);
    s2 = isScalar (e2);

    switch (prf) {
    case F_add_SxS:
    case F_add_SxV:
    case F_add_VxS:
    case F_add_VxV:
        if (s1) {
            if (s2)
                prf = F_add_SxS;
            else
                prf = F_add_SxV;
        } else {
            if (s2)
                prf = F_add_VxS;
            else
                prf = F_add_VxV;
        }
        break;

    case F_mul_SxS:
    case F_mul_SxV:
    case F_mul_VxS:
    case F_mul_VxV:
        if (s1) {
            if (s2)
                prf = F_mul_SxS;
            else
                prf = F_mul_SxV;
        } else {
            if (s2)
                prf = F_mul_VxS;
            else
                prf = F_mul_VxV;
        }
        break;

    case F_max_SxS:
    case F_max_SxV:
    case F_max_VxS:
    case F_max_VxV:

    case F_min_SxS:
    case F_min_SxV:
    case F_min_VxS:
    case F_min_VxV:

    case F_and_SxS:
    case F_and_SxV:
    case F_and_VxS:
    case F_and_VxV:

    case F_or_SxS:
    case F_or_SxV:
    case F_or_VxS:
    case F_or_VxV:
        break;

    default:
        DBUG_UNREACHABLE ("Illegal prf!");
    }

    DBUG_RETURN (prf);
}

static bool
isArg1Scl (prf prf)
{
    bool res;
    DBUG_ENTER ();

    switch (prf) {
    case F_add_SxS:
    case F_add_SxV:
    case F_mul_SxS:
    case F_mul_SxV:
        res = TRUE;
        break;
    default:
        res = FALSE;
    }

    DBUG_RETURN (res);
}

static bool
isArg2Scl (prf prf)
{
    bool res;
    DBUG_ENTER ();

    switch (prf) {
    case F_add_SxS:
    case F_add_VxS:
    case F_mul_SxS:
    case F_mul_VxS:
        res = TRUE;
        break;
    default:
        res = FALSE;
    }

    DBUG_RETURN (res);
}

static bool
isSingletonOrEmpty (node *expr)
{
    bool res;

    DBUG_ENTER ();

    res = ((expr == NULL) || (EXPRS_NEXT (expr) == NULL));

    DBUG_RETURN (res);
}

static node *
consumeHead (node **exprs)
{
    node *res;

    DBUG_ENTER ();

    res = EXPRS_EXPR (*exprs);
    EXPRS_EXPR (*exprs) = NULL;
    *exprs = FREEdoFreeNode (*exprs);

    DBUG_RETURN (res);
}

static node *
CombineExprs2Prf (prf prf, node *expr1, node *expr2, info *arg_info)
{
    node *rhs;
    node *avis = NULL;
    node *assign;
    node *id;
    ntype *prod;

    DBUG_ENTER ();

    rhs = TCmakePrf2 (getPrf (prf, expr1, expr2), expr1, expr2);

    prod = NTCnewTypeCheck_Expr (rhs);
    avis = TBmakeAvis (TRAVtmpVar (), TYcopyType (TYgetProductMember (prod, 0)));
    prod = TYfreeType (prod);

    FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
      = TBmakeVardec (avis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

    assign
      = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), rhs), INFO_PREASSIGN (arg_info));
    AVIS_SSAASSIGN (avis) = assign;

    INFO_PREASSIGN (arg_info) = assign;

    id = TBmakeId (avis);
    ID_ISSCLPRF (id) = isScalar (expr1) && isScalar (expr2);

    DBUG_RETURN (id);
}

static node *
revert (node *ass, node *agg)
{
    node *res;

    DBUG_ENTER ();

    if (ass == NULL) {
        res = agg;
    } else {
        res = ASSIGN_NEXT (ass);
        ASSIGN_NEXT (ass) = agg;
        res = revert (res, ass);
    }

    DBUG_RETURN (res);
}

static node *
Exprs2PrfTree (prf prf, node *exprs, info *arg_info)
{
    node *res;

    DBUG_ENTER ();

    if (exprs == NULL) {
        res = NULL;
    } else {
        if (EXPRS_NEXT (exprs) == NULL) {
            res = consumeHead (&exprs);
        } else {
            node *e1, *e2;
            e1 = consumeHead (&exprs);
            e2 = consumeHead (&exprs);
            exprs = TBmakeExprs (CombineExprs2Prf (prf, e1, e2, arg_info), exprs);
            res = Exprs2PrfTree (prf, exprs, arg_info);
        }
    }
    DBUG_RETURN (res);
}

static node *
CollectExprs (prf prf, node *a, bool sclprf)
{
    node *res, *rhs;
    node *left, *right;
    node *ids, *exprs;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (a) == N_id, "CollectExprs called with illegal node type");

    res = NULL;

    if (AVIS_ISDEFINEDINCURRENTBLOCK (ID_AVIS (a))
        && AVIS_SSAASSIGN (ID_AVIS (a)) != NULL) {
        AVIS_ISALACTIVE (ID_AVIS (a)) = FALSE;

        rhs = ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (a)));

        switch (NODE_TYPE (rhs)) {

        case N_id:
            res = CollectExprs (prf, rhs, sclprf);
            break;

        case N_prf:
            if (compatiblePrf (prf, PRF_PRF (rhs))) {
                left = CollectExprs (prf, PRF_ARG1 (rhs), isArg1Scl (PRF_PRF (rhs)));
                right = CollectExprs (prf, PRF_ARG2 (rhs), isArg2Scl (PRF_PRF (rhs)));
                res = TCappendExprs (left, right);
            } else if (IsGuardPrf (PRF_PRF (rhs))) {
                ids = LET_IDS (ASSIGN_STMT (AVIS_SSAASSIGN (ID_AVIS (a))));
                exprs = PRF_ARGS (rhs);
                while (IDS_AVIS (ids) != ID_AVIS (a) && exprs != NULL) {
                    ids = IDS_NEXT (ids);
                    exprs = EXPRS_NEXT (exprs);
                    DBUG_ASSERT (ids != NULL,
                                 "Syntax tree broken: "
                                 "AVIS must be found within IDS of AVIS_SSAASSIGN");
                }
                if (exprs != NULL && IDS_NEXT (ids) != NULL) {
                    /*
                     * we only continue through guards if we are in the transparent
                     * data flow through the guard, i.e. there is a right hand side
                     * corresponding expression and this is not the last left hand
                     * side identifier, i.e. not the guard's predicate.
                     */
                    DBUG_PRINT ("Ignoring guard: %s", global.prf_name[PRF_PRF (rhs)]);
                    DBUG_ASSERT (TYeqTypes (IDS_NTYPE (ids),
                                            ID_NTYPE (EXPRS_EXPR (exprs))),
                                 "Bug in guards: result id '%s' and arg id '%s' do have "
                                 "different types",
                                 IDS_NAME (ids), ID_NAME (EXPRS_EXPR (exprs)));
                    res = CollectExprs (prf, EXPRS_EXPR (exprs), sclprf);
                }
            }
            break;

        default:
            break;
        }
    }

    if (res == NULL) {
        res = TBmakeExprs (DUPdoDupNode (a), NULL);
        ID_ISSCLPRF (EXPRS_EXPR (res)) = sclprf;
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * Associativiy optimization traversal (al_tab)
 *
 * prefix: AL
 *
 *****************************************************************************/

node *
ALmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
ALfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node) != NULL && !FUNDEF_ISWRAPPERFUN (arg_node)) {
        DBUG_PRINT ("traversing body of %s", FUNDEF_NAME (arg_node));

        INFO_ISLOOPBODY (arg_info) = FUNDEF_ISLOOPFUN (arg_node);
        INFO_FUNDEF (arg_info) = arg_node;

        if (FUNDEF_ISLOOPFUN (arg_node)) {
            DBUG_ASSERT (FUNDEF_LOOPRECURSIVEAP (arg_node) != NULL,
                         "Loop fun found without RecursiveAp set: %s.",
                         FUNDEF_NAME (arg_node));

            INFO_RECURSIVEAPARGS (arg_info) = AP_ARGS (FUNDEF_LOOPRECURSIVEAP (arg_node));
            FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);

            DBUG_ASSERT (INFO_RECURSIVEAPARGS (arg_info) == NULL,
                         "Arity of loop function does not match arity of recursive call");
        } else {
            FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);
        }

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        INFO_FUNDEF (arg_info) = NULL;
        INFO_ISLOOPBODY (arg_info) = FALSE;
        DBUG_PRINT ("leaving body of %s", FUNDEF_NAME (arg_node));
    }

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
ALarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_ISLOOPBODY (arg_info)) {

        DBUG_ASSERT (INFO_RECURSIVEAPARGS (arg_info) != NULL,
                     "Arity of loop function does not match that of recursive call");

        if (ARG_AVIS (arg_node)
            == ID_AVIS (EXPRS_EXPR (INFO_RECURSIVEAPARGS (arg_info)))) {
            AVIS_ISLOOPINVARIANT (ARG_AVIS (arg_node)) = TRUE;
        } else {
            AVIS_ISLOOPINVARIANT (ARG_AVIS (arg_node)) = FALSE;
        }

        INFO_RECURSIVEAPARGS (arg_info) = EXPRS_NEXT (INFO_RECURSIVEAPARGS (arg_info));
    } else {
        AVIS_ISLOOPINVARIANT (ARG_AVIS (arg_node)) = FALSE;
    }

    ARG_NEXT (arg_node) = TRAVopt (ARG_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
ALblock (node *arg_node, info *arg_info)
{
    trav_mode_t old_mode;

    DBUG_ENTER ();

    old_mode = INFO_MODE (arg_info);

    INFO_MODE (arg_info) = MODE_recurse;
    DBUG_PRINT ("Traversing assignment chain, mode %d", INFO_MODE (arg_info));
    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

    INFO_MODE (arg_info) = MODE_mark;
    DBUG_PRINT ("Traversing assignment chain, mode %d", INFO_MODE (arg_info));
    INFO_WITHID (arg_info) = TRAVopt (INFO_WITHID (arg_info), arg_info);
    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);
    INFO_WITHID (arg_info) = TRAVopt (INFO_WITHID (arg_info), arg_info);

    INFO_MODE (arg_info) = old_mode;

    DBUG_RETURN (arg_node);
}

node *
ALassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    } else {
        DBUG_PRINT ("Reaching end of assignment chain, mode %d", INFO_MODE (arg_info));

        switch (INFO_MODE (arg_info)) {
        case MODE_recurse:
            INFO_MODE (arg_info) = MODE_noop;
            break;
        case MODE_mark:
            INFO_MODE (arg_info) = MODE_transform;
            break;
        default:
            DBUG_UNREACHABLE ("Illegal mode encountered at end of assign chain.");
            break;
        }
        DBUG_PRINT ("Reaching end of assignment chain, new mode %d",
                    INFO_MODE (arg_info));
    }

    if (INFO_MODE (arg_info) == MODE_transform) {
        ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

        if (INFO_PREASSIGN (arg_info) != NULL) {
            arg_node
              = TCappendAssign (revert (INFO_PREASSIGN (arg_info), NULL), arg_node);
            INFO_PREASSIGN (arg_info) = NULL;
        }
    }

    DBUG_RETURN (arg_node);
}

node *
ALlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    switch (INFO_MODE (arg_info)) {
    case MODE_recurse:
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
        break;
    case MODE_mark:
        LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);
        break;
    case MODE_transform:
        INFO_ISALCANDIDATE (arg_info) = FALSE;
        LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);
        if (INFO_ISALCANDIDATE (arg_info) && (NODE_TYPE (LET_EXPR (arg_node)) == N_prf)) {
            INFO_LHS (arg_info) = LET_IDS (arg_node);
            LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
            INFO_LHS (arg_info) = NULL;
        }
        break;
    case MODE_noop:
        break;
    }

    DBUG_RETURN (arg_node);
}

node *
ALids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    switch (INFO_MODE (arg_info)) {
    case MODE_mark:
        AVIS_ISDEFINEDINCURRENTBLOCK (IDS_AVIS (arg_node)) = TRUE;
        AVIS_ISALACTIVE (IDS_AVIS (arg_node)) = TRUE;
        break;
    case MODE_transform:
        if (AVIS_ISALACTIVE (IDS_AVIS (arg_node))) {
            INFO_ISALCANDIDATE (arg_info) = TRUE;
            AVIS_ISDEFINEDINCURRENTBLOCK (IDS_AVIS (arg_node)) = FALSE;
            AVIS_ISALACTIVE (IDS_AVIS (arg_node)) = FALSE;
        }
        break;
    default:
        break;
    }

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
ALwith (node *arg_node, info *arg_info)
{
    bool isloopbody, iswithloopbody;

    DBUG_ENTER ();

    isloopbody = INFO_ISLOOPBODY (arg_info);
    INFO_ISLOOPBODY (arg_info) = FALSE;

    iswithloopbody = INFO_ISWITHLOOPBODY (arg_info);
    INFO_ISWITHLOOPBODY (arg_info) = TRUE;

    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    /*
     * We do not traverse the N_code chain but follow the partitions.
     */

    INFO_ISLOOPBODY (arg_info) = isloopbody;
    INFO_ISWITHLOOPBODY (arg_info) = iswithloopbody;

    DBUG_RETURN (arg_node);
}

node *
ALpart (node *arg_node, info *arg_info)
{
    node *keep_withid;

    DBUG_ENTER ();

    keep_withid = INFO_WITHID (arg_info);
    INFO_WITHID (arg_info) = PART_WITHID (arg_node);
    PART_CODE (arg_node) = TRAVdo (PART_CODE (arg_node), arg_info);
    INFO_WITHID (arg_info) = keep_withid;

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
ALcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
    /*
     * We do not traverse the N_code chain but follow the partitions.
     * Hence, we need this function to not follow the next pointer.
     */

    DBUG_RETURN (arg_node);
}

node *
ALprf (node *arg_node, info *arg_info)
{
    ntype *ltype;
    prf prf;
    node *exprs, *tmp;
    node *consts, *scalars, *vectors;
    node *consts_id, *scalars_id, *vectors_id;
    node *scalars_inv, *scalars_inv_id, *vectors_inv, *vectors_inv_id;
    node *scalars_ext, *vectors_ext, *externals, *externals_id;

    DBUG_ENTER ();

    prf = PRF_PRF (arg_node);

    if ((INFO_MODE (arg_info) == MODE_transform) && (!isEqNeqPrf (prf))
        && ALisAssociativeAndCommutativePrf (prf)) {

        ltype = IDS_NTYPE (INFO_LHS (arg_info));

        if ((!global.enforce_ieee)
            || ((TYgetSimpleType (TYgetScalar (ltype)) != T_float)
                && (TYgetSimpleType (TYgetScalar (ltype)) != T_double))) {

            DBUG_PRINT ("Eligible start node found: %s", IDS_NAME (INFO_LHS (arg_info)));
            DBUG_PRINT ("Associative operator used: %s", global.prf_name[prf]);

            exprs
              = TCappendExprs (CollectExprs (prf, PRF_ARG1 (arg_node), isArg1Scl (prf)),
                               CollectExprs (prf, PRF_ARG2 (arg_node), isArg2Scl (prf)));

            if (EXPRS_EXPRS3 (exprs) == NULL) {
                DBUG_PRINT ("Giving up, operand set too short:");
                DBUG_EXECUTE (printOperands (exprs));

                exprs = FREEdoFreeTree (exprs);
                /*
                 * AL optimization only makes sense if the operand list
                 * consists of more than two elements
                 */
            } else {
                DBUG_PRINT ("Complete operand set:");
                DBUG_EXECUTE (printOperands (exprs));

                consts = TCfilterExprs (isConst, &exprs);
                scalars = TCfilterExprs (isNonConstScalar, &exprs);
                vectors = exprs;

                scalars_inv = NULL;

                if (!isSingletonOrEmpty (scalars)) {
                    if (isPrfAdd (prf)) {
                        scalars_inv = identifyInverses (F_neg_S, &scalars);
                    } else if (isPrfMul (prf)) {
                        scalars_inv = identifyInverses (F_reciproc_S, &scalars);
                    }
                }

                vectors_inv = NULL;

                if (!isSingletonOrEmpty (vectors)) {
                    if (isPrfAdd (prf)) {
                        vectors_inv = identifyInverses (F_neg_S, &vectors);
                    } else if (isPrfMul (prf)) {
                        vectors_inv = identifyInverses (F_reciproc_S, &vectors);
                    }
                }

                if (INFO_ISWITHLOOPBODY (arg_info)) {
                    /*
                     * Here, we try AL to enable with-loop invariant removal.
                     * At the end, we combine the lists of scalar and vector identifiers
                     * because loop invariant combinations of scalar and vector
                     * identifiers are also relevant for optimisation. We capitalise
                     * on the fact that Exprs2PrfTree used later to construct the tree
                     * does so element by element starting at the head of the list,
                     * which means we still combine all the scalar identifiers before
                     * combine the scalar result with potential further vector
                     * identifiers.
                     */
                    scalars_ext = TCfilterExprs (isNonLocal, &scalars);
                    vectors_ext = TCfilterExprs (isNonLocal, &vectors);
                    externals = TCappendExprs (scalars_ext, vectors_ext);
                    scalars_ext = NULL;
                    vectors_ext = NULL;
                } else if (INFO_ISLOOPBODY (arg_info)) {
                    /*
                     * Here, we try AL to enable loop invariant removal.
                     * At the end, we combine the lists of scalar and vector identifiers
                     * because loop invariant combinations of scalar and vector
                     * identifiers are also relevant for optimisation. We capitalise
                     * on the fact that Exprs2PrfTree used later to construct the tree
                     * does so element by element starting at the head of the list,
                     * which means we still combine all the scalar identifiers before
                     * combine the scalar result with potential further vector
                     * identifiers.
                     */
                    scalars_ext = TCfilterExprs (isLoopInvariant, &scalars);
                    vectors_ext = TCfilterExprs (isLoopInvariant, &vectors);
                    externals = TCappendExprs (scalars_ext, vectors_ext);
                    scalars_ext = NULL;
                    vectors_ext = NULL;
                } else {
                    scalars_ext = NULL;
                    vectors_ext = NULL;
                    externals = NULL;
                }

                DBUG_PRINT ("Constant operand set:");
                DBUG_EXECUTE (printOperands (consts));

                DBUG_PRINT ("(With-)Loop invariant operand set:");
                DBUG_EXECUTE (printOperands (externals));

                DBUG_PRINT ("Scalar neutralised operand set:");
                DBUG_EXECUTE (printOperands (scalars_inv));

                DBUG_PRINT ("Vector neutralised operand set:");
                DBUG_EXECUTE (printOperands (vectors_inv));

                if (!isSingletonOrEmpty (consts) || (scalars_inv != NULL)
                    || (vectors_inv != NULL) || !isSingletonOrEmpty (externals)) {

                    /*
                     * We only rewrite the multi-operand expression if we have
                     *  - 2 or more constants, allowing for constant folding;
                     *  - at least one pair of variable and its inverse among
                     *    - the scalar operands or
                     *    - the vector operands.
                     */

                    DBUG_PRINT ("AL optimisation !!.");

                    consts_id = Exprs2PrfTree (prf, consts, arg_info);
                    scalars_id = Exprs2PrfTree (prf, scalars, arg_info);
                    vectors_id = Exprs2PrfTree (prf, vectors, arg_info);
                    externals_id = Exprs2PrfTree (prf, externals, arg_info);

                    exprs = NULL;
                    exprs = TCcombineExprs (vectors_id, exprs);
                    exprs = TCcombineExprs (scalars_id, exprs);
                    exprs = TCcombineExprs (consts_id, exprs);
                    exprs = TCcombineExprs (externals_id, exprs);

                    while (scalars_inv != NULL) {
                        tmp = EXPRS_NEXT (EXPRS_NEXT (scalars_inv));
                        EXPRS_NEXT (EXPRS_NEXT (scalars_inv)) = NULL;
                        scalars_inv_id = Exprs2PrfTree (prf, scalars_inv, arg_info);
                        exprs = TCcombineExprs (scalars_inv_id, exprs);
                        scalars_inv = tmp;
                    }

                    while (vectors_inv != NULL) {
                        tmp = EXPRS_NEXT (EXPRS_NEXT (vectors_inv));
                        EXPRS_NEXT (EXPRS_NEXT (vectors_inv)) = NULL;
                        vectors_inv_id = Exprs2PrfTree (prf, vectors_inv, arg_info);
                        exprs = TCcombineExprs (vectors_inv_id, exprs);
                        vectors_inv = tmp;
                    }

                    arg_node = FREEdoFreeNode (arg_node);
                    arg_node = Exprs2PrfTree (prf, exprs, arg_info);

                    global.optcounters.al_expr++;
                } else {
                    /*
                     * We decided not to do any AL transformation, so we need
                     * to remove the various identifier lists.
                     */

                    DBUG_PRINT ("No AL optimisation !!.");

                    consts = FREEoptFreeTree(consts);
                    scalars = FREEoptFreeTree(scalars);
                    vectors = FREEoptFreeTree(vectors);
                    externals = FREEoptFreeTree(externals);
                }
            }
        }
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
