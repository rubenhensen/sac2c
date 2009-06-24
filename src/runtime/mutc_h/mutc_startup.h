#ifndef _SAC_MUTC_STARTUP_H_

#define _SAC_MUTC_STARTUP_H_

#define SAC_MUTC_INCLUDE m4_include (sac_helpers.slh)

#define SAC_MUTC_STARTUP                                                                 \
    SAC_MUTC_STARTUP_ANON ()                                                             \
    SAC_MUTC_INCLUDE                                                                     \
    SAC_MUTC_WORLD_OBJECT                                                                \
    SAC_MUTC_UNIN                                                                        \
    SAC_MUTC_TOSTRING                                                                    \
    SAC_MUTC_SAC_SVP_IO_PUTN

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
    T_SHP (SCL, T_HID (NHD, T_UNQ (UNQ, T_REG (INT, T_SCO (GLO, T_USG (PAR, T_EMPTY))))))

#define SAC_MUTC_STARTUP_ANON()                                                          \
    m4_define ([[_sl_anon_counter]], 0)                                                  \
      m4_define ([[sl_anon]],                                                            \
                 [[m4_step ([[_sl_anon_counter]]) _sl_anonarg[[]] _sl_anon_counter]])

#define SAC_MUTC_CLIB_STRNCPY                                                            \
    m4_define ([[strncpy]], [[({                                                         \
                   char *restrict s1 = ([[$1]]);                                         \
                   const char *restrict s2 = ([[$2]]);                                   \
                   size_t n = ([[$3]]), x = 0;                                           \
                   while (x++ < n && *s2)                                                \
                       *s1++ = *s2++;                                                    \
                   while (x++ < n)                                                       \
                       *s1++ = 0;                                                        \
                   s1;                                                                   \
               })]])

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
                         SAC_ND_ARG_FLAG_out (SAC_MUTC_MAIN_RES_NT, int, FUN));          \
    }                                                                                    \
    sl_enddef

#if SAC_MUTC_MACROS
SAC_MUTC_STARTUP
#endif /* SAC_MUTC_MACROS */

#endif /* _SAC_MUTC_STARTUP_H_ */
