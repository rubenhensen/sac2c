#define SAC_MUTC_WORLD_OBJECT                                                            \
    int SAC__MUTC_IO__dummy_value_which_is_completely_useless = 0;                       \
    SAC_ND_DECL__DATA ((SACo_MutcIO__mutcWorld,                                          \
                        (SCL, (NHD, (NUQ, (INT, (GLO, (NON, ))))))),                     \
                       int, )                                                            \
    SAC_ND_DECL__DESC ((SACo_MutcIO__mutcWorld,                                          \
                        (SCL, (NHD, (NUQ, (INT, (GLO, (NON, ))))))), )                   \
    SAC_NOTHING ()                                                                       \
    SAC_MUTC_DEF_THREADFUN_BEGIN2 (SACf_MutcIO__to_MUTC_IO__i, ,                         \
                                   SAC_ND_PARAM_out ((SAC_arg_1,                         \
                                                      (SCL,                              \
                                                       (NHD,                             \
                                                        (NUQ, (INT, (GLO, (TPA, ))))))), \
                                                     int),                               \
                                   SAC_ND_PARAM_in ((SACl_from,                          \
                                                     (SCL,                               \
                                                      (NHD,                              \
                                                       (NUQ, (INT, (GLO, (TPA, ))))))),  \
                                                    int))                                \
    {                                                                                    \
        SAC_NOTHING ()                                                                   \
        SAC_ND_RET_out ((SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (TPA, ))))))),         \
                        (SACl_from, (SCL, (NHD, (NUQ, (INT, (GLO, (TPA, )))))))) return; \
    }                                                                                    \
    SAC_MUTC_THREADFUN_DEF_END (SACf_MutcIO__to_MUTC_IO__i)                              \
    SAC_MUTC_DEF_THREADFUN_BEGIN2 (                                                      \
      SACf_MutcIO_CL_INIT__init_mutcWorld__SACt_MutcIO__MutcIO, ,                        \
      SAC_ND_PARAM_inout ((SACp_OI_object, (SCL, (NHD, (NUQ, (INT, (GLO, (TPO, ))))))),  \
                          int))                                                          \
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
                                  (SCL, (NHD, (NUQ, (INT, (GLO, (TPO, ))))))),           \
                                 int) SAC_NOTHING ()                                     \
          SAC_ND_ALLOC_BEGIN ((SACp_emal_2726__isaa_1643__OI_object,                     \
                               (SCL, (NHD, (NUQ, (INT, (GLO, (NON, ))))))),              \
                              1, 0, int)                                                 \
            SAC_ASSURE_TYPE_LINE ("SAC_MUTC_WORLD_OBJECT", 9, 0,                         \
                                  (SAC_ND_A_DIM (                                        \
                                     (SACp_emal_2726__isaa_1643__OI_object,              \
                                      (SCL, (NHD, (NUQ, (INT, (GLO, (NON, ))))))))       \
                                   == 0),                                                \
                                  "Assignment with incompatible types found");           \
        SAC_NOOP ()                                                                      \
        SAC_ND_ALLOC_END ((SACp_emal_2726__isaa_1643__OI_object,                         \
                           (SCL, (NHD, (NUQ, (INT, (GLO, (NON, ))))))),                  \
                          1, 0, int)                                                     \
        SAC_ND_CREATE__SCALAR__DATA ((SACp_emal_2726__isaa_1643__OI_object,              \
                                      (SCL, (NHD, (NUQ, (INT, (GLO, (NON, ))))))),       \
                                     0)                                                  \
        SAC_MUTC_THREAD_FUNAP (SACf_MutcIO__to_MUTC_IO__i,                               \
                               SAC_ND_ARG_out ((SACp_OI_object__SSA0_1,                  \
                                                (SCL,                                    \
                                                 (NHD, (NUQ, (INT, (GLO, (TAG, ))))))),  \
                                               int),                                     \
                               SAC_ND_ARG_in ((SACp_emal_2726__isaa_1643__OI_object,     \
                                               (SCL,                                     \
                                                (NHD, (NUQ, (INT, (GLO, (TAG, ))))))),   \
                                              int))                                      \
        SAC_NOOP ()                                                                      \
        SAC_ASSURE_TYPE_LINE ("SAC_MUTC_WORLD_OBJECT", 9, 0,                             \
                              (SAC_ND_A_DIM (                                            \
                                 (SACp_OI_object,                                        \
                                  (SCL, (NHD, (NUQ, (INT, (GLO, (TPO, ))))))))           \
                               == 0),                                                    \
                              "Assignment with incompatible types found");               \
        SAC_NOOP ()                                                                      \
        SAC_NOOP ()                                                                      \
        SAC_NOOP ()                                                                      \
        SAC_ND_ASSIGN__DATA ((SACp_OI_object,                                            \
                              (SCL, (NHD, (NUQ, (INT, (GLO, (TPO, ))))))),               \
                             (SACp_OI_object__SSA0_1,                                    \
                              (SCL, (NHD, (NUQ, (INT, (GLO, (NON, ))))))), )             \
        SAC_ND_RET_inout ((SACp_OI_object, (SCL, (NHD, (NUQ, (INT, (GLO, (TPO, ))))))),  \
                          (SACp_OI_object,                                               \
                           (SCL, (NHD, (NUQ, (INT, (GLO, (TPO, )))))))) return;          \
    }                                                                                    \
    SAC_MUTC_THREADFUN_DEF_END (SACf_MutcIO_CL_INIT__init_mutcWorld__SACt_MutcIO__MutcIO)
