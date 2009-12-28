/*
 * $Id$
 */

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
#include "DataFlowMask.h"
#include "DupTree.h"
#include "inferneedcounters.h"
#include "pattern_match.h"

#include "associative_law.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    dfmask_base_t *dfmbase;
    dfmask_t *localmask;
    node *preassign;
    enum { DIR_down, DIR_up } direction;
    bool travrhs;
    bool onefundef;
    node *lhs;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_DFMBASE(n) ((n)->dfmbase)
#define INFO_LOCALMASK(n) ((n)->localmask)
#define INFO_PREASSIGN(n) ((n)->preassign)
#define INFO_DIRECTION(n) ((n)->direction)
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
    INFO_DFMBASE (result) = NULL;
    INFO_LOCALMASK (result) = NULL;
    INFO_PREASSIGN (result) = NULL;
    INFO_DIRECTION (result) = DIR_down;
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
ALdoAssocLawOptimizationModule (node *arg_node)
{
    info *info;

    DBUG_ENTER ("ALdoAssocLawOptimizationModule");

    info = MakeInfo ();
    INFO_ONEFUNDEF (info) = FALSE;

    TRAVpush (TR_al);
    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ALdoAssocLawOptimizationOneFundef( node *arg_node)
 *
 * @brief starting point of associativity optimization
 *
 * @param arg_node
 *
 * @return
 *
 *****************************************************************************/

node *
ALdoAssocLawOptimizationOneFundef (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("ALdoAssocLawOptimization");

    info = MakeInfo ();
    INFO_ONEFUNDEF (info) = TRUE;

    TRAVpush (TR_al);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
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
CollectExprs (prf prf, node *a, bool sclprf, dfmask_t *localmask)
{
    node *res = NULL;

    DBUG_ENTER ("CollectExprs");

    res = TBmakeExprs (DUPdoDupNode (a), NULL);
    if (NODE_TYPE (EXPRS_EXPR (res)) == N_id) {
        ID_ISSCLPRF (EXPRS_EXPR (res)) = sclprf;
    }

    if ((NODE_TYPE (a) == N_id) && (DFMtestMaskEntry (localmask, NULL, ID_AVIS (a)))
        && (AVIS_SSAASSIGN (ID_AVIS (a)) != NULL)) {

        node *rhs = ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (a)));

        switch (NODE_TYPE (rhs)) {

        case N_id:
            res = FREEdoFreeTree (res);
            res = CollectExprs (prf, rhs, sclprf, localmask);
            AVIS_NEEDCOUNT (ID_AVIS (a)) = 0;
            break;

        case N_prf:
            if (compatiblePrf (prf, PRF_PRF (rhs))) {
                node *left, *right;

                left = CollectExprs (prf, PRF_ARG1 (rhs), isArg1Scl (PRF_PRF (rhs)),
                                     localmask);
                right = CollectExprs (prf, PRF_ARG2 (rhs), isArg2Scl (PRF_PRF (rhs)),
                                      localmask);
#if 0
        if (  !isSingleton( left) || !isSingleton( right) ||
              !eqClass( EXPRS_EXPR( left), EXPRS_EXPR( right))) {
          res = FREEdoFreeTree( res);
          res = TCappendExprs( left, right);
        }
        else {
          left = FREEdoFreeTree( left);
          right = FREEdoFreeTree( right);
        }
#else
                res = FREEdoFreeTree (res);
                res = TCappendExprs (left, right);
#endif
                AVIS_NEEDCOUNT (ID_AVIS (a)) = 0;
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
 * Associativiy optimization traversal (assoc_tab)
 *
 * prefix: AL
 *
 *****************************************************************************/
node *
ALfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ALfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        /*
         * Infer need counters
         */
        arg_node = INFNCdoInferNeedCountersOneFundef (arg_node, TR_al);

        INFO_FUNDEF (arg_info) = arg_node;
        INFO_DFMBASE (arg_info)
          = DFMgenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        INFO_DFMBASE (arg_info) = DFMremoveMaskBase (INFO_DFMBASE (arg_info));
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
    dfmask_t *oldmask;

    DBUG_ENTER ("ALblock");

    oldmask = INFO_LOCALMASK (arg_info);
    INFO_LOCALMASK (arg_info) = DFMgenMaskClear (INFO_DFMBASE (arg_info));

    BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);

    INFO_LOCALMASK (arg_info) = DFMremoveMask (INFO_LOCALMASK (arg_info));
    INFO_LOCALMASK (arg_info) = oldmask;

    DBUG_RETURN (arg_node);
}

node *
ALassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ALassign");

    /*
     * Traverse LHS identifiers to mark them as local in the current block
     */
    INFO_DIRECTION (arg_info) = DIR_down;
    ASSIGN_INSTR (arg_node) = TRAVopt (ASSIGN_INSTR (arg_node), arg_info);

    /*
     * Bottom-up traversal
     */
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    INFO_DIRECTION (arg_info) = DIR_up;
    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_PREASSIGN (arg_info) != NULL) {
        arg_node = TCappendAssign (revert (INFO_PREASSIGN (arg_info), NULL), arg_node);
        INFO_PREASSIGN (arg_info) = NULL;
        global.optcounters.al_expr++;
    }

    DBUG_RETURN (arg_node);
}

node *
ALlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ALlet");

    if (INFO_DIRECTION (arg_info) == DIR_down) {
        LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);
    } else {
        INFO_TRAVRHS (arg_info) = FALSE;
        LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);
        if (INFO_TRAVRHS (arg_info)) {
            INFO_LHS (arg_info) = LET_IDS (arg_node);
            LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

node *
ALids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ALids");

    if (INFO_DIRECTION (arg_info) == DIR_down) {
        DFMsetMaskEntrySet (INFO_LOCALMASK (arg_info), NULL, IDS_AVIS (arg_node));
    } else {
        if (AVIS_NEEDCOUNT (IDS_AVIS (arg_node)) != 0) {
            INFO_TRAVRHS (arg_info) = TRUE;
        }
    }

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
ALprf (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("ALprf");

    if (isAssociativeAndCommutativePrf (PRF_PRF (arg_node))) {
        ntype *ltype = IDS_NTYPE (INFO_LHS (arg_info));
        if ((!global.enforce_ieee)
            || ((TYgetSimpleType (TYgetScalar (ltype)) != T_float)
                && (TYgetSimpleType (TYgetScalar (ltype)) != T_double))) {
            prf prf;
            node *exprs, *tmp;
            node *consts, *ncss;
            node *constid, *ncsid, *arrayid;
            node *ncss_inv, *ncss_inv_id, *array_inv, *array_inv_id;

            prf = PRF_PRF (arg_node);

            exprs
              = TCappendExprs (CollectExprs (prf, PRF_ARG1 (arg_node), isArg1Scl (prf),
                                             INFO_LOCALMASK (arg_info)),
                               CollectExprs (prf, PRF_ARG2 (arg_node), isArg2Scl (prf),
                                             INFO_LOCALMASK (arg_info)));

            /*
             * The optimization can only be performed if the combined expression
             * consists of more than two elements
             */
            if (EXPRS_EXPRS3 (exprs) != NULL) {
                consts = TCfilterExprs (isConst, &exprs);
                ncss = TCfilterExprs (isNonConstScalar, &exprs);

                if ((isSingletonOrEmpty (exprs)) && (isSingletonOrEmpty (consts))
                    && (isSingletonOrEmpty (ncss))) {

                    if (exprs != NULL)
                        exprs = FREEdoFreeTree (exprs);
                    if (consts != NULL)
                        consts = FREEdoFreeTree (consts);
                    if (ncss != NULL)
                        ncss = FREEdoFreeTree (ncss);
                } else {
                    if (isPrfAdd (prf)) {
                        ncss_inv = identifyInverses (F_neg_S, &ncss);
                        array_inv = identifyInverses (F_neg_S, &exprs);
                    } else if (isPrfMul (prf)) {
                        ncss_inv = identifyInverses (F_reciproc_S, &ncss);
                        array_inv = identifyInverses (F_reciproc_S, &exprs);
                    } else {
                        ncss_inv = NULL;
                        array_inv = NULL;
                    }

                    constid = Exprs2PrfTree (prf, consts, arg_info);
                    ncsid = Exprs2PrfTree (prf, ncss, arg_info);
                    arrayid = Exprs2PrfTree (prf, exprs, arg_info);

                    exprs = NULL;
                    exprs = TCcombineExprs (arrayid, exprs);
                    exprs = TCcombineExprs (ncsid, exprs);
                    exprs = TCcombineExprs (constid, exprs);

                    while (ncss_inv != NULL) {
                        tmp = EXPRS_NEXT (EXPRS_NEXT (ncss_inv));
                        EXPRS_NEXT (EXPRS_NEXT (ncss_inv)) = NULL;
                        ncss_inv_id = Exprs2PrfTree (prf, ncss_inv, arg_info);
                        exprs = TCcombineExprs (ncss_inv_id, exprs);
                        ncss_inv = tmp;
                    }

                    while (array_inv != NULL) {
                        tmp = EXPRS_NEXT (EXPRS_NEXT (array_inv));
                        EXPRS_NEXT (EXPRS_NEXT (array_inv)) = NULL;
                        array_inv_id = Exprs2PrfTree (prf, array_inv, arg_info);
                        exprs = TCcombineExprs (array_inv_id, exprs);
                        array_inv = tmp;
                    }

                    arg_node = FREEdoFreeNode (arg_node);
                    arg_node = Exprs2PrfTree (prf, exprs, arg_info);

                    /*
                     * update the maskbase
                     */
                    INFO_DFMBASE (arg_info)
                      = DFMupdateMaskBase (INFO_DFMBASE (arg_info),
                                           FUNDEF_ARGS (INFO_FUNDEF (arg_info)),
                                           FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));
                }
            } else {
                exprs = FREEdoFreeTree (exprs);
            }
        }
    }

    DBUG_RETURN (arg_node);
}
