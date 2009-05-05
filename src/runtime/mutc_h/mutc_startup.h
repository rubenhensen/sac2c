#ifndef _SAC_MUTC_STARTUP_H_

#define _SAC_MUTC_STARTUP_H_

#if 0

#if SAC_MUTC_MACROS
SAC_MUTC_STARTUP
#endif /* SAC_MUTC_MACROS */

#define SAC_MUTC_STARTUP SAC_MUTC_STARTUP_ANON
SAC_MUTC_MAIN

#define SAC_MUTC_STARTUP_ANON                                                            \
    m4_define ([[_sl_anon_counter]], 0)                                                  \
      m4_define ([[sl_anon]],                                                            \
                 [[m4_step ([[_sl_anon_counter]]) _sl_anonarg[[]] _sl_anon_counter]])

#define SAC_MUTC_MAIN_RES_NT                                                             \
    (SAC_res, T_SHP (SCL, T_HND (NHD, T_UNQ (UNQ, T_REG (INT, T_EMPTY)))))
#define SAC_MUTC_MAIN                                                                    \
    sl_def (t_main, void)                                                                \
    {                                                                                    \
        SAC_ND_DECL__DATA (SAC_MUTC_MAIN_RES_NT, int, )                                  \
        SAC_ND_DECL__DESC (SAC_MUTC_MAIN_RES_NT, )                                       \
        SAC_NOTHING ()                                                                   \
        SAC_COMMANDLINE_SET (0, null);                                                   \
        SACf__MAIN__main (SAC_ND_ARG_out (SAC_MUTC_MAIN_RES_NT));                        \
    }                                                                                    \
    sl_enddef

#endif /* _SAC_MUTC_STARTUP_H_ */

#endif
