
#define SAC_MUTC_TOSTRING                                                                \
    SAC_ND_DEF_FUN_BEGIN2 (                                                              \
      to_string, void,                                                                   \
      SAC_ND_PARAM_out ((out,                                                            \
                         T_SHP (SCL,                                                     \
                                T_HID (HID,                                              \
                                       T_UNQ (NUQ,                                       \
                                              T_REG (INT,                                \
                                                     T_SCO (GLO,                         \
                                                            T_USG (FPA, T_EMPTY))))))),  \
                        void *),                                                         \
      SAC_ND_PARAM_in ((ar,                                                              \
                        T_SHP (AUD,                                                      \
                               T_HID (NHD,                                               \
                                      T_UNQ (NUQ,                                        \
                                             T_REG (INT,                                 \
                                                    T_SCO (GLO,                          \
                                                           T_USG (FPA, T_EMPTY))))))),   \
                       unsigned char),                                                   \
      SAC_ND_PARAM_in ((length,                                                          \
                        T_SHP (SCL,                                                      \
                               T_HID (NHD,                                               \
                                      T_UNQ (UNQ,                                        \
                                             T_REG (INT,                                 \
                                                    T_SCO (GLO,                          \
                                                           T_USG (FPA, T_EMPTY))))))),   \
                       int))                                                             \
    {                                                                                    \
        SAC_ND_DECL__DESC ((str,                                                         \
                            T_SHP (SCL,                                                  \
                                   T_HID (HID,                                           \
                                          T_UNQ (NUQ,                                    \
                                                 T_REG (INT,                             \
                                                        T_SCO (GLO,                      \
                                                               T_USG (NON,               \
                                                                      T_EMPTY))))))), )  \
        SAC_ND_DECL__DATA ((str,                                                         \
                            T_SHP (SCL,                                                  \
                                   T_HID (HID,                                           \
                                          T_UNQ (NUQ,                                    \
                                                 T_REG (INT,                             \
                                                        T_SCO (GLO,                      \
                                                               T_USG (NON,               \
                                                                      T_EMPTY))))))),    \
                           void *, )                                                     \
        if ((SAC_ND_A_RC (                                                               \
               (ar,                                                                      \
                T_SHP (AUD,                                                              \
                       T_HID (NHD,                                                       \
                              T_UNQ (NUQ,                                                \
                                     T_REG (INT, T_SCO (GLO, T_USG (FPA, T_EMPTY)))))))) \
             == 1)                                                                       \
            && (SAC_ND_READ (                                                            \
                  (ar,                                                                   \
                   T_SHP (AUD,                                                           \
                          T_HID (NHD,                                                    \
                                 T_UNQ (NUQ,                                             \
                                        T_REG (INT,                                      \
                                               T_SCO (GLO, T_USG (FPA, T_EMPTY))))))),   \
                  SAC_ND_GETVAR (                                                        \
                    (length,                                                             \
                     T_SHP (SCL,                                                         \
                            T_HID (NHD,                                                  \
                                   T_UNQ (UNQ,                                           \
                                          T_REG (INT,                                    \
                                                 T_SCO (GLO, T_USG (FPA, T_EMPTY))))))), \
                    SAC_ND_A_FIELD (                                                     \
                      (length,                                                           \
                       T_SHP (SCL,                                                       \
                              T_HID (NHD,                                                \
                                     T_UNQ (UNQ,                                         \
                                            T_REG (INT,                                  \
                                                   T_SCO (GLO,                           \
                                                          T_USG (FPA, T_EMPTY))))))))))  \
                == '\0')) {                                                              \
            SAC_ND_A_DESC (                                                              \
              (str,                                                                      \
               T_SHP (SCL,                                                               \
                      T_HID (HID,                                                        \
                             T_UNQ (NUQ,                                                 \
                                    T_REG (INT, T_SCO (GLO, T_USG (NON, T_EMPTY))))))))  \
              = SAC_ND_A_DESC (                                                          \
                (ar,                                                                     \
                 T_SHP (AUD,                                                             \
                        T_HID (NHD,                                                      \
                               T_UNQ (NUQ,                                               \
                                      T_REG (INT,                                        \
                                             T_SCO (GLO, T_USG (FPA, T_EMPTY))))))));    \
            SAC_ND_GETVAR ((str,                                                         \
                            T_SHP (SCL,                                                  \
                                   T_HID (HID,                                           \
                                          T_UNQ (NUQ,                                    \
                                                 T_REG (INT,                             \
                                                        T_SCO (GLO,                      \
                                                               T_USG (NON,               \
                                                                      T_EMPTY))))))),    \
                           SAC_ND_A_FIELD (                                              \
                             (str,                                                       \
                              T_SHP (SCL,                                                \
                                     T_HID (HID,                                         \
                                            T_UNQ (NUQ,                                  \
                                                   T_REG (INT,                           \
                                                          T_SCO (GLO,                    \
                                                                 T_USG (NON,             \
                                                                        T_EMPTY))))))))) \
              = SAC_ND_GETVAR (                                                          \
                (ar, T_SHP (AUD,                                                         \
                            T_HID (NHD,                                                  \
                                   T_UNQ (NUQ,                                           \
                                          T_REG (INT,                                    \
                                                 T_SCO (GLO, T_USG (FPA, T_EMPTY))))))), \
                SAC_ND_A_FIELD (                                                         \
                  (ar,                                                                   \
                   T_SHP (AUD,                                                           \
                          T_HID (NHD,                                                    \
                                 T_UNQ (NUQ,                                             \
                                        T_REG (INT,                                      \
                                               T_SCO (GLO, T_USG (FPA, T_EMPTY))))))))); \
        } else {                                                                         \
            SAC_ND_ALLOC__DESC (                                                         \
              (str,                                                                      \
               T_SHP (SCL,                                                               \
                      T_HID (HID,                                                        \
                             T_UNQ (NUQ,                                                 \
                                    T_REG (INT, T_SCO (GLO, T_USG (NON, T_EMPTY))))))),  \
              0)                                                                         \
            SAC_ND_SET__RC ((str,                                                        \
                             T_SHP (SCL,                                                 \
                                    T_HID (HID,                                          \
                                           T_UNQ (NUQ,                                   \
                                                  T_REG (INT,                            \
                                                         T_SCO (GLO,                     \
                                                                T_USG (NON,              \
                                                                       T_EMPTY))))))),   \
                            1)                                                           \
            SAC_HM_MALLOC (                                                              \
              SAC_ND_A_FIELD (                                                           \
                (str,                                                                    \
                 T_SHP (SCL,                                                             \
                        T_HID (HID,                                                      \
                               T_UNQ (NUQ,                                               \
                                      T_REG (INT,                                        \
                                             T_SCO (GLO, T_USG (NON, T_EMPTY)))))))),    \
              SAC_ND_GETVAR (                                                            \
                (length,                                                                 \
                 T_SHP (SCL,                                                             \
                        T_HID (NHD,                                                      \
                               T_UNQ (UNQ,                                               \
                                      T_REG (INT,                                        \
                                             T_SCO (GLO, T_USG (FPA, T_EMPTY))))))),     \
                SAC_ND_A_FIELD (                                                         \
                  (length,                                                               \
                   T_SHP (SCL,                                                           \
                          T_HID (NHD,                                                    \
                                 T_UNQ (UNQ,                                             \
                                        T_REG (INT,                                      \
                                               T_SCO (GLO, T_USG (FPA, T_EMPTY)))))))))  \
                + 1,                                                                     \
              void *);                                                                   \
            strncpy (SAC_ND_GETVAR (                                                     \
                       (str,                                                             \
                        T_SHP (SCL,                                                      \
                               T_HID (HID,                                               \
                                      T_UNQ (NUQ,                                        \
                                             T_REG (INT,                                 \
                                                    T_SCO (GLO,                          \
                                                           T_USG (NON, T_EMPTY))))))),   \
                       SAC_ND_A_FIELD (                                                  \
                         (str,                                                           \
                          T_SHP (SCL,                                                    \
                                 T_HID (HID,                                             \
                                        T_UNQ (NUQ,                                      \
                                               T_REG (INT,                               \
                                                      T_SCO (GLO,                        \
                                                             T_USG (NON,                 \
                                                                    T_EMPTY))))))))),    \
                     (char *)SAC_ND_GETVAR (                                             \
                       (ar,                                                              \
                        T_SHP (AUD,                                                      \
                               T_HID (NHD,                                               \
                                      T_UNQ (NUQ,                                        \
                                             T_REG (INT,                                 \
                                                    T_SCO (GLO,                          \
                                                           T_USG (FPA, T_EMPTY))))))),   \
                       SAC_ND_A_FIELD (                                                  \
                         (ar,                                                            \
                          T_SHP (AUD,                                                    \
                                 T_HID (NHD,                                             \
                                        T_UNQ (NUQ,                                      \
                                               T_REG (INT,                               \
                                                      T_SCO (GLO,                        \
                                                             T_USG (FPA,                 \
                                                                    T_EMPTY))))))))),    \
                     SAC_ND_GETVAR (                                                     \
                       (length,                                                          \
                        T_SHP (SCL,                                                      \
                               T_HID (NHD,                                               \
                                      T_UNQ (UNQ,                                        \
                                             T_REG (INT,                                 \
                                                    T_SCO (GLO,                          \
                                                           T_USG (FPA, T_EMPTY))))))),   \
                       SAC_ND_A_FIELD (                                                  \
                         (length,                                                        \
                          T_SHP (SCL,                                                    \
                                 T_HID (NHD,                                             \
                                        T_UNQ (UNQ,                                      \
                                               T_REG (INT,                               \
                                                      T_SCO (GLO,                        \
                                                             T_USG (FPA,                 \
                                                                    T_EMPTY))))))))));   \
            ((char *)SAC_ND_GETVAR (                                                     \
              (str,                                                                      \
               T_SHP (SCL,                                                               \
                      T_HID (HID,                                                        \
                             T_UNQ (NUQ,                                                 \
                                    T_REG (INT, T_SCO (GLO, T_USG (NON, T_EMPTY))))))),  \
              SAC_ND_A_FIELD (                                                           \
                (str,                                                                    \
                 T_SHP (SCL,                                                             \
                        T_HID (HID,                                                      \
                               T_UNQ (NUQ,                                               \
                                      T_REG (INT,                                        \
                                             T_SCO (GLO, T_USG (NON, T_EMPTY))))))))))   \
              [SAC_ND_GETVAR (                                                           \
                (length,                                                                 \
                 T_SHP (SCL,                                                             \
                        T_HID (NHD,                                                      \
                               T_UNQ (UNQ,                                               \
                                      T_REG (INT,                                        \
                                             T_SCO (GLO, T_USG (FPA, T_EMPTY))))))),     \
                SAC_ND_A_FIELD (                                                         \
                  (length,                                                               \
                   T_SHP (SCL,                                                           \
                          T_HID (NHD,                                                    \
                                 T_UNQ (UNQ,                                             \
                                        T_REG (INT,                                      \
                                               T_SCO (GLO, T_USG (FPA, T_EMPTY)))))))))] \
              = '\0';                                                                    \
            SAC_ND_DEC_RC_FREE (                                                         \
              (ar,                                                                       \
               T_SHP (AUD,                                                               \
                      T_HID (NHD,                                                        \
                             T_UNQ (NUQ,                                                 \
                                    T_REG (INT, T_SCO (GLO, T_USG (FPA, T_EMPTY))))))),  \
              1, )                                                                       \
        }                                                                                \
        SAC_ND_RET_out ((out,                                                            \
                         T_SHP (SCL,                                                     \
                                T_HID (HID,                                              \
                                       T_UNQ (NUQ,                                       \
                                              T_REG (INT,                                \
                                                     T_SCO (GLO,                         \
                                                            T_USG (FPA, T_EMPTY))))))),  \
                        (str,                                                            \
                         T_SHP (SCL,                                                     \
                                T_HID (HID,                                              \
                                       T_UNQ (NUQ,                                       \
                                              T_REG (INT,                                \
                                                     T_SCO (GLO,                         \
                                                            T_USG (NON, T_EMPTY))))))))  \
    }                                                                                    \
    SAC_ND_FUN_DEF_END2 ()

#define SAC_STRING2ARRAY(array, string_arg)                                              \
    {                                                                                    \
        int i = 0, j = 0;                                                                \
        char *string = string_arg;                                                       \
        while (string[j] != '\0') {                                                      \
            if (string[j] == '\\') {                                                     \
                switch (string[j + 1]) {                                                 \
                case 'n':                                                                \
                    array[i++] = '\n';                                                   \
                    j += 2;                                                              \
                    break;                                                               \
                case 't':                                                                \
                    array[i++] = '\t';                                                   \
                    j += 2;                                                              \
                    break;                                                               \
                case 'v':                                                                \
                    array[i++] = '\v';                                                   \
                    j += 2;                                                              \
                    break;                                                               \
                case 'b':                                                                \
                    array[i++] = '\b';                                                   \
                    j += 2;                                                              \
                    break;                                                               \
                case 'r':                                                                \
                    array[i++] = '\r';                                                   \
                    j += 2;                                                              \
                    break;                                                               \
                case 'f':                                                                \
                    array[i++] = '\f';                                                   \
                    j += 2;                                                              \
                    break;                                                               \
                case 'a':                                                                \
                    array[i++] = '\a';                                                   \
                    j += 2;                                                              \
                    break;                                                               \
                case '"':                                                                \
                    array[i++] = '"';                                                    \
                    j += 2;                                                              \
                    break;                                                               \
                default:                                                                 \
                    array[i++] = '\\';                                                   \
                    j += 1;                                                              \
                }                                                                        \
            } else {                                                                     \
                array[i++] = string[j++];                                                \
            }                                                                            \
        }                                                                                \
        array[i] = '\0';                                                                 \
    }
