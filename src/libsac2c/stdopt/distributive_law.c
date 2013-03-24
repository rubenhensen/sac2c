/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @file distributive_law.c
 *
 * Prefix: DL
 *
 * This optimization attempts to reduce the number of multiplications by
 * applying the distributive law, i.e.,  ( a * b) + ( a * c) is transformed
 * into a * ( b + c).
 * To have the best possible impact, this optimisation implicitly applies
 * commutative law if that facilitates further applications.
 *
 *
 *
 * Overall Approach:
 * ==================
 *
 * The overall approach is as follows:
 * First, compound expressions of the form
 *
 *    ( s_1 + ... + s_n)        n >= 2
 *
 * where
 *
 *    s_i = ( e_1 * ... * e_m_i)  or  f_i = e
 *
 * are identified and stored in an optimisation-specific structure.
 * The function              BuildMopTree
 * is responsible for this.
 *
 * Once such a structure has been created, the function   OptimizeMop
 * is responsible for converting it into the compound expression with
 * potentially fewer multiplications. This is done by repeatedly
 *  1) identifying the most frequently used factor found in ALL summands
 *     s_1 ... s_n  (MostCommonFactor)
 *  2) pulling out those summands that contain that very factor and,
 *     while doing so, applying distributive law to it.
 * Eventually, the original expression is replaced by a flattened form
 * of the optimised version (Mop2Ast).
 *
 * Example: Assume we have an expression of the form:
 *
 *   ( ( a * b * c) + a + ( b * c * d) + ( b * d))
 *
 * Internally, we represent this as
 *
 *  ADD{ MUL{a,b,c},
 *       MUL{a},
 *       MUL{b,c,d},
 *       MUL{b,d} }
 *
 * round 1:
 *  The most common factor is b.
 *  We obtain ADD{ MUL{b, ADD{ MUL{a,c},
 *                             MUL{c,d},
 *                             MUL{d}}},
 *                 MUL{a} }
 *
 * round 2:
 *   The most common factors (per level) are c and d. Inference is bottom
 *   up, so d is chosen here.
 *   So, ADD{ MUL{a,c},
 *            MUL{c,d},
 *            MUL{d}}
 *   is replaced by
 *       ADD{ MUL{d, ADD{ MUL{c},
 *                        MUL{}}},
 *            MUL{a,c}}
 *   equating to
 *       ADD{ MUL{b, ADD{ MUL{d, ADD{ MUL{c},
 *                                    MUL{}}},
 *                        MUL{a,c}}},
 *            MUL{a}}
 *   overall.
 *
 * NB: The intermediate rounds can be made visible using -#d,DL  !!
 *
 *  This yields as result:
 *
 *  ( b * ( d * ( c + 1) + ( a * c) ) + a)
 *
 * Note here, that c would have been a better choice in round two as it would
 * have avoided the transformation of ( d * c) + d into ( d * ( c + 1)) and
 * instead would have gotten rid of one more multiplication.
 * At the time being (May 2011) I (sbs) do not quite understand why this is
 * done. It seems that there is no particular reason other than that the
 * implementation would get more complicated [really???]
 *
 * Implementation details:
 * =======================
 *
 * - The internal representation of the multi-operand expressions is a bit
 * weird. All operations are represented as SaC AST prf's with n >= 0
 * argument expressions. Furthermore, ALL prfs are changed to their SxS
 * counterparts when creating the lists. After optimisation, the appropriate
 * versions (potentially VxS, SxV or VxV) are inserted. I assume that the
 * former is measure of convenience, and that the latter simplifies the
 * actual optimisation process.
 *
 * - The actual transformation is split up between two functions. The main
 * "work" is done in SplitMop. It separates the summands that do contain
 * the most common factor from those that do not. While doing so it eliminates
 * the occurrecnces of the most common factor.
 * In round 1 of the example above, this transforms
 *  ADD{ MUL{a,b,c},
 *       MUL{a},
 *       MUL{b,c,d},
 *       MUL{b,d} }
 *
 * into
 *   ADD{ MUL{a,c},
 *        MUL{c,d},
 *        MUL{d} }
 * and
 *   ADD{ MUL{a}}
 *
 * which are subsequently recombined in OptimizeMop into:
 *   ADD{ MUL{b, ADD{ MUL{a,c},
 *                    MUL{c,d},
 *                    MUL{d}}},
 *        MUL{a} }
 *
 *****************************************************************************/

#define DBUG_PREFIX "DL"
#include "debug.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "globals.h"
#include "new_types.h"
#include "new_typecheck.h"
#include "type_utils.h"
#include "traverse.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "DupTree.h"
#include "inferneedcounters.h"
#include "compare_tree.h"
#include "constants.h"
#include "shape.h"
#include "print.h"
#include "pattern_match.h"
#include "flattengenerators.h"

#include "distributive_law.h"

/*
 * INFO structure
 */
struct INFO {
    node *topblock;
    node *funargs;
    node *preassign;
    bool travrhs;
    node *lhs;
    node *vardecs;
};

/*
 * INFO macros
 */
#define INFO_TOPBLOCK(n) ((n)->topblock)
#define INFO_FUNARGS(n) ((n)->funargs)
#define INFO_PREASSIGN(n) ((n)->preassign)
#define INFO_TRAVRHS(n) ((n)->travrhs)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_VARDECS(n) ((n)->vardecs)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_TOPBLOCK (result) = NULL;
    INFO_FUNARGS (result) = NULL;
    INFO_PREASSIGN (result) = NULL;
    INFO_TRAVRHS (result) = FALSE;
    INFO_LHS (result) = FALSE;
    INFO_VARDECS (result) = NULL;

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
 * @fn node *DLdoDistributiveLawOptimization( node *arg_node)
 *
 * @brief starting point of distributivity optimization for
 *        module or fundef
 *
 * @param arg_node: an N_module or N_fundef node
 *
 * @return updated node
 *
 *****************************************************************************/

node *
DLdoDistributiveLawOptimization (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();

    TRAVpush (TR_dl);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Helper functions
 *
 *****************************************************************************/

/******************************************************************************
 *
 * Utility traversal to clear avis flags
 *
 *****************************************************************************/

static node *
ATravClearDLavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    AVIS_ISDLACTIVE (arg_node) = FALSE;
    DBUG_RETURN (arg_node);
}

static node *
ClearDLActiveFlags (node *arg_node)
{
    anontrav_t ddl_trav[2] = {{N_avis, &ATravClearDLavis}, {(nodetype)0, NULL}};

    DBUG_ENTER ();

    TRAVpushAnonymous (ddl_trav, &TRAVsons);
    arg_node = TRAVopt (arg_node, NULL);

    TRAVpop ();
    DBUG_RETURN (arg_node);
}

static node *
ATravSetDLavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    AVIS_ISDLACTIVE (arg_node) = TRUE;
    DBUG_RETURN (arg_node);
}

static node *
SetDLActiveFlags (node *arg_node)
{
    anontrav_t ddl_trav[2] = {{N_avis, &ATravSetDLavis}, {(nodetype)0, NULL}};

    DBUG_ENTER ();

    TRAVpushAnonymous (ddl_trav, &TRAVsons);
    arg_node = TRAVopt (arg_node, NULL);

    TRAVpop ();
    DBUG_RETURN (arg_node);
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

static node *
LocalSkipControl (void *param, node *expr)
{
    DBUG_ENTER ();
    node *avis;

    if (NODE_TYPE (expr) == N_id) {
        avis = ID_AVIS (expr);
        if ((AVIS_NEEDCOUNT (avis) != 1) || !AVIS_ISDLACTIVE (avis)) {
            expr = NULL; /* ABORT skipping !! */
        }
    }
    DBUG_RETURN (expr);
}

static pm_mode_t dl_pm_mode[3]
  = {{LocalSkipControl, NULL}, {PMMskipId, NULL}, {NULL, NULL}};

static bool
compatiblePrf (prf p1, prf p2)
{
    bool res;

    DBUG_ENTER ();

    res = (normalizePrf (p1) == normalizePrf (p2));

    DBUG_RETURN (res);
}

static bool
isScalar (node *n)
{
    bool res;

    DBUG_ENTER ();

    switch (NODE_TYPE (n)) {
    case N_num:
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
    case N_char:
    case N_bool:
    case N_double:
    case N_float:
        res = TRUE;
        break;

    case N_id:
#ifdef BUG760
        res = TUisScalar (AVIS_TYPE (ID_AVIS (n)));
#endif // BUG760
        res = ID_ISSCLPRF (n);
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

    DBUG_ENTER ();

    s1 = isScalar (e1);
    s2 = isScalar (e2);

    switch (prf) {
    case F_add_SxS:
    case F_add_SxV:
    case F_add_VxS:
    case F_add_VxV:
        if (s1) {
            if (s2) {
                prf = F_add_SxS;
            } else {
                prf = F_add_SxV;
            }
        } else {
            if (s2) {
                prf = F_add_VxS;
            } else {
                prf = F_add_VxV;
            }
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

/******************************************************************************
 *
 * Utility function to flatten non-id nodes. We assume the nodes
 * are simple scalars.
 *
 *****************************************************************************/

static node *
flattenPrfarg (node *arg_node, info *arg_info)
{
    node *res;
    DBUG_ENTER ();

    simpletype typ;
    if (N_id != NODE_TYPE (arg_node)) {
        typ = NTCnodeToType (arg_node);
        res
          = FLATGflattenExpression (arg_node, &INFO_VARDECS (arg_info),
                                    &INFO_PREASSIGN (arg_info),
                                    TYmakeAKS (TYmakeSimpleType (typ), SHmakeShape (0)));
        res = TBmakeId (res);
        ID_ISSCLPRF (res) = TRUE;
    } else {
        res = arg_node;
    }
    DBUG_RETURN (res);
}

static node *
consumeHead (node *mop)
{
    node *res;

    DBUG_ENTER ();

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

    DBUG_ENTER ();

    rhs = TCmakePrf2 (getPrf (prf, expr1, expr2), expr1, expr2);

    prod = NTCnewTypeCheck_Expr (rhs);
    avis = TBmakeAvis (TRAVtmpVar (), TYcopyType (TYgetProductMember (prod, 0)));
    prod = TYfreeType (prod);

    BLOCK_VARDECS (INFO_TOPBLOCK (arg_info))
      = TBmakeVardec (avis, BLOCK_VARDECS (INFO_TOPBLOCK (arg_info)));

    assign
      = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), rhs), INFO_PREASSIGN (arg_info));
    AVIS_SSAASSIGN (avis) = assign;

    INFO_PREASSIGN (arg_info) = assign;

    id = TBmakeId (avis);
    ID_ISSCLPRF (id) = isScalar (expr1) && isScalar (expr2);

    DBUG_RETURN (id);
}

static node *
ReverseAssignChain (node *ass, node *agg)
{
    node *res;

    DBUG_ENTER ();

    if (ass == NULL) {
        res = agg;
    } else {
        res = ASSIGN_NEXT (ass);
        ASSIGN_NEXT (ass) = agg;
        res = ReverseAssignChain (res, ass);
    }

    DBUG_RETURN (res);
}

static node *
Mop2Ast (node *mop, info *arg_info)
{
    node *res;

    DBUG_ENTER ();

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
            e1 = flattenPrfarg (Mop2Ast (e1, arg_info), arg_info);
            e2 = consumeHead (mop);
            e2 = flattenPrfarg (Mop2Ast (e2, arg_info), arg_info);
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
CollectExprs (prf target_prf, node *a, bool is_scalar_arg)
{
    node *res = NULL;
    pattern *pat;
    prf found_prf;
    node *arg1;
    node *arg2;
    node *left;
    node *right;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (a) == N_id, "CollectExprs called with non N_id node");
    DBUG_PRINT ("Collecting exprs for %s", AVIS_NAME (ID_AVIS (a)));

    pat = PMprf (1, PMAgetPrf (&found_prf), 2, PMvar (1, PMAgetNode (&arg1), 0),
                 PMvar (1, PMAgetNode (&arg2), 0));

    if (PMmatch (pat, dl_pm_mode, a) && compatiblePrf (target_prf, found_prf)) {
        /**
         * Here, we SHOULD set AVIS_ISDLACTIVE( LHS) to FALSE!
         * Unfortunately, PM does not yet support that feature yet.
         * This does not impede the correctness of the optimised
         * code, it just may lead to redundant work during optimisation!
         */
        left = CollectExprs (target_prf, arg1, isArg1Scl (found_prf));
        right = CollectExprs (target_prf, arg2, isArg2Scl (found_prf));
        res = TCappendExprs (left, right);
    } else {
        res = TBmakeExprs (DUPdoDupNode (a), NULL);
        ID_ISSCLPRF (EXPRS_EXPR (res)) = is_scalar_arg;
    }

    pat = PMfree (pat);

    DBUG_RETURN (res);
}

static node *
BuildMopTree (node *addition)
{
    node *tmp, *exprs;
    node *res;
    node *left;
    node *right;
    bool sclprf;

    DBUG_ENTER ();

    left = CollectExprs (F_add_SxS, PRF_ARG1 (addition), isArg1Scl (PRF_PRF (addition)));
    right = CollectExprs (F_add_SxS, PRF_ARG2 (addition), isArg2Scl (PRF_PRF (addition)));
    exprs = TCappendExprs (left, right);

    tmp = exprs;
    while (tmp != NULL) {
        node *mop;
        node *summand;

        summand = EXPRS_EXPR (tmp);
        if (NODE_TYPE (summand) == N_id) {
            sclprf = ID_ISSCLPRF (summand);
        } else {
            sclprf = TRUE;
        }

        mop = TBmakePrf (F_mul_SxS, CollectExprs (F_mul_SxS, summand, sclprf));

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

    DBUG_ENTER ();

    switch (NODE_TYPE (n)) {
    case N_float:
        res = (FLOAT_VAL (n) != 1.0f);
        break;

    case N_double:
        res = (DOUBLE_VAL (n) != 1.0);
        break;

    case N_numbyte:
        res = (NUMBYTE_VAL (n) != (char)1);
        break;

    case N_numshort:
        res = (NUMSHORT_VAL (n) != (short)1);
        break;

    case N_numint:
        res = (NUMINT_VAL (n) != (int)1);
        break;

    case N_numlong:
        res = (NUMLONG_VAL (n) != (long)1);
        break;

    case N_numlonglong:
        res = (NUMLONGLONG_VAL (n) != (long long)1);
        break;

    case N_numubyte:
        res = (NUMUBYTE_VAL (n) != (unsigned char)1);
        break;

    case N_numushort:
        res = (NUMUSHORT_VAL (n) != (unsigned short)1);
        break;

    case N_numuint:
        res = (NUMUINT_VAL (n) != (unsigned int)1);
        break;

    case N_numulong:
        res = (NUMULONG_VAL (n) != (unsigned long)1);
        break;

    case N_numulonglong:
        res = (NUMULONGLONG_VAL (n) != (unsigned long long)1);
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

    DBUG_ENTER ();

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

    DBUG_ENTER ();

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

    DBUG_ENTER ();

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

    DBUG_ENTER ();

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
    node *exprs;

    DBUG_ENTER ();

    if (NODE_TYPE (mop) == N_prf) {
        exprs = PRF_ARGS (mop);

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

                DBUG_EXECUTE (PRTdoPrintFile (stderr, mop););
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
                DBUG_EXECUTE (PRTdoPrintFile (stderr, mop););
                mop = OptimizeMop (mop);
            }
        }
    }

    DBUG_RETURN (mop);
}

static node *
EliminateEmptyProducts (node *mop, simpletype st)
{
    DBUG_ENTER ();

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
 * prefix: DL
 *
 *****************************************************************************/

/******************************************************************************
 *
 * function:
 *    node *DLfundef(node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
DLfundef (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node) != NULL) {
        /*
         * Infer need counters
         */
        DBUG_PRINT ("traversing body of (%s) %s",
                    (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef"),
                    FUNDEF_NAME (arg_node));
        arg_node = INFNCdoInferNeedCountersOneFundef (arg_node, TR_dl);

        INFO_TOPBLOCK (arg_info) = FUNDEF_BODY (arg_node);
        INFO_FUNARGS (arg_info) = FUNDEF_ARGS (arg_node);

        BLOCK_VARDECS (INFO_TOPBLOCK (arg_info))
          = SetDLActiveFlags (BLOCK_VARDECS (INFO_TOPBLOCK (arg_info)));

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        /* If new vardecs were made, append them to the current set */
        if (INFO_VARDECS (arg_info) != NULL) {
            BLOCK_VARDECS (FUNDEF_BODY (arg_node))
              = TCappendVardec (INFO_VARDECS (arg_info),
                                BLOCK_VARDECS (FUNDEF_BODY (arg_node)));
            INFO_VARDECS (arg_info) = NULL;
        }
        BLOCK_VARDECS (INFO_TOPBLOCK (arg_info))
          = ClearDLActiveFlags (BLOCK_VARDECS (INFO_TOPBLOCK (arg_info)));
        DBUG_PRINT ("Exiting body of (%s) %s",
                    (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef"),
                    FUNDEF_NAME (arg_node));
    }

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *DLblock(node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
DLblock (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *DLassign(node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
DLassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * Bottom-up traversal
     */
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_PREASSIGN (arg_info) != NULL) {
        arg_node = TCappendAssign (ReverseAssignChain (INFO_PREASSIGN (arg_info), NULL),
                                   arg_node);
        INFO_PREASSIGN (arg_info) = NULL;
        global.optcounters.al_expr++;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *DLlet(node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
DLlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_TRAVRHS (arg_info) = FALSE;
    LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);

    if (INFO_TRAVRHS (arg_info)) {
        INFO_LHS (arg_info) = LET_IDS (arg_node);
        DBUG_PRINT ("looking at the definition of %s",
                    AVIS_NAME (IDS_AVIS (LET_IDS (arg_node))));
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    } else {
        DBUG_PRINT ("skipping definition of %s",
                    AVIS_NAME (IDS_AVIS (LET_IDS (arg_node))));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *DLids(node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
DLids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (AVIS_ISDLACTIVE (IDS_AVIS (arg_node))) {
        INFO_TRAVRHS (arg_info) = TRUE;
    }
    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *DLprf(node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
DLprf (node *arg_node, info *arg_info)
{
    ntype *ltype;
    int oldoptcounter;
    node *mop;
    prf prf;

    DBUG_ENTER ();

    prf = PRF_PRF (arg_node);

    switch (prf) {
    case F_add_SxS:
    case F_add_SxV:
    case F_add_VxS:
    case F_add_VxV:
        ltype = IDS_NTYPE (INFO_LHS (arg_info));

        if ((!global.enforce_ieee)
            || ((TYgetSimpleType (TYgetScalar (ltype)) != T_float)
                && (TYgetSimpleType (TYgetScalar (ltype)) != T_double))) {

            /*
             * Collect operands into multi-operation (sum of products)
             */
            mop = BuildMopTree (arg_node);

            if (TCcountExprs (PRF_ARGS (mop)) >= 2) {
                DBUG_PRINT ("identified suitable expression:");
                DBUG_EXECUTE (PRTdoPrintFile (stderr, mop););

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
                    DBUG_PRINT ("converted into:");
                    DBUG_EXECUTE (PRTdoPrintFile (stderr, mop););
                    arg_node = FREEdoFreeNode (arg_node);
                    arg_node = Mop2Ast (mop, arg_info);
                } else {
                    DBUG_PRINT ("not converted!");
                }
            } else {
                mop = FREEdoFreeNode (mop);
            }
        }
        break;

    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
