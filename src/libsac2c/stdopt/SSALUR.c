/*****************************************************************************
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
#include "tree_utils.h"

#define DBUG_PREFIX "LUR"
#include "debug.h"

/* INFO structure and macros */
#include "SSALUR_info.h"

/* INFO functions */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));
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
static node *GetCallArg (node *, node *, node *);
static bool GetConstantArg (node *, node *, node *, loopc_t *);
static bool GetPredicateData (node *, prf *, loopc_t *);
static node *UnrollLoopBody (node *, loopc_t, info *);
static node *CreateCopyAssigns (node *, node *, node *);
static node *CreateCopyAssignsHelper (node **, node *, node *, node *);

static node *GetLoopVariable (node *, node *, node *);
static bool GetModifier (node *, struct prf_expr_queue *, struct idx_vector_queue *, bool,
                         node **);

loopc_t CalcUnrolling (node *, node *, struct idx_vector_queue *);
static void set_extrema (node *, node *, struct idx_vector_queue *, node *, node *);

#ifndef DBUG_OFF
static void print_idx_queue (struct idx_vector_queue *);
static void print_prf_queue (struct prf_expr_queue *);
#endif

static bool CheckPredicateNF (node *, int *, int *);
static bool Newton (struct idx_vector_queue *, prf, loopc_t, double, double, int,
                    double *);
static struct idx_vector *IsOnIdxQueue (struct idx_vector_queue *, node *);
static struct idx_vector *IsLoopvarOnIdxQueue (struct idx_vector_queue *, node *);
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

    bool error = FALSE;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (fundef) == N_fundef, "%s called for non-fundef node",
                 __func__);

    /* check for do special fundef */
    if (!FUNDEF_ISLOOPFUN (fundef)) {
        DBUG_RETURN ((DBUG_PRINT ("no do-loop special fundef"), UNR_NONE));
    }

    DBUG_ASSERT (NULL != FUNDEF_BODY (fundef), "function with body required");

    /* search conditional in do fundef */
    cond_assign = FindAssignOfType (BLOCK_ASSIGNS (FUNDEF_BODY (fundef)), N_cond);
    DBUG_ASSERT (NULL != cond_assign, "fundef with none or multiple conditionals");

    /* get condition */
    condition = COND_COND (ASSIGN_STMT (cond_assign));
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
    DBUG_ASSERT (NODE_TYPE (ASSIGN_STMT (predicate_assign)) == N_let,
                 "definition assignment without let");

    /* check predicate for having the correct form */
    predicate = LET_EXPR (ASSIGN_STMT (predicate_assign));
    if (!IsLURPredicate (predicate)) {
        DBUG_RETURN ((DBUG_PRINT ("predicate has incorrect form"), UNR_NONE));
    }

    if (!GetLoopIdentifiers (condition, predicate, &stack, &ext_ivs)) {
        DBUG_PRINT ("GetLoopIentifiers returned false");
        goto cleanup;
    }

    /* extract recursive call behind cond, skipping N_annotate */
    then_instr = COND_THENASSIGNS (ASSIGN_STMT (cond_assign));
    then_instr = FindAssignOfType (then_instr, N_let);

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

        ivtmp->loopvar = GetLoopVariable (ivtmp->var, fundef,
                                          AP_ARGS (LET_EXPR (ASSIGN_STMT (then_instr))));

        if (ivtmp->loopvar == NULL) {
            DBUG_PRINT ("no candidate for loop counter");
            goto cleanup;
        }

        loopvar_name = AVIS_NAME (ID_AVIS (ivtmp->var));

        /* check that loop entrance identifier is an external constant.  */
        if (!GetConstantArg (ivtmp->var, fundef, ext_assign, &(ivtmp->init_value))) {
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
    if (!error)
        unroll = CalcUnrolling (predicate, modifier, &ext_ivs);

    if (unroll == UNR_NONE)
        set_extrema (predicate, modifier, &ext_ivs, fundef, ext_assign);

    DBUG_PRINT ("predicate unrollable returned %li", unroll);

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

/* Helpers for set_extrema.  */
enum arg_sign { arg_plus, arg_minus };

struct addition {
    enum arg_sign sign;
    node *arg;
    TAILQ_ENTRY (addition) entries;
};

TAILQ_HEAD (addition_queue, addition);

#define flip_sign(sign) ((enum arg_sign)(sign == arg_plus ? arg_minus : arg_plus))

/*  This function converts a nested addition/substraction expression
    of numbers and variables, creating a list of elements with the
    sign.  For example:
      a + b - (3 - (c + d))  will be transformed into:
      [+ a], [+ b], [- 3], [+ c], [+ d]

    In case one of the variables of the list is VAR or LOOPVAR,
    VAR_FOUND or LOOPVAR_FOUND is set on, the variable is not
    included in the list, but the sign of the variable is saved
    in the VAR_OR_LOOPVAR_SIGN argument.  */
bool
make_additions (node *target, node *var, bool *var_found, node *loopvar,
                bool *loopvar_found, enum arg_sign *var_or_loopvar_sign,
                enum arg_sign sign, struct addition_queue *q)
{
    if (NODE_TYPE (target) == N_num) {
        struct addition *add;
        add = (struct addition *)MEMmalloc (sizeof (struct addition));
        add->sign = sign;
        add->arg = DUPdoDupTree (target);
        TAILQ_INSERT_TAIL (q, add, entries);
        return TRUE;
    }

    if (NODE_TYPE (target) == N_id) {
        if (ID_AVIS (target) == ID_AVIS (var)) {
            *var_found = TRUE; *var_or_loopvar_sign = sign;
        } else if (ID_AVIS (target) == ID_AVIS (loopvar)) {
            *loopvar_found = TRUE; *var_or_loopvar_sign = sign;
        } else {
            struct addition *add;
            add = (struct addition *)MEMmalloc (sizeof (struct addition));
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

        if (PRF_PRF (target) == F_add_SxS) {
            s1 = arg_plus; s2 = arg_plus;
        } else if (PRF_PRF (target) == F_sub_SxS) {
            s1 = arg_plus; s2 = arg_minus;
        } else
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

static void
set_extrema (node *predicate, node *modifier, struct idx_vector_queue *ivs, node *fundef,
             node *ext_assign)
{
    struct idx_vector *ivtmp, *ivptr = NULL;
    unsigned var_count = 0;
    struct addition_queue addq;
    struct addition *add;
    bool var_found = FALSE, loopvar_found = FALSE;
    enum arg_sign var_or_loopvar_sign = arg_plus;

    DBUG_ENTER ();

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

            /* Fail in case modifier has incorrect form.  */
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

#if 0
  /* This is a case when we have an intial value, and we set an extrema.  */
  if (ivptr->init_value_set)
    {
      if (ivptr->mfunc.b > 0)
	AVIS_MIN (ID_AVIS (ivptr->var))
	  = TBmakeNum (ivptr->extrema_init_value);
      else
	AVIS_MAX (ID_AVIS (ivptr->var))
	  = TBmakeNum (ivptr->extrema_init_value + 1);
    }
  else
    {
      node * ex = GetCallArg (ivptr->var, fundef, ext_assign);
      if (ivptr->mfunc.b > 0)
	AVIS_MIN (ID_AVIS (ivptr->var)) = ex;
      else
	AVIS_MAX (ID_AVIS (ivptr->var))
	  = TCmakePrf2 (F_add_SxS, ex, TBmakeNum (1));
    }
#endif

    if (make_additions (modifier, ivptr->var, &var_found, ivptr->loopvar, &loopvar_found,
                        &var_or_loopvar_sign, arg_plus, &addq)) {
        node *expr = NULL;

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

        /* Adding reveres-application of the modifier.  */
        if (var_found) {
            struct addition *t;
            t = (struct addition *)MEMmalloc (sizeof (struct addition));
            if (var_or_loopvar_sign == arg_minus)
                t->sign = arg_plus;
            else
                t->sign = arg_minus;

            t->arg = TBmakeNum (ivptr->mfunc.b);
            TAILQ_INSERT_TAIL (&addq, t, entries);
        }

        /* convert the addq back into an expression.  */
        expr = TBmakeNum (0);
        TAILQ_FOREACH (add, &addq, entries)
        {
            expr = TCmakePrf2 (F_add_SxS, expr,
                               add->sign == arg_minus ? TCmakePrf1 (F_neg_S, (add->arg))
                                                      : add->arg);
        }

        /* Check predicate if stride is positive, and:
              -i + expr >	 t  ===>  i <   expr - t;  extrema_max =  expr - t;
              -i + expr >= t  ===>  i <=  expr - t;  extrema_max =  expr - t + 1;
               i + expr <  t  ===>  i <  -expr + t;  extrema_max = -expr + t;
               i + expr <= t  ===>  i <= -expr + t;  extrema_max = -expr + t + 1;
            In case stride is negative:
              -i + expr <  t  ===>  i >   expr - t;  extrema_min =  expr - t;
              -i + expr <= t  ===>  i >=  expr - t;  extrema_min =  expr - t - 1;
               i + expr >  t  ===>  i >  -expr + t;  extrema_min = -expr + t;
               i + expr >= t  ===>  i >= -expr + t;  extrema_min = -expr + t - 1;  */


#if 0
      /* This is a case when we set an extrema using
	 information from the loop modifier.  */
      if (ivptr->mfunc.b > 0)
	{
          node *t = PRF_ARG1 (predicate);
	  /* Stride is positive.  */
	  if (var_or_loopvar_sign == arg_minus)
	    {
	      if (PRF_PRF (predicate) == F_lt_SxS)
		AVIS_MAX (ID_AVIS (ivptr->var))
		  = TCmakePrf2 (F_sub_SxS, expr, t);
	      else if (PRF_PRF (predicate) == F_le_SxS)
		AVIS_MAX (ID_AVIS (ivptr->var))
		  = TCmakePrf2 (F_add_SxS, TCmakePrf2 (F_sub_SxS, expr, t),
				TBmakeNum (1));
	    }
	  else if (var_or_loopvar_sign == arg_plus)
	    {
	      if (PRF_PRF (predicate) == F_lt_SxS)
		AVIS_MAX (ID_AVIS (ivptr->var))
		  = TCmakePrf2 (F_sub_SxS, t, expr);
	      else if (PRF_PRF (predicate) == F_le_SxS)
		AVIS_MAX (ID_AVIS (ivptr->var))
		  = TCmakePrf2 (F_add_SxS, TCmakePrf2 (F_sub_SxS, t, expr),
				TBmakeNum (1));
	    }
	}
      else
	{
	  /* Stride is negative.  */
	  if (var_or_loopvar_sign == arg_minus)
	    {
	      if (PRF_PRF (predicate) == F_gt_SxS)
		AVIS_MIN (ID_AVIS (ivptr->var))
		  = TCmakePrf2 (F_sub_SxS, expr, t);
	      else if (PRF_PRF (predicate) == F_ge_SxS)
		AVIS_MIN (ID_AVIS (ivptr->var))
		  = TCmakePrf2 (F_sub_SxS, TCmakePrf2 (F_sub_SxS, expr, t),
				TBmakeNum (1));
	    }
	  else if (var_or_loopvar_sign == arg_plus)
	    {
	      if (PRF_PRF (predicate) == F_gt_SxS)
		AVIS_MIN (ID_AVIS (ivptr->var))
		  = TCmakePrf2 (F_sub_SxS, t, expr);
	      else if (PRF_PRF (predicate) == F_ge_SxS)
		AVIS_MIN (ID_AVIS (ivptr->var))
		  = TCmakePrf2 (F_sub_SxS, TCmakePrf2 (F_sub_SxS, t, expr),
				TBmakeNum (1));
	    }

	}
#endif
    } else
        DBUG_PRINT ("cannot extract extrema from the loop comparison");

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

    DBUG_RETURN ();
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

    if (NODE_TYPE (ASSIGN_STMT (arg_node)) == linfo->nt) {
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
    struct local_info linfo;

    DBUG_ENTER ();

    linfo.res = NULL; linfo.nt = n;

    TRAVpushAnonymous ((anontrav_t[]){{N_assign, &ATravFilter}, {(nodetype)0, NULL}},
                       &TRAVsons);
    assigns = TRAVopt (assigns, (info *)(void *)&linfo);
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

static double
f (struct m_func mfunc, loopc_t init_l, double iter)
{
    double init = (double)init_l;
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

static double
f_prime (struct m_func mfunc, loopc_t init_l, double iter)
{
    double init = (double)init_l;
    if (mfunc.a == 1)
        /* iff f(x) = bx, then f'(x) = b  */
        return mfunc.b;

    else if (mfunc.func == F_mul_SxS) {
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

        /* f (prev) - term */
        TAILQ_FOREACH (ivtmp, ivs, entries)
        {
            f_prev += f (ivtmp->mfunc, ivtmp->init_value, prev);
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
loopc_t
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
        loopc_t c_sum = 0;
        loopc_t t;

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
                    DBUG_RETURN (res > 0 ? (loopc_t) + floor (res) + 1 : 1);
                } else if (loop_pred == F_ge_SxS)
                    DBUG_RETURN (ivtmp->init_value * ivtmp->mfunc.a < t ? 1 : UNR_NONE);
            }
            /* (-x0)*a0^k <= (-c) | x0 > 0, c > 0 */
            else if (ivtmp->init_value < 0 && t < 0) {
                if (loop_pred == F_ge_SxS) {
                    double res = log ((double)(t / ivtmp->init_value))
                                 / log ((double)(ivtmp->mfunc.a));
                    DBUG_RETURN (res > 0 ? (loopc_t) + floor (res) + 1 : 1);
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
                    DBUG_RETURN (res > 0 ? (loopc_t) + floor (res) + 1 : 1);
                } else if (loop_pred == F_le_SxS)
                    DBUG_RETURN (ivtmp->init_value / ivtmp->mfunc.a > t ? 1 : UNR_NONE);
            }
            /* (-x0)/(a0^k) >= (-c) , x0 > 0, c > 0 */
            else if (ivtmp->init_value < 0 && t < 0) {
                if (loop_pred == F_le_SxS) {
                    double res = log ((double)(ivtmp->init_value / t))
                                 / log ((double)(ivtmp->mfunc.a));
                    DBUG_RETURN (res > 0 ? (loopc_t) + floor (res) + 1 : 1);
                } else if (loop_pred == F_ge_SxS)
                    DBUG_RETURN (ivtmp->init_value / ivtmp->mfunc.a < t ? 1 : UNR_NONE);
            }
        }

    } else {
        int max_iter = 100;
        double res, tol = 0.001;

        if (Newton (ivs, loop_pred, term - cst_value, 0, tol, max_iter, &res)
            && res > 0) {
            loopc_t iter_count = (loopc_t) + floor (res) + 1;
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
 * @fn static bool GetPredicateData (node * expr, prf * pred, loopc_t * term)
 *
 * @brief get prf and const from expression and adjust prf to the form:
 *              id <prf> const
 *        by inverting the comparison function if necessary.
 *
 ******************************************************************************/
static bool
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
        if (!PMmatchFlat (pat, arg2)) {
            DBUG_PRINT ("Constant not found where constant expected");
            DBUG_RETURN (FALSE);
        }
    } else {
        /* second arg is identifier */
        if (!PMmatchFlat (pat, arg1)) {
            DBUG_PRINT ("Constant not found where constant expected");
            DBUG_RETURN (FALSE);
        }

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

    DBUG_RETURN (TRUE);
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
static struct idx_vector *
IsOnIdxQueue (struct idx_vector_queue *ivs, node *var)
{
    struct idx_vector *iv;

    DBUG_ENTER ();

    if (ivs == NULL)
        DBUG_RETURN (NULL);

    TAILQ_FOREACH (iv, ivs, entries)
    {
        if (iv->var && ID_AVIS (iv->var) == ID_AVIS (var))
            DBUG_RETURN (iv);
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
    expr = (struct prf_expr *)MEMmalloc (sizeof (struct prf_expr));
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
            idxv = (struct idx_vector *)MEMmalloc (sizeof (struct idx_vector));
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
            idxv = (struct idx_vector *)MEMmalloc (sizeof (struct idx_vector));
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
                        if (TYeqTypes (ID_NTYPE (var),
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

static node *
GetCallArg (node *id, node *fundef, node *ext_assign)
{
    node *arg_chain;
    node *param_chain;
    node *param;
    int pos;
    int i;

    DBUG_ENTER ();

    /* Check if id is an arg of this fundef */
    if (NODE_TYPE (AVIS_DECL (ID_AVIS (id))) != N_arg) {
        DBUG_PRINT ("identifier %s is not fundef argument", AVIS_NAME (ID_AVIS (id)));
        DBUG_RETURN (NULL);
    }

    /* Get argument position in fundef arg chain */
    arg_chain = FUNDEF_ARGS (fundef);
    pos = 1;
    while ((arg_chain != NULL) && (arg_chain != AVIS_DECL (ID_AVIS (id)))) {
        arg_chain = ARG_NEXT (arg_chain);
        pos++;
    }

    DBUG_ASSERT (arg_chain != NULL, "arg not found in fundef arg chain");

    /* Get matching parameter expr-node */
    param_chain = AP_ARGS (ASSIGN_RHS (ext_assign));

    for (i = 1; i < pos; i++) {
        param_chain = EXPRS_NEXT (param_chain);
    }

    DBUG_ASSERT (param_chain != NULL, "missing matching parameter");
    param = EXPRS_EXPR (param_chain);

    DBUG_RETURN (param);
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
    node *param;
    constant *co;
    node *num;

    DBUG_ENTER ();

    param = GetCallArg (id, fundef, ext_assign);

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

    DBUG_PRINT ("loop entrance counter: %s = %li", AVIS_NAME (ID_AVIS (id)),
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
UnrollLoopBody (node *fundef, loopc_t unrolling, info *arg_info)
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
    loop_body = BLOCK_ASSIGNS (FUNDEF_BODY (fundef));

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
        DBUG_ASSERT (AP_FUNDEF (ASSIGN_RHS (then_instr)) == fundef,
                     "cond of loop fun w/o recursiv call in then body");

        /* append copy assignments to loop-body */
        loop_body = TCappendAssign (loop_body,
                                    CreateCopyAssigns (FUNDEF_ARGS (fundef),
                                                       AP_ARGS (ASSIGN_RHS (then_instr)),
                                                       fundef));

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

    FUNDEF_VARDECS (fundef) = TBmakeVardec (predavis, FUNDEF_VARDECS (fundef));

    predass = TBmakeAssign (TBmakeLet (TBmakeIds (predavis, NULL), TBmakeBool (FALSE)),
                            cond_assign);
    AVIS_SSAASSIGN (predavis) = predass;

#ifdef LETISAADOIT
    if (PHisSAAMode ()) {
        AVIS_DIM (predavis) = TBmakeNum (0);
        AVIS_SHAPE (predavis) = TCmakeIntVector (NULL);
    }
#endif // LETISAADOIT

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
    BLOCK_ASSIGNS (FUNDEF_BODY (fundef)) = new_body;

    DBUG_RETURN (fundef);
}

/** <!--********************************************************************-->
 *
 * @fn static node *CreateCopyAssigns( node *arg_chain,
 *                                     node *rec_chain,
 *                                     node *fundef)
 *
 * @brief
 *   this functions builds up an assignment chain of copy assignments for
 *   all identifiers used in the recursive call to have loop back to the
 *   args in the functions signature.
 *
 *   We need to perform a rename encase an argument is passed back to the
 *   loop as a different argument
 *
 *   loop( int a, int b){
 *     loop( b, a);
 *   }
 *
 *   !=
 *
 *   loop( int a, int b){
 *     a = b;
 *     b = a;
 *     loop( a, b);
 *   }
 *
 *
 ******************************************************************************/
static node *
CreateCopyAssigns (node *arg_chain, node *rec_chain, node *fundef)
{
    node *copy_assigns;
    node *copy_assigns2 = NULL;

    DBUG_ENTER ();

    DBUG_ASSERT (arg_chain != NULL && rec_chain != NULL,
                 "arg_chain and rec_chain must not be NULL");

    copy_assigns = CreateCopyAssignsHelper (&copy_assigns2, arg_chain, rec_chain, fundef);
    copy_assigns = TCappendAssign (copy_assigns2, copy_assigns);

    DBUG_RETURN (copy_assigns);
}

static node *
CreateCopyAssignsHelper (node **copy_assigns2, node *arg_chain, node *rec_chain,
                         node *fundef)
{
    node *copy_assigns;
    node *right_id;
    node *right_id2;
    node *avis;

    DBUG_ENTER ();

    if (arg_chain != NULL) {
        /* process further identifiers in chain */
        copy_assigns = CreateCopyAssignsHelper (copy_assigns2, ARG_NEXT (arg_chain),
                                                EXPRS_NEXT (rec_chain), fundef);

        /* make right identifer as used in recursive call */
        DBUG_ASSERT (NODE_TYPE (EXPRS_EXPR (rec_chain)) == N_id,
                     "non id node as paramter in recursive call");

        avis = TBmakeAvis (TRAVtmpVar (), TYcopyType (AVIS_TYPE (ARG_AVIS (arg_chain))));

        FUNDEF_VARDECS (fundef) = TBmakeVardec (avis, FUNDEF_VARDECS (fundef));

        right_id = TBmakeId (ID_AVIS (EXPRS_EXPR (rec_chain)));
        right_id2 = TBmakeId (avis);
        /* make copy assignment */
        *copy_assigns2
          = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), right_id), *copy_assigns2);
        copy_assigns
          = TBmakeAssign (TBmakeLet (TBmakeIds (ARG_AVIS (arg_chain), NULL), right_id2),
                          copy_assigns);
        AVIS_SSAASSIGN (avis) = *copy_assigns2;
        AVIS_SSAASSIGN (ARG_AVIS (arg_chain)) = copy_assigns;

    } else {
        DBUG_ASSERT (rec_chain == NULL,
                     "different chains of args and calling parameters");

        copy_assigns = NULL;
    }

    DBUG_RETURN (copy_assigns);
}

/* traversal functions for SSALUR travsersal */

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
 * @brief   - traverse fundef and subordinated special fundefs for
 *            LUR and WLUR
 *          - analyse fundef for unrolling
 *          - do the infered unrolling
 *
 ******************************************************************************/
node *
LURfundef (node *arg_node, info *arg_info)
{
    loopc_t unrolling = UNR_NONE;

    DBUG_ENTER ();

    INFO_FUNDEF (arg_info) = arg_node;

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

    // analyse fundef for possible unrolling

    // Use FUNDEF_LOOPCOUNT, if available
    if (UNR_NONE != FUNDEF_LOOPCOUNT (arg_node)) {
        unrolling = FUNDEF_LOOPCOUNT (arg_node);
    } else {
        if (global.optimize.dolur) {
            unrolling = GetLoopUnrolling (arg_node, INFO_EXT_ASSIGN (arg_info));
        }
    }

    // Even if we do not unroll, this value may be of use to PHUT, etc.
    FUNDEF_LOOPCOUNT (arg_node) = (int)unrolling;

    if (unrolling != UNR_NONE) {
        if (unrolling <= global.unrnum) {
            DBUG_PRINT ("unrolling loop %s %ld times ", FUNDEF_NAME (arg_node), unrolling);

            global.optcounters.lunr_expr++;

            // start do-loop unrolling - this leads to non ssa form code
            arg_node = UnrollLoopBody (arg_node, unrolling, arg_info);

        } else {
            DBUG_PRINT ("no unrolling of %s: should be %ld (but set to maxlur %d)",
                        FUNDEF_NAME (arg_node), unrolling, global.unrnum);

            if (unrolling <= 32) {
                CTInote ("LUR: -maxlur %ld would unroll loop", unrolling);
                /*
                 * We use the hard-wired constant 32 here because otherwise
                 * we become annoyed by messages like "-maxlur 1000000000
                 * would unroll loop".
                 */
            }
        }
    }

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

    /* integrate pre_assignments in assignment chain and remove this
     * assign */
    if (pre_assigns != NULL) {
        tmp = arg_node;
        TUclearSsaAssign (arg_node);
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
 * @fn node *  LURdoLoopUnrolling (node *  fundef)
 *
 * @brief Starts the LoopUnrolling traversal for the given fundef.
 *        Does not start in special fundefs.
 *
 ******************************************************************************/
node *
LURdoLoopUnrolling (node *fundef)
{
    info *arg_info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (fundef) == N_fundef, "SSALUR called for non-fundef node");

    /* Wrapper code is created in non-SSA form and later on transformed into
     * SSA form using the standard transformation modules lac2fun and
     * ssa_transform. Therefore, we adjust the global control flag.
     */
    global.valid_ssaform = FALSE;

    arg_info = MakeInfo ();

    TRAVpush (TR_lur);
    fundef = TRAVdo (fundef, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (fundef);
}

#undef DBUG_PREFIX
