/*
 *
 * $Log$
 * Revision 3.6  2003/03/09 19:16:30  dkr
 * TRACE_AA added
 *
 * Revision 3.5  2002/11/08 13:29:45  cg
 * Removed TRACE_OWL macro since old with-loops left sac2c several
 * years ago.  :-))))
 *
 * Revision 3.4  2002/07/03 17:29:48  dkr
 * some ; shifted in macros
 *
 * Revision 3.3  2002/04/30 08:18:24  dkr
 * some comments added
 *
 * Revision 3.2  2000/11/21 13:59:26  cg
 * Bug fixed in tracing fold with-loops in multithreaded execution.
 *
 * Revision 3.1  2000/11/20 18:02:22  sacbase
 * new release made
 *
 * Revision 2.2  2000/01/17 16:25:58  cg
 * Completely reorganized the tracing facility:
 * Macros for code sections are mostly replaced by function calls.
 * The current implementation of the trace facility is now
 * thread-safe !!
 *
 * Revision 2.1  1999/02/23 12:44:01  sacbase
 * new release made
 *
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
 * Revision 1.1  1998/03/19 16:55:06  cg
 * Initial revision
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

#ifndef _SAC_TRACE_H
#define _SAC_TRACE_H

/*
 *  External declarations of global variables and functions defined in trace.c
 *  as part of libsac.
 */

#if SAC_DO_TRACE

extern int SAC_TR_hidden_memcnt;
extern int SAC_TR_array_memcnt;

extern void SAC_TR_Print (char *format, ...);
extern void SAC_TR_IncArrayMemcnt (int size);
extern void SAC_TR_DecArrayMemcnt (int size);
extern void SAC_TR_IncHiddenMemcnt (int size);
extern void SAC_TR_DecHiddenMemcnt (int size);

#define SAC_TR_PRINT(msg) SAC_TR_Print msg;

#else /* SAC_DO_TRACE */

#define SAC_TR_PRINT(msg)

#endif /* SAC_DO_TRACE */

#if SAC_DO_TRACE_FUN

#define SAC_TR_FUN_PRINT(msg) SAC_TR_PRINT (msg)

#else /* SAC_DO_TRACE_FUN */

#define SAC_TR_FUN_PRINT(msg)

#endif /* SAC_DO_TRACE_FUN */

#if SAC_DO_TRACE_PRF

#define SAC_TR_PRF_PRINT(msg) SAC_TR_PRINT (msg)

#else /* SAC_DO_TRACE_PRF */

#define SAC_TR_PRF_PRINT(msg)

#endif /* SAC_DO_TRACE_PRF */

#if SAC_DO_TRACE_REF

#define SAC_TR_REF_PRINT(msg) SAC_TR_PRINT (msg)

#define SAC_TR_REF_PRINT_RC(name)                                                        \
    SAC_TR_REF_PRINT (("refcnt of %s: %d", #name, SAC_ND_A_RC (name)))

#else /* SAC_DO_TRACE_REF */

#define SAC_TR_REF_PRINT(msg)
#define SAC_TR_REF_PRINT_RC(name)

#endif /* SAC_DO_TRACE_REF */

#if SAC_DO_TRACE_WL

#define SAC_TR_WL_PRINT(msg) SAC_TR_PRINT (msg)

#else /* SAC_DO_TRACE_WL */

#define SAC_TR_WL_PRINT(msg)

#endif /* SAC_DO_TRACE_WL */

#if SAC_DO_TRACE_AA

#define SAC_TR_AA_PRINT(str, array, pos)                                                 \
    SAC_TR_PRINT (("%s access %s[%d]", str, #array, pos))

#else /* SAC_DO_TRACE_AA */

#define SAC_TR_AA_PRINT(class, array, idx)

#endif /* SAC_DO_TRACE_AA */

#if SAC_DO_TRACE_MT

typedef enum {
    SAC_TR_foldres_int,
    SAC_TR_foldres_float,
    SAC_TR_foldres_double,
    SAC_TR_foldres_array,
    SAC_TR_foldres_array_rc,
    SAC_TR_foldres_hidden
} SAC_TR_foldres_t;

#define SAC_TR_MT_PRINT(msg) SAC_TR_PRINT (msg)

#define SAC_TR_MT_PRINT_FOLD_RESULT(type, accu_var, msg)                                 \
    {                                                                                    \
        const SAC_TR_foldres_t SAC_TR_foldres = SAC_TR_foldres_##type;                   \
        switch (SAC_TR_foldres) {                                                        \
        case SAC_TR_foldres_int:                                                         \
            SAC_TR_MT_PRINT ((msg " (int) %d", accu_var));                               \
            break;                                                                       \
        case SAC_TR_foldres_float:                                                       \
            SAC_TR_MT_PRINT ((msg " (float) %.15g", accu_var));                          \
            break;                                                                       \
        case SAC_TR_foldres_double:                                                      \
            SAC_TR_MT_PRINT ((msg " (double) %.15g", accu_var));                         \
            break;                                                                       \
        case SAC_TR_foldres_array:                                                       \
            SAC_TR_MT_PRINT ((msg " (array) %p", accu_var));                             \
            break;                                                                       \
        case SAC_TR_foldres_array_rc:                                                    \
            SAC_TR_MT_PRINT ((msg " (array) %p", accu_var));                             \
            break;                                                                       \
        case SAC_TR_foldres_hidden:                                                      \
            SAC_TR_MT_PRINT ((msg " (hidden)"));                                         \
            break;                                                                       \
        }                                                                                \
    }

#else /* SAC_DO_TRACE_MT */

#define SAC_TR_MT_PRINT(msg)
#define SAC_TR_MT_PRINT_FOLD_RESULT(type, accu_var, msg)

#endif /* SAC_DO_TRACE_MT */

#if SAC_DO_TRACE_MEM

#define SAC_TR_MEM_PRINT(msg) SAC_TR_PRINT (msg)

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

#endif /* _SAC_TRACE_H */
