/*
 * $Id$
 */

#ifndef _SAC_SSALUR_H_
#define _SAC_SSALUR_H_

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
    node *var;
    node *loopvar;
    struct m_func mfunc;
    loopc_t init_value;
    TAILQ_ENTRY (idx_vector) entries;
};

TAILQ_HEAD (prf_expr_queue, prf_expr);
TAILQ_HEAD (idx_vector_queue, idx_vector);

/*****************************************************************************
 *
 * SSALUR
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

extern node *LURfundef (node *arg_node, info *arg_info);
extern node *LURassign (node *arg_node, info *arg_info);
extern node *LURap (node *arg_node, info *arg_info);

#endif /* _SAC_SSALUR_H_ */
