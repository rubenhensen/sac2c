#ifndef _SAC_MUTC_STARTUP_H_

#define _SAC_MUTC_STARTUP_H_

#define MUTC 1
#if SAC_BACKEND == MUTC
#define shutup_cstdlib_h
#include <stdlib.h>
#define shutup_cstring_h
#include <string.h>
#define shutup_calloca_h
#include <alloca.h>
#include <svp/sep.h>
#include <svp/slr.h>
#include <svp/testoutput.h>
#include <svp/abort.h>
#include <svp/perf.h>

sl_place_t SAC_mutc_rc_place;

#endif /* SAC_BACKEND */
#undef MUTC

#if SAC_DO_COMPILE_MODULE

#define SAC_MUTC_STARTUP SAC_MUTC_STARTUP_ANON ()

#else

#define SAC_MUTC_STARTUP                                                                 \
    SAC_MUTC_STARTUP_ANON ()                                                             \
    SAC_MUTC_UNIN                                                                        \
    SAC_MUTC_TOSTRING                                                                    \
    SAC_MUTC_SAC_SVP_IO_PUTN

/*SAC_MUTC_WORLD_OBJECT*/
#endif

#define SAC_MUTC_SAC_SVP_IO_PUTN                                                         \
    sl_decl (svp_io_putn, void, sl_glparm (long long, a), sl_glparm (int, t));           \
    sl_def (sac_svp_io_putn, void, sl_glparm (int, a), sl_glparm (int, t))               \
    {                                                                                    \
        long long b = (long long)sl_getp (a);                                            \
        sl_proccall (svp_io_putn, sl_glarg (long long, sl_anon, b),                      \
                     sl_glarg (int, sl_anon, sl_getp (t)));                              \
    }                                                                                    \
    sl_enddef

#define SAC_MUTC_THE_WORLD_TAGS()                                                        \
    T_SHP (SCL, T_HID (NHD, T_UNQ (UNQ, T_REG (INT, T_SCO (GLO, T_USG (FPA, T_EMPTY))))))

#define SAC_MUTC_STARTUP_ANON()                                                          \
    m4_define ([[_sl_anon_counter]], 0)                                                  \
      m4_define ([[sl_anon]],                                                            \
                 [[m4_step ([[_sl_anon_counter]]) _sl_anonarg[[]] _sl_anon_counter]])

#define SAC_MUTC_MAIN_RES_NT                                                             \
    (SAC_res, T_SHP (SCL, T_HID (NHD, T_UNQ (UNQ, T_REG (INT, T_SCO (GLO, T_EMPTY))))))

#if SVP_HAS_SEP
#define SAC_MUTC_SEPALLOC(P, N)                                                          \
    do {                                                                                 \
        sl_create (, root_sep->sep_place | 1, , , , , , root_sep->sep_alloc,             \
                   sl_glarg (struct SEP *, , root_sep),                                  \
                   sl_glarg (unsigned long, , SAL_EXACT | N),                            \
                   sl_sharg (struct placeinfo *, p, 0));                                 \
        sl_sync ();                                                                      \
        if (!sl_geta (p)) {                                                              \
            output_string ("Place allocation failed!\n", 2);                             \
            svp_abort ();                                                                \
        }                                                                                \
        (P) = sl_geta (p)->pid;                                                          \
    } while (0)
#else
#define SAC_MUTC_SEPALLOC(P, N)                                                          \
    do {                                                                                 \
        (P) = PLACE_DEFAULT;                                                             \
    } while (0)
#endif

#if 0
#if SAC_MUTC_MACROS
struct s_interval *sac_benchmark_intervals;
static int sac_benchmark_count;
#endif

#define SAC_MUTC_SAC_MAIN                                                                \
    slr_decl (slr_var (unsigned, ncores));                                               \
    sl_def (sac_main, void)                                                              \
    {                                                                                    \
        sac_benchmark_intervals = mtperf_alloc_intervals (1024);                         \
        sac_benchmark_count = 0;                                                         \
        unsigned P = 1;                                                                  \
        if (slr_len (ncores))                                                            \
            P = slr_get (ncores)[0];                                                     \
        sl_place_t svp_pid;                                                              \
        SAC_MUTC_SEPALLOC (svp_pid, P);                                                  \
        SAC_ND_DECL__DATA (SAC_MUTC_MAIN_RES_NT, int, )                                  \
        SAC_ND_DECL__DESC (SAC_MUTC_MAIN_RES_NT, )                                       \
        SAC_NOTHING ()                                                                   \
        sl_create (, svp_pid, , , , , , SACwf__MAIN__main,                               \
                   SAC_ND_ARG_out (SAC_MUTC_MAIN_RES_NT, int));                          \
        sl_sync ();                                                                      \
        mtperf_free_intervals (sac_benchmark_intervals);                                 \
    }                                                                                    \
    sl_enddef
#else
#define SAC_MUTC_SAC_MAIN                                                                \
    sl_def (sac_main, void)                                                              \
    {                                                                                    \
        sl_create (, root_sep->sep_place, , , , , sl__exclusive, root_sep->sep_alloc,    \
                   sl_glarg (struct SEP *, , root_sep),                                  \
                   sl_glarg (unsigned long, , SAL_EXCLUSIVE),                            \
                   sl_sharg (struct placeinfo *, p, 0));                                 \
        sl_sync ();                                                                      \
        if (sl_geta (p) == 0) {                                                          \
            output_string (                                                              \
              "Place allocation for exclusive place for reference counting failed!\n",   \
              2);                                                                        \
            svp_abort ();                                                                \
        }                                                                                \
        SAC_mutc_rc_place = sl_geta (p)->pid;                                            \
                                                                                         \
        SAC_ND_DECL__DATA (SAC_MUTC_MAIN_RES_NT, int, )                                  \
        SAC_ND_DECL__DESC (SAC_MUTC_MAIN_RES_NT, )                                       \
        SAC_NOTHING ()                                                                   \
        sl_create (, , , , , , , SACwtf__MAIN__main,                                     \
                   SAC_ND_ARG_out (SAC_MUTC_MAIN_RES_NT, int));                          \
        sl_sync ();                                                                      \
    }                                                                                    \
    sl_enddef
#endif

#define SAC_MUTC_T_MAIN                                                                  \
    sl_def (t_main, void)                                                                \
    {                                                                                    \
        sl_proccall (sac_main);                                                          \
    }                                                                                    \
    sl_enddef

#define SAC_MUTC_MAIN SAC_MUTC_SAC_MAIN SAC_MUTC_T_MAIN

#if SAC_MUTC_MACROS
SAC_MUTC_STARTUP
#endif /* SAC_MUTC_MACROS */

#endif /* _SAC_MUTC_STARTUP_H_ */
