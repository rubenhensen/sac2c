#ifndef _SAC_MUTC_STARTUP_H_

#define _SAC_MUTC_STARTUP_H_

#define SAC_MUTC_STARTUP                                                                 \
    SAC_MUTC_STARTUP_ANON                                                                \
    SAC_MUTC_MAIN

#define SAC_MUTC_STARTUP_ANON                                                            \
    m4_define ([[_sl_anon_counter]], 0)                                                  \
      m4_define ([[sl_anon]],                                                            \
                 [[m4_step ([[_sl_anon_counter]]) _sl_anonarg[[]] _sl_anon_counter]])

#define SAC_MUTC_MAIN_RES_NT                                                             \
    (SAC_res, T_SHP (SCL, T_HID (NHD, T_UNQ (UNQ, T_REG (INT, T_SCO (GLO, T_EMPTY))))))
#define SAC_MUTC_MAIN                                                                    \
    sl_def (t_main, void)                                                                \
    {                                                                                    \
        SAC_ND_DECL__DATA (SAC_MUTC_MAIN_RES_NT, int, )                                  \
        SAC_ND_DECL__DESC (SAC_MUTC_MAIN_RES_NT, )                                       \
        SAC_NOTHING ()                                                                   \
        SAC_COMMANDLINE_SET (0, NULL);                                                   \
        SAC_MUTC_FUNAP2 (SACwf__MAIN__main,                                              \
                         SAC_ND_ARG_FLAG_out (SAC_MUTC_MAIN_RES_NT, int *, FUN));        \
    }                                                                                    \
    sl_enddef

#if SAC_MUTC_MACROS
SAC_MUTC_STARTUP
#endif /* SAC_MUTC_MACROS */

#endif /* _SAC_MUTC_STARTUP_H_ */
