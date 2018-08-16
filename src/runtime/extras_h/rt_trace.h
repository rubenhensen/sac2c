/*****************************************************************************
 *
 * file:   sac_rt_trace.h
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

#ifndef _SAC_RT_TRACE_H
#define _SAC_RT_TRACE_H

#if SAC_DO_TRACE
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

#define SAC_TR_AA_FPRINT(str, array, pos, ...)                                           \
    SAC_TR_PRINT ((str " access to array %s at position %d", __VA_ARGS__, #array, pos)),

#else /* SAC_DO_TRACE_AA */

#define SAC_TR_AA_PRINT(str, array, idx)

#define SAC_TR_AA_FPRINT(str, array, pos, ...)

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
/*
 * FIXME: Bug 978
 * This should not directly use functions from stdio (snprintf) as we cannot
 * include them into sac.h.
 */
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

#if SAC_DO_TRACE_GPU
#define SAC_TR_GPU_PRINT(...) SAC_TR_PRINT (("GPU -> " __VA_ARGS__))
#else  /* SAC_DO_TRACE_GPU */
#define SAC_TR_GPU_PRINT( ...)
#endif /* SAC_DO_TRACE_GPU */

#if SAC_DO_TRACE_DISTMEM

#define SAC_TR_DISTMEM_PRINT(...) SAC_TR_PRINT (("DSM -> " __VA_ARGS__))

#define SAC_TR_DISTMEM_PRINT_EXPR(...) SAC_TR_PRINT (("DSM -> " __VA_ARGS__)),

#if SAC_DO_TRACE_AA

/*
 * This allows to print tracing information only if distributed memory AND
 * array access tracing is activated.
 * Why? Tracing per element operations (such as pointer calculations) clutters
 * the trace output and this way it can be switched off easily.
 */
#define SAC_TR_DISTMEM_AA_PRINT(...) SAC_TR_PRINT (("DSM -> " __VA_ARGS__)),

#else /* SAC_DO_TRACE_AA */

#define SAC_TR_DISTMEM_AA_PRINT(...)

#endif /* SAC_DO_TRACE_AA */

#else /* SAC_DO_TRACE_MEM */

#define SAC_TR_DISTMEM_PRINT(...)

#define SAC_TR_DISTMEM_PRINT_EXPR(...)

#define SAC_TR_DISTMEM_AA_PRINT(...)

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

#endif /* _SAC_RT_TRACE_H */

