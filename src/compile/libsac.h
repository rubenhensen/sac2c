/*
 *
 * $Log$
 * Revision 1.13  1997/10/10 13:25:27  dkr
 * added a prototype for prf abs(): int abs(int)
 *
 * Revision 1.12  1997/08/29 12:36:49  sbs
 * PRINT_PRF inserted
 *
 * Revision 1.11  1997/05/28 12:35:25  sbs
 * Profiling integrated
 *
 * Revision 1.10  1997/05/16  09:52:19  sbs
 * ANALSE-TOOL extended to function-application specific timing
 *
 * Revision 1.9  1997/05/14  08:11:24  sbs
 * ANALYSE macros added
 *
 * Revision 1.8  1997/04/24  10:06:45  cg
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

extern int abs (int x);

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

#ifdef TRACE_PRF

#define PRINT_PRF(text) __SAC__Runtime_Print text

#else

#define PRINT_PRF(text)

#endif /* TRACE_PRF */

#ifdef CHECK_MALLOC

#define MALLOC(size) __SAC__Runtime_malloc (size)

#else /* CHECK_MALLOC  */

#define MALLOC(size) malloc (size)

#endif /* CHECK_MALLOC  */

/*
 * PROFILING-MACROS:
 */

/***********************************************************************/

#ifdef PROFILE_WITH

#define PROFILE_BEGIN_WITH(str)                                                          \
    __PF_TIMER *__PF_mem_act;                                                            \
    __PF_STOP_CLOCK ();                                                                  \
    __PF_mem_act = __PF_act_timer;                                                       \
    __PF_ADD_TO_TIMER (*__PF_act_timer);                                                 \
    __PF_act_timer                                                                       \
      = (__PF_with_level == 0                                                            \
           ? &__PF_with_##str##_timer[__PF_act_funno][__PF_act_funapno]                  \
           : &__PF_fw_with_##str##_timer[__PF_act_funno][__PF_act_funapno]);             \
    __PF_with_level++;                                                                   \
    __PF_START_CLOCK ()

#define PROFILE_END_WITH(str)                                                            \
    __PF_STOP_CLOCK ();                                                                  \
    __PF_with_level--;                                                                   \
    __PF_ADD_TO_TIMER (*__PF_act_timer);                                                 \
    __PF_act_timer = __PF_mem_act;                                                       \
    __PF_START_CLOCK ()

#define DISPLAY_WITH 1

#else /* PROFILE_WITH */

#define PROFILE_BEGIN_WITH(str)
#define PROFILE_END_WITH(str)
#define DISPLAY_WITH 0

#endif /* PROFILE_WITH */

/***********************************************************************/

#ifdef PROFILE_FUN

#define PROFILE_BEGIN_UDF(funno, funapno)                                                \
    {                                                                                    \
        __PF_TIMER *__PF_mem_act;                                                        \
        int __PF_mem_funno;                                                              \
        int __PF_mem_funapno;                                                            \
        __PF_STOP_CLOCK ();                                                              \
        __PF_mem_act = __PF_act_timer;                                                   \
        __PF_mem_funno = __PF_act_funno;                                                 \
        __PF_mem_funapno = __PF_act_funapno;                                             \
        __PF_ADD_TO_TIMER (*__PF_act_timer);                                             \
        __PF_act_funno = funno;                                                          \
        __PF_act_funapno = funapno;                                                      \
        __PF_act_timer = (__PF_with_level == 0 ? &__PF_fun_timer[funno][funapno]         \
                                               : &__PF_fw_fun_timer[funno][funapno]);    \
        __PF_START_CLOCK ()

#define PROFILE_END_UDF(funno, funapno)                                                  \
    __PF_STOP_CLOCK ();                                                                  \
    __PF_ADD_TO_TIMER (*__PF_act_timer);                                                 \
    __PF_act_timer = __PF_mem_act;                                                       \
    __PF_act_funno = __PF_mem_funno;                                                     \
    __PF_act_funapno = __PF_mem_funapno;                                                 \
    __PF_START_CLOCK ();                                                                 \
    }

#define DISPLAY_FUN 1

#else /* PROFILE_FUN */

#define PROFILE_BEGIN_UDF(funno, funapno)
#define PROFILE_END_UDF(funno, funapno)
#define DISPLAY_FUN 0

#endif /* PROFILE_FUN */

/***********************************************************************/

#ifdef PROFILE_INL

#define PROFILE_INLINE(x) x

#else /* PROFILE_INL */

#define PROFILE_INLINE(x)

#endif /* PROFILE_INL */

/***********************************************************************/

#ifdef PROFILE_LIB

#define PROFILE_LIBRARY(x) x

#else /* PROFILE_LIB */

#define PROFILE_LIBRARY(x)

#endif /* PROFILE_LIB */

/***********************************************************************/

#ifdef PROFILE

#define __PF_INIT_CLOCK()
#define __PF_INIT_TIMER(timer) timer##.tv_sec = timer##.tv_usec = 0
#define __PF_START_CLOCK() getrusage (RUSAGE_SELF, &start_timer)
#define __PF_STOP_CLOCK() getrusage (RUSAGE_SELF, &stop_timer)
#define __PF_ADD_TO_TIMER(timer)                                                         \
    if (((timer).tv_usec += stop_timer.ru_utime.tv_usec - start_timer.ru_utime.tv_usec)  \
        < 0) {                                                                           \
        (timer).tv_usec += 1000000;                                                      \
        (timer).tv_sec += stop_timer.ru_utime.tv_sec - start_timer.ru_utime.tv_sec - 1;  \
    } else {                                                                             \
        (timer).tv_sec += stop_timer.ru_utime.tv_sec - start_timer.ru_utime.tv_sec;      \
    }

typedef struct timeval __PF_TIMER;

struct rusage start_timer;
struct rusage stop_timer;

extern __PF_TIMER *__SAC__Runtime_PrintProfileOverall (
  int display_with, int __PF_maxfun, int *__PF_maxfunap, void *__PF_fun_timer,
  void *__PF_with_modarray_timer, void *__PF_with_genarray_timer,
  void *__PF_with_fold_timer, void *__PF_fw_fun_timer, void *__PF_fw_with_modarray_timer,
  void *__PF_fw_with_genarray_timer, void *__PF_fw_with_fold_timer);

extern void __SAC__Runtime_PrintProfileFuns (
  __PF_TIMER *total_time, int display_fun, int display_with, int __PF_maxfun,
  int *__PF_maxfunap, void *__PF_fun_timer, void *__PF_with_modarray_timer,
  void *__PF_with_genarray_timer, void *__PF_with_fold_timer, void *__PF_fw_fun_timer,
  void *__PF_fw_with_modarray_timer, void *__PF_fw_with_genarray_timer,
  void *__PF_fw_with_fold_timer, void *__PF_fun_name);

#define PROFILE_SETUP(maxfun)                                                            \
    __PF_act_timer = &__PF_fun_timer[0][0];                                              \
    __PF_act_funno = 0;                                                                  \
    __PF_act_funapno = 0;                                                                \
    __PF_maxfun = maxfun;                                                                \
    __PF_INIT_CLOCK ();                                                                  \
    {                                                                                    \
        int i, j;                                                                        \
        for (i = 0; i < maxfun; i++) {                                                   \
            for (j = 0; j < __PF_maxfunap[i]; j++) {                                     \
                __PF_INIT_TIMER (__PF_fun_timer[i][j]);                                  \
                __PF_INIT_TIMER (__PF_with_modarray_timer[i][j]);                        \
                __PF_INIT_TIMER (__PF_with_genarray_timer[i][j]);                        \
                __PF_INIT_TIMER (__PF_with_fold_timer[i][j]);                            \
                __PF_INIT_TIMER (__PF_fw_fun_timer[i][j]);                               \
                __PF_INIT_TIMER (__PF_fw_with_modarray_timer[i][j]);                     \
                __PF_INIT_TIMER (__PF_fw_with_genarray_timer[i][j]);                     \
                __PF_INIT_TIMER (__PF_fw_with_fold_timer[i][j]);                         \
            }                                                                            \
        }                                                                                \
    }                                                                                    \
    __PF_START_CLOCK ()

#define PROFILE_PRINT()                                                                  \
    __PF_STOP_CLOCK ();                                                                  \
    __PF_ADD_TO_TIMER (*__PF_act_timer);                                                 \
    {                                                                                    \
        __PF_TIMER *tmp;                                                                 \
                                                                                         \
        tmp                                                                              \
          = __SAC__Runtime_PrintProfileOverall (DISPLAY_WITH, __PF_maxfun,               \
                                                __PF_maxfunap, __PF_fun_timer,           \
                                                __PF_with_modarray_timer,                \
                                                __PF_with_genarray_timer,                \
                                                __PF_with_fold_timer, __PF_fw_fun_timer, \
                                                __PF_fw_with_modarray_timer,             \
                                                __PF_fw_with_genarray_timer,             \
                                                __PF_fw_with_fold_timer);                \
        __SAC__Runtime_PrintProfileFuns (tmp, DISPLAY_FUN, DISPLAY_WITH, __PF_maxfun,    \
                                         __PF_maxfunap, __PF_fun_timer,                  \
                                         __PF_with_modarray_timer,                       \
                                         __PF_with_genarray_timer, __PF_with_fold_timer, \
                                         __PF_fw_fun_timer, __PF_fw_with_modarray_timer, \
                                         __PF_fw_with_genarray_timer,                    \
                                         __PF_fw_with_fold_timer, __PF_fun_name);        \
    }

#else /* PROFILE */

#define PROFILE_SETUP(maxfun)
#define PROFILE_PRINT()

#endif /* PROFILE */

#endif /* _sac_libsac_h */
