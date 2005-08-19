/*
 * $Log$
 * Revision 1.1  2005/08/19 18:18:01  ktr
 * Initial revision
 *
 */
#include "tree_basic.h"
#include "tree_compound.h"
#include "globals.h"
#include "new_types.h"
#include "new_typecheck.h"
#include "traverse.h"
#include "dbug.h"
#include "internal_lib.h"
#include "optimize.h"
#include "free.h"
#include "DataFlowMask.h"
#include "DupTree.h"
#include "inferneedcounters.h"
#include "associativity.h"

/*
 * INFO structure
 */
struct INFO {
    node *topblock;
    dfmask_base_t *dfmbase;
    dfmask_t *localmask;
    node *preassign;
    enum { DIR_down, DIR_up } direction;
    bool travrhs;
    bool onefundef;
};

/*
 * INFO macros
 */
#define INFO_TOPBLOCK(n) ((n)->topblock)
#define INFO_DFMBASE(n) ((n)->dfmbase)
#define INFO_LOCALMASK(n) ((n)->localmask)
#define INFO_PREASSIGN(n) ((n)->preassign)
#define INFO_DIRECTION(n) ((n)->direction)
#define INFO_TRAVRHS(n) ((n)->travrhs)
#define INFO_ONEFUNDEF(n) ((n)->onefundef)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_TOPBLOCK (result) = NULL;
    INFO_DFMBASE (result) = NULL;
    INFO_LOCALMASK (result) = NULL;
    INFO_PREASSIGN (result) = NULL;
    INFO_DIRECTION (result) = DIR_down;
    INFO_TRAVRHS (result) = FALSE;
    INFO_ONEFUNDEF (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn node *ASSOCdoAssociativityOptimization( node *arg_node)
 *
 * @brief starting point of associativity optimization
 *
 * @param arg_node
 *
 * @return
 *
 *****************************************************************************/
node *
ASSOCdoAssociativityOptimization (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("ASSOCdoAssociativityOptimization");

    DBUG_PRINT ("OPT", ("Starting associativity optimization..."));

    info = MakeInfo ();

    TRAVpush (TR_assoc);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    DBUG_PRINT ("OPT", ("Associativity optimization complete."));

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *ASSOCdoAssociativityOptimizationOneFundef( node *arg_node)
 *
 * @brief starting point of associativity optimization
 *
 * @param arg_node
 *
 * @return
 *
 *****************************************************************************/
node *
ASSOCdoAssociativityOptimizationOneFundef (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("ASSOCdoAssociativityOptimization");

    info = MakeInfo ();
    INFO_ONEFUNDEF (info) = TRUE;

    DBUG_PRINT ("OPT", ("Starting associativity optimization..."));

    TRAVpush (TR_assoc);
    syntax_tree = TRAVdo (syntax_tree, NULL);
    TRAVpop ();

    DBUG_PRINT ("OPT", ("Associativity optimization complete."));

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
    case F_add_SxA:
    case F_add_AxS:
    case F_add_AxA:

    case F_mul_SxS:
    case F_mul_SxA:
    case F_mul_AxS:
    case F_mul_AxA:

    case F_min:
    case F_max:

    case F_or:
    case F_and:
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
    case F_add_SxA:
    case F_add_AxS:
    case F_add_AxA:
        prf = F_add_SxS;
        break;

    case F_mul_SxS:
    case F_mul_SxA:
    case F_mul_AxS:
    case F_mul_AxA:
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
    case F_add_SxA:
    case F_add_AxS:
    case F_add_AxA:
        if (s1) {
            if (s2)
                prf = F_add_SxS;
            else
                prf = F_add_SxA;
        } else {
            if (s2)
                prf = F_add_AxS;
            else
                prf = F_add_AxA;
        }
        break;

    case F_mul_SxS:
    case F_mul_SxA:
    case F_mul_AxS:
    case F_mul_AxA:
        if (s1) {
            if (s2)
                prf = F_mul_SxS;
            else
                prf = F_mul_SxA;
        } else {
            if (s2)
                prf = F_mul_AxS;
            else
                prf = F_mul_AxA;
        }

    case F_min:
    case F_max:
    case F_or:
    case F_and:
        break;

    default:
        DBUG_ASSERT (FALSE, "Illegal prf!");
    }

    DBUG_RETURN (prf);
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

    DBUG_ENTER ("CombineExprs2Prf");

    rhs = TCmakePrf2 (getPrf (prf, expr1, expr2), expr1, expr2);

    avis = TBmakeAvis (ILIBtmpVar (), NTCnewTypeCheck_Expr (rhs));

    BLOCK_VARDEC (INFO_TOPBLOCK (arg_info))
      = TBmakeVardec (avis, BLOCK_VARDEC (INFO_TOPBLOCK (arg_info)));

    assign = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), rhs), NULL);
    AVIS_SSAASSIGN (avis) = assign;

    INFO_PREASSIGN (arg_info) = TCappendAssign (INFO_PREASSIGN (arg_info), assign);

    DBUG_RETURN (TBmakeId (avis));
}

static bool
isArg1Scl (prf prf)
{
    bool res;
    DBUG_ENTER ("isArg1Scl");

    switch (prf) {
    case F_add_SxS:
    case F_add_SxA:
    case F_mul_SxS:
    case F_mul_SxA:
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
    case F_add_AxS:
    case F_mul_SxS:
    case F_mul_AxS:
        res = TRUE;
        break;
    default:
        res = FALSE;
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

    if ((NODE_TYPE (a) == N_id) && (AVIS_SSAASSIGN (ID_AVIS (a)) != NULL)
        && (DFMtestMaskEntry (localmask, NULL, ID_AVIS (a)))
        && (AVIS_NEEDCOUNT (ID_AVIS (a)) == 1)) {

        node *rhs = ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (a)));

        if (NODE_TYPE (rhs) == N_id) {
            res = CollectExprs (prf, rhs, sclprf, localmask);
            AVIS_NEEDCOUNT (ID_AVIS (a)) = 0;
        } else {
            if (NODE_TYPE (rhs) == N_prf) {
                if (compatiblePrf (prf, PRF_PRF (rhs))) {
                    res = TCappendAssign (CollectExprs (prf, PRF_ARG1 (rhs),
                                                        isArg1Scl (PRF_PRF (rhs)),
                                                        localmask),
                                          CollectExprs (prf, PRF_ARG2 (rhs),
                                                        isArg2Scl (PRF_PRF (rhs)),
                                                        localmask));
                    AVIS_NEEDCOUNT (ID_AVIS (a)) = 0;
                }
            } else {
                res = TBmakeExprs (DUPdoDupNode (a), NULL);
                ID_ISSCLPRF (EXPRS_EXPR (res)) = sclprf;
            }
        }
    } else {
        res = TBmakeExprs (DUPdoDupNode (a), NULL);
        ID_ISSCLPRF (EXPRS_EXPR (res)) = sclprf;
    }
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * Associativiy optimization traversal (assoc_tab)
 *
 * prefix: ASSOC
 *
 *****************************************************************************/
node *
ASSOCfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ASSOCfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        /*
         * Infer need counters
         */
        arg_node = INFNCdoInferNeedCountersOneFundef (arg_node);

        INFO_TOPBLOCK (arg_info) = FUNDEF_BODY (arg_node);
        INFO_DFMBASE (arg_info)
          = DFMgenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));

        if (FUNDEF_ARGS (arg_node) != NULL) {
            FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
        }

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
ASSOCblock (node *arg_node, info *arg_info)
{
    dfmask_t *oldmask;

    DBUG_ENTER ("ASSOCblock");

    oldmask = INFO_LOCALMASK (arg_info);
    INFO_LOCALMASK (arg_info) = DFMgenMaskClear (INFO_DFMBASE (arg_info));

    BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);

    INFO_LOCALMASK (arg_info) = DFMremoveMask (INFO_LOCALMASK (arg_info));
    INFO_LOCALMASK (arg_info) = oldmask;

    DBUG_RETURN (arg_node);
}

node *
ASSOCassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ASSOCassign");

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
        arg_node = TCappendAssign (INFO_PREASSIGN (arg_info), arg_node);
        INFO_PREASSIGN (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

node *
ASSOClet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ASSOClet");

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
            LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

node *
ASSOCids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ASSOCids");

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
ASSOCprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ASSOCprf");

    if (isAssociativeAndCommutativePrf (PRF_PRF (arg_node))) {
        prf p;
        node *exprs;
        node *consts, *nonconstscalars;
        node *constid, *ncsid, *arrayid;

        p = PRF_PRF (arg_node);

        exprs = TCappendExprs (CollectExprs (p, PRF_ARG1 (arg_node), isArg1Scl (p),
                                             INFO_LOCALMASK (arg_info)),
                               CollectExprs (p, PRF_ARG2 (arg_node), isArg2Scl (p),
                                             INFO_LOCALMASK (arg_info)));

        consts = TCfilterExprs (isConst, &exprs);
        nonconstscalars = TCfilterExprs (isNonConstScalar, &exprs);

        constid = Exprs2PrfTree (p, consts, arg_info);
        ncsid = Exprs2PrfTree (p, nonconstscalars, arg_info);
        arrayid = Exprs2PrfTree (p, exprs, arg_info);

        exprs = NULL;
        exprs = TCcombineExprs (arrayid, exprs);
        exprs = TCcombineExprs (ncsid, exprs);
        exprs = TCcombineExprs (constid, exprs);

        arg_node = FREEdoFreeNode (arg_node);
        arg_node = Exprs2PrfTree (p, exprs, arg_info);
    }

    DBUG_RETURN (arg_node);
}
