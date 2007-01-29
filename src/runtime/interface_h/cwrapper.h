/*
 *
 * $Id$
 *
 */

#ifndef _SAC_CWRAPPER_H_
#define _SAC_CWRAPPER_H_

/*
 *
 * Makros for compiling the SAC <-> c Interface wrapper functions
 *
 */

/* check for refcount >=1 , decrement refcounter */
#define SAC_IW_CHECKDEC_RC(a, CONST_T_HIDDEN)                                            \
    if (SAC_ARG_LRC (a) <= 0) {                                                          \
        SAC_RuntimeError ("Referencecounter reaches 0, no data available!\n");           \
    }                                                                                    \
    if ((SAC_ARG_DIM (a) > 0) || (SAC_ARG_TYPE (a) == CONST_T_HIDDEN)) {                 \
        /* decrement only if refcounted arg */                                           \
        SAC_ARG_LRC (a) = SAC_ARG_LRC (a) - 1;                                           \
    }

/* restore old refcounter */
#define SAC_IW_INC_RC(a, CONST_T_HIDDEN)                                                 \
    if ((SAC_ARG_DIM (a) > 0) || (SAC_ARG_TYPE (a) == CONST_T_HIDDEN)) {                 \
        /* increment only if refcounted arg */                                           \
        SAC_ARG_LRC (a) = SAC_ARG_LRC (a) + 1;                                           \
    }

/* argument is simple type */
#define SAC_ARGCALL_SIMPLE(var, type) (*((type *)(SAC_ARG_ELEMS (var))))

/* argument is array type - expand with additional refcounter */
#define SAC_ARGCALL_REFCNT(var, type) (type *)(SAC_ARG_ELEMS (var)), (SAC_ARG_RC (var))

/* argument is simple type */
#define SAC_ARGCALL_INOUT_SIMPLE(var, type) ((type *)(SAC_ARG_ELEMS ((var))))

/* argument is array type - expand with additional refcounter */
#define SAC_ARGCALL_INOUT_REFCNT(var, type)                                              \
    (type **)(&SAC_ARG_ELEMS ((var))), (int **)(&SAC_ARG_RC ((var)))

/* result is simple type */
#define SAC_RESULT_SIMPLE(var, type) ((type *)(SAC_ARG_ELEMS ((*var))))

/* result is array type - expand with additional refcounter
 * also used if arg is tagged as inout
 */
#define SAC_RESULT_REFCNT(var, type)                                                     \
    (type **)(&SAC_ARG_ELEMS ((*var))), (int **)(&SAC_ARG_RC ((*var)))

/* result is T_hidden and refcounted */
#define SAC_RESULT_HIDDEN_RC(var, type)                                                  \
    (type *)(&SAC_ARG_ELEMS ((*var))), (int **)(&SAC_ARG_RC ((*var)))

/* alloc data memory for simple results */
#define SAC_CI_INIT_SIMPLE_RESULT(var, type)                                             \
    SAC_ARG_ELEMS ((*var)) = SAC_MALLOC (sizeof (type));

/* simple type used as direct result */
#define SAC_ASSIGN_RESULT(var, type) (*((type *)SAC_ARG_ELEMS ((*var))))

/* set local refcount of var to value */
#define SAC_SETLOCALRC(var, value) SAC_ARG_LRC ((*var)) = value;

/*
 * decrement local refcounter
 * if refcount reaches 0, data and refcounter has been freed by SAC-function
 */
#define SAC_DECLOCALRC(var)                                                              \
    SAC_ARG_LRC (var) = SAC_ARG_LRC (var) - 1;                                           \
    if (SAC_ARG_LRC (var) == 0) {                                                        \
        SAC_ARG_ELEMS (var) = NULL;                                                      \
        SAC_ARG_RC (var) = NULL;                                                         \
    }

/* checks at the beginning of each wrapper if the module has been initialized */
#define SAC_IW_CHECK_MODINIT(flag, initfun)                                              \
    if (!flag) {                                                                         \
        flag = true;                                                                     \
        initfun ();                                                                      \
    }

#endif /* _SAC_CWRAPPER_H_ */
