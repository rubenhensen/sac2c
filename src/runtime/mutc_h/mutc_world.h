#define SAC_MUTC_WORLD_OBJECT                                                            \
    int SAC__MUTC_IO__dummy_value_which_is_completely_useless = 0;                       \
    SAC_ND_DECL__DATA ((SACo_MutcIO__mutcWorld,                                          \
                        (SCL, (NHD, (NUQ, (INT, (GLO, (NON, ))))))),                     \
                       int, )                                                            \
    SAC_ND_DECL__DESC ((SACo_MutcIO__mutcWorld,                                          \
                        (SCL, (NHD, (NUQ, (INT, (GLO, (NON, ))))))), )                   \
    SAC_NOTHING ()                                                                       \
    SAC_MUTC_START_DEF_FUN2 (SACf_MutcIO__to_MUTC_IO__i, void, VOID,                     \
                             SAC_ND_PARAM_FLAG_out ((SAC_arg_1,                          \
                                                     (SCL,                               \
                                                      (NHD,                              \
                                                       (NUQ, (INT, (GLO, (PAR, ))))))),  \
                                                    int, FUN),                           \
                             SAC_ND_PARAM_FLAG_in ((SACl_from,                           \
                                                    (SCL,                                \
                                                     (NHD,                               \
                                                      (NUQ, (INT, (GLO, (PAR, ))))))),   \
                                                   int, FUN))                            \
    {                                                                                    \
        SAC_NOTHING ()                                                                   \
        SAC_ND_RET_out ((SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (PAR, ))))))),         \
                        (SACl_from, (SCL, (NHD, (NUQ, (INT, (GLO, (PAR, )))))))) return; \
    }                                                                                    \
    SAC_MUTC_END_DEF_FUN ()                                                              \
    SAC_MUTC_START_DEF_FUN2 (SACf_MutcIO_CL_INIT__init_mutcWorld__SACt_MutcIO__MutcIO,   \
                             void, VOID,                                                 \
                             SAC_ND_PARAM_FLAG_inout ((SACp_OI_object,                   \
                                                       (SCL,                             \
                                                        (NHD,                            \
                                                         (NUQ,                           \
                                                          (INT, (GLO, (PIO, ))))))),     \
                                                      int, FUN))                         \
    {                                                                                    \
        SAC_ND_DECL__DATA ((SACp_emal_2726__isaa_1643__OI_object,                        \
                            (SCL, (NHD, (NUQ, (INT, (GLO, (NON, ))))))),                 \
                           int, )                                                        \
        SAC_ND_DECL__DESC ((SACp_emal_2726__isaa_1643__OI_object,                        \
                            (SCL, (NHD, (NUQ, (INT, (GLO, (NON, ))))))), )               \
        SAC_NOTHING ()                                                                   \
        SAC_ND_DECL__DATA ((SACp_OI_object__SSA0_1,                                      \
                            (SCL, (NHD, (NUQ, (INT, (GLO, (NON, ))))))),                 \
                           int, )                                                        \
        SAC_ND_DECL__DESC ((SACp_OI_object__SSA0_1,                                      \
                            (SCL, (NHD, (NUQ, (INT, (GLO, (NON, ))))))), )               \
        SAC_NOTHING ()                                                                   \
        SAC_ND_DECL_PARAM_inout ((SACp_OI_object,                                        \
                                  (SCL, (NHD, (NUQ, (INT, (GLO, (PIO, ))))))),           \
                                 int) SAC_NOTHING ()                                     \
          SAC_ND_ALLOC_BEGIN ((SACp_emal_2726__isaa_1643__OI_object,                     \
                               (SCL, (NHD, (NUQ, (INT, (GLO, (NON, ))))))),              \
                              1, 0)                                                      \
            SAC_ASSURE_TYPE_LINE ((SAC_ND_A_DIM (                                        \
                                     (SACp_emal_2726__isaa_1643__OI_object,              \
                                      (SCL, (NHD, (NUQ, (INT, (GLO, (NON, ))))))))       \
                                   == 0),                                                \
                                  9, "Assignment with incompatible types found!");       \
        SAC_NOOP ()                                                                      \
        SAC_ND_ALLOC_END ((SACp_emal_2726__isaa_1643__OI_object,                         \
                           (SCL, (NHD, (NUQ, (INT, (GLO, (NON, ))))))),                  \
                          1, 0)                                                          \
        SAC_ND_CREATE__SCALAR__DATA ((SACp_emal_2726__isaa_1643__OI_object,              \
                                      (SCL, (NHD, (NUQ, (INT, (GLO, (NON, ))))))),       \
                                     0)                                                  \
        SAC_MUTC_FUNAP2 (SACf_MutcIO__to_MUTC_IO__i,                                     \
                         SAC_ND_ARG_FLAG_out ((SACp_OI_object__SSA0_1,                   \
                                               (SCL,                                     \
                                                (NHD, (NUQ, (INT, (GLO, (NON, ))))))),   \
                                              int, FUN),                                 \
                         SAC_ND_ARG_FLAG_in ((SACp_emal_2726__isaa_1643__OI_object,      \
                                              (SCL,                                      \
                                               (NHD, (NUQ, (INT, (GLO, (NON, ))))))),    \
                                             int, FUN))                                  \
        SAC_NOOP ()                                                                      \
        SAC_ASSURE_TYPE_LINE ((SAC_ND_A_DIM (                                            \
                                 (SACp_OI_object,                                        \
                                  (SCL, (NHD, (NUQ, (INT, (GLO, (PIO, ))))))))           \
                               == 0),                                                    \
                              9, "Assignment with incompatible types found!");           \
        SAC_NOOP ()                                                                      \
        SAC_NOOP ()                                                                      \
        SAC_NOOP ()                                                                      \
        SAC_ND_ASSIGN__DATA ((SACp_OI_object,                                            \
                              (SCL, (NHD, (NUQ, (INT, (GLO, (PIO, ))))))),               \
                             (SACp_OI_object__SSA0_1,                                    \
                              (SCL, (NHD, (NUQ, (INT, (GLO, (NON, ))))))), )             \
        SAC_ND_RET_inout ((SACp_OI_object, (SCL, (NHD, (NUQ, (INT, (GLO, (PIO, ))))))),  \
                          (SACp_OI_object,                                               \
                           (SCL, (NHD, (NUQ, (INT, (GLO, (PIO, )))))))) return;          \
    }                                                                                    \
    SAC_MUTC_END_DEF_FUN ()
