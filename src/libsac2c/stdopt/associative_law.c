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
 * @fn node *ALdoAssocLawOptimization( node *arg_node)
 *
 * @brief starting point of associativity optimization
 *
 * @param arg_node
 *
 * @return
 *
 *****************************************************************************/

node *
ALdoAssocLawOptimization (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("ALdoAssocLawOptimization");

    info = MakeInfo ();

    TRAVpush (TR_al);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
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
isConst (node *n)
{
    bool res;

    DBUG_ENTER ("isConst");

    switch (NODE_TYPE (n)) {
    case N_char:
    case N_bool:
    case N_double:
    case N_float:
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
isNonConstArray (node *n)
{
    bool res;

    DBUG_ENTER ("isNonConstArray");

    res = !(isConst (n) || isNonConstScalar (n));

    DBUG_RETURN (res);
}

static bool
isScalar (node *n)
{
    bool res;

    DBUG_ENTER ("isScalar");

    switch (NODE_TYPE (n)) {
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

static bool
isSingleton (node *expr)
{
    bool res;

    DBUG_ENTER ("isSingleton");

    res = ((expr != NULL) && (EXPRS_NEXT (expr) == NULL));

    DBUG_RETURN (res);
}

static bool
eqClass (node *expr1, node *expr2)
{
    bool res;

    DBUG_ENTER ("eqClass");

    res = ((isConst (expr1) && isConst (expr2))
           || (isNonConstScalar (expr1) && isNonConstScalar (expr2))
           || (isNonConstArray (expr1) && isNonConstArray (expr2)));

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
                if (!isSingleton (left) || !isSingleton (right)
                    || !eqClass (EXPRS_EXPR (left), EXPRS_EXPR (right))) {
                    res = FREEdoFreeTree (res);
                    res = TCappendExprs (left, right);
                } else {
                    left = FREEdoFreeTree (left);
                    right = FREEdoFreeTree (right);
                }
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
        arg_node = INFNCdoInferNeedCountersOneFundef (arg_node);

        INFO_FUNDEF (arg_info) = arg_node;
        INFO_DFMBASE (arg_info)
          = DFMgenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        INFO_DFMBASE (arg_info) = DFMremoveMaskBase (INFO_DFMBASE (arg_info));
    }

    if (!INFO_ONEFUNDEF (arg_info)) {
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        }
    }

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
    if (ASSIGN_INSTR (arg_node) != NULL) {
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    }

    /*
     * Bottom-up traversal
     */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

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
        if (LET_IDS (arg_node) != NULL) {
            LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
        }
    } else {
        INFO_TRAVRHS (arg_info) = FALSE;
        if (LET_IDS (arg_node) != NULL) {
            LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
        }
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

    if (IDS_NEXT (arg_node) != NULL) {
        IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
    }

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
            prf p;
            node *exprs;
            node *consts, *ncss;
            node *constid, *ncsid, *arrayid;

            p = PRF_PRF (arg_node);

            exprs = TCappendExprs (CollectExprs (p, PRF_ARG1 (arg_node), isArg1Scl (p),
                                                 INFO_LOCALMASK (arg_info)),
                                   CollectExprs (p, PRF_ARG2 (arg_node), isArg2Scl (p),
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
                    constid = Exprs2PrfTree (p, consts, arg_info);
                    ncsid = Exprs2PrfTree (p, ncss, arg_info);
                    arrayid = Exprs2PrfTree (p, exprs, arg_info);

                    exprs = NULL;
                    exprs = TCcombineExprs (arrayid, exprs);
                    exprs = TCcombineExprs (ncsid, exprs);
                    exprs = TCcombineExprs (constid, exprs);

                    arg_node = FREEdoFreeNode (arg_node);
                    arg_node = Exprs2PrfTree (p, exprs, arg_info);

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
