#ifndef _MUTC_BENCH_H_
#define _MUTC_BENCH_H_

#define SAC_MUTC_BENCHMARK                                                               \
    SAC_ND_DEF_FUN_BEGIN2 (benchStart, void)                                             \
    {                                                                                    \
        start_interval (sac_state->wl);                                                  \
        return;                                                                          \
    }                                                                                    \
    SAC_ND_FUN_DEF_END ()                                                                \
    SAC_ND_DEF_FUN_BEGIN2 (benchEnd, void)                                               \
    {                                                                                    \
        finish_interval (sac_state->wl);                                                 \
        return;                                                                          \
    }                                                                                    \
    SAC_ND_FUN_DEF_END ()                                                                \
    SAC_ND_DEF_FUN_BEGIN2 (benchThis, void)                                              \
    {                                                                                    \
        return;                                                                          \
    }                                                                                    \
    SAC_ND_FUN_DEF_END ()

#endif
