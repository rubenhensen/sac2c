/*
 *
 * $Log$
 * Revision 2.1  1999/02/23 12:43:51  sacbase
 * new release made
 *
 * Revision 1.6  1998/06/29 08:52:19  cg
 * streamlined tracing facilities
 * tracing on new with-loop and multi-threading operations implemented
 *
 * Revision 1.5  1998/05/18 09:43:57  dkr
 * fixed a bug in SAC_ND_IDX_MODARRAY_AxVxA_CHECK_REUSE
 *   'i' renamed to '__i'
 *
 * Revision 1.4  1998/05/07 14:16:02  cg
 * converted to new naming conventions
 *
 * Revision 1.3  1998/05/07 14:10:09  cg
 * converted to new naming conventions
 *
 * Revision 1.2  1998/05/07 11:14:59  cg
 * converted to new naming conventions
 *
 * Revision 1.1  1998/05/07 08:38:05  cg
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   sac_idx.h
 *
 * description:
 *
 *   This file is part of the SAC standard header file sac.h
 *
 *   It provides definitions of Intermediate Code Macros (ICM)
 *   implemented as real macros.
 *
 *****************************************************************************/

#ifndef SAC_IDX_H

#define SAC_IDX_H

/*
 * Macros used for primitive function idx_psi:
 * ===========================================
 */

#define SAC_ND_IDX_PSI_S(s, a, res)                                                      \
    SAC_TR_PRF_PRINT (("ND_IDX_PSI_S( %s, %s, %s)\n", #s, #a, #res));                    \
    res = SAC_ND_A_FIELD (a)[s];

#define SAC_ND_IDX_PSI_A(s, a, res)                                                      \
    SAC_TR_PRF_PRINT (("ND_IDX_PSI_A( %s, %s, %s)\n", #s, #a, #res));                    \
    {                                                                                    \
        int __i, __s = s;                                                                \
        for (__i = 0; __i < SAC_ND_A_SIZE (res); __i++)                                  \
            SAC_ND_A_FIELD (res)[__i] = SAC_ND_A_FIELD (a)[__s++];                       \
    };

/*
 * Macros used for primitive function idx_modarray:
 * ================================================
 */

#define SAC_ND_IDX_MODARRAY_AxVxA_CHECK_REUSE(line, basetype, res, a, s, val)            \
    SAC_TR_PRF_PRINT (("ND_IDX_MODARRAY_AxVxA_CHECK_REUSE( %s, %s, %s, %s, %s, %s)\n",   \
                       #line, #basetype, #res, #a, #s, #val));                           \
    SAC_ND_CHECK_REUSE_ARRAY (a, res)                                                    \
    {                                                                                    \
        int __i;                                                                         \
        SAC_ND_ALLOC_ARRAY (basetype, res, 0);                                           \
        for (__i = 0; __i < s; __i++)                                                    \
            SAC_ND_A_FIELD (res)[__i] = SAC_ND_A_FIELD (a)[__i];                         \
        for (__i = __i + SAC_ND_A_SIZE (val); __i < SAC_ND_A_SIZE (res); __i++)          \
            SAC_ND_A_FIELD (res)[__i] = SAC_ND_A_FIELD (a)[__i];                         \
    }                                                                                    \
    {                                                                                    \
        int __i, __s;                                                                    \
        for (__s = 0, __i = s; __s < SAC_ND_A_SIZE (val); __i++, __s++)                  \
            SAC_ND_A_FIELD (res)[__i] = SAC_ND_A_FIELD (val)[__s];                       \
    }

#define SAC_ND_IDX_MODARRAY_AxVxA(line, basetype, res, a, s, val)                        \
    SAC_TR_PRF_PRINT (("ND_IDX_MODARRAY_AxVxA( %s, %s, %s, %s, %s, %s)\n", #line,        \
                       #basetype, #res, #a, #s, #val));                                  \
    {                                                                                    \
        int __i, __s;                                                                    \
        SAC_ND_ALLOC_ARRAY (basetype, res, 0);                                           \
        for (__i = 0; __i < s; __i++)                                                    \
            SAC_ND_A_FIELD (res)[__i] = SAC_ND_A_FIELD (a)[__i];                         \
        for (__s = 0; __s < SAC_ND_A_SIZE (val); __i++, __s++)                           \
            SAC_ND_A_FIELD (res)[__i] = SAC_ND_A_FIELD (val)[__s];                       \
        for (; __i < SAC_ND_A_SIZE (res); __i++)                                         \
            SAC_ND_A_FIELD (res)[__i] = SAC_ND_A_FIELD (a)[__i];                         \
    }

#define SAC_ND_IDX_MODARRAY_AxVxS_CHECK_REUSE(line, basetype, res, a, s, val)            \
    SAC_TR_PRF_PRINT (("ND_IDX_MODARRAY_AxVxS_CHECK_REUSE( %s, %s, %s, %s, %s, %s)\n",   \
                       #line, #basetype, #res, #a, #s, #val));                           \
    SAC_ND_CHECK_REUSE_ARRAY (a, res)                                                    \
    {                                                                                    \
        int __i;                                                                         \
        SAC_ND_ALLOC_ARRAY (basetype, res, 0);                                           \
        for (__i = 0; __i < SAC_ND_A_SIZE (res); __i++)                                  \
            SAC_ND_A_FIELD (res)[__i] = SAC_ND_A_FIELD (a)[__i];                         \
    }                                                                                    \
    SAC_ND_A_FIELD (res)[s] = val;

#define SAC_ND_IDX_MODARRAY_AxVxS(line, basetype, res, a, s, val)                        \
    SAC_TR_PRF_PRINT (("ND_IDX_MODARRAY_AxVxS( %s, %s, %s, %s, %s, %s)\n", #line,        \
                       #basetype, #res, #a, #s, #val));                                  \
    {                                                                                    \
        int __i;                                                                         \
        SAC_ND_ALLOC_ARRAY (basetype, res, 0);                                           \
        for (__i = 0; __i < SAC_ND_A_SIZE (res); __i++)                                  \
            SAC_ND_A_FIELD (res)[__i] = SAC_ND_A_FIELD (a)[__i];                         \
    }                                                                                    \
    SAC_ND_A_FIELD (res)[s] = val;

#define SAC_ND_KS_USE_GENVAR_OFFSET(offsetvar, res) offsetvar = res##__destptr;

#endif /* SAC_IDX_H */
