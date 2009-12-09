#ifndef _MUTC_BENCH_H_
#define _MUTC_BENCH_H_

#if SAC_MUTC_MACROS
#define SAC_MUTC_BENCHMARK                                                               \
    struct bench {                                                                       \
        int num;                                                                         \
        struct s_interval interval;                                                      \
    };                                                                                   \
                                                                                         \
    void benchStart (struct bench *interval)                                             \
    {                                                                                    \
        mtperf_start_interval (interval->interval, 0, interval->num, "");                \
        return;                                                                          \
    }                                                                                    \
                                                                                         \
    void benchEnd (struct bench *interval)                                               \
    {                                                                                    \
        mtperf_finish_interval (interval->interval, 0);                                  \
        return;                                                                          \
    }                                                                                    \
                                                                                         \
    void benchThis ()                                                                    \
    {                                                                                    \
        return;                                                                          \
    }                                                                                    \
                                                                                         \
    void benchPrint (struct bench *interval)                                             \
    {                                                                                    \
        mtperf_report_intervals (interval->interval, 1, REPORT_FIBRE);                   \
        return;                                                                          \
    }                                                                                    \
                                                                                         \
    void benchGetInterval_i (struct bench **interval, int num)                           \
    {                                                                                    \
        interval->interval = (struct bench *)malloc (sizeof (struct bench));             \
        interval->num = num;                                                             \
        return;                                                                          \
    }                                                                                    \
                                                                                         \
    void benchDestroyInterval (struct bench *intival)                                    \
    {                                                                                    \
        free (*intival->interval);                                                       \
        free (*intival);                                                                 \
        return;                                                                          \
    }                                                                                    \
    void benchCreate (int *a)                                                            \
    {                                                                                    \
        *a = 1;                                                                          \
        return;                                                                          \
    }
#endif
#endif
