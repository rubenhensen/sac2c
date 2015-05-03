/*****************************************************************************
 *
 * file:   sac_trace.h
 *
 * description:
 *
 *   This file is part of the SAC standard header file sac.h
 *
 *   Trace operations may be selectively activated by the global switches
 *    TRACE_MEM  for memory operations
 *    TRACE_REF  for refcount operations
 *    TRACE_PRF  for primitive function calls
 *
 *   The global switch TRACE indicates any trace activations.
 *
 *****************************************************************************/

#ifndef _SAC_TRACE_H
#define _SAC_TRACE_H

#ifndef SAC_C_EXTERN
#define SAC_C_EXTERN extern
#endif /* SAC_C_EXTERN */

/*
 *  External declarations of global variables and functions defined in trace.c
 *  as part of libsac.
 */

#if SAC_DO_TRACE

#include <stdio.h>
#include <string.h>

SAC_C_EXTERN int SAC_TR_hidden_memcnt;
SAC_C_EXTERN int SAC_TR_array_memcnt;

SAC_C_EXTERN void SAC_TR_Print (char *format, ...);
SAC_C_EXTERN void SAC_TR_IncArrayMemcnt (int size);
SAC_C_EXTERN void SAC_TR_DecArrayMemcnt (int size);
SAC_C_EXTERN void SAC_TR_IncHiddenMemcnt (int size);
SAC_C_EXTERN void SAC_TR_DecHiddenMemcnt (int size);

/*
 * CAUTION!
 * This should be an expression (due to SAC_TR_AA_PRINT), therefore,
 * no ; at the end!
 */
#define SAC_TR_PRINT(msg) SAC_TR_Print msg

#else /* SAC_DO_TRACE */

#define SAC_TR_PRINT(msg)

#endif /* SAC_DO_TRACE */

#if SAC_DO_TRACE_FUN

#define SAC_TR_FUN_PRINT(msg) SAC_TR_PRINT (msg);

#else /* SAC_DO_TRACE_FUN */

#define SAC_TR_FUN_PRINT(msg)

#endif /* SAC_DO_TRACE_FUN */

#if SAC_DO_TRACE_PRF

#define SAC_TR_PRF_PRINT(msg) SAC_TR_PRINT (msg);

#else /* SAC_DO_TRACE_PRF */

#define SAC_TR_PRF_PRINT(msg)

#endif /* SAC_DO_TRACE_PRF */

#if SAC_DO_TRACE_REF

#define SAC_TR_REF_PRINT(msg) SAC_TR_PRINT (msg);

#define SAC_TR_REF_PRINT_RC(var_NT)                                                      \
    SAC_TR_REF_PRINT (("refcnt of %s at address %p: %d", NT_STR (var_NT),                \
                       SAC_ND_GETVAR (var_NT, SAC_ND_A_FIELD (var_NT)),                  \
                       SAC_ND_A_RC (var_NT)))

#else /* SAC_DO_TRACE_REF */

#define SAC_TR_REF_PRINT(msg)
#define SAC_TR_REF_PRINT_RC(name)

#endif /* SAC_DO_TRACE_REF */

#if SAC_DO_TRACE_WL

#define SAC_TR_WL_PRINT(msg) SAC_TR_PRINT (msg);

#else /* SAC_DO_TRACE_WL */

#define SAC_TR_WL_PRINT(msg)

#endif /* SAC_DO_TRACE_WL */

#if SAC_DO_TRACE_AA

#define SAC_TR_AA_PRINT(str, array, pos)                                                 \
    SAC_TR_PRINT (("%s access to array %s at position %d", str, #array, pos)),

#else /* SAC_DO_TRACE_AA */

#define SAC_TR_AA_PRINT(class, array, idx)

#endif /* SAC_DO_TRACE_AA */

#if SAC_DO_TRACE_MT

typedef enum {
    SAC_TR_foldres_int,
    SAC_TR_foldres_float,
    SAC_TR_foldres_double,
    SAC_TR_foldres_hidden
} SAC_TR_foldres_t;

#define SAC_TR_MT_PRINT(msg) SAC_TR_PRINT (msg);

#define SAC_TR_MT_PRINT_FOLD_RESULT(basetype, accu_NT, msg)                              \
    CAT13 (SAC_TR_MT_PRINT_FOLD_RESULT__,                                                \
           NT_SHP (accu_NT) BuildArgs3 (basetype, accu_NT, msg))
#define SAC_TR_MT_PRINT_FOLD_RESULT__SCL(basetype, accu_NT, msg)                         \
    {                                                                                    \
        const SAC_TR_foldres_t SAC_TR_foldres = SAC_TR_foldres_##basetype;               \
        switch (SAC_TR_foldres) {                                                        \
        case SAC_TR_foldres_int:                                                         \
            SAC_TR_MT_PRINT ((msg " (int) %d", SAC_ND_A_FIELD (accu_NT)));               \
            break;                                                                       \
        case SAC_TR_foldres_float:                                                       \
            SAC_TR_MT_PRINT ((msg " (float) %.15g", SAC_ND_A_FIELD (accu_NT)));          \
            break;                                                                       \
        case SAC_TR_foldres_double:                                                      \
            SAC_TR_MT_PRINT ((msg " (double) %.15g", SAC_ND_A_FIELD (accu_NT)));         \
            break;                                                                       \
        case SAC_TR_foldres_hidden:                                                      \
            SAC_TR_MT_PRINT ((msg " (hidden)"));                                         \
            break;                                                                       \
        }                                                                                \
    }
#define SAC_TR_MT_PRINT_FOLD_RESULT__AKS(basetype, accu_NT, msg)                         \
    {                                                                                    \
        int SAC_i;                                                                       \
        char SAC_tmp[19];                                                                \
        char *SAC_s;                                                                     \
        const SAC_TR_foldres_t SAC_TR_foldres = SAC_TR_foldres_##basetype;               \
        SAC_s = (char *)SAC_MALLOC (200 * sizeof (char));                                \
        SAC_s[0] = '\0';                                                                 \
        strcat (SAC_s, "[ ");                                                            \
        for (SAC_i = 0; SAC_i < SAC_MIN (SAC_ND_A_SIZE (accu_NT), 10); SAC_i++) {        \
            switch (SAC_TR_foldres) {                                                    \
            case SAC_TR_foldres_int:                                                     \
                snprintf (SAC_tmp, 19, "%d ", SAC_ND_READ (accu_NT, SAC_i));             \
                break;                                                                   \
            case SAC_TR_foldres_float:                                                   \
                snprintf (SAC_tmp, 19, "%g ", SAC_ND_READ (accu_NT, SAC_i));             \
                break;                                                                   \
            case SAC_TR_foldres_double:                                                  \
                snprintf (SAC_tmp, 19, "%g ", SAC_ND_READ (accu_NT, SAC_i));             \
                break;                                                                   \
            case SAC_TR_foldres_hidden:                                                  \
                snprintf (SAC_tmp, 19, ". ");                                            \
                break;                                                                   \
            }                                                                            \
            strcat (SAC_s, SAC_tmp);                                                     \
        }                                                                                \
        if (SAC_ND_A_SIZE (accu_NT) > 10) {                                              \
            strcat (SAC_s, "... ");                                                      \
        }                                                                                \
        strcat (SAC_s, "]");                                                             \
        SAC_TR_MT_PRINT (                                                                \
          (msg " (%s array) %p = %s", #basetype, SAC_ND_A_FIELD (accu_NT), SAC_s));      \
    }
#define SAC_TR_MT_PRINT_FOLD_RESULT__AKD(basetype, accu_NT, msg)                         \
    SAC_TR_MT_PRINT_FOLD_RESULT__AKS (basetype, accu_NT, msg)
#define SAC_TR_MT_PRINT_FOLD_RESULT__AUD(basetype, accu_NT, msg)                         \
    SAC_TR_MT_PRINT_FOLD_RESULT__AKS (basetype, accu_NT, msg)

#else /* SAC_DO_TRACE_MT */

#define SAC_TR_MT_PRINT(msg)
#define SAC_TR_MT_PRINT_FOLD_RESULT(basetype, accu_var, msg)

#endif /* SAC_DO_TRACE_MT */

#if SAC_DO_TRACE_DISTMEM

#define SAC_TR_DISTMEM_PRINT(msg) SAC_TR_PRINT (msg);

#else /* SAC_DO_TRACE_MEM */

#define SAC_TR_DISTMEM_PRINT(msg)

#endif

#if SAC_DO_TRACE_MEM

#define SAC_TR_MEM_PRINT(msg) SAC_TR_PRINT (msg);

#define SAC_TR_INC_ARRAY_MEMCNT(size) SAC_TR_IncArrayMemcnt (size);

#define SAC_TR_DEC_ARRAY_MEMCNT(size) SAC_TR_DecArrayMemcnt (size);

#define SAC_TR_INC_HIDDEN_MEMCNT(size) SAC_TR_IncHiddenMemcnt (size);

#define SAC_TR_DEC_HIDDEN_MEMCNT(size) SAC_TR_DecHiddenMemcnt (size);

#else /* SAC_DO_TRACE_MEM */

#define SAC_TR_MEM_PRINT(msg)

#define SAC_TR_INC_ARRAY_MEMCNT(size)
#define SAC_TR_DEC_ARRAY_MEMCNT(size)

#define SAC_TR_INC_HIDDEN_MEMCNT(size)
#define SAC_TR_DEC_HIDDEN_MEMCNT(size)

#endif /* SAC_DO_TRACE_MEM */
#else
#define SAC_HM_DEFINE()
#endif /* _SAC_TRACE_H */
