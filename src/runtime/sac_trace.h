/*
 *
 * $Log$
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

extern void SAC_TR_PrintTraceHeader (char *format, ...);
extern void SAC_TR_PrintTraceInfo (char *format, ...);

#endif /* SAC_DO_TRACE */

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
