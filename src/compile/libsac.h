/*
 *
 * $Log$
 * Revision 1.8  1997/04/24 10:06:45  cg
 * non-icm macros moved from icm2c.h to libsac.h
 *
 * Revision 1.7  1996/04/02  13:52:47  cg
 * typedef of string removed
 *
 * Revision 1.6  1996/02/21  15:10:33  cg
 * typedefs and defines taken from icm2c.h
 * new typedef char* string
 *
 * Revision 1.5  1996/02/05  09:21:48  sbs
 * RuntimError => Runtime_Error
 *
 * Revision 1.4  1996/01/25  15:04:57  cg
 * added __SAC__Runtime_hidden_memcnt and __SAC__Runtime_array_memcnt
 *
 * Revision 1.3  1996/01/21  18:07:34  cg
 * added declaration of __SAC__Runtime_trace_memcnt
 *
 * Revision 1.2  1996/01/09  08:54:00  cg
 * __SAC__Runtime_malloc(int size) now returns void*
 *
 * Revision 1.1  1996/01/09  08:31:41  cg
 * Initial revision
 *
 *
 *
 *
 */

#ifndef _sac_libsac_h

#define _sac_libsac_h

#define true 1
#define false 0

typedef int bool;

extern int __SAC__Runtime_hidden_memcnt;
extern int __SAC__Runtime_array_memcnt;

extern void __SAC__Runtime_Error (char *format, ...);
extern void __SAC__Runtime_Print (char *format, ...);
extern void __SAC__Runtime_PrintTraceHeader (char *format, ...);
extern void __SAC__Runtime_PrintTraceInfo (char *format, ...);
extern void *__SAC__Runtime_malloc (int size);

/*
 * Internal Macros :
 * =================
 *
 */

#if (defined(TRACE_MEM) || defined(TRACE_REF))

#define PRINT_TRACEHEADER_ALL(text) __SAC__Runtime_PrintTraceHeader text

#define PRINT_ARRAY_FREE(name)                                                           \
    __SAC__Runtime_PrintTraceInfo ("freeing array %s (adr: %p)", #name, ND_A_FIELD (name))

#define PRINT_HIDDEN_FREE(name)                                                          \
    __SAC__Runtime_PrintTraceInfo ("freeing hidden %s (adr: %p)", #name, name)

#else

#define PRINT_TRACEHEADER_ALL(text)
#define PRINT_ARRAY_FREE(name)
#define PRINT_HIDDEN_FREE(name)

#endif /* TRACE_MEM || TRACE_REF */

#ifdef TRACE_REF

#define PRINT_TRACEHEADER_REF(text) __SAC__Runtime_PrintTraceHeader text

#define PRINT_REF(name)                                                                  \
    __SAC__Runtime_PrintTraceInfo ("refcnt of %s: %d", #name, ND_A_RC (name))

#else

#define PRINT_TRACEHEADER_REF(text)
#define PRINT_REF(name)

#endif /* TRACE_REF */

#ifdef TRACE_MEM

#define PRINT_TRACEHEADER_MEM(text) __SAC__Runtime_PrintTraceHeader text

#define PRINT_ARRAY_MEM(name)                                                            \
    __SAC__Runtime_PrintTraceInfo ("adr: %p, size: %d elements", ND_A_FIELD (name),      \
                                   ND_A_SIZE (name));                                    \
    __SAC__Runtime_PrintTraceInfo ("total # of array elements: %d",                      \
                                   __SAC__Runtime_array_memcnt)

#define PRINT_HIDDEN_MEM(name)                                                           \
    __SAC__Runtime_PrintTraceInfo ("adr: %p", name);                                     \
    __SAC__Runtime_PrintTraceInfo ("total # of hidden objects: %d",                      \
                                   __SAC__Runtime_hidden_memcnt)

#define INC_ARRAY_MEMCNT(size) __SAC__Runtime_array_memcnt += size
#define DEC_ARRAY_MEMCNT(size) __SAC__Runtime_array_memcnt -= size

#define INC_HIDDEN_MEMCNT(size) __SAC__Runtime_hidden_memcnt += size
#define DEC_HIDDEN_MEMCNT(size) __SAC__Runtime_hidden_memcnt -= size

#else

#define PRINT_TRACEHEADER_MEM(text)
#define PRINT_ARRAY_MEM(name)
#define PRINT_HIDDEN_MEM(name)
#define INC_ARRAY_MEMCNT(size)
#define DEC_ARRAY_MEMCNT(size)
#define INC_HIDDEN_MEMCNT(size)
#define DEC_HIDDEN_MEMCNT(size)

#endif /* TRACE_MEM */

#ifdef CHECK_MALLOC

#define MALLOC(size) __SAC__Runtime_malloc (size)

#else /* CHECK_MALLOC  */

#define MALLOC(size) malloc (size)

#endif /* CHECK_MALLOC  */

#endif /* _sac_libsac_h */
