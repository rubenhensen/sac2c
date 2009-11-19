#ifndef _MUTC_BENCH_H_
#define _MUTC_BENCH_H_

#define SAC_MUTC_BENCHMARK                                                               \
    SAC_ND_DEF_FUN_BEGIN2 (benchStart, void)                                             \
    {                                                                                    \
        mtperf_start_interval (sac_benchmark_intervals, sac_benchmark_count, -1, "");    \
        return;                                                                          \
    }                                                                                    \
    SAC_ND_FUN_DEF_END ()                                                                \
    SAC_ND_DEF_FUN_BEGIN2 (benchEnd, void)                                               \
    {                                                                                    \
        mtperf_finish_interval (sac_benchmark_intervals, sac_benchmark_count++);         \
        return;                                                                          \
    }                                                                                    \
    SAC_ND_FUN_DEF_END ()                                                                \
    SAC_ND_DEF_FUN_BEGIN2 (benchThis, void)                                              \
    {                                                                                    \
        return;                                                                          \
    }                                                                                    \
    SAC_ND_FUN_DEF_END ()                                                                \
    SAC_ND_DEF_FUN_BEGIN2 (benchPrint, void)                                             \
    {                                                                                    \
        mtperf_report_intervals (sac_benchmark_intervals, sac_benchmark_count,           \
                                 REPORT_FIBRE);                                          \
        return;                                                                          \
    }                                                                                    \
    SAC_ND_FUN_DEF_END ()                                                                \
    SAC_ND_DEF_FUN_BEGIN2 (benchGetInterval, void, sl_parm (void **, intival),           \
                           sl_parm (int, num))                                           \
    {                                                                                    \
        int *lnum = (int *)*intival;                                                     \
        *lnum = malloc (sizeof (int));                                                   \
        lnum[0] = num;                                                                   \
        return;                                                                          \
    }                                                                                    \
    SAC_ND_FUN_DEF_END ()                                                                \
    SAC_ND_DEF_FUN_BEGIN2 (benchDestroyInterval, void, sl_parm (void **, intival))       \
    {                                                                                    \
        free (*intival);                                                                 \
        return;                                                                          \
    }                                                                                    \
    SAC_ND_FUN_DEF_END ()
#endif
