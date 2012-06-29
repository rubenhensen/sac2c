/*
 * $Id$
 */

#ifndef _SAC_LUR_H_
#define _SAC_LUR_H_

#include "types.h"
#include <sys/queue.h>

/* type to perform loop unrolling operations on */
typedef long loopc_t;

struct int_or_var {
    bool is_int;
    union union_int_or_var {
        node *var;
        /* FIXME: What type does
           PMint (1, PMAgetValue (x)) returns?  */
        int num;
    } value;
};

struct prf_expr {
    node *lhs;
    prf func;
    struct int_or_var arg1;
    struct int_or_var arg2;
    TAILQ_ENTRY (prf_expr) entries;
};

/* Structure to store function a*x + b  or  x/a + b  */
struct m_func {
    prf func;
    int a, b;
};

struct idx_vector {
    /* Loop predicate is normally using a set of variables introduced
       being different from the parameters of the loop function.  So
       assuming you have:  for(i = 0; i < 10; i++), SaC would generate
       the following code for generator:
         _al_55 = -9;
         _al_56 = _add_SxS_( _al_55, i);
         _ctz_53 = 0;
         _pinl_25__flat_59 = _lt_SxS_( _al_56, _ctz_53);
         if (_pinl_25__flat_59)
            ...
      So we start with the if-expression, and recursively save the
      definitions of the variables, the variables that are not reachable
      from the assignment are called external variables.  */
    node *var;

    /* For each external variable, we have to make sure that we will
       be able to find it in the recursive call.  We have a separate
       field, to catch smart-ass renaming cases like:
         int while_loppXXX (int i, int j)
           if (...)
             xxx = while_loopXXX (j, i)

       so in this particular case we will have a variable `i', and
       loop-variable `j'.  */
    node *loopvar;

    /* Used internally to avoid recursion when creating a predicate.  */
    bool seen;

    /* Modifier of the variable stored in a normal form.  */
    struct m_func mfunc;

    /* Initial value of the variable.  */
    loopc_t init_value;

    /* We know that GetModifier applied on the modifier of the loop
       will compensate application of index-modifiers by reverse-applying
       it on the initial value.  It does not help in case we want to
       set extremas.  So we will copy init_value into extrema_init_value
       to use it in set_extremas.  */
    loopc_t extrema_init_value;

    /* In case we keep the results for further MIN/MAX/STRIDE
       we ay not have initial value available.  In that case
       this flag is going to be on.  */
    bool init_value_set;

    /* Internal field used when checking the result of interpolation.  */
    loopc_t value;

    /* Queue entry.  */
    TAILQ_ENTRY (idx_vector) entries;
};

TAILQ_HEAD (prf_expr_queue, prf_expr);
TAILQ_HEAD (idx_vector_queue, idx_vector);

/*****************************************************************************
 *
 * LUR
 *
 * prefix: LUR
 *
 * description:
 *
 *   This module implements loop-unrolling for special do-functions in ssa
 *   form. all while loops have been removed and converted to do-loops before
 *   so we have to deal only with the do loops.
 *
 *****************************************************************************/
extern node *LURdoLoopUnrolling (node *fundef);

extern node *LURmodule (node *arg_node, info *arg_info);
extern node *LURfundef (node *arg_node, info *arg_info);
extern node *LURassign (node *arg_node, info *arg_info);
extern node *LURap (node *arg_node, info *arg_info);

#endif /* _SAC_LUR_H_ */
