#ifndef _sac_interface_makrodefs_h
#define _sac_interface_makrodefs_h

/*
 *  makros for the SAC <-> Interface wrapper functions
 *
 */

/* check for refcount >=1 */
#define SAC_IW_CHECK_RC(a)                                                               \
    if (SAC_ARG_LRC (a) <= 0) {                                                          \
        fprintf (stderr, "Referencecounter reaches 0, no data available!\n");            \
        return (2);                                                                      \
    }

/* argument is simple type */
#define SAC_ARGCALL_SIMPLE(var, type) (*((type *)(SAC_ARG_ELEMS (var))))

/* argument is array type - expand with additional refcounter */
#define SAC_ARGCALL_REFCNT(var, type) (type *)(SAC_ARG_ELEMS (var)), (SAC_ARG_RC (var))

/* result is simple type */
#define SAC_RESULT_SIMPLE(var, type) ((type *)(SAC_ARG_ELEMS ((*var))))

/* result is array type - expand with additional refcounter */
#define SAC_RESULT_REFCNT(var, type)                                                     \
    (type **)(&SAC_ARG_ELEMS ((*var))), (int **)(&SAC_ARG_RC ((*var)))

/* alloc data memory for simple results */
#define SAC_CI_INIT_SIMPLE_RESULT(var, type)                                             \
    SAC_ARG_ELEMS ((*var)) = SAC_MALLOC (sizeof (type));

/* simple type used as direct result */
#define SAC_ASSIGN_RESULT(var, type) (*((type *)SAC_ARG_ELEMS ((*var))))

/* set local refcount of var to value */
#define SAC_SETLOCALRC(var, value) SAC_ARG_LRC ((*var)) = value;

/* */
#define SAC_DECANDFREERC(var)                                                            \
    {                                                                                    \
        if (SAC_ARG_LRC (var) == 0) {                                                    \
            SAC_CI_FreeSACArg (var);                                                     \
        }                                                                                \
    }

#endif
