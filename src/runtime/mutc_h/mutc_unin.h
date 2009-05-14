#define SAC_MUTC_UNIN                                                                    \
    SAC_MUTC_START_DEF_FUN2 (                                                            \
      unin_double, void, VOID,                                                           \
      SAC_ND_PARAM_out ((out,                                                            \
                         T_SHP (AKD,                                                     \
                                T_HID (HID,                                              \
                                       T_UNQ (NUQ,                                       \
                                              T_REG (INT,                                \
                                                     T_SCO (GLO,                         \
                                                            T_USG (PAR, T_EMPTY))))))),  \
                        double),                                                         \
      SAC_ND_PARAM_in ((in,                                                              \
                        T_SHP (SCL,                                                      \
                               T_HID (HID,                                               \
                                      T_UNQ (NUQ,                                        \
                                             T_REG (INT,                                 \
                                                    T_SCO (GLO,                          \
                                                           T_USG (PAR, T_EMPTY))))))),   \
                       int))                                                             \
    {                                                                                    \
        SAC_ND_DECL__DESC (                                                              \
          (tmp,                                                                          \
           T_SHP (AKD,                                                                   \
                  T_HID (HID,                                                            \
                         T_UNQ (NUQ, T_REG (INT, T_SCO (GLO, T_USG (PAR, T_EMPTY)))))))) \
        SAC_ND_DECL__DATA ((tmp,                                                         \
                            T_SHP (AKD,                                                  \
                                   T_HID (HID,                                           \
                                          T_UNQ (NUQ,                                    \
                                                 T_REG (INT,                             \
                                                        T_SCO (GLO,                      \
                                                               T_USG (PAR,               \
                                                                      T_EMPTY))))))),    \
                           double, )                                                     \
        SAC_ND_ALLOC__DESC ((tmp,                                                        \
                             T_SHP (AKD,                                                 \
                                    T_HID (HID,                                          \
                                           T_UNQ (NUQ,                                   \
                                                  T_REG (INT,                            \
                                                         T_SCO (GLO,                     \
                                                                T_USG (PAR,              \
                                                                       T_EMPTY))))))),   \
                            1)                                                           \
        SAC_ND_SET__RC ((tmp,                                                            \
                         T_SHP (AKD,                                                     \
                                T_HID (HID,                                              \
                                       T_UNQ (NUQ,                                       \
                                              T_REG (INT,                                \
                                                     T_SCO (GLO,                         \
                                                            T_USG (PAR, T_EMPTY))))))),  \
                        1)                                                               \
        SAC_ND_DESC_SIZE (                                                               \
          (tmp,                                                                          \
           T_SHP (AKD,                                                                   \
                  T_HID (HID,                                                            \
                         T_UNQ (NUQ, T_REG (INT, T_SCO (GLO, T_USG (PAR, T_EMPTY)))))))) \
          = SAC_ND_A_FIELD (                                                             \
            (in,                                                                         \
             T_SHP (SCL,                                                                 \
                    T_HID (HID,                                                          \
                           T_UNQ (NUQ,                                                   \
                                  T_REG (INT, T_SCO (GLO, T_USG (PAR, T_EMPTY))))))));   \
        SAC_ND_DESC_SHAPE ((tmp,                                                         \
                            T_SHP (AKD,                                                  \
                                   T_HID (HID,                                           \
                                          T_UNQ (NUQ,                                    \
                                                 T_REG (INT,                             \
                                                        T_SCO (GLO,                      \
                                                               T_USG (PAR,               \
                                                                      T_EMPTY))))))),    \
                           0)                                                            \
          = SAC_ND_A_FIELD (                                                             \
            (in,                                                                         \
             T_SHP (SCL,                                                                 \
                    T_HID (HID,                                                          \
                           T_UNQ (NUQ,                                                   \
                                  T_REG (INT, T_SCO (GLO, T_USG (PAR, T_EMPTY))))))));   \
        SAC_ND_ALLOC__DATA (                                                             \
          (tmp,                                                                          \
           T_SHP (AKD,                                                                   \
                  T_HID (HID,                                                            \
                         T_UNQ (NUQ, T_REG (INT, T_SCO (GLO, T_USG (PAR, T_EMPTY)))))))) \
        SAC_ND_RET_out ((out,                                                            \
                         T_SHP (AKD,                                                     \
                                T_HID (HID,                                              \
                                       T_UNQ (NUQ,                                       \
                                              T_REG (INT,                                \
                                                     T_SCO (GLO,                         \
                                                            T_USG (PAR, T_EMPTY))))))),  \
                        (tmp,                                                            \
                         T_SHP (AKD,                                                     \
                                T_HID (HID,                                              \
                                       T_UNQ (NUQ,                                       \
                                              T_REG (INT,                                \
                                                     T_SCO (GLO,                         \
                                                            T_USG (PAR, T_EMPTY))))))))  \
    }                                                                                    \
    SAC_MUTC_END_DEF_FUN ()
