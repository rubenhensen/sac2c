#define SAC_MUTC_UNIN                                                                    \
    SAC_ND_DEF_FUN_BEGIN2 (                                                              \
      unin_double, void,                                                                 \
      SAC_ND_PARAM_out ((out,                                                            \
                         T_SHP (AKD,                                                     \
                                T_HID (HID,                                              \
                                       T_UNQ (NUQ,                                       \
                                              T_REG (INT,                                \
                                                     T_SCO (GLO,                         \
                                                            T_USG (FPA, T_EMPTY))))))),  \
                        double),                                                         \
      SAC_ND_PARAM_in ((in,                                                              \
                        T_SHP (SCL,                                                      \
                               T_HID (NHD,                                               \
                                      T_UNQ (UNQ,                                        \
                                             T_REG (INT,                                 \
                                                    T_SCO (GLO,                          \
                                                           T_USG (FPA, T_EMPTY))))))),   \
                       int))                                                             \
    {                                                                                    \
        int SAC_ND_A_MIRROR_DIM (                                                        \
          (tmp,                                                                          \
           T_SHP (AKD,                                                                   \
                  T_HID (HID,                                                            \
                         T_UNQ (NUQ, T_REG (INT, T_SCO (GLO, T_USG (NON, T_EMPTY)))))))) \
          = 1;                                                                           \
        int SAC_ND_A_MIRROR_SIZE (                                                       \
          (tmp,                                                                          \
           T_SHP (AKD,                                                                   \
                  T_HID (HID,                                                            \
                         T_UNQ (NUQ,                                                     \
                                T_REG (INT, T_SCO (GLO, T_USG (NON, T_EMPTY))))))));     \
        SAC_ND_DECL__DESC ((tmp,                                                         \
                            T_SHP (AKD,                                                  \
                                   T_HID (HID,                                           \
                                          T_UNQ (NUQ,                                    \
                                                 T_REG (INT,                             \
                                                        T_SCO (GLO,                      \
                                                               T_USG (NON,               \
                                                                      T_EMPTY))))))), )  \
        SAC_ND_DECL__DATA ((tmp,                                                         \
                            T_SHP (AKD,                                                  \
                                   T_HID (HID,                                           \
                                          T_UNQ (NUQ,                                    \
                                                 T_REG (INT,                             \
                                                        T_SCO (GLO,                      \
                                                               T_USG (NON,               \
                                                                      T_EMPTY))))))),    \
                           double, )                                                     \
        SAC_ND_ALLOC__DESC ((tmp,                                                        \
                             T_SHP (AKD,                                                 \
                                    T_HID (HID,                                          \
                                           T_UNQ (NUQ,                                   \
                                                  T_REG (INT,                            \
                                                         T_SCO (GLO,                     \
                                                                T_USG (NON,              \
                                                                       T_EMPTY))))))),   \
                            1)                                                           \
        SAC_ND_SET__RC ((tmp,                                                            \
                         T_SHP (AKD,                                                     \
                                T_HID (HID,                                              \
                                       T_UNQ (NUQ,                                       \
                                              T_REG (INT,                                \
                                                     T_SCO (GLO,                         \
                                                            T_USG (NON, T_EMPTY))))))),  \
                        1)                                                               \
        SAC_ND_A_MIRROR_SIZE (                                                           \
          (tmp,                                                                          \
           T_SHP (AKD,                                                                   \
                  T_HID (HID,                                                            \
                         T_UNQ (NUQ, T_REG (INT, T_SCO (GLO, T_USG (NON, T_EMPTY)))))))) \
          = SAC_ND_READ ((in,                                                            \
                          T_SHP (SCL,                                                    \
                                 T_HID (NHD,                                             \
                                        T_UNQ (UNQ,                                      \
                                               T_REG (INT,                               \
                                                      T_SCO (GLO,                        \
                                                             T_USG (FPA,                 \
                                                                    T_EMPTY))))))), );   \
        SAC_ND_A_DESC_SIZE (                                                             \
          (tmp,                                                                          \
           T_SHP (AKD,                                                                   \
                  T_HID (HID,                                                            \
                         T_UNQ (NUQ, T_REG (INT, T_SCO (GLO, T_USG (NON, T_EMPTY)))))))) \
          = SAC_ND_READ ((in,                                                            \
                          T_SHP (SCL,                                                    \
                                 T_HID (NHD,                                             \
                                        T_UNQ (UNQ,                                      \
                                               T_REG (INT,                               \
                                                      T_SCO (GLO,                        \
                                                             T_USG (FPA,                 \
                                                                    T_EMPTY))))))), );   \
        SAC_ND_A_DESC_SHAPE ((tmp,                                                       \
                              T_SHP (AKD,                                                \
                                     T_HID (HID,                                         \
                                            T_UNQ (NUQ,                                  \
                                                   T_REG (INT,                           \
                                                          T_SCO (GLO,                    \
                                                                 T_USG (NON,             \
                                                                        T_EMPTY))))))),  \
                             0)                                                          \
          = SAC_ND_READ ((in,                                                            \
                          T_SHP (SCL,                                                    \
                                 T_HID (NHD,                                             \
                                        T_UNQ (UNQ,                                      \
                                               T_REG (INT,                               \
                                                      T_SCO (GLO,                        \
                                                             T_USG (FPA,                 \
                                                                    T_EMPTY))))))), );   \
        SAC_ND_ALLOC__DATA (                                                             \
          (tmp,                                                                          \
           T_SHP (AKD,                                                                   \
                  T_HID (HID,                                                            \
                         T_UNQ (NUQ, T_REG (INT, T_SCO (GLO, T_USG (NON, T_EMPTY)))))))) \
        SAC_ND_RET_out ((out,                                                            \
                         T_SHP (AKD,                                                     \
                                T_HID (HID,                                              \
                                       T_UNQ (NUQ,                                       \
                                              T_REG (INT,                                \
                                                     T_SCO (GLO,                         \
                                                            T_USG (FPA, T_EMPTY))))))),  \
                        (tmp,                                                            \
                         T_SHP (AKD,                                                     \
                                T_HID (HID,                                              \
                                       T_UNQ (NUQ,                                       \
                                              T_REG (INT,                                \
                                                     T_SCO (GLO,                         \
                                                            T_USG (NON, T_EMPTY))))))))  \
    }                                                                                    \
    SAC_ND_FUN_DEF_END (unin_double)
