/*
 *
 * $Log$
 * Revision 1.6  1998/07/10 08:10:49  cg
 *  some bugs fixed, appropriate renaming of macros
 *
 * Revision 1.5  1998/07/02 09:28:49  cg
 * added macro SAC_TR_MT_PRINT_FOLD_RESULT
 *
 * Revision 1.4  1998/06/29 08:52:19  cg
 * streamlined tracing facilities
 * tracing on new with-loop and multi-threading operations implemented
 *
 * Revision 1.3  1998/05/07 08:17:51  cg
 * SAC header files converted to new naming conventions.
 *
 * Revision 1.2  1998/03/24 13:56:21  cg
 * *** empty log message ***
 *
 * Revision 1.1  1998/03/19 16:55:06  cg
 * Initial revision
 *
 *
 */

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

#ifndef SAC_TRACE_H

#define SAC_TRACE_H

/*
 *  External declarations of global variables and functions defined in trace.c
 *  as part of libsac.
 */

#if SAC_DO_TRACE

extern int SAC_TR_hidden_memcnt;
extern int SAC_TR_array_memcnt;

#if SAC_DO_MULTITHREAD

extern void SAC_TRMT_Print (char *format, ...);

extern pthread_mutex_t SAC_TRMT_array_memcnt_lock;
extern pthread_mutex_t SAC_TRMT_hidden_memcnt_lock;

#define SAC_TR_PRINT(msg) SAC_TRMT_Print msg

#define SAC_TR_GET_ACCESS_MEMCNT(lock) pthread_mutex_lock (&lock)
#define SAC_TR_RELEASE_ACCESS_MEMCNT(lock) pthread_mutex_unlock (&lock)

#else /* SAC_DO_MULTITHREAD */

extern void SAC_TR_Print (char *format, ...);

#define SAC_TR_PRINT(msg) SAC_TR_Print msg

#define SAC_TR_GET_ACCESS_MEMCNT(lock)
#define SAC_TR_RELEASE_ACCESS_MEMCNT(lock)

#endif /* SAC_DO_MULTITHREAD */

#endif /* SAC_DO_TRACE */

#if SAC_DO_TRACE_FUN

#define SAC_TR_FUN_PRINT(msg) SAC_TR_PRINT (msg)

#else

#define SAC_TR_FUN_PRINT(msg)

#endif

#if SAC_DO_TRACE_PRF

#define SAC_TR_PRF_PRINT(msg) SAC_TR_PRINT (msg)

#else

#define SAC_TR_PRF_PRINT(msg)

#endif

#if SAC_DO_TRACE_REF

#define SAC_TR_REF_PRINT(msg) SAC_TR_PRINT (msg)

#else

#define SAC_TR_REF_PRINT(msg)

#endif

#if SAC_DO_TRACE_OWL

#define SAC_TR_OWL_PRINT(msg) SAC_TR_PRINT (msg)

#else

#define SAC_TR_OWL_PRINT(msg)

#endif

#if SAC_DO_TRACE_WL

#define SAC_TR_WL_PRINT(msg) SAC_TR_PRINT (msg)

#else

#define SAC_TR_WL_PRINT(msg)

#endif

#if SAC_DO_TRACE_MT

typedef enum { FOLD_int, FOLD_float, FOLD_double, FOLD_array, FOLD_hidden } foldres_t;

#define SAC_TR_MT_PRINT(msg) SAC_TR_PRINT (msg)

#define SAC_TR_MT_PRINT_FOLD_RESULT(type, accu_var, msg)                                 \
    {                                                                                    \
        switch (FOLD_##type) {                                                           \
        case FOLD_int:                                                                   \
            SAC_TR_MT_PRINT ((msg " (int) %d", accu_var));                               \
            break;                                                                       \
        case FOLD_float:                                                                 \
            SAC_TR_MT_PRINT ((msg " (float) %.15g", accu_var));                          \
            break;                                                                       \
        case FOLD_double:                                                                \
            SAC_TR_MT_PRINT ((msg " (double) %.15g", accu_var));                         \
            break;                                                                       \
        case FOLD_array:                                                                 \
            SAC_TR_MT_PRINT ((msg " (array) %p", accu_var));                             \
            break;                                                                       \
        case FOLD_hidden:                                                                \
            SAC_TR_MT_PRINT ((msg " (hidden)"));                                         \
            break;                                                                       \
        }                                                                                \
    }

#else

#define SAC_TR_MT_PRINT(msg)
#define SAC_TR_MT_PRINT_FOLD_RESULT(type, accu_var, msg)

#endif

#if SAC_DO_TRACE_MEM

#define SAC_TR_MEM_PRINT(msg) SAC_TR_PRINT (msg)

#define SAC_TR_INC_ARRAY_MEMCNT(size)                                                    \
    {                                                                                    \
        SAC_TR_GET_ACCESS_MEMCNT (SAC_TR_array_memcnt_lock);                             \
        SAC_TR_array_memcnt += size;                                                     \
        SAC_TR_PRINT (                                                                   \
          ("%d array elements allocated, total now: %d.", size, SAC_TR_array_memcnt));   \
        SAC_TR_RELEASE_ACCESS_MEMCNT (SAC_TR_array_memcnt_lock);                         \
    }

#define SAC_TR_DEC_ARRAY_MEMCNT(size)                                                    \
    {                                                                                    \
        SAC_TR_GET_ACCESS_MEMCNT (SAC_TR_array_memcnt_lock);                             \
        SAC_TR_array_memcnt -= size;                                                     \
        SAC_TR_PRINT (("%d array elements de-allocated, total now: %d.", size,           \
                       SAC_TR_array_memcnt));                                            \
        SAC_TR_RELEASE_ACCESS_MEMCNT (SAC_TR_array_memcnt_lock);                         \
    }

#define SAC_TR_INC_HIDDEN_MEMCNT(size)                                                   \
    {                                                                                    \
        SAC_TR_GET_ACCESS_MEMCNT (SAC_TR_hidden_memcnt_lock);                            \
        SAC_TR_hidden_memcnt += size;                                                    \
        SAC_TR_PRINT (("%d hidden object(s) allocated, total now: %d.", size,            \
                       SAC_TR_hidden_memcnt));                                           \
        SAC_TR_RELEASE_ACCESS_MEMCNT (SAC_TR_hidden_memcnt_lock);                        \
    }

#define SAC_TR_DEC_HIDDEN_MEMCNT(size)                                                   \
    {                                                                                    \
        SAC_TR_GET_ACCESS_MEMCNT (SAC_TR_hidden_memcnt_lock);                            \
        SAC_TR_hidden_memcnt -= size;                                                    \
        SAC_TR_PRINT (("%d hidden object(s) de-allocated, total now: %d.", size,         \
                       SAC_TR_hidden_memcnt));                                           \
        SAC_TR_RELEASE_ACCESS_MEMCNT (SAC_TR_hidden_memcnt_lock);                        \
    }

#else /* SAC_DO_TRACE_MEM */

#define SAC_TR_MEM_PRINT(msg)

#define SAC_TR_INC_ARRAY_MEMCNT(size)
#define SAC_TR_DEC_ARRAY_MEMCNT(size)

#define SAC_TR_INC_HIDDEN_MEMCNT(size)
#define SAC_TR_DEC_HIDDEN_MEMCNT(size)

#endif /* SAC_DO_TRACE_MEM */

#define SAC_TR_REF_PRINT_RC(name)                                                        \
    SAC_TR_REF_PRINT (("refcnt of %s: %d", #name, SAC_ND_A_RC (name)));

#endif /* SAC_TRACE_H */
