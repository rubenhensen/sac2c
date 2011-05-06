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
#include <svp/testoutput.h>

#if SAC_MUTC_RC_INDIRECT == 1
#define SAC_IF_MUTC_RC_INDIRECT(A) A
#define SAC_IF_NOT_MUTC_RC_INDIRECT(A)
#else
#define SAC_IF_MUTC_RC_INDIRECT(A)
#define SAC_IF_NOT_MUTC_RC_INDIRECT(A) A
#endif

#ifndef SAC_MUTC_RC_PLACES
#define SAC_DO_COMPILE_MODULE 1
#endif
sl_place_t SAC_mutc_rc_place;
#if SAC_DO_COMPILE_MODULE == 0
sl_place_t SAC_mutc_rc_place_many[SAC_MUTC_RC_PLACES];
#endif
SAC_IF_MUTC_RC_INDIRECT (sl_place_t SAC_mutc_rc_place_w;)

void *tls_malloc (size_t arg1);

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
#if 0
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
        int res = sep_alloc (root_sep, &(P), SAL_EXACT, 1);                              \
        if (res == -1) {                                                                 \
            output_string ("Can not allocate place @ " #P "\n");                         \
            svp_abort ();                                                                \
        }                                                                                \
    } while (0);
#endif
#if 0
#define SAC_MUTC_RC_ALLOC(P)                                                             \
    do {                                                                                 \
        sl_create (, root_sep->sep_place, , , , , sl__exclusive, root_sep->sep_alloc,    \
                   sl_glarg (struct SEP *, , root_sep),                                  \
                   sl_glarg (unsigned long, , SAL_EXCLUSIVE),                            \
                   sl_sharg (struct placeinfo *, p, 0));                                 \
        sl_sync ();                                                                      \
        if (sl_geta (p) == 0) {                                                          \
            output_string ("Place allocation for exclusive place "                       \
                           "for reference counting failed!\n",                           \
                           2);                                                           \
            svp_abort ();                                                                \
        }                                                                                \
        P = sl_geta (p)->pid;                                                            \
    } while (0)
#else
#define SAC_MUTC_RC_ALLOC(P)                                                             \
    do {                                                                                 \
        int res = sep_alloc (root_sep, &(P), SAL_EXACT, 1);                              \
        if (res == -1) {                                                                 \
            output_string ("Place allocation of exclusive place "                        \
                           "for reference counting failed @ " #P "\n",                   \
                           2);                                                           \
            svp_abort ();                                                                \
        }                                                                                \
    } while (0)
#endif

#define SAC_MUTC_RC_ALLOC_MANY(P, N)                                                     \
    do {                                                                                 \
        int i;                                                                           \
        for (i = 0; i < N; i++) {                                                        \
            SAC_MUTC_RC_ALLOC (P[i]);                                                    \
        }                                                                                \
    } while (0)

#define SAC_MUTC_RC_ALLOC_W(P) SAC_MUTC_SEPALLOC (P, 1)
#else
#define SAC_MUTC_SEPALLOC(P, N)                                                          \
    do {                                                                                 \
        (P) = PLACE_DEFAULT;                                                             \
    } while (0)
#define SAC_MUTC_RC_ALLOC(P)                                                             \
    do {                                                                                 \
        (P) = PLACE_DEFAULT;                                                             \
    } while (0)
#define SAC_MUTC_RC_ALLOC_W(P)                                                           \
    do {                                                                                 \
        (P) = PLACE_DEFAULT;                                                             \
    } while (0)
#define SAC_MUTC_RC_ALLOC_MANY(P, N)                                                     \
    do {                                                                                 \
        int i;                                                                           \
        for (i = 0; i < N; i++) {                                                        \
            (P[i]) = PLACE_DEFAULT;                                                      \
        }                                                                                \
    } while (0)
#endif

#if defined(__slc_arch_mtalpha__)
#define SAC_MUTC_OUTPUT(a) __asm__ __volatile__("print %0, 0x10" : : "r"(0xa##ULL));
#else
#define SAC_MUTC_OUTPUT(a)
#endif

#define SAC_MUTC_DISPLAY_ENV                                                             \
    SAC_MUTC_OUTPUT (0x5052433020)                                                       \
    output_hex (SAC_mutc_rc_place_many[0], 0);                                           \
    if (SAC_MUTC_RC_PLACES > 1) {                                                        \
        SAC_MUTC_OUTPUT (0x5052433120)                                                   \
        output_hex (SAC_mutc_rc_place_many[1], 0);                                       \
    }                                                                                    \
    if (SAC_MUTC_RC_PLACES > 2) {                                                        \
        SAC_MUTC_OUTPUT (0x5052433220)                                                   \
        output_hex (SAC_mutc_rc_place_many[2], 0);                                       \
    }                                                                                    \
    if (SAC_MUTC_RC_PLACES > 3) {                                                        \
        SAC_MUTC_OUTPUT (0x5052433320)                                                   \
        output_hex (SAC_mutc_rc_place_many[3], 0);                                       \
    }                                                                                    \
    if (SAC_MUTC_RC_PLACES > 4) {                                                        \
        SAC_MUTC_OUTPUT (0x5052433420)                                                   \
        output_hex (SAC_mutc_rc_place_many[4], 0);                                       \
    }                                                                                    \
    if (SAC_MUTC_RC_PLACES > 5) {                                                        \
        SAC_MUTC_OUTPUT (0x5052433520)                                                   \
        output_hex (SAC_mutc_rc_place_many[5], 0);                                       \
    }                                                                                    \
    if (SAC_MUTC_RC_PLACES > 6) {                                                        \
        SAC_MUTC_OUTPUT (0x5052433620)                                                   \
        output_hex (SAC_mutc_rc_place_many[6], 0);                                       \
    }                                                                                    \
    if (SAC_MUTC_RC_PLACES > 7) {                                                        \
        SAC_MUTC_OUTPUT (0x5052433720)                                                   \
        output_hex (SAC_mutc_rc_place_many[7], 0);                                       \
    }                                                                                    \
    if (SAC_MUTC_RC_PLACES > 8) {                                                        \
        SAC_MUTC_OUTPUT (0x5052433820)                                                   \
        output_hex (SAC_mutc_rc_place_many[8], 0);                                       \
    }                                                                                    \
    if (SAC_MUTC_RC_PLACES > 9) {                                                        \
        SAC_MUTC_OUTPUT (0x5052433920)                                                   \
        output_hex (SAC_mutc_rc_place_many[9], 0);                                       \
    }                                                                                    \
    SAC_IF_MUTC_RC_INDIRECT (SAC_MUTC_OUTPUT (0x5052435720))                             \
    SAC_IF_MUTC_RC_INDIRECT (output_hex (SAC_mutc_rc_place_w, 0);)

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
        SAC_MUTC_RC_ALLOC (SAC_mutc_rc_place);                                           \
        SAC_MUTC_RC_ALLOC_MANY (SAC_mutc_rc_place_many, SAC_MUTC_RC_PLACES);             \
        SAC_IF_MUTC_RC_INDIRECT (SAC_MUTC_RC_ALLOC_W (SAC_mutc_rc_place_w);)             \
        SAC_MUTC_DISPLAY_ENV                                                             \
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
