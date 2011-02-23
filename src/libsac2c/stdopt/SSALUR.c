/*****************************************************************************
 *
 * $Id$
 *
 * file:   SSALUR.c
 *
 * prefix: SSALUR
 *
 * description:
 *
 *   This module implements loop-unrolling for special do-functions in ssa
 *   form. All while loops have been removed and converted to do-loops before
 *   so we have to deal only with the do loops.
 *   We also do the withloop unrolling by using the existing implementation
 *   in WLUnroll().
 *   If we can infer the number of loops and if this number is smaller than
 *   the specified maximum unrolling (maxlur and maxwlur parameter) we
 *   duplicate the code for this number of times.
 *
 *   To have all necessary information about constant data, you should do
 *   a SSAConstantFolding traversal first.
 *
 *****************************************************************************/

#include "types.h"
#include "new_types.h"
#include "shape.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"
#include "str.h"
#include "memory.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "SSALUR.h"
#include "constants.h"
#include "math.h"
#include "ctinfo.h"
#include "SSAWLUnroll.h"
#include "globals.h"
#include "phase.h"
#include "pattern_match.h"

#define DBUG_PREFIX "SSALUR"
#include "debug.h"

/* INFO structure and macros */
#include "SSALUR_info.h"

/* INFO functions */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));
    INFO_ASSIGN (result) = NULL;
    INFO_EXT_ASSIGN (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_REMASSIGN (result) = FALSE;
    INFO_PREASSIGN (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

#define UNR_NONE -1

/* helper functions for internal use only */
static loopc_t GetLoopUnrolling (node *, node *);
static bool IsLURPredicate (node *);
static bool GetLoopIdentifiers (node *, node *, struct prf_expr_queue *,
                                struct idx_vector_queue *);
static bool IsLURModifier (node *, struct idx_vector *);
static bool GetConstantArg (node *, node *, node *, loopc_t *);
static void GetPredicateData (node *, prf *, loopc_t *);
static node *UnrollLoopBody (node *, loopc_t);
static node *CreateCopyAssigns (node *, node *);

static node *GetLoopVariable (node *, node *, node *);
static bool GetModifier (node *, struct prf_expr_queue *, struct idx_vector_queue *, bool,
                         node **);

static loopc_t CalcUnrolling (node *, node *, struct idx_vector_queue *);

#ifndef DBUG_OFF
static void print_idx_queue (struct idx_vector_queue *);
static void print_prf_queue (struct prf_expr_queue *);
#endif

static bool CheckPredicateNF (node *, int *, int *);
static bool Newton (struct idx_vector_queue *, prf, loopc_t, double, double, int,
                    double *);
static bool IsOnIdxQueue (struct idx_vector_queue *, node *);
static bool IsLoopvarOnIdxQueue (struct idx_vector_queue *, node *);
static bool UpdatePrfStack (node *, node *, struct prf_expr_queue *,
                            struct idx_vector_queue *);
static node *FindAssignOfType (node *, nodetype);

/** <!--********************************************************************-->
 *
 * @fn static loopc_t GetLoopUnrolling(node *fundef, node *ext_assign)
 *
 * @brief   checks the given fundef for unrolling.
 *          the used do-loop-normal form:
 *
 *          ...
 *          x = f_do(..., init_counter, ...);
 *          ...
 *
 *
 *          res_t f_do(..., int loop_entrance_counter_id, ...) {
 *             LOOP-BODY;
 *             loop_counter_id = modifier_expr (loop_entrance_counter_id,
 *                                              loop_prf,
 *                                              loop_increment);
 *             LOOP-BODY;
 *             cond_id = predicate_expr(loop_counter_id,
 *                                      pred_prf,
 *                                      term_counter);
 *             LOOP-BODY;
 *             if (cond_id) {
 *               r1 = f_do(..., loop_counter_id, ...)
 *             } else {
 *               r2 = ...
 *             }
               r = (cond_id ? r1 : r2);
 *             return(r);
 *          }
 *
 *
 *
 * @return  number of unrollings of the do-loop-body or UNR_NONE
 *          if no unrolling is possible.
 *
 ******************************************************************************/
static loopc_t
GetLoopUnrolling (node *fundef, node *ext_assign)
{
    node *cond_assign;      /* N_assign */
    node *then_instr;       /* N_assign */
    node *condition;        /* N_expr */
    node *predicate_assign; /* N_assign */
    node *predicate;        /* N_expr */
    node *modifier_assign;  /* N_assign */
    node *modifier;         /* N_expr */

    loopc_t unroll = UNR_NONE; /* return result  */

    struct prf_expr_queue stack;
    struct idx_vector_queue ext_ivs;
    struct idx_vector *ivtmp;

    TAILQ_INIT (&stack);
    TAILQ_INIT (&ext_ivs);

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (fundef) == N_fundef, "%s called for non-fundef node",
                 __func__);

    /* check for do special fundef */
    if (!FUNDEF_ISDOFUN (fundef)) {
        DBUG_RETURN ((DBUG_PRINT ("no do-loop special fundef"), UNR_NONE));
    }

    DBUG_ASSERT (NULL != FUNDEF_BODY (fundef), "function with body required");

    /* search conditional in do fundef */
    cond_assign = FindAssignOfType (BLOCK_INSTR (FUNDEF_BODY (fundef)), N_cond);
    DBUG_ASSERT (NULL != cond_assign, "fundef with none or multiple conditionals");

    /* get condition */
    condition = COND_COND (ASSIGN_INSTR (cond_assign));
    DBUG_ASSERT (NULL != condition, "missing condition in conditional");

    /* check for identifier as condition */
    if (NODE_TYPE (condition) != N_id) {
        DBUG_RETURN ((DBUG_PRINT ("condition is no identifier"), UNR_NONE));
    }

    /* identifier must be a locally defined vardec -- no arg */
    if (NODE_TYPE (AVIS_DECL (ID_AVIS (condition))) != N_vardec) {
        DBUG_PRINT ("identifier is no locally defined vardec");
        DBUG_RETURN (UNR_NONE);
    }

    /* get defining assignment */
    predicate_assign = AVIS_SSAASSIGN (ID_AVIS (condition));
    DBUG_ASSERT (NULL != predicate_assign, "missing SSAASSIGN attribute for condition");
    DBUG_ASSERT (NODE_TYPE (ASSIGN_INSTR (predicate_assign)) == N_let,
                 "definition assignment without let");

    /* check predicate for having the correct form */
    predicate = LET_EXPR (ASSIGN_INSTR (predicate_assign));
    if (!IsLURPredicate (predicate)) {
        DBUG_RETURN ((DBUG_PRINT ("predicate has incorrect form"), UNR_NONE));
    }

    if (!GetLoopIdentifiers (condition, predicate, &stack, &ext_ivs)) {
        DBUG_PRINT ("GetLoopIentifiers returned false");
        goto cleanup;
    }

    /* extract recursive call behind cond, skipping N_annotate */
    then_instr = COND_THENINSTR (ASSIGN_INSTR (cond_assign));
    then_instr = FindAssignOfType (then_instr, N_let);

    DBUG_ASSERT (NULL != then_instr, "Cannot extract recursive call");
    DBUG_ASSERT (NODE_TYPE (ASSIGN_RHS (then_instr)) == N_ap,
                 "cond of loop fun w/o N_ap in then body");
    DBUG_ASSERT (AP_FUNDEF (ASSIGN_RHS (then_instr)) == fundef,
                 "cond of loop fun w/o recursiv call in then body");

    DBUG_EXECUTE ({
        print_idx_queue (&ext_ivs);
        print_prf_queue (&stack);
    });

    /* Now we check if all the external variables are constant function
     * parameters and getting modifiers for every loop variable.  */
    TAILQ_FOREACH (ivtmp, &ext_ivs, entries)
    {
#ifndef DBUG_OFF
        const char *var_name = AVIS_NAME (ID_AVIS (ivtmp->var));
#endif
        const char *loopvar_name = "?";
        node *m;

        ivtmp->loopvar = GetLoopVariable (ivtmp->var, fundef,
                                          AP_ARGS (LET_EXPR (ASSIGN_INSTR (then_instr))));

        if (ivtmp->loopvar == NULL) {
            DBUG_PRINT ("no candidate for loop counter");
            goto cleanup;
        }

        loopvar_name = AVIS_NAME (ID_AVIS (ivtmp->var));

        /* check that loop entrance identifier is an external constant  */
        if (!GetConstantArg (ivtmp->var, fundef, ext_assign, &(ivtmp->init_value))) {
            DBUG_PRINT ("cannot get value for variable %s", var_name);
            goto cleanup;
        }

        if (!GetModifier (ivtmp->loopvar, &stack, NULL, FALSE, &m)) {
            if (NULL != m) {
                m = FREEdoFreeNode (m);
            }

            DBUG_PRINT ("cannot get modifier for variable %s", var_name);
            goto cleanup;
        }

        /* If modifier is not on the stack */
        if (m == ivtmp->loopvar) {
            node *p = AVIS_SSAASSIGN (ID_AVIS (ivtmp->loopvar));
            if (NULL == p || NODE_TYPE (ASSIGN_INSTR (p)) != N_let) {
                DBUG_PRINT ("cannot get modifier for variable %s", loopvar_name);
                goto cleanup;
            }

            p = LET_EXPR (ASSIGN_INSTR (p));
            if (!GetLoopIdentifiers (ivtmp->loopvar, p, &stack, &ext_ivs)
                || !GetModifier (ivtmp->loopvar, &stack, NULL, FALSE, &m)) {
                if (NULL != m) {
                    m = FREEdoFreeNode (m);
                }
            }

            goto cleanup;
        }

        /* This is an implementation of IsLURModifier */
        if (!IsLURModifier (m, ivtmp))
            goto cleanup;

        m = FREEdoFreeNode (m);
    }

    /* First variable would be always loop conditional variable */
    modifier_assign = TAILQ_NEXT (TAILQ_FIRST (&stack), entries)->lhs;
    if (!GetModifier (modifier_assign, &stack, &ext_ivs, TRUE, &modifier)) {
        DBUG_PRINT ("cannot evaluate loop modifier");
        goto cleanup;
    }

    /* Turn the condition into normal form.  */
    unroll = CalcUnrolling (predicate, modifier, &ext_ivs);
    DBUG_PRINT ("predicate unrollable returned %i", unroll);

cleanup:
    if (!TAILQ_EMPTY (&ext_ivs)) {
        struct idx_vector *ptr, *tmpptr;
        ptr = TAILQ_FIRST (&ext_ivs);
        while (ptr != NULL) {
            tmpptr = TAILQ_NEXT (ptr, entries);
            if (ptr)
                MEMfree (ptr);
            ptr = tmpptr;
        }
    }

    if (!TAILQ_EMPTY (&stack)) {
        struct prf_expr *ptr, *tmpptr;
        ptr = TAILQ_FIRST (&stack);
        while (ptr != NULL) {
            tmpptr = TAILQ_NEXT (ptr, entries);
            if (ptr)
                MEMfree (ptr);
            ptr = tmpptr;
        }
    }

    DBUG_RETURN (unroll);
}

/* Structure for the Anonymous traversal.  */
struct local_info {
    node *res;
    nodetype nt;
};

/* Helper function for Anonymous traversal searching for the
   only occurence of local_info->nt type in the chain
   of assignments.  */
static node *
ATravFilter (node *arg_node, info *arg_info)
{
    struct local_info *linfo = (struct local_info *)arg_info;
    DBUG_ENTER ();

    if (NODE_TYPE (ASSIGN_INSTR (arg_node)) == linfo->nt) {
        if (linfo->res == NULL) {
            ((struct local_info *)arg_info)->res = arg_node;
            arg_node = TRAVcont (arg_node, arg_info);

        } else {
            ((struct local_info *)arg_info)->res = NULL;
        }
    } else if (arg_node != NULL) {
        arg_node = TRAVcont (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/* Function searches for the node of type N in the sequence of assignments
 * ASSIGNS and returns the assignment. Function returns NULL when either
 * the node of type N was not found or found more than once.  */
static node *
FindAssignOfType (node *assigns, nodetype n)
{
    struct local_info linfo = {.res = NULL, .nt = n};
    DBUG_ENTER ();

    TRAVpushAnonymous ((anontrav_t[]){{N_assign, &ATravFilter}, {0, NULL}}, &TRAVsons);
    assigns = TRAVopt (assigns, (info *)&linfo);
    TRAVpop ();

    DBUG_RETURN (linfo.res);
}

/** <!--********************************************************************-->
 *
 * @fn static bool IsLURPredicate(predicate)
 *
 * @brief checks the given expression to be of the form:
 *        expr    = id    {<=, >=, <, >, != } const
 *        or expr = const {<=, >=, <, >, != } id
 *
 ******************************************************************************/
static bool
IsLURPredicate (node *predicate)
{
    node *arg1;
    node *arg2;
    prf comp_prf;
    pattern *pat;

    DBUG_ENTER ();

    /* expression must be a primitive function */
    if (NODE_TYPE (predicate) != N_prf) {
        DBUG_RETURN ((DBUG_PRINT ("predicate expression without prf"), FALSE));
    }

    /* prf must be one of the comparison prfs */
    comp_prf = PRF_PRF (predicate);

    if (comp_prf != F_le_SxS && comp_prf != F_lt_SxS && comp_prf != F_ge_SxS
        && comp_prf != F_gt_SxS && comp_prf != F_neq_SxS) {
        DBUG_RETURN ((DBUG_PRINT ("predicate with non comparison prf"), FALSE));
    }

    /* args must be one constant (N_num)
     * and one identifier (N_id) node  */
    DBUG_ASSERT (PRF_ARGS (predicate), "missing arguments to primitive function");
    DBUG_ASSERT (EXPRS_NEXT (PRF_ARGS (predicate)),
                 "missing second arg of primitive function");

    arg1 = PRF_ARG1 (predicate);
    arg2 = PRF_ARG2 (predicate);

    pat = PMint (0, 0);

    if ((PMmatchFlat (pat, arg1) && NODE_TYPE (arg2) == N_id)
        || (NODE_TYPE (arg1) == N_id && PMmatchFlat (pat, arg2))) {
        DBUG_RETURN ((DBUG_PRINT ("loop predicate has correct form"), TRUE));
    } else {
        DBUG_RETURN ((DBUG_PRINT ("loop predicate without id and constant args"), FALSE));
    }
}

/** <!--********************************************************************-->
 *
 * @fn static bool CheckPredicateNF (node * expr, int *cst_count,
 *                                   int *cst_value)
 *
 * @brief Checks that EXPR is in the following form:
 *        x0 + x1 + ... + c,  where
 *           xi -- variable
 *           c  -- const
 *        Functoin saves c value in CST_VALUE.
 *
 ******************************************************************************/
static bool
CheckPredicateNF (node *expr, int *cst_count, int *cst_value)
{
    DBUG_ENTER ();
    switch (NODE_TYPE (expr)) {
    case N_prf:
        if (PRF_PRF (expr) == F_add_SxS) {
            bool first = CheckPredicateNF (PRF_ARG1 (expr), cst_count, cst_value);
            bool second = CheckPredicateNF (PRF_ARG2 (expr), cst_count, cst_value);
            DBUG_RETURN (first & second);
        } else
            DBUG_RETURN (FALSE);
    case N_id:
        DBUG_RETURN (TRUE);
    case N_num:
        if (*cst_count == 0) {
            *cst_count = 1;
            *cst_value = NUM_VAL (expr);
            DBUG_RETURN (TRUE);
        } else
            DBUG_RETURN (FALSE);
    default:
        DBUG_RETURN (FALSE);
    }
}

/** <!--********************************************************************-->
 *
 * @fn static bool Newton (struct idx_vector_queue *ivs, prf loop_pred,
 *                         loopc_t term, double x0, double tol,
 *                         int max_iter, double *res)
 *
 *
 * @brief Function iteratively finds the number of unrollings for the
 *        following do-loop condition:
 *                  m0 + m1 + ... + mn <LOOP_PRED> TERM,
 *                      where  mi is IVS->mfunc(i)
 *        Each mi is a loop modifier in one of the following forms:
 *              x = a * x + b  --> a^k*x0 + b*k
 *              x = x / a + b  --> x0/a^k + b*k
 *              x = x     + b  --> x0 + b*k
 *
 *  @param x0           Starting point of the iterative process
 *  @param tol          Tolerance
 *  @param max_iter     Maximal number of iterations allowed
 *  @param res          Solution of the equation
 *
 *  @return             Iterative process ended successfully (true/false)
 *
 ******************************************************************************/
static bool
Newton (struct idx_vector_queue *ivs, prf loop_pred, loopc_t term, double x0, double tol,
        int max_iter, double *res)
{
    struct idx_vector *ivtmp;
    double x = x0;
    double prev;
    int iter = 0;

    DBUG_ENTER ();
    /* check that each modifier is in form
     * x = a * x + b  or
     * x = x / a + b  or
     * x = x     + b, where a > 0  */
    TAILQ_FOREACH (ivtmp, ivs, entries)
    {
        if (ivtmp->mfunc.func != F_mul_SxS && ivtmp->mfunc.func != F_div_SxS
            && ivtmp->mfunc.a <= 0) {
            DBUG_RETURN (FALSE);
        }
    }

    do {
        double f_prev, f_prime_prev;

        ++iter;
        prev = x;

        /* we solve f (k) <op> term, rewrite it
         * as f (k) - term <op> 0  */
        f_prev = (double)-term;

        /* f (perv) - term */
        TAILQ_FOREACH (ivtmp, ivs, entries)
        {
            if (ivtmp->mfunc.a == 1)
                f_prev += ivtmp->init_value + ivtmp->mfunc.b * prev;

            /* a ^ prev * init_value + b * prev  */
            else if (ivtmp->mfunc.func == F_mul_SxS)
                f_prev += ivtmp->init_value * exp (prev * log (ivtmp->mfunc.a))
                          + ivtmp->mfunc.b * prev;
            /* init_value / a ^ prev + b * prev  */
            else if (ivtmp->mfunc.func == F_div_SxS)
                f_prev += ivtmp->init_value / exp (prev * log (ivtmp->mfunc.a))
                          + ivtmp->mfunc.b * prev;
            else
                DBUG_ASSERT (FALSE, "unreachable situation");
        }

        /* for the first iteration we check if loop
         * comparison is correct in this situation  */
        if (iter == 1)
            if ((f_prev < 0 && loop_pred != F_le_SxS)
                || (f_prev > 0 && loop_pred != F_ge_SxS))
                DBUG_RETURN (FALSE);

        /* Now the derrivative (f (prev) - term)'  */
        f_prime_prev = 0;

        TAILQ_FOREACH (ivtmp, ivs, entries)
        {
            /* (b * prev)' = b  */
            if (ivtmp->mfunc.a == 1)
                f_prime_prev += ivtmp->mfunc.b;
            /* (init_value * a ^ prev + b * prev)'
             * == init_value * a ^ prev * ln (a) + b  */
            else if (ivtmp->mfunc.func == F_mul_SxS)
                f_prime_prev += ivtmp->init_value * exp (prev * log (ivtmp->mfunc.a))
                                  * log (ivtmp->mfunc.a)
                                + ivtmp->mfunc.b;
            /* x / y ^ z == x * y ^ (-z)
             * (init_value * a ^ (-prev) + b * prev)'
             * == - init_value * a ^ (-prev) * ln (a) + b  */
            else if (ivtmp->mfunc.func == F_div_SxS)
                f_prime_prev += -ivtmp->init_value / exp (prev * log (ivtmp->mfunc.a))
                                  * log (ivtmp->mfunc.a)
                                + ivtmp->mfunc.b;
            else
                DBUG_ASSERT (FALSE, "unreachable situation");
        }

        x = prev - f_prev / f_prime_prev;

    } while (fabs (x - prev) > tol && iter < max_iter);

    *res = x;
    DBUG_RETURN (fabs (x - prev) <= tol);
}

/** <!--********************************************************************-->
 *
 * @fn static loopc_t CalcUnrolling (node * predicate, node * expr,
 *                                   struct idx_vector_queue *ivs)
 *
 * @brief  Function checks if the predicate is acceptable for unrolling
 *         and returns the number of iterations or UNR_NONE.
 *
 ******************************************************************************/
static loopc_t
CalcUnrolling (node *predicate, node *expr, struct idx_vector_queue *ivs)
{
    struct idx_vector *ivtmp;
    prf loop_pred;
    loopc_t term;
    int cst_count = 0, cst_value = 0;
    int additions_only = 0, mult_only = 0, div_only = 0, count = 0;

    DBUG_ENTER ();

    /* Check the form of predicate.  */
    if (!CheckPredicateNF (expr, &cst_count, &cst_value)) {
        DBUG_RETURN ((DBUG_PRINT ("un-unrollable predicate form"), FALSE));
    }

    GetPredicateData (predicate, &loop_pred, &term);

    switch (loop_pred) {
    case F_lt_SxS:
        loop_pred = F_le_SxS;
        --term;
        break;

    case F_gt_SxS:
        loop_pred = F_ge_SxS;
        ++term;
        break;

    case F_le_SxS:
    case F_ge_SxS:
    case F_neq_SxS:
        /* allow F_neq_SxS in cases like:
         * do {
         *   i += 1;
         * } while (i != const);  */
        break;

    default:
        DBUG_RETURN (UNR_NONE);
    }

    /* check the form of modifier */
    TAILQ_FOREACH (ivtmp, ivs, entries)
    {
        ++count;
        if (ivtmp->mfunc.a == 1)
            ++additions_only;
        else if (ivtmp->mfunc.func == F_mul_SxS && ivtmp->mfunc.b == 0)
            ++mult_only;
        else if (ivtmp->mfunc.func == F_div_SxS && ivtmp->mfunc.b == 0)
            ++div_only;
    }

    /* x0 + b0*k + x1 + b1*k + ... + cst_value ? term
     * k = (term - (x0 + x1 + ... + xn + cst_value)) / (b0 + b1 + ... + bn) */
    if (additions_only == count) {
        int b_sum = 0;
        int c_sum = 0;
        int t;

        /* c_sum = x0 + x1 + ... + xn + cst_value
         * b_sum = b0 + b1 + ... + bn */
        TAILQ_FOREACH (ivtmp, ivs, entries)
        {
            b_sum += ivtmp->mfunc.b;
            c_sum += ivtmp->init_value;
        }
        c_sum += cst_value;

        if (b_sum == 0) {
            DBUG_RETURN (UNR_NONE);
        }

        /* handle != condition  */
        if (loop_pred == F_neq_SxS && (term - c_sum) % b_sum == 0) {
            if (term - c_sum > 0 && b_sum > 0) {
                loop_pred = F_le_SxS;
                term -= 1;
            } else if (term - c_sum < 0 && b_sum < 0) {
                loop_pred = F_ge_SxS;
                term += 1;
            } else {
                DBUG_RETURN (UNR_NONE);
            }
        }

        /* iteration count  */
        t = (term - c_sum) / b_sum;

        if ((b_sum > 0 && loop_pred == F_ge_SxS)
            || (b_sum < 0 && loop_pred == F_le_SxS)) {
            if (t > 0)
                DBUG_RETURN (UNR_NONE);
            else
                DBUG_RETURN (1);
        } else if ((b_sum > 0 && loop_pred == F_le_SxS)
                   || (b_sum < 0 && loop_pred == F_ge_SxS)) {
            if (t >= 0)
                DBUG_RETURN (t + 1);
            else
                DBUG_RETURN (1);
        }
    }
    /* x0 * a0^k + cst_value ? term
     * k = log (a0, (term - cst_value) / x0)
     * k = ln ((term - cst_value) / (x0 * ln (a0))) */
    else if (mult_only == count && count == 1) {
        ivtmp = TAILQ_FIRST (ivs);
        if (ivtmp->mfunc.a > 1 && ivtmp->mfunc.b == 0) {
            loopc_t t = term - cst_value;
            /* (+x0)*a0^k <= +c | x0 > 0, c >0 */
            if (ivtmp->init_value > 0 && t > 0) {
                if (loop_pred == F_le_SxS) {
                    double res = log ((double)(t / ivtmp->init_value))
                                 / log ((double)(ivtmp->mfunc.a));
                    DBUG_RETURN (res > 0 ? floor (res) + 1 : 1);
                } else if (loop_pred == F_ge_SxS)
                    DBUG_RETURN (ivtmp->init_value * ivtmp->mfunc.a < t ? 1 : UNR_NONE);
            }
            /* (-x0)*a0^k <= (-c) | x0 > 0, c > 0 */
            else if (ivtmp->init_value < 0 && t < 0) {
                if (loop_pred == F_ge_SxS) {
                    double res = log ((double)(t / ivtmp->init_value))
                                 / log ((double)(ivtmp->mfunc.a));
                    DBUG_RETURN (res > 0 ? floor (res) + 1 : 1);
                } else if (loop_pred == F_le_SxS)
                    DBUG_RETURN (ivtmp->init_value * ivtmp->mfunc.a > t ? 1 : UNR_NONE);
            }
        }
    }
    /* x0 / (a0 ^k) + cst_value ? term
     * k = log (a0, x0 / (term - cst_value))
     * k = ln (x0 / ((term - cst_value) * ln (a0)) */
    else if (div_only == count && count == 1) {
        ivtmp = TAILQ_FIRST (ivs);
        if (ivtmp->mfunc.a > 1 && ivtmp->mfunc.b == 0) {
            loopc_t t = term - cst_value;
            /* (+x0)/(a0^k) >= +c , x0 > 0, c >0 */
            if (ivtmp->init_value > 0 && t > 0) {
                if (loop_pred == F_ge_SxS) {
                    double res = log ((double)(ivtmp->init_value / t))
                                 / log ((double)(ivtmp->mfunc.a));
                    DBUG_RETURN (res > 0 ? floor (res) + 1 : 1);
                } else if (loop_pred == F_le_SxS)
                    DBUG_RETURN (ivtmp->init_value / ivtmp->mfunc.a > t ? 1 : UNR_NONE);
            }
            /* (-x0)/(a0^k) >= (-c) , x0 > 0, c > 0 */
            else if (ivtmp->init_value < 0 && t < 0) {
                if (loop_pred == F_le_SxS) {
                    double res = log ((double)(ivtmp->init_value / t))
                                 / log ((double)(ivtmp->mfunc.a));
                    DBUG_RETURN (res > 0 ? floor (res) + 1 : 1);
                } else if (loop_pred == F_ge_SxS)
                    DBUG_RETURN (ivtmp->init_value / ivtmp->mfunc.a < t ? 1 : UNR_NONE);
            }
        }

    } else {
        int max_iter = 10;
        double res, tol = 0.00001;

        if (Newton (ivs, loop_pred, term, 0, tol, max_iter, &res) && res > 0)
            DBUG_RETURN (floor (res) + 1);
    }

    DBUG_RETURN (UNR_NONE);
}

/** <!--********************************************************************-->
 *
 * @fn static voidb GetPredicateData (node * expr, prf * pred, loopc_t * term)
 *
 * @brief get prf and const from expression and adjust prf to the form:
 *              id <prf> const
 *        by inverting the comparison function if necessary.
 *
 ******************************************************************************/
static void
GetPredicateData (node *expr, prf *pred, loopc_t *term)
{
    node *arg1;
    node *arg2;
    int local_term;
    pattern *pat;

    DBUG_ENTER ();

    arg1 = EXPRS_EXPR (PRF_ARGS (expr));
    arg2 = EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (expr)));

    /* get primitive comparison function from AST */
    *pred = PRF_PRF (expr);

    pat = PMint (1, PMAgetIVal (&local_term));

    /* get constant from AST */
    if (!PMmatchFlat (PMconst (0, 0), arg1)) {
        /* first arg is identifier */
        DBUG_ASSERT (PMmatchFlat (pat, arg2),
                     "Constant not found where constent expected");
    } else {
        /* second arg is identifier */
        DBUG_ASSERT (PMmatchFlat (pat, arg1),
                     "Constant not found where constent expected");

        /* change prf to have normal form cond = id <prf> const */
        switch (*pred) {
        case F_lt_SxS:
            *pred = F_gt_SxS;
            break;

        case F_le_SxS:
            *pred = F_ge_SxS;
            break;

        case F_gt_SxS:
            *pred = F_lt_SxS;
            break;

        case F_ge_SxS:
            *pred = F_le_SxS;
            break;

        default:; /* no change necessary */
        }
    }

    *term = (loopc_t)local_term;

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 *  @fn static bool IsLURModifier (node *m, struct idx_vector *iv)
 *
 *  @brief Checks if modifier is one of the following forms:
 *            cst + id
 *            cst * id
 *            id / cst
 *            cst + (cst * id)
 *            cst + (id / cst)
 *
 *          And that id is an IV->var
 *
 ******************************************************************************/
static bool
IsLURModifier (node *m, struct idx_vector *iv)
{
#ifndef DBUG_OFF
    const char *var_name = AVIS_NAME (ID_AVIS (iv->var));
#endif

    DBUG_ENTER ();

    /* (cst + ...) */
    if (NODE_TYPE (m) == N_prf && PRF_PRF (m) == F_add_SxS
        && NODE_TYPE (PRF_ARG1 (m)) == N_num) {
        node *arg2 = PRF_ARG2 (m);

        iv->mfunc.b = NUM_VAL (PRF_ARG1 (m));
        /* (cst + id) */
        if (NODE_TYPE (arg2) == N_id) {
            if (ID_AVIS (arg2) == ID_AVIS (iv->var)) {
                iv->mfunc.a = 1;
                iv->mfunc.func = F_mul_SxS;
            } else {
                DBUG_PRINT ("unallowed variable found in %s modifier", var_name);
                DBUG_RETURN (FALSE);
            }
        }
        /* (cst + (cst * id)) */
        else if (NODE_TYPE (arg2) == N_prf && PRF_PRF (arg2) == F_mul_SxS) {
            if (NODE_TYPE (PRF_ARG1 (arg2)) == N_num
                && NODE_TYPE (PRF_ARG2 (arg2)) == N_id
                && ID_AVIS (PRF_ARG2 (arg2)) == ID_AVIS (iv->var)) {
                iv->mfunc.a = NUM_VAL (PRF_ARG1 (arg2));
                iv->mfunc.func = PRF_PRF (arg2);
            } else {
                DBUG_PRINT ("un-unrollable modifier found, variable %s", var_name);
                DBUG_RETURN (FALSE);
            }
        }
        /* (cst + (id / cst)) */
        else if (NODE_TYPE (arg2) == N_prf && PRF_PRF (arg2) == F_div_SxS) {
            if (NODE_TYPE (PRF_ARG2 (arg2)) == N_num
                && NODE_TYPE (PRF_ARG1 (arg2)) == N_id
                && ID_AVIS (PRF_ARG1 (arg2)) == ID_AVIS (iv->var)) {
                iv->mfunc.a = NUM_VAL (PRF_ARG2 (arg2));
                iv->mfunc.func = PRF_PRF (arg2);
            } else {
                DBUG_PRINT ("un-unrollable modifier found, variable %s", var_name);
                DBUG_RETURN (FALSE);
            }
        } else {
            DBUG_PRINT ("un-unrollable modifier found, variable %s", var_name);
            DBUG_RETURN (FALSE);
        }
    }
    /* (cst * var) */
    else if (NODE_TYPE (m) == N_prf && PRF_PRF (m) == F_mul_SxS) {
        if (NODE_TYPE (PRF_ARG1 (m)) == N_num && NODE_TYPE (PRF_ARG2 (m)) == N_id
            && ID_AVIS (PRF_ARG2 (m)) == ID_AVIS (iv->var)) {
            iv->mfunc.func = PRF_PRF (m);
            iv->mfunc.b = 0;
            iv->mfunc.a = NUM_VAL (PRF_ARG1 (m));
        } else {
            DBUG_PRINT ("incorrect form of modifier, variable %s", var_name);
            DBUG_RETURN (FALSE);
        }
    }
    /* (var / cst) */
    else if (NODE_TYPE (m) == N_prf && PRF_PRF (m) == F_div_SxS) {
        if (NODE_TYPE (PRF_ARG2 (m)) == N_num && NODE_TYPE (PRF_ARG1 (m)) == N_id
            && ID_AVIS (PRF_ARG1 (m)) == ID_AVIS (iv->var)) {
            iv->mfunc.func = PRF_PRF (m);
            iv->mfunc.b = 0;
            iv->mfunc.a = NUM_VAL (PRF_ARG2 (m));
        } else {
            DBUG_PRINT ("incorrect form of modifier, variable %s", var_name);
            DBUG_RETURN (FALSE);
        }
    } else {
        DBUG_PRINT ("un-unrollable modifier found, variable %s", var_name);
        DBUG_RETURN (FALSE);
    }

    DBUG_RETURN (TRUE);
}

/** <!--********************************************************************-->
 *
 *  @fn static bool IsOnIdxQueue (struct idx_vector_queue *ivs, node * var)
 *
 *  @brief Checks if variable can be found in IVS
 *
 ******************************************************************************/
static bool
IsOnIdxQueue (struct idx_vector_queue *ivs, node *var)
{
    struct idx_vector *iv;

    DBUG_ENTER ();
    TAILQ_FOREACH (iv, ivs, entries)
    {
        if (iv->var && ID_AVIS (iv->var) == ID_AVIS (var))
            DBUG_RETURN (TRUE);
    }

    DBUG_RETURN (FALSE);
}

/** <!--********************************************************************-->
 *
 *  @fn static bool IsLoopvarOnIdxQueue (struct idx_vector_queue *ivs,
 *                                       node * var)
 *
 *  @brief Checks if loop variable can be found in IVS
 *
 ******************************************************************************/
static bool
IsLoopvarOnIdxQueue (struct idx_vector_queue *ivs, node *var)
{
    struct idx_vector *iv;

    DBUG_ENTER ();
    TAILQ_FOREACH (iv, ivs, entries)
    {
        if (iv->loopvar && ID_AVIS (iv->loopvar) == ID_AVIS (var))
            DBUG_RETURN (TRUE);
    }

    DBUG_RETURN (FALSE);
}

/** <!--********************************************************************-->
 *
 * @fn static bool UpdatePrfStack (node * predicate, node * var,
 *                                 struct prf_expr_queue *stack,
 *                                 struct idx_vector_queue *ivs)
 *
 * @brief  Function analyzes the PREDICATE which should be a primitive
 *         function and converts it into struct prf_expr.
 *         When function finds a variable inside of PREDICATE it puts
 *         the variable on the IVS stack in case the variable is not
 *         there already.
 *
 *****************************************************************************/
static bool
UpdatePrfStack (node *predicate, node *var, struct prf_expr_queue *stack,
                struct idx_vector_queue *ivs)
{
    int tv = -1;
    pattern *int_val_tv = PMint (1, PMAgetIVal (&tv), 0);
    struct prf_expr *expr;
    node *arg1, *arg2;

    DBUG_ENTER ();
    if (!predicate || !PRF_ARGS (predicate) || !EXPRS_NEXT (PRF_ARGS (predicate))
        || NULL == (arg1 = PRF_ARG1 (predicate))
        || NULL == (arg2 = PRF_ARG2 (predicate))) {
        DBUG_RETURN (
          (DBUG_PRINT ("%s expected prf with two arguments", __func__), FALSE));
    }

    /* Parse the prf function, convert const to int, if it is int, put
     * the assignment on the stack.  */
    expr = MEMmalloc (sizeof (struct prf_expr));
    expr->lhs = var;
    expr->func = PRF_PRF (predicate);

    /* Check if value is available.  */
    if (PMmatchFlat (int_val_tv, arg1)) {
        expr->arg1.is_int = TRUE;
        expr->arg1.value.num = tv;
    } else {
        expr->arg1.is_int = FALSE;
        expr->arg1.value.var = arg1;
        if (!IsOnIdxQueue (ivs, arg1)) {
            struct idx_vector *idxv;
            idxv = MEMmalloc (sizeof (struct idx_vector));
            idxv->var = arg1;
            idxv->loopvar = NULL;
            TAILQ_INSERT_TAIL (ivs, idxv, entries);
        }
    }
    /* check if second value is available.  */
    if (PMmatchFlat (int_val_tv, arg2)) {
        expr->arg2.is_int = TRUE;
        expr->arg2.value.num = tv;
    } else {
        expr->arg2.is_int = FALSE;
        expr->arg2.value.var = arg2;

        if (!IsOnIdxQueue (ivs, arg2)) {
            struct idx_vector *idxv;
            idxv = MEMmalloc (sizeof (struct idx_vector));
            idxv->var = arg2;
            idxv->loopvar = NULL;
            TAILQ_INSERT_TAIL (ivs, idxv, entries);
        }
    }

    TAILQ_INSERT_TAIL (stack, expr, entries);

    DBUG_RETURN (TRUE);
}

/** <!--********************************************************************-->
 *
 * @fn static struct prf_expr *PrfExprFind (struct prf_expr_queue *stack,
 *                                          node * var)
 * @brief Find a prf_expr on the STACK for the given variable.
 *
 *****************************************************************************/
static struct prf_expr *
PrfExprFind (struct prf_expr_queue *stack, node *var)
{
    struct prf_expr *pe;

    DBUG_ENTER ();
    if (var != NULL && NODE_TYPE (var) != N_id)
        DBUG_RETURN (NULL);

    TAILQ_FOREACH (pe, stack, entries)
    {
        if (var == NULL && pe->lhs == NULL)
            DBUG_RETURN (pe);
        else if (var != NULL && pe->lhs != NULL && ID_AVIS (pe->lhs) == ID_AVIS (var))
            DBUG_RETURN (pe);
    }

    DBUG_RETURN (NULL);
}

/* Return the node containing the result of applying PRF to ARG1 and ARG2.  */
static bool
evaluate_i_i_prf (prf function, int arg1, int arg2, node **res)
{
    DBUG_ENTER ();
    switch (function) {
    case F_add_SxS:
        *res = TBmakeNum (arg1 + arg2);
        break;
    case F_sub_SxS:
        *res = TBmakeNum (arg1 - arg2);
        break;
    case F_mul_SxS:
        *res = TBmakeNum (arg1 * arg2);
        break;
    case F_div_SxS:
        *res = TBmakeNum (arg1 / arg2);
        break;
        /* Some more operations we want to support? */
    default:
        *res = NULL;
        DBUG_RETURN ((DBUG_PRINT ("unsupported primitive function in modifier"), FALSE));
    }

    DBUG_RETURN (TRUE);
}

/* Function solves simple constant folding cases for the
   set of primitive function. M is a prf.

   Function can modify M.  */
static bool
evaluate_i_p_prf (prf function, int arg1, node *m, node **res)
{
    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (m) == N_prf, "M is not a primitive function");

    if (NODE_TYPE (PRF_ARG1 (m)) == N_num)
        switch (function) {
        case F_add_SxS:
            /* c + (c + x) or c + (c - x) */
            if (PRF_PRF (m) == F_add_SxS || PRF_PRF (m) == F_sub_SxS) {
                node *v = PRF_ARG1 (m);
                PRF_ARG1 (m) = TBmakeNum (NUM_VAL (v) + arg1);
                v = FREEdoFreeNode (v);
                *res = m;
                DBUG_RETURN (TRUE);
            }
            break;
        case F_sub_SxS:
            /* c - (c + x) */
            if (PRF_PRF (m) == F_add_SxS) {
                node *v = PRF_ARG1 (m);
                PRF_ARG1 (m) = TBmakeNum (arg1 - NUM_VAL (v));
                PRF_PRF (m) = F_sub_SxS;
                v = FREEdoFreeNode (v);
                *res = m;
                DBUG_RETURN (TRUE);
            }
            /* c - (c - x) */
            else if (PRF_PRF (m) == F_sub_SxS) {
                node *v = PRF_ARG1 (m);
                PRF_ARG1 (m) = TBmakeNum (arg1 - NUM_VAL (v));
                PRF_PRF (m) = F_add_SxS;
                v = FREEdoFreeNode (v);
                *res = m;
                DBUG_RETURN (TRUE);
            }
            break;

        case F_mul_SxS:
            /* c * (c + x) or c * (c - x) */
            if (PRF_PRF (m) == F_add_SxS || PRF_PRF (m) == F_sub_SxS) {
                node *v = PRF_ARG1 (m);
                PRF_ARG1 (m) = TBmakeNum (arg1 * NUM_VAL (v));
                PRF_ARG2 (m) = TCmakePrf2 (F_mul_SxS, TBmakeNum (arg1), PRF_ARG2 (m));
                v = FREEdoFreeNode (v);
                *res = m;
                DBUG_RETURN (TRUE);
            }
            /* c * (c * x) */
            else if (PRF_PRF (m) == F_mul_SxS) {
                node *v = PRF_ARG1 (m);
                PRF_ARG1 (m) = TBmakeNum (arg1 * NUM_VAL (v));
                PRF_ARG2 (m) = TCmakePrf2 (F_mul_SxS, TBmakeNum (arg1), PRF_ARG2 (m));
                v = FREEdoFreeNode (v);
                *res = m;
                DBUG_RETURN (TRUE);
            }
            break;
        default:
            break;
        }

    /* FIXME Are there any cases when we may
     * consider looking in (x <op> c)?  */

    *res = TCmakePrf2 (function, TBmakeNum (arg1), m);
    return TRUE;
}

/** <!--********************************************************************-->
 *
 * @fn static bool GetModifier (node * var, struct prf_expr_queue *stack,
 *                              struct idx_vector_queue *ext_ivs,
 *                              bool loopvars, node ** res)
 *
 * @brief  Builds an expression for the variable from the list of primitive
 *         functions trying to simplify and normalize it.
 *
 *         Normalization -- bring constant to the first position
 *         Simplification -- constant folding for the number of cases
 *
 *         When LOOPVARS is true, whenever we meet a loop-variable we would
 *         not expand it further.
 *
 * @note   This is a recursive function.
 *
 *****************************************************************************/
static bool
GetModifier (node *var, struct prf_expr_queue *stack, struct idx_vector_queue *ext_ivs,
             bool loopvars, node **res)
{
    struct prf_expr *pe;

    /* If variable is not on the stack, assume it is external.  */
    if (NULL == (pe = PrfExprFind (stack, var))) {
        /* *res = var; */
        *res = TBmakeId (ID_AVIS (var));
        return TRUE;
    }

    if (loopvars && ext_ivs && IsLoopvarOnIdxQueue (ext_ivs, var)) {
        /* *res = var; */
        *res = TBmakeId (ID_AVIS (var));
        return TRUE;
    }

    if (pe->arg1.is_int && pe->arg2.is_int) {
        return evaluate_i_i_prf (pe->func, pe->arg1.value.num, pe->arg2.value.num, res);
    }

    /* If we can, we should swap the arguments in order to put a
     * constant in a first place.  */
    if (pe->arg2.is_int && !pe->arg1.is_int) {
        switch (pe->func) {
        case F_add_SxS:
        case F_mul_SxS:
            /* (x + c) -> (c + x) && (x * c) -> (c * x) */
            {
                struct int_or_var tmp;
                tmp = pe->arg1;
                pe->arg1 = pe->arg2;
                pe->arg2 = tmp;
                break;
            }
        case F_sub_SxS:
            /* x - c -> -c + x */
            {
                struct int_or_var tmp;
                tmp = pe->arg1;
                pe->arg1 = pe->arg2;
                pe->arg1.value.num = -pe->arg1.value.num;
                pe->arg2 = tmp;
                pe->func = F_add_SxS;
                break;
            }
        default:
            break;
        }
    }

    if (pe->arg1.is_int && !pe->arg2.is_int) {
        node *m;
        /* When a function is commutative and one of the arguments is a
         * constant we must put it on the first place (c + x) or (c *
         * x) */
        if (!GetModifier (pe->arg2.value.var, stack, ext_ivs, loopvars, &m)) {
            *res = m;
            return FALSE;
        }

        switch (NODE_TYPE (m)) {
        case N_num:
            return evaluate_i_i_prf (pe->func, pe->arg1.value.num, NUM_VAL (m), res);
        case N_prf: {
            node *v;
            if (!evaluate_i_p_prf (pe->func, pe->arg1.value.num, m, &v)) {
                *res = m;
                return FALSE;
            }
            *res = v;
            return TRUE;
        }

        default:
            break;
        }
        *res = TCmakePrf2 (pe->func, TBmakeNum (pe->arg1.value.num), m);
        return TRUE;
    }
    /* No constant in arguments, or constant cannot be placed on the
     * first position. Anyway we create now just a prf.  */
    else {
        node *a1, *a2;
        if (pe->arg1.is_int)
            a1 = TBmakeNum (pe->arg1.value.num);
        else if (!GetModifier (pe->arg1.value.var, stack, ext_ivs, loopvars, &a1)) {
            *res = a1;
            return FALSE;
        }

        if (pe->arg2.is_int)
            a2 = TBmakeNum (pe->arg2.value.num);
        else if (!GetModifier (pe->arg2.value.var, stack, ext_ivs, loopvars, &a2)) {
            *res = a2;
            return FALSE;
        }

        *res = TCmakePrf2 (pe->func, a1, a2);
        return TRUE;
    }

    return FALSE;
}

#ifndef DBUG_OFF
/* Helper functions to print queue of type struct idx_vector_queue.  */
static void
print_idx_queue (struct idx_vector_queue *ivs)
{
    struct idx_vector *ivtmp;

    DBUG_ENTER ();
    DBUG_PRINT ("------ Ext variable stack -------");
    TAILQ_FOREACH (ivtmp, ivs, entries)
    DBUG_PRINT ("%s ", AVIS_NAME (ID_AVIS (ivtmp->var)));

    DBUG_RETURN ();
}

/* Helper functions to print queue of type struct prf_expr_queue.  */
static void
print_prf_queue (struct prf_expr_queue *stack)
{
    struct prf_expr *petmp;

    DBUG_ENTER ();
    DBUG_PRINT ("------ Prf expr stack -------");
    TAILQ_FOREACH (petmp, stack, entries)
    {
        if (petmp->arg1.is_int && petmp->arg2.is_int)
            DBUG_PRINT ("%s = %s (%d, %d)",
                        petmp->lhs == NULL ? "NULL" : AVIS_NAME (ID_AVIS (petmp->lhs)),
                        global.prf_name[petmp->func], petmp->arg1.value.num,
                        petmp->arg2.value.num);
        else if (!petmp->arg1.is_int && petmp->arg2.is_int)
            DBUG_PRINT ("%s = %s (%s, %d)",
                        petmp->lhs == NULL ? "NULL" : AVIS_NAME (ID_AVIS (petmp->lhs)),
                        global.prf_name[petmp->func],
                        AVIS_NAME (ID_AVIS (petmp->arg1.value.var)),
                        petmp->arg2.value.num);
        else if (petmp->arg1.is_int && !petmp->arg2.is_int)
            DBUG_PRINT ("%s = %s (%d, %s)",
                        petmp->lhs == NULL ? "NULL" : AVIS_NAME (ID_AVIS (petmp->lhs)),
                        global.prf_name[petmp->func], petmp->arg1.value.num,
                        AVIS_NAME (ID_AVIS (petmp->arg2.value.var)));
        else
            DBUG_PRINT ("%s = %s (%s, %s)",
                        petmp->lhs == NULL ? "NULL" : AVIS_NAME (ID_AVIS (petmp->lhs)),
                        global.prf_name[petmp->func],
                        AVIS_NAME (ID_AVIS (petmp->arg1.value.var)),
                        AVIS_NAME (ID_AVIS (petmp->arg2.value.var)));
    }

    DBUG_PRINT ("------ End -------");
    DBUG_RETURN ();
}
#endif

/** <!--********************************************************************-->
 *
 * @fn static bool GetLoopIdentifiers (node * targetvar, node * predicate,
 *                                     struct prf_expr_queue *stack,
 *                                     struct idx_vector_queue *ext_ivs)
 *
 * @brief Expand all the local variables used in PREDICATE, put them
 *        on the stack. If local variable is an integer constant -- store
 *        its value on the stack.
 *
 ******************************************************************************/
static bool
GetLoopIdentifiers (node *targetvar, node *predicate, struct prf_expr_queue *stack,
                    struct idx_vector_queue *ext_ivs)
{
    struct idx_vector_queue ivs;
    bool ret = FALSE;

    DBUG_ENTER ();

    TAILQ_INIT (&ivs);

    /* get the first prf for the predicate */
    if (!UpdatePrfStack (predicate, targetvar, stack, &ivs)) {
        DBUG_PRINT ("UpdatePrfStack first call failed");
        goto cleanup;
    }

    while (!TAILQ_EMPTY (&ivs)) {
        struct idx_vector *ptr, *tmpptr;

        ptr = TAILQ_FIRST (&ivs);
        while (ptr != NULL) {
            tmpptr = TAILQ_NEXT (ptr, entries);

            if (!ptr->var) {
                DBUG_PRINT ("NULL variable found");
                goto cleanup;
            }

            /* If we can get an SSA assignment for variable, get it.  */
            if (AVIS_SSAASSIGN (ID_AVIS (ptr->var))
                && NODE_TYPE (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (ptr->var))))
                     == N_let) {
                node *new_pred;
                new_pred = LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (ptr->var))));

                if (!PMmatchFlat (PMprf (0, 0), new_pred)) {
                    DBUG_PRINT ("Loop condition is not a "
                                "primitive function composition");
                    goto cleanup;
                }

                if (!UpdatePrfStack (new_pred, ptr->var, stack, &ivs)) {
                    DBUG_PRINT ("update_prf_stack failed");
                    goto cleanup;
                }

                TAILQ_REMOVE (&ivs, ptr, entries);
                MEMfree (ptr);
            }
            /* If we cannot, variable is external.  */
            else {
                TAILQ_REMOVE (&ivs, ptr, entries);
                if (!IsOnIdxQueue (ext_ivs, ptr->var))
                    TAILQ_INSERT_TAIL (ext_ivs, ptr, entries);
            }
            ptr = tmpptr;
        }
    }

    ret = TRUE;

cleanup:
    if (!TAILQ_EMPTY (&ivs)) {
        struct idx_vector *ptr, *tmpptr;
        ptr = TAILQ_FIRST (&ivs);
        while (ptr != NULL) {
            tmpptr = TAILQ_NEXT (ptr, entries);
            if (ptr)
                MEMfree (ptr);
            ptr = tmpptr;
        }
    }

    DBUG_RETURN (ret);
}

/** <!--********************************************************************-->
 *
 * @fn bool GetConstantArg(node *id, node *fundef, node *ext_assign,
 *                         loopc_t *init_counter)
 *
 * @brief  checks, if given id is an argument with external constant value.
 *         because loop-variant args are not attributed with SSACONST we have
 *         to find the matching argument in the external special fundef call
 *         to get the right constant value.
 *
 ******************************************************************************/
static bool
GetConstantArg (node *id, node *fundef, node *ext_assign, loopc_t *init_counter)
{
    node *arg_chain;
    node *param_chain;
    node *param;
    int pos;
    int i;
    constant *co;
    node *num;

    DBUG_ENTER ();

    /* check if id is an arg of this fundef */
    if (NODE_TYPE (AVIS_DECL (ID_AVIS (id))) != N_arg) {
        DBUG_PRINT ("identifier %s is no fundef argument", AVIS_NAME (ID_AVIS (id)));
        DBUG_RETURN (FALSE);
    }

    /* get argument position in fundef arg chain */
    arg_chain = FUNDEF_ARGS (fundef);
    pos = 1;
    while ((arg_chain != NULL) && (arg_chain != AVIS_DECL (ID_AVIS (id)))) {
        arg_chain = ARG_NEXT (arg_chain);
        pos++;
    }

    DBUG_ASSERT (arg_chain != NULL, "arg not found in fundef arg chain");

    /* get matching parameter expr-node */
    param_chain = AP_ARGS (ASSIGN_RHS (ext_assign));

    for (i = 1; i < pos; i++) {
        param_chain = EXPRS_NEXT (param_chain);
    }

    DBUG_ASSERT (param_chain != NULL, "missing matching parameter");
    param = EXPRS_EXPR (param_chain);

    /* check parameter to be constant */
    if (!(COisConstant (param))) {
        DBUG_PRINT ("external parameter is not constant");
        DBUG_RETURN (FALSE);
    }

    /* get constant value (conversion with constant resolves propagated
     * data */
    co = COaST2Constant (param);
    num = COconstant2AST (co);
    co = COfreeConstant (co);
    if (NODE_TYPE (num) != N_num) {
        num = FREEdoFreeNode (num);
        DBUG_PRINT ("external parameter is no numercial constant");
        DBUG_RETURN (FALSE);
    }

    /* set result value */
    *init_counter = (loopc_t)NUM_VAL (num);

    /* free temp. data */
    num = FREEdoFreeNode (num);

    DBUG_PRINT ("loop entrance counter: %s = %d", AVIS_NAME (ID_AVIS (id)),
                (*init_counter));

    DBUG_RETURN (TRUE);
}

/** <!--********************************************************************-->
 *
 * @fn static node *GetLoopVariable (node * var, node * fundef, node * params)
 *
 * @brief Find the variable in the recursive call which has the same
 *        position as VAR in FUNDEF params.
 *
 *****************************************************************************/
static node *
GetLoopVariable (node *var, node *fundef, node *params)
{
    node *ret = NULL;
    node *fargs = FUNDEF_ARGS (fundef);

    DBUG_ENTER ();

    while (params && fargs && ID_AVIS (var) != ARG_AVIS (fargs)) {
        params = EXPRS_NEXT (params);
        fargs = ARG_NEXT (fargs);
    }

    if (params)
        ret = EXPRS_EXPR (params);

    DBUG_RETURN (ret);
}

/** <!--********************************************************************-->
 *
 * @fn
 *   node *SSALURUnrollLoopBody(node *fundef, loopc_t unrolling)
 *
 * @brief
 *   does the unrolling of the do-loop body for "unrolling" times.
 *   therefore we duplicate the body and insert a list of copy assignments
 *   to get the right references between loop-end and loop-start.
 *   this duplicating destroys the ssa form that has to be restored afterwards.
 *   the contained conditional is set to FALSE to skip recursion.
 *   More precisely, we transform our pattern:
 *
 *     res_t f_do(..., int loop_entrance_counter_id, ...) {
 *             LOOP-BODY;
 *             loop_counter_id = modifier_expr (loop_entrance_counter_id,
 *                                              loop_prf,
 *                                              loop_increment);
 *             LOOP-BODY;
 *             cond_id = predicate_expr(loop_counter_id,
 *                                      pred_prf,
 *                                      term_counter);
 *             LOOP-BODY;
 *             if (cond_id) {
 *               r1 = f_do(..., loop_counter_id, ...)
 *             } else {
 *               r2 = ...
 *             }
 *             r = (cond_id ? r1 : r2);
 *             return(r);
 *           }
 *
 *   into:
 *
 *     res_t f_do(..., int loop_entrance_counter_id, ...) {
 *       /     LOOP-BODY;
 *       |     loop_counter_id = modifier_expr (loop_entrance_counter_id,
 *       |                                      loop_prf,
 *       |                                      loop_increment);
 *       |     LOOP-BODY;
 *       |     cond_id = predicate_expr(loop_counter_id,
 *  n-times                             pred_prf,
 *       |                              term_counter);
 *       |     LOOP-BODY;
 *       |     ...
 *       |     loop_entrance_counter_id = loop_counter_id;
 *       |     ...
 *       \_
 *             tmp_var = false;
 *             if (tmp_var) {
 *               r1 = f_do(..., loop_counter_id, ...)
 *             } else {
 *               r2 = ...
 *             }
 *             r = (cond_id ? r1 : r2);
 *             return(r);
 *           }
 *
 * !!! WARNING - this transformation destroys the ssa form              !!!
 * !!! the ssa form has to be restored before leaving this optimization !!!
 *
 ******************************************************************************/
static node *
UnrollLoopBody (node *fundef, loopc_t unrolling)
{
    node *loop_body;
    node *then_instr; /* N_assign */
    node *cond_assign;
    node *last;
    node *new_body;
    node *predavis;
    node *predass;

    DBUG_ENTER ();

    DBUG_ASSERT (unrolling >= 1, "unsupported unrolling number");

    /* separate loop body assignment chain */
    loop_body = BLOCK_INSTR (FUNDEF_BODY (fundef));

    last = loop_body;
    cond_assign = ASSIGN_NEXT (last);
    while ((cond_assign != NULL) && (NODE_TYPE (ASSIGN_INSTR (cond_assign)) != N_cond)) {
        last = cond_assign;
        cond_assign = ASSIGN_NEXT (cond_assign);
    }

    /*
     * last points to the last assignment of the loop body
     * and cond_assign to the conditional of the loop
     */
    DBUG_ASSERT (last != NULL, "error: missing loop body");
    DBUG_ASSERT (cond_assign != NULL, "error: missing conditional in loop");

    /* unchain loop body */
    ASSIGN_NEXT (last) = NULL;

    /* build up unrolled body */
    new_body = NULL;
    if (unrolling == 1) {
        /* only one time used */
        new_body = loop_body;
    } else {
        /* unrolling */

        /* extract recursive call behind cond */
        then_instr = COND_THENINSTR (ASSIGN_INSTR (cond_assign));
        DBUG_ASSERT (NODE_TYPE (then_instr) == N_assign,
                     "cond of loop fun w/o N_assign in then body");
        DBUG_ASSERT (NODE_TYPE (ASSIGN_INSTR (then_instr)) == N_let,
                     "cond of loop fun w/o N_let in then body");
        DBUG_ASSERT (NODE_TYPE (ASSIGN_RHS (then_instr)) == N_ap,
                     "cond of loop fun w/o N_ap in then body");
        DBUG_ASSERT (AP_FUNDEF (ASSIGN_RHS (then_instr)) == fundef,
                     "cond of loop fun w/o recursiv call in then body");

        /* append copy assignments to loop-body */
        loop_body
          = TCappendAssign (loop_body,
                            CreateCopyAssigns (FUNDEF_ARGS (fundef),
                                               AP_ARGS (ASSIGN_RHS (then_instr))));

        new_body = NULL;

        do {
            new_body = TCappendAssign (DUPdoDupTree (loop_body), new_body);
            unrolling--;
        } while (unrolling > 1);

        /* finally reuse oringinal loop body as last instance */
        new_body = TCappendAssign (loop_body, new_body);
    }

    /* set condition of conditional to false -> no more recursion */
    predavis = TBmakeAvis (TRAVtmpVar (), TYmakeAKV (TYmakeSimpleType (T_bool),
                                                     COmakeFalse (SHmakeShape (0))));

    FUNDEF_VARDEC (fundef) = TBmakeVardec (predavis, FUNDEF_VARDEC (fundef));

    predass = TBmakeAssign (TBmakeLet (TBmakeIds (predavis, NULL), TBmakeBool (FALSE)),
                            cond_assign);
    AVIS_SSAASSIGN (predavis) = predass;

#ifdef LETISAADOIT
    if (PHisSAAMode ()) {
        AVIS_DIM (predavis) = TBmakeNum (0);
        AVIS_SHAPE (predavis) = TCmakeIntVector (NULL);
    }
#endif // LETISAADOIT

    COND_COND (ASSIGN_INSTR (cond_assign))
      = FREEdoFreeTree (COND_COND (ASSIGN_INSTR (cond_assign)));

    COND_COND (ASSIGN_INSTR (cond_assign)) = TBmakeId (predavis);

    cond_assign = ASSIGN_NEXT (cond_assign);
    while (NODE_TYPE (ASSIGN_INSTR (cond_assign)) == N_let) {
        DBUG_ASSERT (NODE_TYPE (ASSIGN_RHS (cond_assign)) == N_funcond,
                     "All node between COND and RETURN must be FUNCOND");

        FUNCOND_IF (ASSIGN_RHS (cond_assign))
          = FREEdoFreeTree (FUNCOND_IF (ASSIGN_RHS (cond_assign)));

        FUNCOND_IF (ASSIGN_RHS (cond_assign)) = TBmakeId (predavis);

        cond_assign = ASSIGN_NEXT (cond_assign);
    }

    /* append rest of fundef assignment chain */
    new_body = TCappendAssign (new_body, predass);

    /* add new body to toplevel block of function */
    BLOCK_INSTR (FUNDEF_BODY (fundef)) = new_body;

    DBUG_RETURN (fundef);
}

/** <!--********************************************************************-->
 *
 * @fn static node *CreateCopyAssigns(node *arg_chain, node *rec_chain)
 *
 * @brief
 *   this functions builds up an assignment chain of copy assignments for
 *   all identifiers used in the recursive call to have loop back to the
 *   args in the functions signature.
 *
 ******************************************************************************/
static node *
CreateCopyAssigns (node *arg_chain, node *rec_chain)
{
    node *copy_assigns;
    node *right_id;

    DBUG_ENTER ();

    if (arg_chain != NULL) {
        /* process further identifiers in chain */
        copy_assigns = CreateCopyAssigns (ARG_NEXT (arg_chain), EXPRS_NEXT (rec_chain));

        /* make right identifer as used in recursive call */
        DBUG_ASSERT (NODE_TYPE (EXPRS_EXPR (rec_chain)) == N_id,
                     "non id node as paramter in recursive call");
        right_id = TBmakeId (ID_AVIS (EXPRS_EXPR (rec_chain)));

        /* make copy assignment */
        copy_assigns
          = TBmakeAssign (TBmakeLet (TBmakeIds (ARG_AVIS (arg_chain), NULL), right_id),
                          copy_assigns);
        AVIS_SSAASSIGN (ARG_AVIS (arg_chain)) = copy_assigns;

    } else {
        DBUG_ASSERT (rec_chain == NULL,
                     "different chains of args and calling parameters");

        copy_assigns = NULL;
    }

    DBUG_RETURN (copy_assigns);
}

/* traversal functions for SSALUR travsersal */
/** <!--********************************************************************-->
 *
 * @fn node *LURfundef(node *arg_node, info *arg_info)
 *
 * @brief   - traverse fundef and subordinated special fundefs for
 *            LUR and WLUR
 *          - analyse fundef for unrolling
 *          - do the infered unrolling
 *
 ******************************************************************************/
node *
LURfundef (node *arg_node, info *arg_info)
{
    loopc_t unrolling;
    int start_wlunr_expr;
    int start_lunr_expr;

    DBUG_ENTER ();

    INFO_FUNDEF (arg_info) = arg_node;
    /* save start values of opt counters */
    start_wlunr_expr = global.optcounters.wlunr_expr;
    start_lunr_expr = global.optcounters.lunr_expr;

    /*
     * traverse body to get wlur (and special fundefs unrolled)
     * the withloop unrolling itself destroys the ssa form of the AST
     * but because the withloop are not insteresting for the do-loop
     * unrolling we need not to restore the ssa form now (we will do it
     * after the do loop unrolling, because do-loop unrolling destroys
     * the ssa form, too).
     */

    /* traverse block of fundef */
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    /* analyse fundef for possible unrolling */
    unrolling = GetLoopUnrolling (arg_node, INFO_EXT_ASSIGN (arg_info));

    if (unrolling != UNR_NONE) {
        if (unrolling <= global.unrnum) {
            DBUG_PRINT ("unrolling loop %s %d times ", FUNDEF_NAME (arg_node), unrolling);

            global.optcounters.lunr_expr++;

            /* start do-loop unrolling - this leads to non ssa form
             * code */
            arg_node = UnrollLoopBody (arg_node, unrolling);

        } else {
            DBUG_PRINT ("no unrolling of %s: should be %d (but set to maxlur %d)",
                        FUNDEF_NAME (arg_node), unrolling, global.unrnum);

            if (unrolling <= 32) {
                CTInote ("LUR: -maxlur %d would unroll loop", unrolling);
                /*
                 * We use the hard-wired constant 32 here because otherwise
                 * we become annoyed by messages like "-maxlur 1000000000
                 * would unroll loop".
                 */
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *LURassign(node *arg_node, info *arg_info)
 *
 * @brief traverses assignment chain and integrate unrolled with-loop code
 *
 ******************************************************************************/
node *
LURassign (node *arg_node, info *arg_info)
{
    node *pre_assigns;
    node *tmp;
    node *old_assign;

    DBUG_ENTER ();

    DBUG_ASSERT (ASSIGN_INSTR (arg_node) != NULL, "assign node without instruction");

    /* stack actual assign */
    old_assign = INFO_ASSIGN (arg_info);
    INFO_ASSIGN (arg_info) = arg_node;

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    pre_assigns = INFO_PREASSIGN (arg_info);
    INFO_PREASSIGN (arg_info) = NULL;

    /* restore stacked assign */
    INFO_ASSIGN (arg_info) = old_assign;

    /* traverse to next assignment in chain */
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    /* integrate pre_assignments in assignment chain and remove this
     * assign */
    if (pre_assigns != NULL) {
        tmp = arg_node;
        arg_node = TCappendAssign (pre_assigns, ASSIGN_NEXT (arg_node));
        tmp = FREEdoFreeNode (tmp);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *LURap(node *arg_node, info *arg_info)
 *
 * @brief  traverses the args and starts the traversal in the called
 *         fundef if it is a special fundef.
 *
 ******************************************************************************/
node *
LURap (node *arg_node, info *arg_info)
{
    info *new_arg_info;
    DBUG_ENTER ();

    DBUG_ASSERT (AP_FUNDEF (arg_node) != NULL, "missing fundef in ap-node");

    AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);

    /* traverse special fundef without recursion */
    if ((FUNDEF_ISLACFUN (AP_FUNDEF (arg_node)))
        && (AP_FUNDEF (arg_node) != INFO_FUNDEF (arg_info))) {
        DBUG_PRINT ("traverse in special fundef %s", FUNDEF_NAME (AP_FUNDEF (arg_node)));

        /* stack arg_info frame for new fundef */
        new_arg_info = MakeInfo ();

        INFO_EXT_ASSIGN (new_arg_info) = INFO_ASSIGN (arg_info);

        /* start traversal of special fundef */
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), new_arg_info);

        DBUG_PRINT ("traversal of special fundef %s finished\n",
                    FUNDEF_NAME (AP_FUNDEF (arg_node)));
        new_arg_info = FreeInfo (new_arg_info);

    } else {
        DBUG_PRINT ("do not traverse in normal fundef %s",
                    FUNDEF_NAME (AP_FUNDEF (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node* SSALoopUnrolling(node* fundef)
 *
 * @brief starts the LoopUnrolling traversal for the given fundef.
 *        Does not start in special fundefs.
 *
 ******************************************************************************/
node *
LURdoLoopUnrolling (node *fundef)
{
    info *arg_info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (fundef) == N_fundef, "SSALUR called for non-fundef node");

    global.valid_ssaform = FALSE;
    /*
     * Wrapper code is created in non-SSA form and later on transformed into
     * SSA form using the standard transformation modules lac2fun and
     * ssa_transform. Therefore, we adjust the global control flag.
     */

    /* do not start traversal in special functions */
    if (!(FUNDEF_ISLACFUN (fundef))) {
        arg_info = MakeInfo ();

        TRAVpush (TR_lur);
        fundef = TRAVdo (fundef, arg_info);
        TRAVpop ();

        arg_info = FreeInfo (arg_info);
    }

    DBUG_RETURN (fundef);
}

#undef DBUG_PREFIX
