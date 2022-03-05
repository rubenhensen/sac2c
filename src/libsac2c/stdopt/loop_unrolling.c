/*****************************************************************************
 *
 * file:   LUR.c
 *
 * prefix: LUR
 *
 * description:
 *
 *   This module implements loop-unrolling for LOOPFUNs.
 *   All while loops have already been removed and converted to do-loops,
 *   so we have to deal only with do loops.
 *
 *   If we can infer the number of loops, N, and if this number is smaller than
 *   maxlur, we duplicate the code N number of times.
 *
 *   This traversal leaves the AST in NON-SSA form; the next traversal
 *   restores SSA form.
 *
 *
 *   We also attempt to compute extrema for the Loop Induction Variable (LIV).
 *   This requires that:
 *
 *     1. We know the sign of the loop stride/increment.
 *
 *     2. We know the initial value of the loop. This is trivial,
 *        because it is the LIV passed into the LOOPFUN from the
 *        external call.
 *
 *     3. We know the limiting value of the loop. E.g., in this example,
 *        the value is N:
 *
 *           for( i=0; i<N; i=i+3) { ...}
 *
 *        FIXME: Can we get away with using N, or do we really need
 *               to compute the final value of i?
 *
 *     4, The limiting value of the loop does NOT change within the loop.
 *        If it does, then the extrema can not be computed statically.
 *        An example of such a function is a binary search, where the
 *        upper or lower limits of the search change on each iteration.
 *        Hence, the limiting value must be a constant or an N_arg
 *        (perhaps moved outside the LACFUN by DLIR).
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
#include "loop_unrolling.h"
#include "constants.h"
#include "math.h"
#include "ctinfo.h"
#include "globals.h"
#include "pattern_match.h"
#include "flattengenerators.h"
#include "ivexpropagation.h"
#include "lacfun_utilities.h"

#define DBUG_PREFIX "LUR"
#include "debug.h"

/*
 * INFO structure
 */
struct INFO {
    node *assign;
    node *ext_assign;
    node *fundef;
    bool remassign;
    node *preassign;
    node *vardecs;
    node *newouterapargs;
};

/*
 * INFO macros
 */
#define INFO_ASSIGN(n) (n->assign)
#define INFO_EXT_ASSIGN(n) (n->ext_assign)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_REMASSIGN(n) (n->remassign)
#define INFO_PREASSIGN(n) (n->preassign)
#define INFO_VARDECS(n) (n->vardecs)
#define INFO_NEWOUTERAPARGS(n) (n->newouterapargs)

/* INFO functions */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));
    INFO_ASSIGN (result) = NULL;
    INFO_EXT_ASSIGN (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_REMASSIGN (result) = FALSE;
    INFO_PREASSIGN (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_NEWOUTERAPARGS (result) = NULL;

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

#ifndef DBUG_OFF
static void print_idx_queue (struct idx_vector_queue *);
static void print_prf_queue (struct prf_expr_queue *);
#endif

/* Helpers for set_extrema.  */
enum arg_sign { arg_plus, arg_minus };

struct addition {
    enum arg_sign sign;
    node *arg;
    TAILQ_ENTRY (addition) entries;
};

TAILQ_HEAD (addition_queue, addition);

#define flip_sign(sign) (sign == arg_plus ? arg_minus : arg_plus)

/** <!--********************************************************************-->
 * @fn
 *
 * @brief
 *
    This function converts a nested addition/subtraction expression
    of numbers and variables, creating a list of elements with the
    sign.  For example:
      a + b - (3 - (c + d))  will be transformed into:
      [+ a], [+ b], [- 3], [+ c], [+ d]

    In case one of the variables of the list is VAR or LOOPVAR,
    VAR_FOUND or LOOPVAR_FOUND is set on, the variable is not
    included in the list, but the sign of the variable is saved
    in the VAR_OR_LOOPVAR_SIGN argument.
 * @result
 *
 ******************************************************************************/
static bool
make_additions (node *target, node *var, bool *var_found, node *loopvar,
                bool *loopvar_found, enum arg_sign *var_or_loopvar_sign,
                enum arg_sign sign, struct addition_queue *q)
{
    if (NODE_TYPE (target) == N_num) {
        struct addition *add;
        add = MEMmalloc (sizeof (struct addition));
        add->sign = sign;
        add->arg = DUPdoDupTree (target);
        TAILQ_INSERT_TAIL (q, add, entries);
        return TRUE;
    }

    if (NODE_TYPE (target) == N_id) {
        if (ID_AVIS (target) == ID_AVIS (var))
            *var_found = TRUE, *var_or_loopvar_sign = sign;
        else if (ID_AVIS (target) == ID_AVIS (loopvar))
            *loopvar_found = TRUE, *var_or_loopvar_sign = sign;
        else {
            struct addition *add;
            add = MEMmalloc (sizeof (struct addition));
            add->sign = sign;
            add->arg = DUPdoDupTree (target);
            TAILQ_INSERT_TAIL (q, add, entries);
        }

        return TRUE;
    }

    if (NODE_TYPE (target) == N_prf) {
        enum arg_sign s1, s2;
        node *arg1, *arg2;
        bool b1, b2;

        if (PRF_PRF (target) == F_add_SxS)
            s1 = arg_plus, s2 = arg_plus;
        else if (PRF_PRF (target) == F_sub_SxS)
            s1 = arg_plus, s2 = arg_minus;
        else
            return FALSE;

        arg1 = PRF_ARG1 (target);
        arg2 = PRF_ARG2 (target);

        b1 = make_additions (arg1, var, var_found, loopvar, loopvar_found,
                             var_or_loopvar_sign, sign == arg_minus ? flip_sign (s1) : s1,
                             q);
        b2 = make_additions (arg2, var, var_found, loopvar, loopvar_found,
                             var_or_loopvar_sign, sign == arg_minus ? flip_sign (s2) : s2,
                             q);
        if (b1 && b2)
            return TRUE;
    }

    return FALSE;
}

/** <!--********************************************************************-->
 *
 * @fn node *FlattenMM( node *val, info *arg_info)
 *
 * @brief Flatten the scalar val
 *
 * @result The generated N_avis node
 *
 ******************************************************************************/
static node *
FlattenMM (node *val, info *arg_info)
{
    node *avis;

    DBUG_ENTER ();

    avis
      = FLATGexpression2Avis (val, &INFO_VARDECS (arg_info), &INFO_PREASSIGN (arg_info),
                              TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));

    DBUG_RETURN (avis);
}

/** <!--********************************************************************-->
 *
 * @fn bool IsConstantArg(node *id, node *fundef, node *ext_assign,
 *                         loopc_t *init_counter)
 *
 * @brief  Predicate: Return TRUE if id is an argument
 *         with external constant value.
 *         We find the matching argument in the
 *         LACFUN external call, to get the correct value, then
 *         check to see if it is a constant.
 *
 *         SIDE EFFECT: Set init_counter to the integer value of
 *                      id, IFF id is an integer scalar constant.
 *
 ******************************************************************************/
static bool
IsConstantArg (node *id, node *fundef, node *ext_assign, loopc_t *init_counter)
{
    node *param;
    constant *co;
    node *num;
    bool z = FALSE;

    DBUG_ENTER ();

    param = LFUgetLoopVariable (id, fundef, ext_assign);

    /* get constant value (conversion w/constant resolves propagated data */
    co = COaST2Constant (param);
    if (NULL != co) {
        num = COconstant2AST (co);
        co = COfreeConstant (co);
        if (N_num != NODE_TYPE (num)) {
            num = FREEdoFreeNode (num);
            DBUG_PRINT ("external parameter is not numeric constant");
        } else {
            /* set result value */
            *init_counter = (loopc_t)NUM_VAL (num);

            /* free temp. data */
            num = FREEdoFreeNode (num);
            DBUG_PRINT ("loop entrance counter: %s = %d", AVIS_NAME (ID_AVIS (id)),
                        (*init_counter));
            z = TRUE;
        }
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn void InitialValueExtrema(...)
 *
 * @brief Generate extrema associated with the initial value
 *        of the for-loop
 *
 *        The initial value is obtained from the calling environment,
 *        and is the calling N_ap and the LOOPFUN header/recursive call
 *        are modified to pass in the initial value as another parameter.
 *        This avoids confusion with the recursive call.
 *
 *        That initial value becomes either AVIS_MIN or AVIS_MAX for
 *        the Loop Induction Variable (LIV), depending on the loop predicate
 *        function and the LIV stride.
 *
 * @result None
 *         Side effects:
 *         Modify Loopfun & caller as per brief above.
 *         Set AVIS_MIN/AVIS_MAX for the LIV.
 *
 ******************************************************************************/
static void
InitialValueExtrema (struct idx_vector *ivptr, info *arg_info)
{

    node *liv;
    node *val;
    node *newavis;

    DBUG_ENTER ();

    liv = ID_AVIS (ivptr->var);
    val = LFUgetLoopVariable (ivptr->var, INFO_FUNDEF (arg_info),
                              INFO_EXT_ASSIGN (arg_info));
    newavis = LFUprefixFunctionArgument (INFO_FUNDEF (arg_info), ID_AVIS (val),
                                         &INFO_NEWOUTERAPARGS (arg_info));

    if (ivptr->mfunc.b > 0) { /* LIV stride is positive */
        IVEXPsetMinvalIfNotNull (liv, newavis);
        DBUG_PRINT ("LIV initial value %s is AVIS_MIN( %s)", AVIS_NAME (newavis),
                    AVIS_NAME (liv));
    } else {                               /* LIV stride is negative */
        newavis = IVEXPadjustExtremaBound (/* Normalize maxv */
                                           newavis, 1, &INFO_VARDECS (arg_info),
                                           &INFO_PREASSIGN (arg_info), "lurex1");
        IVEXPsetMaxvalIfNotNull (liv, newavis);
        DBUG_PRINT ("LIV initial value %s is AVIS_MAX( %s)", AVIS_NAME (newavis),
                    AVIS_NAME (liv));
    }

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn int AdjustToNormalizePredicate( predicate, var_or_loopvar_sign);
 *
 * @brief Determine count to normalize predicate count, and
 *        to normalize AVIS_MAX, similarly.
 *
 * @param  predicate: The N_prf predicate function for the LOOPFUN,
 *                    e.g., _lt_SxS_( x, y);
 *
 *         var_or_loopvar_sign: an enum: signum(stride) for the LOOPFUN.
 *
 * @result Integer
 *
 ******************************************************************************/
static int
AdjustToNormalizePredicate (node *predicate, enum arg_sign var_or_loopvar_sign)
{
    int z;

    DBUG_ENTER ();

    switch (PRF_PRF (predicate)) {
    case F_lt_SxS:
    case F_gt_SxS:
        z = 0;
        break;

    case F_le_SxS:
    case F_ge_SxS:
        z = (arg_plus == var_or_loopvar_sign) ? 1 : -1;
        break;

    default:
        DBUG_UNREACHABLE ("Confusion over LOOPFUN predicate");
        z = -1;
        break;
    }

    /* Normalization for AVIS_MAX setting */
    z = z + (arg_plus == var_or_loopvar_sign) ? 1 : 0;

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn void ModifierValueExtrema(...)
 *
 * @brief Generate extrema associated with the modifier value
 *        of the for-loop
 *
 * @param ivptr->mfubc.b is an integer that represents the constant
 *        stride of the Loop.
 *
 *        [This is annoyingly restrictive - we should be able to handle
 *         any stride for which we know its signum.]
 *
 * @result None
 *         Side effects: Set AVIS_MIN/AVIS_MAX for the
 *         loop's induction variable.
 *
 * If stride is positive, and, assuming
 * that p1 = PRF_ARG1( predicate):
 *
 * -i + expr >  p1  ===>  i <   expr - p1;  extrema_max =  expr - p1;
 * -i + expr >= p1  ===>  i <=  expr - p1;  extrema_max =  expr - p1   + 1;
 *	i + expr <  p1  ===>  i <  -expr + p1;  extrema_max =  p1   - expr;
 *	i + expr <= p1  ===>  i <= -expr + p1;  extrema_max =  p1   - expr + 1;
 *
 * If stride is negative:
 *
 *	-i + expr <  p1  ===>  i >   expr - p1;  extrema_min =  expr - p1;
 *	-i + expr <= p1  ===>  i >=  expr - p1;  extrema_min =  expr - p1   - 1;
 *	 i + expr >  p1  ===>  i >  -expr + p1;  extrema_min =  p1   - expr;
 *	 i + expr >= p1  ===>  i >= -expr + p1;  extrema_min =  p1   - expr - 1;
 *
 ******************************************************************************/
static void
ModifierValueExtrema (struct idx_vector *ivptr, info *arg_info,
                      enum arg_sign var_or_loopvar_sign, node *expr, node *predicate)
{
    node *liv;
    node *val;
    node *p1;
    node *tmp;
    int adjval;
    node *avis;

    DBUG_ENTER ();

    liv = ID_AVIS (ivptr->var);
    p1 = TBmakeId (ID_AVIS (PRF_ARG1 (predicate)));
    adjval = AdjustToNormalizePredicate (predicate, var_or_loopvar_sign);
    avis = FlattenMM (TBmakeNum (adjval), arg_info);
    DBUG_ASSERT (N_id == NODE_TYPE (expr), "Expected N_id expr");

    if (ivptr->mfunc.b > 0) { /* Loop stride is positive. Set AVIS_MAX  */
        DBUG_PRINT ("Set AVIS_MAX( %s) for positive stride", AVIS_NAME (liv));
        /* Swap args for < <= */
        if ((F_lt_SxS == PRF_PRF (predicate)) || (F_le_SxS == PRF_PRF (predicate))) {
            tmp = expr;
            expr = p1;
            p1 = tmp;
        }
    } else {
        DBUG_PRINT ("Set AVIS_MAX( %s) for negative stride", AVIS_NAME (liv));
        /* Swap args for < <= */
        if ((F_gt_SxS == PRF_PRF (predicate)) || (F_ge_SxS == PRF_PRF (predicate))) {
            tmp = expr;
            expr = p1;
            p1 = tmp;
        }
    }
    val = FlattenMM (TCmakePrf2 (F_sub_SxS, expr, p1), arg_info);
    val = TCmakePrf2 (F_add_SxS, TBmakeId (val), TBmakeId (avis));
    val = FlattenMM (val, arg_info);
    IVEXPsetMaxvalIfNotNull (liv, val);

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn set_extrema(...)
 *
 * @brief Impedance matcher for Artem vs. Earth
 *
 * @params:  predicate: the relational expression that is used to
 *           terminate the Loop(). E.g., _lt_SxS_( II, limit)
 *
 *           modifier: The value added to the Loop Induction Variable.
 *
 *           No idea what idx_vector_queue or ivs are.
 *
 *           fundef is the N_fundef of the LOOPFUN.
 *
 * @result None
 *
 ******************************************************************************/
static void
set_extrema (node *predicate, node *modifier, struct idx_vector_queue *ivs,
             info *arg_info)
{
    struct idx_vector *ivtmp, *ivptr = NULL;
    unsigned var_count = 0;
    struct addition_queue addq;
    struct addition *add;
    bool var_found = FALSE, loopvar_found = FALSE;
    enum arg_sign var_or_loopvar_sign = arg_plus;
    node *expr;
    node *expr2;

    DBUG_ENTER ();

    /* FIXME - indenting needs to be fixed */
    if (global.optimize.dopetl) {
        TAILQ_INIT (&addq);

        /* Check that we have a loop with a single index-variable and
           the modifier of this variable is:
              i = i + const   or    i = i - const.  */
        TAILQ_FOREACH (ivtmp, ivs, entries)
        {
            if (ID_AVIS (ivtmp->var) != ID_AVIS (ivtmp->loopvar)) {
                var_count += 1;

                /* Fail if we there are several indexes in the loop.  */
                if (var_count > 1) {
                    DBUG_PRINT ("multi-index loop found, cannot set extremas");
                    DBUG_RETURN ();
                }

                /* Fail if modifier has incorrect form.  */
                if (ivtmp->mfunc.a != 1) {
                    DBUG_PRINT ("unsupported modifier for variable `%s'",
                                AVIS_NAME (ID_AVIS (ivtmp->var)));
                    DBUG_RETURN ();
                }

                ivptr = ivtmp;
            }
        }

        if (NODE_TYPE (predicate) != N_prf || !ivptr) {
            DBUG_PRINT ("unsupported predicate");
            DBUG_RETURN ();
        }

        /* We have an intial value: set an extrema for it */
        InitialValueExtrema (ivptr, arg_info);

        if (make_additions (modifier, ivptr->var, &var_found, ivptr->loopvar,
                            &loopvar_found, &var_or_loopvar_sign, arg_plus, &addq)) {

            DBUG_PRINT ("extrema from the comparison is extracted");

            /* As experience suggests, in case we have VAR_FOUND, we
               will have to reverse-apply the modifier, as some
               smart-ass optimization applied it before and ruined
               the extrema.  In case LOOP_VAR is found, the extrema
               would be just the sum of ADDQ.  */
            if (loopvar_found && var_found) {
                DBUG_PRINT ("loop modifier is too complicated for extrema");
                goto cleanup;
            }

            /* Adding reverse-application of the modifier.  */
            if (var_found) {
                struct addition *t;
                t = MEMmalloc (sizeof (struct addition));
                if (var_or_loopvar_sign == arg_minus) {
                    t->sign = arg_plus;
                } else {
                    t->sign = arg_minus;
                }

                t->arg = TBmakeNum (ivptr->mfunc.b);
                TAILQ_INSERT_TAIL (&addq, t, entries);
            }

            /* convert the addq back into an expression.  */

            expr = FlattenMM (TBmakeNum (0), arg_info);
            TAILQ_FOREACH (add, &addq, entries)
            {
                expr2 = (add->sign == arg_minus) ? TCmakePrf1 (F_neg_S, (add->arg))
                                                 : add->arg;
                expr2 = FlattenMM (expr2, arg_info);
                expr = TCmakePrf2 (F_add_SxS, expr, expr2);
                expr = FlattenMM (expr, arg_info);
            }

            ModifierValueExtrema (ivptr, arg_info, var_or_loopvar_sign, expr, predicate);
        } else {
            DBUG_PRINT ("cannot extract extrema from the loop comparison");
        }

    cleanup:
        if (!TAILQ_EMPTY (&addq)) {
            struct addition *ptr, *tmpptr;

            ptr = TAILQ_FIRST (&addq);
            while (ptr != NULL) {
                tmpptr = TAILQ_NEXT (ptr, entries);
                if (ptr) {
                    MEMfree (ptr);
                    ptr = tmpptr;
                }
            }
        }
    }
    DBUG_RETURN ();
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
 *        Function saves c value in CST_VALUE.
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
 * @fn static bool GetPredicateData (node * expr, prf * pred, loopc_t * term)
 *
 * @brief get prf and const from expression and adjust prf to the form:
 *              id <prf> const
 *        by inverting the comparison function if necessary.
 *
 * @params: expr: I have no idea. Artem considers documentation beneath him.
 *          pred: I have no idea. Artem considers documentation beneath him.
 *         term: I have no idea. Artem considers documentation beneath him.
 *
 *        SIDE EFFECT: sets term.
 *                     sets pred.
 *
 ******************************************************************************/
static bool
GetPredicateData (node *expr, prf *pred, loopc_t *term)
{
    int local_term;
    pattern *pat;
    bool z = TRUE;

    DBUG_ENTER ();

    /* get primitive comparison function from AST */
    *pred = PRF_PRF (expr);

    pat = PMint (1, PMAgetIVal (&local_term));

    /* get constant from AST */
    if (!PMmatchFlat (PMconst (0, 0), PRF_ARG1 (expr))) {
        DBUG_ASSERT (N_id == NODE_TYPE (PRF_ARG1 (expr)), "Expected N_id PRF_ARG1");
        if (!PMmatchFlat (pat, PRF_ARG2 (expr))) {
            DBUG_PRINT ("PRF_ARG2 not integer constant");
            z = FALSE;
        }
    } else {
        DBUG_ASSERT (N_id == NODE_TYPE (PRF_ARG2 (expr)), "Expected N_id PRF_ARG2");
        if (!PMmatchFlat (pat, PRF_ARG1 (expr))) {
            DBUG_PRINT ("PRF_ARG1 not integer constant");
            z = FALSE;
        }
    }

    /* change prf to have normal form cond = id <prf> const */
    if (z) {
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

    DBUG_RETURN (z);
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

static double
f_prime (struct m_func mfunc, loopc_t init, double iter)
{
    if (mfunc.a == 1) {
        /* iff f(x) = bx, then f'(x) = b  */
        return mfunc.b;
    } else if (mfunc.func == F_mul_SxS) {
        double alpha = init;
        double beta = mfunc.a;
        double gamma = mfunc.b;

        /* iff f(x) = k1 + k2 * beta^x, then f'(x) = k2 * beta^x * ln (beta)  */
        return (alpha - gamma - beta * alpha) / (1. - beta) * exp (iter * log (beta))
               * log (beta);
    } else if (mfunc.func == F_div_SxS) {
        double alpha = init;
        double beta = mfunc.a;
        double gamma = mfunc.b;

        /* iff f(x) = k1 + k2 * 1 / beta^x, then
               f(x) = k1 + k2 * beta^(-x), then
               f'(x) = -k2 * beta^(-x) * ln (beta)  */
        return -gamma * beta / (beta - 1.)
               + (alpha * beta - gamma * beta - alpha)
                   / ((beta - 1.) * exp (iter * log (beta))) * log (beta);
    } else {
        DBUG_UNREACHABLE ("Unreachable situation");
    }

    return nan ("");
}

static double
NewtonF (struct m_func mfunc, loopc_t init, double iter)
{
    if (mfunc.a == 1)
        return init + mfunc.b * iter;

    else if (mfunc.func == F_mul_SxS) {
        double alpha = init;
        double beta = mfunc.a;
        double gamma = mfunc.b;

        return gamma / (1. - beta)
               + (alpha - gamma - beta * alpha) / (1. - beta) * exp (iter * log (beta));
    } else if (mfunc.func == F_div_SxS) {
        double alpha = init;
        double beta = mfunc.a;
        double gamma = mfunc.b;

        return gamma * beta / (beta - 1.)
               + (alpha * beta - gamma * beta - alpha)
                   / ((beta - 1.) * exp (iter * log (beta)));
    } else
        DBUG_UNREACHABLE ("Unreachable situation");

    return nan ("");
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
    double f_prev;
    double f_prime_prev;

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

        ++iter;
        prev = x;

        /* we solve f (k) <op> term, rewrite it
         * as f (k) - term <op> 0  */
        f_prev = (double)-term;

        /* f (prev) - term */
        TAILQ_FOREACH (ivtmp, ivs, entries)
        {
            f_prev += NewtonF (ivtmp->mfunc, ivtmp->init_value, prev);
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
            f_prime_prev += f_prime (ivtmp->mfunc, ivtmp->init_value, prev);
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
        DBUG_RETURN ((DBUG_PRINT ("un-unrollable predicate expression"), UNR_NONE));
    }

    if (!GetPredicateData (predicate, &loop_pred, &term)) {
        DBUG_RETURN ((DBUG_PRINT ("un-unrollable predicate"), UNR_NONE));
    }

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

        /* fprintf (stderr, "-- term = %i, b_sum = %i, c_sum = %i\n",
                         term, b_sum, c_sum);*/

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
        int max_iter = 100;
        double res, tol = 0.001;

        if (Newton (ivs, loop_pred, term - cst_value, 0, tol, max_iter, &res)
            && res > 0) {
            loopc_t iter_count = floor (res) + 1;
            loopc_t res_val = 0;

            /* Numerical methods are great of course, but we have to check
               if the floating-point arithmetics do not poop into our
               glorious results.  */
            for (loopc_t i = 0; i < iter_count; ++i) {
                if (i == 0)
                    TAILQ_FOREACH (ivtmp, ivs, entries)
                ivtmp->value = ivtmp->init_value;
                else TAILQ_FOREACH (ivtmp, ivs, entries)
                {
                    if (ivtmp->mfunc.func == F_mul_SxS) {
                        ivtmp->value = ivtmp->value * ivtmp->mfunc.a;
                        ivtmp->value += ivtmp->mfunc.b;
                    } else if (ivtmp->mfunc.func == F_div_SxS) {
                        ivtmp->value = ivtmp->value / ivtmp->mfunc.a;
                        ivtmp->value += ivtmp->mfunc.b;
                    } else
                        DBUG_UNREACHABLE ("Unreachable situation");
                }
            }

            TAILQ_FOREACH (ivtmp, ivs, entries)
            {
                res_val += ivtmp->value;
                ivtmp->value = 0;
            }
            res_val += cst_value;

            if (loop_pred == F_le_SxS)
                DBUG_RETURN (res_val <= term ? iter_count : UNR_NONE);
            else if (loop_pred == F_ge_SxS)
                DBUG_RETURN (res_val >= term ? iter_count : UNR_NONE);
        }
    }

    DBUG_RETURN (UNR_NONE);
}

/** <!--********************************************************************-->
 *
 *  @fn static bool IsOnIdxQueue (struct idx_vector_queue *ivs, node * var)
 *
 *  @brief Checks if variable can be found in IVS
 *
 ******************************************************************************/
static struct idx_vector *
IsOnIdxQueue (struct idx_vector_queue *ivs, node *var)
{
    struct idx_vector *iv;

    DBUG_ENTER ();

    if (ivs == NULL) {
        DBUG_RETURN (NULL);
    }

    TAILQ_FOREACH (iv, ivs, entries)
    {
        if (iv->var && ID_AVIS (iv->var) == ID_AVIS (var)) {
            DBUG_RETURN (iv);
        }
    }

    DBUG_RETURN (NULL);
}

/** <!--********************************************************************-->
 *
 *  @fn static bool IsLoopvarOnIdxQueue (struct idx_vector_queue *ivs,
 *                                       node * var)
 *
 *  @brief Checks if loop variable can be found in IVS
 *
 ******************************************************************************/
static struct idx_vector *
IsLoopvarOnIdxQueue (struct idx_vector_queue *ivs, node *var)
{
    struct idx_vector *iv;

    DBUG_ENTER ();

    if (ivs == NULL)
        DBUG_RETURN (NULL);

    TAILQ_FOREACH (iv, ivs, entries)
    {
        if (iv->loopvar && ID_AVIS (iv->loopvar) == ID_AVIS (var))
            DBUG_RETURN (iv);
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

/** <!--********************************************************************-->
 *
 * @fn
 * @brief Function solves simple constant folding cases for the
 *        set of primitive function. m is an N_prf.
 *
 *        Function can modify m.
 *
 *****************************************************************************/
static bool
evaluate_i_p_prf (prf function, int arg1, node *m, node **res)
{
    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (m) == N_prf, "m is not a primitive function");

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
    DBUG_RETURN (TRUE);
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
    struct idx_vector *iv;

    DBUG_ENTER ();

    /* If variable is not on the stack, assume it is external.  */
    if (NULL == (pe = PrfExprFind (stack, var))) {

        /* In case we are inside the loop-comparison expression, check
           that the variable can be found on the EXT_IVS queue and mark
           it as seen.  In case the variable cannot be found, throw an
           error message and return FALSE.  */
        if (loopvars) {
            if (NULL == (iv = IsOnIdxQueue (ext_ivs, var))) {
                DBUG_PRINT ("External variable found in the loop modifier");
                DBUG_RETURN (FALSE);
            }

            if (iv->seen) {
                DBUG_PRINT ("Loop index-variable `%s' is used more than once "
                            "in the comparison of the loop",
                            AVIS_NAME (ID_AVIS (var)));
                DBUG_RETURN (FALSE);
            }

            iv->seen = TRUE;

            if (ID_AVIS (iv->var) != ID_AVIS (iv->loopvar)) {
                /* XXX In this case we make an assumption that if we found an
                   external variable inside the loop condition, then it means
                   that we compare the variable the same variable after the first
                   iteration, and in order to get the correct initial value
                   we need to undo the first iteration.  In case this assumption
                   does not hold anymore, the loop unrolling will return WRONG
                   RESULTS.  */
                if (iv->init_value - iv->mfunc.b == 0 && iv->mfunc.a != 1) {
                    DBUG_PRINT ("Cannot reverse apply modifier expression for %s",
                                AVIS_NAME (ID_AVIS (var)));
                    DBUG_RETURN (FALSE);
                } else
                    DBUG_PRINT ("reverse-applying modifier for variable %s",
                                AVIS_NAME (ID_AVIS (var)));

                if (iv->mfunc.func == F_div_SxS)
                    iv->init_value = (iv->init_value - iv->mfunc.b) * iv->mfunc.a;
                else
                    iv->init_value = (iv->init_value - iv->mfunc.b) / iv->mfunc.a;
            }
        }

        /* *res = var; */
        *res = TBmakeId (ID_AVIS (var));
        DBUG_RETURN (TRUE);
    }

    if (loopvars) {

        if (NULL != (iv = IsLoopvarOnIdxQueue (ext_ivs, var))) {

            if (iv->seen) {
                DBUG_PRINT ("Loop index-variable `%s' is used more than once "
                            "in the comparison of the loop",
                            AVIS_NAME (ID_AVIS (iv->var)));
                DBUG_RETURN (FALSE);
            }

            iv->seen = TRUE;

            /* *res = iv->var; */
            *res = TBmakeId (ID_AVIS (var));
            DBUG_RETURN (TRUE);
        }
    }

    if (pe->arg1.is_int && pe->arg2.is_int) {
        DBUG_RETURN (
          evaluate_i_i_prf (pe->func, pe->arg1.value.num, pe->arg2.value.num, res));
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
            DBUG_RETURN (FALSE);
        }

        switch (NODE_TYPE (m)) {
        case N_num:
            return evaluate_i_i_prf (pe->func, pe->arg1.value.num, NUM_VAL (m), res);
        case N_prf: {
            node *v;
            if (!evaluate_i_p_prf (pe->func, pe->arg1.value.num, m, &v)) {
                *res = m;
                DBUG_RETURN (FALSE);
            }
            *res = v;
            DBUG_RETURN (TRUE);
        }

        default:
            break;
        }
        *res = TCmakePrf2 (pe->func, TBmakeNum (pe->arg1.value.num), m);
        DBUG_RETURN (TRUE);
    }
    /* No constant in arguments, or constant cannot be placed on the
     * first position. Anyway we create now just a prf.  */
    else {
        node *a1, *a2;
        if (pe->arg1.is_int)
            a1 = TBmakeNum (pe->arg1.value.num);
        else if (!GetModifier (pe->arg1.value.var, stack, ext_ivs, loopvars, &a1)) {
            *res = a1;
            DBUG_RETURN (FALSE);
        }

        if (pe->arg2.is_int)
            a2 = TBmakeNum (pe->arg2.value.num);
        else if (!GetModifier (pe->arg2.value.var, stack, ext_ivs, loopvars, &a2)) {
            *res = a2;
            DBUG_RETURN (FALSE);
        }

        *res = TCmakePrf2 (pe->func, a1, a2);
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
    if (!predicate || NODE_TYPE (predicate) != N_prf || !PRF_ARGS (predicate)
        || !EXPRS_NEXT (PRF_ARGS (predicate)) || NULL == (arg1 = PRF_ARG1 (predicate))
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
                && NODE_TYPE (ASSIGN_STMT (AVIS_SSAASSIGN (ID_AVIS (ptr->var))))
                     == N_let) {
                node *new_pred;
                node *var = ptr->var;

                /* This loop is used to handle stupid type-conversions
                   inserted everywhere in the cyc:lur code.  */
                while (TRUE) {
                    new_pred = LET_EXPR (ASSIGN_STMT (AVIS_SSAASSIGN (ID_AVIS (var))));

                    if (!PMmatchFlat (PMprf (0, 0), new_pred)) {
                        DBUG_PRINT ("Loop condition is not a "
                                    "primitive function composition");
                        goto cleanup;
                    }

                    /* FIXME: this is a hack, we should do something about
                       type conversions presented in the form of primitive
                       functions.  */
                    if (PRF_PRF (new_pred) == F_type_conv) {
                        if (TYeqTypes (TYPE_TYPE (ID_AVIS (var)),
                                       ID_NTYPE (PRF_ARG2 (new_pred)))
                            && TYeqTypes (ID_NTYPE (PRF_ARG2 (new_pred)),
                                          TYPE_TYPE (PRF_ARG1 (new_pred)))) {
                            node *n;

                            DBUG_PRINT ("Useless conversion found");
                            var = PRF_ARG2 (new_pred);
                            if (NULL != (n = AVIS_SSAASSIGN (ID_AVIS (var)))
                                && NODE_TYPE (ASSIGN_STMT (n)) == N_let)
                                continue;
                            else
                                goto cleanup;
                        }
                    }

                    break;
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
 *  @fn static bool IsLURModifier (node *m, struct idx_vector *iv)
 *
 *  @brief Checks if modifier m (The value added to the Loop Induction Variable)
 *         is one of the following forms:
 *
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
 * @fn static loopc_t GetLoopUnrolling(node *fundef, info *arg_info)
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
 *                                              loop_stride);
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
GetLoopUnrolling (node *fundef, info *arg_info)
{
    node *cond_assign;         /* N_assign */
    node *then_instr;          /* N_assign */
    node *condition;           /* N_expr */
    node *predicate_assign;    /* N_assign */
    node *predicate;           /* N_expr */
    node *modifier_assign;     /* N_assign */
    node *modifier;            /* N_expr */
    node *ext_assign;          /* N_assign */
    loopc_t unroll = UNR_NONE; /* return result  */

    struct prf_expr_queue stack;
    struct idx_vector_queue ext_ivs;
    struct idx_vector *ivtmp;

    TAILQ_INIT (&stack);
    TAILQ_INIT (&ext_ivs);

    bool error = FALSE;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (fundef) == N_fundef, "%s called for non-fundef node",
                 __func__);
    ext_assign = INFO_EXT_ASSIGN (arg_info);

    /* Function must be LOOPFUN */
    if (!FUNDEF_ISLOOPFUN (fundef)) {
        DBUG_PRINT ("Expected Loop fundef");
        DBUG_RETURN (UNR_NONE);
    }

    DBUG_ASSERT (NULL != FUNDEF_BODY (fundef), "function with body required");

    /* search conditional in do fundef */
    cond_assign = LFUfindAssignOfType (BLOCK_ASSIGNS (FUNDEF_BODY (fundef)), N_cond);
    DBUG_ASSERT (NULL != cond_assign, "fundef with none or multiple conditionals");

    /* get condition */
    condition = COND_COND (ASSIGN_STMT (cond_assign));
    DBUG_ASSERT (NULL != condition, "missing condition in conditional");

    /* check for identifier as condition */
    if (NODE_TYPE (condition) != N_id) {
        DBUG_RETURN ((DBUG_PRINT ("condition is not an identifier"), UNR_NONE));
    }

    /* identifier must be a locally defined vardec -- no arg */
    if (NODE_TYPE (AVIS_DECL (ID_AVIS (condition))) != N_vardec) {
        DBUG_PRINT ("identifier is no locally defined vardec");
        DBUG_RETURN (UNR_NONE);
    }

    /* get defining assignment */
    predicate_assign = AVIS_SSAASSIGN (ID_AVIS (condition));
    DBUG_ASSERT (NULL != predicate_assign, "missing SSAASSIGN attribute for condition");
    DBUG_ASSERT (NODE_TYPE (ASSIGN_STMT (predicate_assign)) == N_let,
                 "definition assignment without let");

    /* check predicate for having the correct form */
    predicate = LET_EXPR (ASSIGN_STMT (predicate_assign));
    if (!LFUisLURPredicate (predicate)) {
        DBUG_RETURN ((DBUG_PRINT ("predicate has incorrect form"), UNR_NONE));
    }

    if (!GetLoopIdentifiers (condition, predicate, &stack, &ext_ivs)) {
        DBUG_PRINT ("GetLoopIentifiers returned false");
        goto cleanup;
    }

    /* extract recursive call behind cond, skipping N_annotate */
    then_instr = COND_THENASSIGNS (ASSIGN_STMT (cond_assign));
    then_instr = LFUfindAssignOfType (then_instr, N_let);

    DBUG_ASSERT (NULL != then_instr, "Cannot extract recursive call");
    DBUG_ASSERT (NODE_TYPE (ASSIGN_RHS (then_instr)) == N_ap,
                 "cond of loop fun w/o N_ap in then body");
    DBUG_ASSERT (AP_FUNDEF (ASSIGN_RHS (then_instr)) == fundef,
                 "cond of loop fun w/o recursive call in then body");

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

        /* Assume that we have inital value.  */
        ivtmp->init_value_set = TRUE;

        ivtmp->loopvar
          = LFUgetLoopVariable (ivtmp->var, fundef,
                                AP_ARGS (LET_EXPR (ASSIGN_STMT (then_instr))));

        if (ivtmp->loopvar == NULL) {
            DBUG_PRINT ("no candidate for loop counter");
            goto cleanup;
        }

        loopvar_name = AVIS_NAME (ID_AVIS (ivtmp->var));

        /* check that loop entrance identifier is an external constant.  */
        if (!IsConstantArg (ivtmp->var, fundef, ext_assign, &(ivtmp->init_value))) {
            DBUG_PRINT ("cannot get value for variable %s", var_name);
            /*goto cleanup;*/
            error = TRUE;

            /* We don't have initial value.  */
            ivtmp->init_value_set = FALSE;
        }

        /* copy init_value for later potential usage in set_extremas.  */
        if (ivtmp->init_value_set)
            ivtmp->extrema_init_value = ivtmp->init_value;

        if (!GetModifier (ivtmp->loopvar, &stack, NULL, FALSE, &m)) {
            if (NULL != m) {
                m = FREEdoFreeNode (m);
                m = NULL;
            }

            DBUG_PRINT ("cannot get modifier for variable %s", var_name);
            /*goto cleanup;*/
            error = TRUE;
        }

        /* If modifier is not on the stack */
        if (ID_AVIS (ivtmp->var) != ID_AVIS (ivtmp->loopvar) && NODE_TYPE (m) == N_id
            && ID_AVIS (m) == ID_AVIS (ivtmp->loopvar)) {
            node *p = AVIS_SSAASSIGN (ID_AVIS (ivtmp->loopvar));
            if (NULL == p || NODE_TYPE (ASSIGN_STMT (p)) != N_let) {
                DBUG_PRINT ("cannot get modifier for variable %s", loopvar_name);
                goto cleanup;
            }

            p = LET_EXPR (ASSIGN_STMT (p));
            if (!GetLoopIdentifiers (ivtmp->loopvar, p, &stack, &ext_ivs)
                || !GetModifier (ivtmp->loopvar, &stack, NULL, FALSE, &m)) {
                if (NULL != m) {
                    m = FREEdoFreeNode (m);
                }
                goto cleanup;
            }
        }

        /* This is an implementation of IsLURModifier */
        if (ID_AVIS (ivtmp->var) != ID_AVIS (ivtmp->loopvar) && !IsLURModifier (m, ivtmp))
            goto cleanup;

        m = FREEdoFreeNode (m);
    }

    /* All the modifiers for loop index variables are constructed.
       Now we need to construct an expression for the comparison
       of the recursive call.  Before doing that we mark all the
       loop variables as "un-seen" to avoid possible recursion
       and multiple usage of the same variable in the comparison
       expression.  */
    TAILQ_FOREACH (ivtmp, &ext_ivs, entries)
    {
        ivtmp->seen = FALSE;
    }

    /* First variable would be always loop conditional variable */
    if (!TAILQ_FIRST (&stack) || TAILQ_FIRST (&stack)->arg1.is_int) {
        DBUG_PRINT ("invalid loop predicate variable stack");
        goto cleanup;
    }

    modifier_assign = TAILQ_FIRST (&stack)->arg1.value.var;
    if (!GetModifier (modifier_assign, &stack, &ext_ivs, TRUE, &modifier)) {
        DBUG_PRINT ("cannot evaluate loop modifier");
        goto cleanup;
    }

    DBUG_EXECUTE ({
        print_idx_queue (&ext_ivs);
        print_prf_queue (&stack);
    });

    /* Turn the condition into normal form.  */
    if (!error) {
        unroll = CalcUnrolling (predicate, modifier, &ext_ivs);
    }

    if (unroll == UNR_NONE) {
        set_extrema (predicate, modifier, &ext_ivs, arg_info);
    }
    DBUG_PRINT ("Unrolling value required is %i", unroll);

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

#ifndef DBUG_OFF
/* Helper functions to print queue of type struct idx_vector_queue.  */
static void
print_idx_queue (struct idx_vector_queue *ivs)
{
    struct idx_vector *ivtmp;

    DBUG_ENTER ();
    DBUG_PRINT ("------ Ext variable stack -------");
    TAILQ_FOREACH (ivtmp, ivs, entries)
    DBUG_PRINT ("var = %s, loopvar= %s, modif= (%s %i * i + %i), init=%i",
                AVIS_NAME (ID_AVIS (ivtmp->var)),
                ivtmp->loopvar ? AVIS_NAME (ID_AVIS (ivtmp->loopvar)) : "NULL",
                global.prf_name[ivtmp->mfunc.func], ivtmp->mfunc.a, ivtmp->mfunc.b,
                (int)ivtmp->init_value);

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

/** <!--********************************************************************-->
 *
 * @fn
 *   node *LURUnrollLoopBody(node *fundef, loopc_t unrolling)
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
 *                                              loop_stride);
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
 *       |                                      loop_stride);
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
UnrollLoopBody (node *arg_node, loopc_t unrolling)
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
    loop_body = BLOCK_ASSIGNS (FUNDEF_BODY (arg_node));

    last = loop_body;
    cond_assign = ASSIGN_NEXT (last);
    while ((cond_assign != NULL) && (NODE_TYPE (ASSIGN_STMT (cond_assign)) != N_cond)) {
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

    DBUG_ASSERT (last == LFUfindAssignBeforeCond (arg_node), "oopsie testing");

    /* build up unrolled body */
    new_body = NULL;
    if (unrolling == 1) {
        /* only one time used */
        new_body = loop_body;
    } else {
        /* unrolling */

        /* extract recursive call behind cond */
        then_instr = COND_THENASSIGNS (ASSIGN_STMT (cond_assign));
        DBUG_ASSERT (NODE_TYPE (then_instr) == N_assign,
                     "cond of loop fun w/o N_assign in then body");
        DBUG_ASSERT (NODE_TYPE (ASSIGN_STMT (then_instr)) == N_let,
                     "cond of loop fun w/o N_let in then body");
        DBUG_ASSERT (NODE_TYPE (ASSIGN_RHS (then_instr)) == N_ap,
                     "cond of loop fun w/o N_ap in then body");
        DBUG_ASSERT (AP_FUNDEF (ASSIGN_RHS (then_instr)) == arg_node,
                     "cond of loop fun w/o recursive call in then body");

        /* append copy assignments to loop-body */
        loop_body
          = TCappendAssign (loop_body,
                            CreateCopyAssigns (FUNDEF_ARGS (arg_node),
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

    FUNDEF_VARDECS (arg_node) = TBmakeVardec (predavis, FUNDEF_VARDECS (arg_node));

    predass = TBmakeAssign (TBmakeLet (TBmakeIds (predavis, NULL), TBmakeBool (FALSE)),
                            cond_assign);
    AVIS_SSAASSIGN (predavis) = predass;

    COND_COND (ASSIGN_STMT (cond_assign))
      = FREEdoFreeTree (COND_COND (ASSIGN_STMT (cond_assign)));

    COND_COND (ASSIGN_STMT (cond_assign)) = TBmakeId (predavis);

    cond_assign = ASSIGN_NEXT (cond_assign);
    while (NODE_TYPE (ASSIGN_STMT (cond_assign)) == N_let) {
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
    BLOCK_ASSIGNS (FUNDEF_BODY (arg_node)) = new_body;

    DBUG_RETURN (arg_node);
}

/* traversal functions for LUR travsersal */

/** <!--********************************************************************-->
 *
 * @fn node *LURmodule(node *arg_node, info *arg_info)
 *
 * @brief   - traverse module for LUR
 *
 ******************************************************************************/
node *
LURmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *LURfundef(node *arg_node, info *arg_info)
 *
 * @brief   - traverse fundef and subordinated special fundefs for LUR
 *          - analyse fundef for unrolling
 *          - do the inferred unrolling
 *
 ******************************************************************************/
node *
LURfundef (node *arg_node, info *arg_info)
{
    loopc_t unrolling;
    node *assgn;

    DBUG_ENTER ();

    DBUG_PRINT ("LUR starting for %s %s",
                (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                FUNDEF_NAME (arg_node));

    INFO_FUNDEF (arg_info) = arg_node;

    /* traverse block of fundef */
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    /* analyse fundef for possible unrolling */
    unrolling = GetLoopUnrolling (arg_node, arg_info);

    /* All the work happens at the fundef level, so our preassigns
     * are dealt with here. FIXME: GO back and clean up the
     * preassign confusion in LURassign
     */
    if (NULL != INFO_PREASSIGN (arg_info)) {
        assgn = LFUfindAssignBeforeCond (arg_node);
        ASSIGN_NEXT (assgn)
          = TCappendAssign (INFO_PREASSIGN (arg_info), ASSIGN_NEXT (assgn));
        INFO_PREASSIGN (arg_info) = NULL;
    }

    /* If new vardecs were made, append them to the current set */
    if (INFO_VARDECS (arg_info) != NULL) {
        BLOCK_VARDECS (FUNDEF_BODY (arg_node))
          = TCappendVardec (INFO_VARDECS (arg_info),
                            BLOCK_VARDECS (FUNDEF_BODY (arg_node)));
        INFO_VARDECS (arg_info) = NULL;
    }

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
                CTInote (EMPTY_LOC, "LUR: -maxlur %d would unroll loop", unrolling);
                /*
                 * We use the hard-wired constant 32 here because otherwise
                 * we become annoyed by messages like "-maxlur 1000000000
                 * would unroll loop".
                 */
            }
        }
    }

    DBUG_PRINT ("LUR ending for %s %s",
                (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                FUNDEF_NAME (arg_node));

    /* In case the optimisation is called from the module.  */
    if (!FUNDEF_ISLACFUN (arg_node)) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
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

    DBUG_ASSERT (ASSIGN_STMT (arg_node) != NULL, "assign node without instruction");

    /* stack actual assign */
    old_assign = INFO_ASSIGN (arg_info);
    INFO_ASSIGN (arg_info) = arg_node;

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    pre_assigns = INFO_PREASSIGN (arg_info);
    INFO_PREASSIGN (arg_info) = NULL;

    /* restore stacked assign */
    INFO_ASSIGN (arg_info) = old_assign;

    /* traverse to next assignment in chain */
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    /* integrate pre_assignments in assignment chain and remove this assign */
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
 *         fundef if it is a LOOPFUN.
 *
 ******************************************************************************/
node *
LURap (node *arg_node, info *arg_info)
{
    info *new_arg_info;
    node *calledfn = NULL;

    DBUG_ENTER ();

    DBUG_ASSERT (AP_FUNDEF (arg_node) != NULL, "missing fundef in ap-node");

    AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);

    /* traverse LOOPFUN fundef without recursion */
    if ((FUNDEF_ISLOOPFUN (AP_FUNDEF (arg_node)))
        && (AP_FUNDEF (arg_node) != INFO_FUNDEF (arg_info))) {
        calledfn = AP_FUNDEF (arg_node);
        DBUG_PRINT ("traverse in LOOPFUN %s", FUNDEF_NAME (AP_FUNDEF (arg_node)));

        /* stack arg_info frame for new fundef */
        new_arg_info = MakeInfo ();

        INFO_EXT_ASSIGN (new_arg_info) = INFO_ASSIGN (arg_info);

        /* start traversal of LOOPFUN */
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), new_arg_info);

        /* Append new outer call arguments if the LACFUN generated them for us */
        if (NULL != INFO_NEWOUTERAPARGS (new_arg_info)) {
            DBUG_PRINT ("Appending new arguments to call of %s from %s",
                        FUNDEF_NAME (calledfn), FUNDEF_NAME (INFO_FUNDEF (new_arg_info)));
            AP_ARGS (arg_node)
              = TCappendExprs (AP_ARGS (arg_node), INFO_NEWOUTERAPARGS (new_arg_info));
            INFO_NEWOUTERAPARGS (new_arg_info) = NULL;
        }

        DBUG_PRINT ("traversal of LOOPFUN fundef %s finished\n",
                    FUNDEF_NAME (AP_FUNDEF (arg_node)));
        new_arg_info = FreeInfo (new_arg_info);

    } else {
        DBUG_PRINT ("no traversal of normal fundef %s",
                    FUNDEF_NAME (AP_FUNDEF (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node* LURdoloopUnrolling(node *arg_node)
 *
 * @brief starts the Loop Unrolling traversal for the given fundef.
 *        Does not start in special fundefs.
 *
 ******************************************************************************/
node *
LURdoLoopUnrolling (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef, "called for non-fundef node");

    /*
     * Wrapper code is created in non-SSA form and later on transformed into
     * SSA form using the standard transformation modules lac2fun and
     * ssa_transform. Therefore, we adjust the global control flag.
     */
    global.valid_ssaform = FALSE;

    arg_info = MakeInfo ();

    TRAVpush (TR_lur);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
