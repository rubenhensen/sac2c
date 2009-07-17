#define SAC_MUTC_TOSTRING                                                                 \
    typedef char *string;                                                                 \
    SAC_MUTC_START_DEF_FUN2 (                                                             \
      to_string, void, VOID,                                                              \
      SAC_ND_PARAM_out ((out,                                                             \
                         T_SHP (SCL,                                                      \
                                T_HID (HID,                                               \
                                       T_UNQ (NUQ,                                        \
                                              T_REG (INT,                                 \
                                                     T_SCO (GLO,                          \
                                                            T_USG (PAR, T_EMPTY))))))),   \
                        string),                                                          \
      SAC_ND_PARAM_in ((ar,                                                               \
                        T_SHP (AUD,                                                       \
                               T_HID (NHD,                                                \
                                      T_UNQ (NUQ,                                         \
                                             T_REG (INT,                                  \
                                                    T_SCO (GLO,                           \
                                                           T_USG (PAR, T_EMPTY))))))),    \
                       char),                                                             \
      sl_glparm (int, length))                                                            \
    {                                                                                     \
        SAC_ND_DECL__DESC ((str,                                                          \
                            T_SHP (SCL,                                                   \
                                   T_HID (HID,                                            \
                                          T_UNQ (NUQ,                                     \
                                                 T_REG (INT,                              \
                                                        T_SCO (GLO,                       \
                                                               T_USG (NON,                \
                                                                      T_EMPTY))))))), )   \
        SAC_ND_DECL__DATA ((str,                                                          \
                            T_SHP (SCL,                                                   \
                                   T_HID (HID,                                            \
                                          T_UNQ (NUQ,                                     \
                                                 T_REG (INT,                              \
                                                        T_SCO (GLO,                       \
                                                               T_USG (NON,                \
                                                                      T_EMPTY))))))),     \
                           string, )                                                      \
        if ((SAC_ND_A_RC (                                                                \
               (ar,                                                                       \
                T_SHP (AUD,                                                               \
                       T_HID (NHD,                                                        \
                              T_UNQ (NUQ,                                                 \
                                     T_REG (INT, T_SCO (GLO, T_USG (PAR, T_EMPTY))))))))  \
             == 1)                                                                        \
            && (SAC_ND_READ ((ar,                                                         \
                              T_SHP (AUD,                                                 \
                                     T_HID (NHD,                                          \
                                            T_UNQ (NUQ,                                   \
                                                   T_REG (INT,                            \
                                                          T_SCO (GLO,                     \
                                                                 T_USG (PAR,              \
                                                                        T_EMPTY))))))),   \
                             sl_getp (length) - 1)                                        \
                == '\0')) {                                                               \
            SAC_ND_A_DESC (                                                               \
              (str,                                                                       \
               T_SHP (SCL,                                                                \
                      T_HID (HID,                                                         \
                             T_UNQ (NUQ,                                                  \
                                    T_REG (INT, T_SCO (GLO, T_USG (NON, T_EMPTY))))))))   \
              = SAC_ND_A_DESC (                                                           \
                (ar,                                                                      \
                 T_SHP (AUD,                                                              \
                        T_HID (NHD,                                                       \
                               T_UNQ (NUQ,                                                \
                                      T_REG (INT,                                         \
                                             T_SCO (GLO, T_USG (PAR, T_EMPTY))))))));     \
            SAC_ND_GET_VAR ((str,                                                         \
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
              = SAC_ND_GET_VAR (                                                          \
                (ar, T_SHP (AUD,                                                          \
                            T_HID (NHD,                                                   \
                                   T_UNQ (NUQ,                                            \
                                          T_REG (INT,                                     \
                                                 T_SCO (GLO, T_USG (PAR, T_EMPTY))))))),  \
                SAC_ND_A_FIELD (                                                          \
                  (ar,                                                                    \
                   T_SHP (AUD,                                                            \
                          T_HID (NHD,                                                     \
                                 T_UNQ (NUQ,                                              \
                                        T_REG (INT,                                       \
                                               T_SCO (GLO, T_USG (PAR, T_EMPTY)))))))));  \
        } else {                                                                          \
            SAC_ND_ALLOC__DESC (                                                          \
              (str,                                                                       \
               T_SHP (SCL,                                                                \
                      T_HID (HID,                                                         \
                             T_UNQ (NUQ,                                                  \
                                    T_REG (INT, T_SCO (GLO, T_USG (NON, T_EMPTY))))))),   \
              0)                                                                          \
            SAC_ND_SET__RC ((str,                                                         \
                             T_SHP (SCL,                                                  \
                                    T_HID (HID,                                           \
                                           T_UNQ (NUQ,                                    \
                                                  T_REG (INT,                             \
                                                         T_SCO (GLO,                      \
                                                                T_USG (NON,               \
                                                                       T_EMPTY))))))),    \
                            1)                                                            \
            SAC_ND_A_FIELD (                                                              \
              (str,                                                                       \
               T_SHP (SCL,                                                                \
                      T_HID (HID,                                                         \
                             T_UNQ (NUQ,                                                  \
                                    T_REG (INT, T_SCO (GLO, T_USG (NON, T_EMPTY))))))))   \
              = (string)SAC_MALLOC (sl_getp (length) + 1);                                \
            strncpy (SAC_ND_GET_VAR (                                                     \
                       (str,                                                              \
                        T_SHP (SCL,                                                       \
                               T_HID (HID,                                                \
                                      T_UNQ (NUQ,                                         \
                                             T_REG (INT,                                  \
                                                    T_SCO (GLO,                           \
                                                           T_USG (NON, T_EMPTY))))))),    \
                       SAC_ND_A_FIELD (                                                   \
                         (str,                                                            \
                          T_SHP (SCL,                                                     \
                                 T_HID (HID,                                              \
                                        T_UNQ (NUQ,                                       \
                                               T_REG (INT,                                \
                                                      T_SCO (GLO,                         \
                                                             T_USG (NON,                  \
                                                                    T_EMPTY))))))))),     \
                     SAC_ND_GET_VAR (                                                     \
                       (ar,                                                               \
                        T_SHP (AUD,                                                       \
                               T_HID (NHD,                                                \
                                      T_UNQ (NUQ,                                         \
                                             T_REG (INT,                                  \
                                                    T_SCO (GLO,                           \
                                                           T_USG (PAR, T_EMPTY))))))),    \
                       SAC_ND_A_FIELD (                                                   \
                         (ar,                                                             \
                          T_SHP (AUD,                                                     \
                                 T_HID (NHD,                                              \
                                        T_UNQ (NUQ,                                       \
                                               T_REG (INT,                                \
                                                      T_SCO (GLO,                         \
                                                             T_USG (PAR,                  \
                                                                    T_EMPTY))))))))),     \
                     sl_getp (length));                                                   \
            SAC_ND_GET_VAR ((str,                                                         \
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
            [sl_getp (length)]                                                            \
              = '\0';                                                                     \
            SAC_ND_DEC_RC_FREE (                                                          \
              (ar,                                                                        \
               T_SHP (AUD,                                                                \
                      T_HID (NHD,                                                         \
                             T_UNQ (NUQ,                                                  \
                                    T_REG (INT, T_SCO (GLO, T_USG (PAR, T_EMPTY))))))),   \
              1, )                                                                        \
        }                                                                                 \
        SAC_ND_RET_out ((out,                                                             \
                         T_SHP (SCL,                                                      \
                                T_HID (HID,                                               \
                                       T_UNQ (NUQ,                                        \
                                              T_REG (INT,                                 \
                                                     T_SCO (GLO,                          \
                                                            T_USG (PAR, T_EMPTY))))))),   \
                        (str,                                                             \
                         T_SHP (SCL,                                                      \
                                T_HID (HID,                                               \
                                       T_UNQ (NUQ,                                        \
                                              T_REG (INT,                                 \
                                                     T_SCO (GLO,                          \
                                                            T_USG (NON, T_EMPTY))))))))   \
    }                                                                                     \
    SAC_MUTC_END_DEF_FUN ()

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
