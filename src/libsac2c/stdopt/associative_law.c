/*
 * $Id$
 */

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
 * This is then split into constants, non-constant scalars, and
 * non-constant vectors ( This ordering minimizes the work to
 * compute the final result.):
 *
 * s = +(+(1,1),+(a,b),+(b,d))
 *
 * Finally, this gets transformed back into a piece of AST:
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
 *****************************************************************************/

#include "tree_basic.h"
#include "tree_compound.h"
#include "globals.h"
#include "new_types.h"
#include "new_typecheck.h"
#include "traverse.h"
#include "dbug.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "DupTree.h"
#include "pattern_match.h"

#include "associative_law.h"

/*
 * INFO structure
 */

typedef enum { MODE_recurse, MODE_mark, MODE_transform, MODE_noop } trav_mode_t;

struct INFO {
    trav_mode_t mode;
    node *fundef;
    node *preassign;
    bool travrhs;
    bool onefundef;
    node *lhs;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_PREASSIGN(n) ((n)->preassign)
#define INFO_MODE(n) ((n)->mode)
#define INFO_TRAVRHS(n) ((n)->travrhs)
#define INFO_ONEFUNDEF(n) ((n)->onefundef)
#define INFO_LHS(n) ((n)->lhs)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_PREASSIGN (result) = NULL;
    INFO_MODE (result) = MODE_noop;
    INFO_TRAVRHS (result) = FALSE;
    INFO_ONEFUNDEF (result) = FALSE;
    INFO_LHS (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn node *ALdoAssocLawOptimizationModule( node *arg_node)
 *
 * @brief starting point of associativity optimization
 *
 * @param arg_node: An N_module node
 *
 * @return
 *
 *****************************************************************************/

node *
ALdoAssocLawOptimization (node *arg_node)
{
    info *info;

    DBUG_ENTER ("ALdoAssocLawOptimization");

    info = MakeInfo ();

    switch (NODE_TYPE (arg_node)) {
    case N_module:
        INFO_ONEFUNDEF (info) = FALSE;
        break;
    case N_fundef:
        INFO_ONEFUNDEF (info) = TRUE;
        break;
    default:
        DBUG_ASSERT (FALSE, "ALdoAssocLawOptimization called with illegal node type.");
    }

    TRAVpush (TR_al);
    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ALdoAssocLawOptimizationOneFundefAnon( node *arg_node,
 *                                                  info *arg_info)
 *
 * @brief starting point of associativity optimization for
 *        a single function, invoked by anonymous traversal
 *
 * @param arg_node: An N_fundef node
 *        arg_info: Ignored; present only to placate anonymous traversal.
 *
 * @return arg_node
 *
 *****************************************************************************/

node *
ALdoAssocLawOptimizationOneFundefAnon (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("ALdoAssocLawOptimizationOneFundefAnon");

    arg_node = ALdoAssocLawOptimization (arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Helper functions
 *
 *****************************************************************************/

static prf
AltPrf (prf op)
{
    prf res;

    DBUG_ENTER ("AltPrf");

    switch (op) {
    case F_neg_S:
        res = F_neg_V;
        break;
    case F_reciproc_S:
        res = F_reciproc_V;
        break;
    default:
        res = F_noop;
        DBUG_ASSERT (FALSE, "We should never get here.");
    }

    DBUG_RETURN (res);
}

static node *
getInverse (prf prf, node *exprs)
{
    node *var, *res;
    pattern *pat;

    DBUG_ENTER ("getInverse");

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

    DBUG_ENTER ("getElement");

    res = ID_AVIS (EXPRS_EXPR (exprs));

    DBUG_ASSERT (NODE_TYPE (res) == N_avis, "Result has wrong node type.");

    DBUG_RETURN (res);
}

static node *
identifyInverses (prf inverse_prf, node **head)
{
    node *left, *right, *res, *tmp, *left_last, *right_last;
    node *left_inv, *right_inv, *left_elem, *right_elem;

    DBUG_ENTER ("identifyInverses");

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

static bool
isAssociativeAndCommutativePrf (prf prf)
{
    bool res;

    DBUG_ENTER ("isAssociativeAndCommutativePrf");

    switch (prf) {
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

    DBUG_ENTER ("isPrfAdd");

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

    DBUG_ENTER ("isPrfMul");

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

    DBUG_ENTER ("isConst");

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

    DBUG_ENTER ("isNonConstScalar");

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

    DBUG_ENTER ("isScalar");

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

static prf
normalizePrf (prf prf)
{
    DBUG_ENTER ("normalizePrf");

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

    DBUG_ENTER ("compatiblePrf");

    res = (normalizePrf (p1) == normalizePrf (p2));

    DBUG_RETURN (res);
}

static prf
getPrf (prf prf, node *e1, node *e2)
{
    bool s1, s2;

    DBUG_ENTER ("getPrf");

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
        DBUG_ASSERT (FALSE, "Illegal prf!");
    }

    DBUG_RETURN (prf);
}

static bool
isArg1Scl (prf prf)
{
    bool res;
    DBUG_ENTER ("isArg1Scl");

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
    DBUG_ENTER ("isArg2Scl");

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

    DBUG_ENTER ("isSingleton");

    res = ((expr == NULL) || (EXPRS_NEXT (expr) == NULL));

    DBUG_RETURN (res);
}

static node *
consumeHead (node **exprs)
{
    node *res;

    DBUG_ENTER ("consumeHead");

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

    DBUG_ENTER ("CombineExprs2Prf");

    rhs = TCmakePrf2 (getPrf (prf, expr1, expr2), expr1, expr2);

    prod = NTCnewTypeCheck_Expr (rhs);
    avis = TBmakeAvis (TRAVtmpVar (), TYcopyType (TYgetProductMember (prod, 0)));
    prod = TYfreeType (prod);

    FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
      = TBmakeVardec (avis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

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

    DBUG_ENTER ("revert");

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

    DBUG_ENTER ("Exprs2PrfTree");

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

    DBUG_ENTER ("CollectExprs");

    DBUG_ASSERT (NODE_TYPE (a) == N_id, "CollectExprs called with illegal node type");

    res = TBmakeExprs (DUPdoDupNode (a), NULL);

    ID_ISSCLPRF (EXPRS_EXPR (res)) = sclprf;

    if (AVIS_ISACTIVE (ID_AVIS (a))) {
        AVIS_ISACTIVE (ID_AVIS (a)) = FALSE;

        rhs = ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (a)));

        switch (NODE_TYPE (rhs)) {

        case N_id:
            res = FREEdoFreeTree (res);
            res = CollectExprs (prf, rhs, sclprf);
            break;

        case N_prf:
            if (compatiblePrf (prf, PRF_PRF (rhs))) {

                left = CollectExprs (prf, PRF_ARG1 (rhs), isArg1Scl (PRF_PRF (rhs)));
                right = CollectExprs (prf, PRF_ARG2 (rhs), isArg2Scl (PRF_PRF (rhs)));
#if 0
        if (  !isSingleton( left) || !isSingleton( right) ||
              !eqClass( EXPRS_EXPR( left), EXPRS_EXPR( right))) {
          res = FREEdoFreeTree( res);
          res = TCappendExprs( left, right);
        } else {
          left = FREEdoFreeTree( left);
          right = FREEdoFreeTree( right);
        }
#else
                res = FREEdoFreeTree (res);
                res = TCappendExprs (left, right);
#endif
            }
            break;

        default:
            break;
        }
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
    DBUG_ENTER ("ALmodule");

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
ALfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ALfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        INFO_FUNDEF (arg_info) = NULL;
    }

    if (!INFO_ONEFUNDEF (arg_info)) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
ALblock (node *arg_node, info *arg_info)
{
    trav_mode_t old_mode;

    DBUG_ENTER ("ALblock");

    old_mode = INFO_MODE (arg_info);

    INFO_MODE (arg_info) = MODE_recurse;
    DBUG_PRINT ("AL", ("Traversing assignment chain, mode %d", INFO_MODE (arg_info)));
    BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);

    INFO_MODE (arg_info) = MODE_mark;
    DBUG_PRINT ("AL", ("Traversing assignment chain, mode %d", INFO_MODE (arg_info)));
    BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);

    INFO_MODE (arg_info) = old_mode;

    DBUG_RETURN (arg_node);
}

node *
ALassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ALassign");

    ASSIGN_INSTR (arg_node) = TRAVopt (ASSIGN_INSTR (arg_node), arg_info);

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    } else {
        DBUG_PRINT ("AL",
                    ("Reaching end of assignment chain, mode %d", INFO_MODE (arg_info)));

        switch (INFO_MODE (arg_info)) {
        case MODE_recurse:
            INFO_MODE (arg_info) = MODE_noop;
            break;
        case MODE_mark:
            INFO_MODE (arg_info) = MODE_transform;
            break;
        default:
            DBUG_ASSERT (FALSE, "Illegal mode encountered at end of assign chain.");
            break;
        }
        DBUG_PRINT ("AL", ("Reaching end of assignment chain, new mode %d",
                           INFO_MODE (arg_info)));
    }

    if (INFO_MODE (arg_info) == MODE_transform) {
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

        if (INFO_PREASSIGN (arg_info) != NULL) {
            DBUG_PRINT ("AL", ("AL optimisation successful !!."));
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
    DBUG_ENTER ("ALlet");

    switch (INFO_MODE (arg_info)) {
    case MODE_recurse:
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
        break;
    case MODE_mark:
        LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);
        break;
    case MODE_transform:
        INFO_TRAVRHS (arg_info) = FALSE;
        LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);
        if (INFO_TRAVRHS (arg_info) && (NODE_TYPE (LET_EXPR (arg_node)) == N_prf)) {
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
    DBUG_ENTER ("ALids");

    switch (INFO_MODE (arg_info)) {
    case MODE_mark:
        AVIS_ISACTIVE (IDS_AVIS (arg_node)) = TRUE;
        break;
    case MODE_transform:
        if (AVIS_ISACTIVE (IDS_AVIS (arg_node))) {
            INFO_TRAVRHS (arg_info) = TRUE;
            AVIS_ISACTIVE (IDS_AVIS (arg_node)) = FALSE;
        }
        break;
    default:
        break;
    }

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
ALprf (node *arg_node, info *arg_info)
{
    ntype *ltype;
    prf prf;
    node *exprs, *tmp;
    node *consts, *scalars, *vects;
    node *consts_id, *scalars_id, *vects_id;
    node *scalars_inv, *scalars_inv_id, *vects_inv, *vects_inv_id;

    DBUG_ENTER ("ALprf");

    prf = PRF_PRF (arg_node);

    if ((INFO_MODE (arg_info) == MODE_transform)
        && isAssociativeAndCommutativePrf (prf)) {
        DBUG_PRINT ("AL", ("Eligible prf node found: %s", global.prf_name[prf]));

        ltype = IDS_NTYPE (INFO_LHS (arg_info));

        if ((!global.enforce_ieee)
            || ((TYgetSimpleType (TYgetScalar (ltype)) != T_float)
                && (TYgetSimpleType (TYgetScalar (ltype)) != T_double))) {

            exprs
              = TCappendExprs (CollectExprs (prf, PRF_ARG1 (arg_node), isArg1Scl (prf)),
                               CollectExprs (prf, PRF_ARG2 (arg_node), isArg2Scl (prf)));

            DBUG_PRINT ("AL", ("Operand set:"));
            DBUG_EXECUTE ("AL", {
                node *tmp = exprs;
                while (tmp != NULL) {
                    DBUG_PRINT ("AL", ("%s ", AVIS_NAME (ID_AVIS (EXPRS_EXPR (tmp)))));
                    tmp = EXPRS_NEXT (tmp);
                }
            });

            /*
             * The optimization can only be performed if the combined expression
             * consists of more than two elements
             */
            if (EXPRS_EXPRS3 (exprs) != NULL) {
                consts = TCfilterExprs (isConst, &exprs);
                scalars = TCfilterExprs (isNonConstScalar, &exprs);
                vects = exprs;

                scalars_inv = NULL;

                if (!isSingletonOrEmpty (scalars)) {
                    if (isPrfAdd (prf)) {
                        scalars_inv = identifyInverses (F_neg_S, &scalars);
                    } else if (isPrfMul (prf)) {
                        scalars_inv = identifyInverses (F_reciproc_S, &scalars);
                    }
                }

                vects_inv = NULL;

                if (!isSingletonOrEmpty (vects)) {
                    if (isPrfAdd (prf)) {
                        vects_inv = identifyInverses (F_neg_S, &vects);
                    } else if (isPrfMul (prf)) {
                        vects_inv = identifyInverses (F_reciproc_S, &vects);
                    }
                }

                if (!isSingletonOrEmpty (consts) || (scalars_inv != NULL)
                    || (vects_inv != NULL)) {

                    /*
                     * We only rewrite the multi-operand expression if we have
                     *  - 2 or more constants, allowing for constant folding;
                     *  - at least one pair of variable and its inverse among
                     *    - the scalar operands or
                     *    - the vector operands.
                     */

                    consts_id = Exprs2PrfTree (prf, consts, arg_info);
                    scalars_id = Exprs2PrfTree (prf, scalars, arg_info);
                    vects_id = Exprs2PrfTree (prf, vects, arg_info);

                    exprs = NULL;
                    exprs = TCcombineExprs (vects_id, exprs);
                    exprs = TCcombineExprs (scalars_id, exprs);
                    exprs = TCcombineExprs (consts_id, exprs);

                    while (scalars_inv != NULL) {
                        tmp = EXPRS_NEXT (EXPRS_NEXT (scalars_inv));
                        EXPRS_NEXT (EXPRS_NEXT (scalars_inv)) = NULL;
                        scalars_inv_id = Exprs2PrfTree (prf, scalars_inv, arg_info);
                        exprs = TCcombineExprs (scalars_inv_id, exprs);
                        scalars_inv = tmp;
                    }

                    while (vects_inv != NULL) {
                        tmp = EXPRS_NEXT (EXPRS_NEXT (vects_inv));
                        EXPRS_NEXT (EXPRS_NEXT (vects_inv)) = NULL;
                        vects_inv_id = Exprs2PrfTree (prf, vects_inv, arg_info);
                        exprs = TCcombineExprs (vects_inv_id, exprs);
                        vects_inv = tmp;
                    }

                    arg_node = FREEdoFreeNode (arg_node);
                    arg_node = Exprs2PrfTree (prf, exprs, arg_info);

                    global.optcounters.al_expr++;
                } else {
                    if (consts != NULL) {
                        consts = FREEdoFreeTree (consts);
                    }
                    if (scalars != NULL) {
                        scalars = FREEdoFreeTree (scalars);
                    }
                    if (vects != NULL) {
                        vects = FREEdoFreeTree (vects);
                    }
                }
            } else {
                exprs = FREEdoFreeTree (exprs);
            }
        }
    }

    DBUG_RETURN (arg_node);
}
