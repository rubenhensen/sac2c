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

#define SAC_ARGCALL_SIMPLE(var, type) (*((type *)(SAC_ARG_ELEMS (var))))
#define SAC_ARGCALL_REFCNT(var, type) (type *)(SAC_ARG_ELEMS (var)), (SAC_ARG_RC (var))
#define SAC_RESULT_SIMPLE(var, type) (*((type **)(SAC_ARG_ELEMS ((*var)))))
#define SAC_RESULT_REFCNT(var, type)                                                     \
    (type **)(SAC_ARG_ELEMS ((*var))), (int **)(SAC_ARG_RC ((*var)))
#define SAC_ASSIGN_RESULT(var, type) (*((type *)SAC_ARG_ELEMS ((*var))))
#define SAC_SETREFCOUNT(var, value)                                                      \
    {                                                                                    \
        *(SAC_ARG_RC ((*var))) = value;                                                  \
        SAC_ARG_LRC ((*var)) = value;                                                    \
    }
#define SAC_DECANDFREERC(var)                                                            \
    {                                                                                    \
        if (SAC_ARG_LRC (var) == 0) {                                                    \
            SAC_CI_FreeSACArg (var);                                                     \
        }                                                                                \
    }

#endif
