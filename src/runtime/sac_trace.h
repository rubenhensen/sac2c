/*
 *
 * $Log$
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

#define SAC_TR_MT_PRINT(msg) SAC_TR_PRINT (msg)

#else

#define SAC_TR_MT_PRINT(msg)

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

#if 0

/* 
 *  Macros for tracing memory and/or refcount operations
 */

#if (SAC_DO_TRACE_MEM || SAC_DO_TRACE_REF || SAC_DO_TRACE)

#define SAC_TR_PRINT_TRACEHEADER_ALL(text) SAC_TR_PrintTraceHeader text

#define SAC_TR_PRINT_ARRAY_FREE(name)                                                    \
    SAC_TR_PrintTraceInfo ("freeing array %s (adr: %p)", #name, SAC_ND_A_FIELD (name))

#define SAC_TR_PRINT_HIDDEN_FREE(name)                                                   \
    SAC_TR_PrintTraceInfo ("freeing hidden %s (adr: %p)", #name, name)

#else /* SAC_DO_TRACE_MEM || SAC_DO_TRACE_REF */

#define SAC_TR_PRINT_TRACEHEADER_ALL(text)
#define SAC_TR_PRINT_ARRAY_FREE(name)
#define SAC_TR_PRINT_HIDDEN_FREE(name)

#endif /* SAC_DO_TRACE_MEM || SAC_DO_TRACE_REF */



/* 
 *  Macros for tracing refcount operations
 */

#if (SAC_DO_TRACE_REF || SAC_DO_TRACE)

#define SAC_TR_PRINT_TRACEHEADER_REF(text) SAC_TR_PrintTraceHeader text

#define SAC_TR_PRINT_REF(name)                                                           \
    SAC_TR_PrintTraceInfo ("refcnt of %s: %d", #name, SAC_ND_A_RC (name))

#else /* SAC_DO_TRACE_REF */

#define SAC_TR_PRINT_TRACEHEADER_REF(text)
#define SAC_TR_PRINT_REF(name)

#endif /* SAC_DO_TRACE_REF */




/* 
 *  Macros for tracing memory operations
 */

#if (SAC_DO_TRACE_MEM || SAC_DO_TRACE)

#define SAC_TR_PRINT_TRACEHEADER_MEM(text) SAC_TR_PrintTraceHeader text

#define SAC_TR_PRINT_ARRAY_MEM(name)                                                     \
    SAC_TR_PrintTraceInfo ("adr: %p, size: %d elements", SAC_ND_A_FIELD (name),          \
                           SAC_ND_A_SIZE (name));                                        \
    SAC_TR_PrintTraceInfo ("total # of array elements: %d", SAC_TR_array_memcnt)

#define SAC_TR_PRINT_HIDDEN_MEM(name)                                                    \
    SAC_TR_PrintTraceInfo ("adr: %p", name);                                             \
    SAC_TR_PrintTraceInfo ("total # of hidden objects: %d", SAC_TR_hidden_memcnt)

#define SAC_TR_INC_ARRAY_MEMCNT(size) SAC_TR_array_memcnt += size
#define SAC_TR_DEC_ARRAY_MEMCNT(size) SAC_TR_array_memcnt -= size

#define SAC_TR_INC_HIDDEN_MEMCNT(size) SAC_TR_hidden_memcnt += size
#define SAC_TR_DEC_HIDDEN_MEMCNT(size) SAC_TR_hidden_memcnt -= size

#else /* SAC_DO_TRACE_MEM */

#define SAC_TR_PRINT_TRACEHEADER_MEM(text)
#define SAC_TR_PRINT_ARRAY_MEM(name)
#define SAC_TR_PRINT_HIDDEN_MEM(name)
#define SAC_TR_INC_ARRAY_MEMCNT(size)
#define SAC_TR_DEC_ARRAY_MEMCNT(size)
#define SAC_TR_INC_HIDDEN_MEMCNT(size)
#define SAC_TR_DEC_HIDDEN_MEMCNT(size)

#endif /* SAC_DO_TRACE_MEM */



/* 
 *  Macros for tracing primitive functions calls
 */

#if SAC_DO_TRACE_PRF

#define SAC_TR_PRINT_PRF(text) SAC_Print text

#else /* SAC_DO_TRACE_PRF */

#define SAC_TR_PRINT_PRF(text)

#endif /* SAC_DO_TRACE_PRF */

#endif /* SAC_TRACE_H */

#endif
