/*
 *
 * $Log$
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

#if TRACE

extern int _SAC_hidden_memcnt;
extern int _SAC_array_memcnt;

extern void _SAC_PrintTraceHeader (char *format, ...);
extern void _SAC_PrintTraceInfo (char *format, ...);

#endif /* TRACE */

/*
 *  Macros for tracing memory and/or refcount operations
 */

#if (TRACE_MEM || TRACE_REF || TRACE)

#define PRINT_TRACEHEADER_ALL(text) _SAC_PrintTraceHeader text

#define PRINT_ARRAY_FREE(name)                                                           \
    _SAC_PrintTraceInfo ("freeing array %s (adr: %p)", #name, ND_A_FIELD (name))

#define PRINT_HIDDEN_FREE(name)                                                          \
    _SAC_PrintTraceInfo ("freeing hidden %s (adr: %p)", #name, name)

#else /* TRACE_MEM || TRACE_REF */

#define PRINT_TRACEHEADER_ALL(text)
#define PRINT_ARRAY_FREE(name)
#define PRINT_HIDDEN_FREE(name)

#endif /* TRACE_MEM || TRACE_REF */

/*
 *  Macros for tracing refcount operations
 */

#if (TRACE_REF || TRACE)

#define PRINT_TRACEHEADER_REF(text) _SAC_PrintTraceHeader text

#define PRINT_REF(name) _SAC_PrintTraceInfo ("refcnt of %s: %d", #name, ND_A_RC (name))

#else /* TRACE_REF */

#define PRINT_TRACEHEADER_REF(text)
#define PRINT_REF(name)

#endif /* TRACE_REF */

/*
 *  Macros for tracing memory operations
 */

#if (TRACE_MEM || TRACE)

#define PRINT_TRACEHEADER_MEM(text) _SAC_PrintTraceHeader text

#define PRINT_ARRAY_MEM(name)                                                            \
    _SAC_PrintTraceInfo ("adr: %p, size: %d elements", ND_A_FIELD (name),                \
                         ND_A_SIZE (name));                                              \
    _SAC_PrintTraceInfo ("total # of array elements: %d", _SAC_array_memcnt)

#define PRINT_HIDDEN_MEM(name)                                                           \
    _SAC_PrintTraceInfo ("adr: %p", name);                                               \
    _SAC_PrintTraceInfo ("total # of hidden objects: %d", _SAC_hidden_memcnt)

#define INC_ARRAY_MEMCNT(size) _SAC_array_memcnt += size
#define DEC_ARRAY_MEMCNT(size) _SAC_array_memcnt -= size

#define INC_HIDDEN_MEMCNT(size) _SAC_hidden_memcnt += size
#define DEC_HIDDEN_MEMCNT(size) _SAC_hidden_memcnt -= size

#else /* TRACE_MEM */

#define PRINT_TRACEHEADER_MEM(text)
#define PRINT_ARRAY_MEM(name)
#define PRINT_HIDDEN_MEM(name)
#define INC_ARRAY_MEMCNT(size)
#define DEC_ARRAY_MEMCNT(size)
#define INC_HIDDEN_MEMCNT(size)
#define DEC_HIDDEN_MEMCNT(size)

#endif /* TRACE_MEM */

/*
 *  Macros for tracing primitive functions calls
 */

#if TRACE_PRF

#define PRINT_PRF(text) _SAC_Print text

#else /* TRACE_PRF */

#define PRINT_PRF(text)

#endif /* TRACE_PRF */

#endif /* SAC_TRACE_H */
