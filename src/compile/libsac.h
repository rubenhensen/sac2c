/*
 *
 * $Log$
 * Revision 1.10  1997/05/16 09:52:19  sbs
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

#ifdef ANALYSE_TIME

#define ANALYSE_BEGIN_WITH(str)                                                          \
    double *__AT_mem_act;                                                                \
    __AT_clock_stop = __AT_CLOCK ();                                                     \
    __AT_mem_act = __AT_act_timer;                                                       \
    *__AT_act_timer += (__AT_clock_stop - __AT_clock_start);                             \
    __AT_act_timer                                                                       \
      = (__AT_with_level == 0                                                            \
           ? &__AT_with_##str##_timer[__AT_act_funno][__AT_act_funapno]                  \
           : &__AT_fw_with_##str##_timer[__AT_act_funno][__AT_act_funapno]);             \
    __AT_with_level++;                                                                   \
    __AT_clock_start = __AT_CLOCK ()

#define ANALYSE_END_WITH(str)                                                            \
    __AT_clock_stop = __AT_CLOCK ();                                                     \
    __AT_with_level--;                                                                   \
    *__AT_act_timer += (__AT_clock_stop - __AT_clock_start);                             \
    __AT_act_timer = __AT_mem_act;                                                       \
    __AT_clock_start = __AT_CLOCK ()

#define ANALYSE_BEGIN_UDF(funno, funapno)                                                \
    {                                                                                    \
        double *__AT_mem_act;                                                            \
        int __AT_mem_funno;                                                              \
        int __AT_mem_funapno;                                                            \
        __AT_clock_stop = __AT_CLOCK ();                                                 \
        __AT_mem_act = __AT_act_timer;                                                   \
        __AT_mem_funno = __AT_act_funno;                                                 \
        __AT_mem_funapno = __AT_act_funapno;                                             \
        *__AT_act_timer += (__AT_clock_stop - __AT_clock_start);                         \
        __AT_act_funno = funno;                                                          \
        __AT_act_funapno = funapno;                                                      \
        __AT_act_timer = (__AT_with_level == 0 ? &__AT_fun_timer[funno][funapno]         \
                                               : &__AT_fw_fun_timer[funno][funapno]);    \
        __AT_clock_start = __AT_CLOCK ()

#define ANALYSE_END_UDF(funno, funapno)                                                  \
    __AT_clock_stop = __AT_CLOCK ();                                                     \
    *__AT_act_timer += (__AT_clock_stop - __AT_clock_start);                             \
    __AT_act_timer = __AT_mem_act;                                                       \
    __AT_act_funno = __AT_mem_funno;                                                     \
    __AT_act_funapno = __AT_mem_funapno;                                                 \
    __AT_clock_start = __AT_CLOCK ();                                                    \
    }

#else /* ANALYSE_TIME */

#define ANALYSE_BEGIN_WITH(str)
#define ANALYSE_END_WITH(str)
#define ANALYSE_BEGIN_UDF(funno, funapno)
#define ANALYSE_END_UDF(funno, funapno)

#endif /* ANALYSE_TIME */

#ifdef ANALYSE_TIME

#define __AT_CLOCK_FACTOR 1000000
#define __AT_CLOCK()                                                                     \
    (getrusage (RUSAGE_SELF, &__AT_rusage),                                              \
     __AT_rusage.ru_utime.tv_sec * __AT_CLOCK_FACTOR + __AT_rusage.ru_utime.tv_usec)

#define ANALYSE_SETUP(maxfun)                                                            \
    __AT_act_timer = &__AT_fun_timer[0][0];                                              \
    __AT_act_funno = 0;                                                                  \
    __AT_act_funapno = 0;                                                                \
    __AT_maxfun = maxfun;                                                                \
    __AT_clock_start = __AT_CLOCK ()

#define AT_PRINT_HEADER(str, fun, funap)                                                 \
    __SAC__Runtime_Print ("****************************************"                     \
                          "****************************************\n");                 \
    __SAC__Runtime_Print ("*** %-16s %-55s ***\n", str ":", fun);                        \
    __SAC__Runtime_Print ("*** called from line # %-5d %-48s***\n", funap, "");          \
    __SAC__Runtime_Print ("****************************************"                     \
                          "****************************************\n")

#define AT_PRINT_TIME(text, var) __SAC__Runtime_Print ("%-20s: %10.3f sec\n", text, var)

#define AT_PRINT_PERCENTAGE(text, var)                                                   \
    __SAC__Runtime_Print ("%-20s:    %7.3f %%\n", text, var)

#define AT_PRINT_SEPERATOR                                                               \
    __SAC__Runtime_Print ("----------------------------------------\n")

#define ANALYSE_PRINT()                                                                  \
    {                                                                                    \
        int i, j;                                                                        \
        double with_total, fun_total;                                                    \
        for (i = 0; i < __AT_maxfun; i++) {                                              \
            for (j = 0; j < __AT_maxfunap[i]; j++) {                                     \
                __AT_fun_timer[i][j] /= __AT_CLOCK_FACTOR;                               \
                __AT_with_genarray_timer[i][j] /= __AT_CLOCK_FACTOR;                     \
                __AT_with_modarray_timer[i][j] /= __AT_CLOCK_FACTOR;                     \
                __AT_with_fold_timer[i][j] /= __AT_CLOCK_FACTOR;                         \
                with_total = __AT_with_genarray_timer[i][j]                              \
                             + __AT_with_modarray_timer[i][j]                            \
                             + __AT_with_fold_timer[i][j];                               \
                fun_total = with_total + __AT_fun_timer[i][j];                           \
                AT_PRINT_HEADER ("time analysis", __AT_fun_name[i],                      \
                                 __AT_funapline[i][j]);                                  \
                AT_PRINT_TIME ("with-loop-genarray", __AT_with_genarray_timer[i][j]);    \
                AT_PRINT_TIME ("with-loop-modarray", __AT_with_modarray_timer[i][j]);    \
                AT_PRINT_TIME ("with-loop-fold", __AT_with_fold_timer[i][j]);            \
                AT_PRINT_SEPERATOR;                                                      \
                AT_PRINT_TIME ("with-loop-total", with_total);                           \
                AT_PRINT_TIME ("non-with-loop", __AT_fun_timer[i][j]);                   \
                AT_PRINT_SEPERATOR;                                                      \
                AT_PRINT_TIME ("runtime-total", fun_total);                              \
                AT_PRINT_PERCENTAGE ("percentage non-with",                              \
                                     (__AT_fun_timer[i][j] / fun_total) * 100);          \
                AT_PRINT_SEPERATOR;                                                      \
                __SAC__Runtime_Print ("within with-loop:\n");                            \
                __AT_fw_fun_timer[i][j] /= __AT_CLOCK_FACTOR;                            \
                __AT_fw_with_genarray_timer[i][j] /= __AT_CLOCK_FACTOR;                  \
                __AT_fw_with_modarray_timer[i][j] /= __AT_CLOCK_FACTOR;                  \
                __AT_fw_with_fold_timer[i][j] /= __AT_CLOCK_FACTOR;                      \
                with_total = __AT_fw_with_genarray_timer[i][j]                           \
                             + __AT_fw_with_modarray_timer[i][j]                         \
                             + __AT_fw_with_fold_timer[i][j];                            \
                fun_total = with_total + __AT_fw_fun_timer[i][j];                        \
                AT_PRINT_SEPERATOR;                                                      \
                AT_PRINT_TIME ("with-loop-genarray", __AT_fw_with_genarray_timer[i][j]); \
                AT_PRINT_TIME ("with-loop-modarray", __AT_fw_with_modarray_timer[i][j]); \
                AT_PRINT_TIME ("with-loop-fold", __AT_fw_with_fold_timer[i][j]);         \
                AT_PRINT_SEPERATOR;                                                      \
                AT_PRINT_TIME ("with-loop-total", with_total);                           \
                AT_PRINT_TIME ("non-with-loop", __AT_fw_fun_timer[i][j]);                \
                AT_PRINT_SEPERATOR;                                                      \
                AT_PRINT_TIME ("runtime-total", fun_total);                              \
                AT_PRINT_PERCENTAGE ("percentage non-with",                              \
                                     (__AT_fw_fun_timer[i][j] / fun_total) * 100);       \
            }                                                                            \
        }                                                                                \
    }

#else /* ANALYSE_TIME */

#define ANALYSE_SETUP(maxfun)
#define ANALYSE_PRINT()

#endif /* ANALYSE_TIME */

#endif /* _sac_libsac_h */
