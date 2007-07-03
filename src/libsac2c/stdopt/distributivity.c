/*
 * $Id$
 */

#include "tree_basic.h"
#include "tree_compound.h"
#include "globals.h"
#include "new_types.h"
#include "new_typecheck.h"
#include "type_utils.h"
#include "traverse.h"
#include "dbug.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "DataFlowMask.h"
#include "DupTree.h"
#include "inferneedcounters.h"
#include "compare_tree.h"
#include "constants.h"
#include "shape.h"
#include "print.h"

#include "distributivity.h"

/*
 * INFO structure
 */
struct INFO {
    node *topblock;
    node *funargs;
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
#define INFO_TOPBLOCK(n) ((n)->topblock)
#define INFO_FUNARGS(n) ((n)->funargs)
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

    INFO_TOPBLOCK (result) = NULL;
    INFO_FUNARGS (result) = NULL;
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
 * @fn node *DISTRIBdoDistributivityOptimization( node *arg_node)
 *
 * @brief starting point of distributivity optimization
 *
 * @param arg_node
 *
 * @return
 *
 *****************************************************************************/

node *
DISTRIBdoDistributivityOptimization (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("DISTRIBdoDistributivityOptimization");

    info = MakeInfo ();

    TRAVpush (TR_assoc);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *DISTRIBdoDistributivityOptimizationOneFundef( node *arg_node)
 *
 * @brief starting point of distributivity optimization
 *
 * @param arg_node
 *
 * @return
 *
 *****************************************************************************/

node *
DISTRIBdoDistributivityOptimizationOneFundef (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("DISTRIBdoDistributivityOptimization");

    info = MakeInfo ();
    INFO_ONEFUNDEF (info) = TRUE;

    TRAVpush (TR_distrib);
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
IsAddPrf (prf prf)
{
    bool res;

    DBUG_ENTER ("IsAddPrf");

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

#ifdef TODO_REMOVE_IF_NOT_USED

static bool
IsMulPrf (prf prf)
{
    bool res;

    DBUG_ENTER ("IsMulPrf");

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

#endif

static bool
YieldsScalar (prf prf)
{
    bool res;

    DBUG_ENTER ("YieldsScalar");

    switch (prf) {
    case F_add_SxS:
    case F_mul_SxS:
        res = TRUE;
        break;
    case F_add_SxV:
    case F_add_VxS:
    case F_add_VxV:
    case F_mul_SxV:
    case F_mul_VxS:
    case F_mul_VxV:
        res = FALSE;
        break;
    default:
        DBUG_ASSERT (FALSE, "Illegal prf!");
    }

    DBUG_RETURN (res);
}

static bool
compatiblePrf (prf p1, prf p2)
{
    bool res;

    DBUG_ENTER ("compatiblePrf");

    res = (normalizePrf (p1) == normalizePrf (p2));

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
#if 1
        /*
         * This marking mechanism does not work. Since I haven't found
         * the bug in reasonable time, we now ask the type system directly.
         */
        res = ID_ISSCLPRF (n);
#else
        res = TUisScalar (ID_NTYPE (n));
#endif

        break;

    default:
        res = FALSE;
    }

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

static node *
consumeHead (node *mop)
{
    node *res;

    DBUG_ENTER ("consumeHead");

    res = PRF_ARG1 (mop);
    PRF_ARG1 (mop) = NULL;
    PRF_ARGS (mop) = FREEdoFreeNode (PRF_ARGS (mop));

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

    BLOCK_VARDEC (INFO_TOPBLOCK (arg_info))
      = TBmakeVardec (avis, BLOCK_VARDEC (INFO_TOPBLOCK (arg_info)));

    DFMupdateMaskBase (INFO_DFMBASE (arg_info), INFO_FUNARGS (arg_info),
                       BLOCK_VARDEC (INFO_TOPBLOCK (arg_info)));

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
Mop2Ast (node *mop, info *arg_info)
{
    node *res;

    DBUG_ENTER ("Mop2Ast");

    if (NODE_TYPE (mop) == N_prf) {
        if (TCcountExprs (PRF_ARGS (mop)) == 1) {
            res = consumeHead (mop);
            res = Mop2Ast (res, arg_info);
            mop = FREEdoFreeNode (mop);
        } else {
            prf prf;
            node *e1, *e2;
            prf = PRF_PRF (mop);
            e1 = consumeHead (mop);
            e1 = Mop2Ast (e1, arg_info);
            e2 = consumeHead (mop);
            e2 = Mop2Ast (e2, arg_info);
            PRF_ARGS (mop)
              = TBmakeExprs (CombineExprs2Prf (prf, e1, e2, arg_info), PRF_ARGS (mop));
            res = Mop2Ast (mop, arg_info);
        }
    } else {
        res = mop;
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

    if ((NODE_TYPE (a) == N_id) && (AVIS_SSAASSIGN (ID_AVIS (a)) != NULL)
        && (DFMtestMaskEntry (localmask, NULL, ID_AVIS (a)))
        && (AVIS_NEEDCOUNT (ID_AVIS (a)) == 1)) {

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

                res = FREEdoFreeTree (res);
                res = TCappendExprs (left, right);

                AVIS_NEEDCOUNT (ID_AVIS (a)) = 0;
            }
            break;

        default:
            break;
        }
    }
    DBUG_RETURN (res);
}

static node *
BuildMopTree (node *avis, bool sclavis, dfmask_t *localmask)
{
    node *tmp, *exprs;
    node *res;
    node *id;

    DBUG_ENTER ("BuildMopTree");

    id = TBmakeId (avis);
    exprs = CollectExprs (F_add_SxS, id, sclavis, localmask);
    FREEdoFreeNode (id);

    tmp = exprs;
    while (tmp != NULL) {
        node *mop;
        node *summand;

        summand = EXPRS_EXPR (tmp);
        mop = TBmakePrf (F_mul_SxS, CollectExprs (F_mul_SxS, summand, isScalar (summand),
                                                  localmask));

        EXPRS_EXPR (tmp) = FREEdoFreeNode (EXPRS_EXPR (tmp));
        EXPRS_EXPR (tmp) = mop;
        tmp = EXPRS_NEXT (tmp);
    }

    res = TBmakePrf (F_add_SxS, exprs);

    DBUG_RETURN (res);
}

static bool
isNotOne (node *n)
{
    bool res;

    DBUG_ENTER ("isNotOne");

    switch (NODE_TYPE (n)) {
    case N_float:
        res = (FLOAT_VAL (n) != 1.0f);
        break;

    case N_double:
        res = (DOUBLE_VAL (n) != 1.0);
        break;

    case N_num:
        res = (NUM_VAL (n) != 1);
        break;

    default:
        res = TRUE;
    }

    DBUG_RETURN (res);
}

static bool
ContainsFactor (node *factor, node *mop)
{
    bool res = FALSE;
    node *f;

    DBUG_ENTER ("ContainsFactor");

    f = PRF_ARGS (mop);
    while (f != NULL) {
        if (CMPTdoCompareTree (EXPRS_EXPR (f), factor) == CMPT_EQ) {
            res = TRUE;
            break;
        }
        f = EXPRS_NEXT (f);
    }

    DBUG_RETURN (res);
}

static bool
RemoveFactorOnce (node *factor, node *mop)
{
    bool res = FALSE;
    node **f;

    DBUG_ENTER ("RemoveFactorOnce");

    f = &PRF_ARGS (mop);
    while ((*f) != NULL) {
        if (CMPTdoCompareTree (EXPRS_EXPR ((*f)), factor) == CMPT_EQ) {
            *f = FREEdoFreeNode (*f);
            res = TRUE;
            break;
        }
        f = &EXPRS_NEXT ((*f));
    }

    DBUG_RETURN (res);
}

static node *
MostCommonFactor (node *mop)
{
    node *factors = NULL;
    node *s, *t, *f;
    node *mcf = NULL;
    int count = 1;

    DBUG_ENTER ("MostCommonFactor");

    /*
     * Collect all factors of all summands
     */
    s = PRF_ARGS (mop);
    while (s != NULL) {
        DBUG_ASSERT (NODE_TYPE (EXPRS_EXPR (s)) == N_prf, "MOP expected!");
        t = PRF_ARGS (EXPRS_EXPR (s));
        while (t != NULL) {
            if ((NODE_TYPE (EXPRS_EXPR (t)) != N_prf) && (isNotOne (EXPRS_EXPR (t)))) {
                factors = TBmakeExprs (DUPdoDupNode (EXPRS_EXPR (t)), factors);
            }
            t = EXPRS_NEXT (t);
        }

        s = EXPRS_NEXT (s);
    }

    /*
     * Find most common factor
     */
    f = factors;
    while (f != NULL) {
        int c = 0;
        s = PRF_ARGS (mop);
        while (s != NULL) {
            if (ContainsFactor (EXPRS_EXPR (f), EXPRS_EXPR (s))) {
                c = c + 1;
            }
            s = EXPRS_NEXT (s);
        }
        if (c > count) {
            mcf = EXPRS_EXPR (f);
            count = c;
        }
        f = EXPRS_NEXT (f);
    }

    /*
     * Prepare result and clean up
     */
    if (mcf != NULL) {
        mcf = DUPdoDupNode (mcf);
    }
    if (factors != NULL) {
        factors = FREEdoFreeTree (factors);
    }

    DBUG_RETURN (mcf);
}

static node *
SplitMop (node *mcf, node *mop)
{
    node *newmop;

    DBUG_ENTER ("SplitMop");

    if (PRF_ARGS (mop) == NULL) {
        newmop = TBmakePrf (F_add_SxS, NULL);
    } else {
        node *s = PRF_ARGS (mop);
        PRF_ARGS (mop) = EXPRS_NEXT (s);
        EXPRS_NEXT (s) = NULL;

        newmop = SplitMop (mcf, mop);

        if (RemoveFactorOnce (mcf, EXPRS_EXPR (s))) {
            EXPRS_NEXT (s) = PRF_ARGS (newmop);
            PRF_ARGS (newmop) = s;
        } else {
            EXPRS_NEXT (s) = PRF_ARGS (mop);
            PRF_ARGS (mop) = s;
        }
    }

    DBUG_RETURN (newmop);
}

static node *
OptimizeMop (node *mop)
{
    DBUG_ENTER ("OptimizeMop");

    if (NODE_TYPE (mop) == N_prf) {
        node *exprs = PRF_ARGS (mop);
        while (exprs != NULL) {
            EXPRS_EXPR (exprs) = OptimizeMop (EXPRS_EXPR (exprs));
            exprs = EXPRS_NEXT (exprs);
        }

        if (PRF_PRF (mop) == F_add_SxS) {
            node *mcf;

            mcf = MostCommonFactor (mop);

            if (mcf != NULL) {
                node *newexprs = TBmakeExprs (NULL, NULL);
                EXPRS_EXPR (newexprs) = SplitMop (mcf, mop);

                newexprs = TBmakeExprs (mcf, newexprs);

                PRF_ARGS (mop)
                  = TBmakeExprs (TBmakePrf (F_mul_SxS, newexprs), PRF_ARGS (mop));

                if (TCcountExprs (PRF_ARGS (mop)) == 1) {
                    node *newmop = PRF_ARG1 (mop);
                    PRF_ARG1 (mop) = NULL;
                    mop = FREEdoFreeTree (mop);
                    mop = newmop;
                }

                DBUG_EXECUTE ("DISTRIB", PRTdoPrint (mop););
                global.optcounters.dl_expr++;
                mop = OptimizeMop (mop);
            }
        } else {
            /*
             * PRF_PRF( mop) == F_mul_SxS
             */
            bool optimized = FALSE;
            node **exp = &PRF_ARGS (mop);
            while ((*exp) != NULL) {
                if ((NODE_TYPE (EXPRS_EXPR (*exp)) == N_prf)
                    && (PRF_PRF (EXPRS_EXPR (*exp)) == F_mul_SxS)) {
                    EXPRS_NEXT (*exp)
                      = TCappendExprs (EXPRS_NEXT (*exp), PRF_ARGS (EXPRS_EXPR (*exp)));
                    PRF_ARGS (EXPRS_EXPR (*exp)) = NULL;
                    *exp = FREEdoFreeNode (*exp);
                } else {
                    exp = &EXPRS_NEXT (*exp);
                }
            }

            if (optimized) {
                DBUG_EXECUTE ("DISTRIB", PRTdoPrint (mop););
                mop = OptimizeMop (mop);
            }
        }
    }

    DBUG_RETURN (mop);
}

static node *
EliminateEmptyProducts (node *mop, simpletype st)
{
    DBUG_ENTER ("EliminateEmptyProducts");

    if (NODE_TYPE (mop) == N_prf) {
        node *n;

        if (PRF_ARGS (mop) == NULL) {
            DBUG_ASSERT (PRF_PRF (mop) == F_mul_SxS, "Empty sum encountered!");

            constant *one = COmakeOne (st, SHmakeShape (0));
            PRF_ARGS (mop) = TBmakeExprs (COconstant2AST (one), NULL);
            one = COfreeConstant (one);
        }

        n = PRF_ARGS (mop);
        while (n != NULL) {
            EXPRS_EXPR (n) = EliminateEmptyProducts (EXPRS_EXPR (n), st);
            n = EXPRS_NEXT (n);
        }
    }

    DBUG_RETURN (mop);
}

/******************************************************************************
 *
 * Distributiviy optimization traversal (assoc_tab)
 *
 * prefix: DISTRIB
 *
 *****************************************************************************/
node *
DISTRIBfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DISTRIBfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        /*
         * Infer need counters
         */
        arg_node = INFNCdoInferNeedCountersOneFundef (arg_node);

        INFO_TOPBLOCK (arg_info) = FUNDEF_BODY (arg_node);
        INFO_FUNARGS (arg_info) = FUNDEF_ARGS (arg_node);
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
DISTRIBblock (node *arg_node, info *arg_info)
{
    dfmask_t *oldmask;

    DBUG_ENTER ("DISTRIBblock");

    oldmask = INFO_LOCALMASK (arg_info);
    INFO_LOCALMASK (arg_info) = DFMgenMaskClear (INFO_DFMBASE (arg_info));

    BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);

    INFO_LOCALMASK (arg_info) = DFMremoveMask (INFO_LOCALMASK (arg_info));
    INFO_LOCALMASK (arg_info) = oldmask;

    DBUG_RETURN (arg_node);
}

node *
DISTRIBassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DISTRIBassign");

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
DISTRIBlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DISTRIBlet");

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
DISTRIBids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DISTRIBids");

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
DISTRIBprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DISTRIBprf");

    if (IsAddPrf (PRF_PRF (arg_node))) {
        ntype *ltype = IDS_NTYPE (INFO_LHS (arg_info));

        if ((!global.enforce_ieee)
            || ((TYgetSimpleType (TYgetScalar (ltype)) != T_float)
                && (TYgetSimpleType (TYgetScalar (ltype)) != T_double))) {
            int oldoptcounter;
            node *mop;
            prf p;

            /*
             * Collect operands into multi-operation (sum of prducts)
             */
            p = PRF_PRF (arg_node);
            mop = BuildMopTree (IDS_AVIS (INFO_LHS (arg_info)), YieldsScalar (p),
                                INFO_LOCALMASK (arg_info));

            if (TCcountExprs (PRF_ARGS (mop)) >= 2) {
                DBUG_EXECUTE ("DISTRIB", PRTdoPrint (mop););

                /*
                 * Optimize multi-operation
                 */
                oldoptcounter = global.optcounters.dl_expr;
                mop = OptimizeMop (mop);

                if (oldoptcounter != global.optcounters.dl_expr) {
                    /*
                     * Eliminate empty products
                     */
                    mop = EliminateEmptyProducts (mop,
                                                  TYgetSimpleType (TYgetScalar (ltype)));

                    /*
                     * Convert mop back into ast representation
                     */
                    DBUG_EXECUTE ("DISTRIB", PRTdoPrint (mop););
                    arg_node = FREEdoFreeNode (arg_node);
                    arg_node = Mop2Ast (mop, arg_info);
                }
            } else {
                mop = FREEdoFreeNode (mop);
            }
        }
    }

    DBUG_RETURN (arg_node);
}
