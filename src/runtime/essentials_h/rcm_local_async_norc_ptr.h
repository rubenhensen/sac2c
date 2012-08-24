#ifdef SAC_BACKEND_C99

/**
 * SAC_RCM_local_async_norc_ptr method.
 * The descriptor can be in one of the three modes:
 *  LOCAL = perform updates locally (descriptor not shared).
 *  ASYNC = perform updates using special atomic operations (descriptor is shared).
 *  NORC = do not update the ref. count.
 *
 * The mode is stored in the descriptor and also in the LSB bits of the descriptor
 * pointer.
 *
 */
#if SAC_RC_METHOD == SAC_RCM_local_async_norc_ptr

/* We use the 'hidden data' feature to store the NORC or OTHER modes.
 * In the NORC mode the descriptor itself may be ASYNC, hence we cannot
 * simply put the NORC mode in the descriptor as it is shared. */

/* Initialize. The mode in descriptor is LOCAL. The mode in the hidden data
 * (LSB of the desc ptr) is OTHER because the real mode is in the desc. */
#define SAC_ND_INIT__RC__DEFAULT(var_NT, rc)                                             \
    {                                                                                    \
        SAC_ND_INIT__RC__C99 (var_NT, rc)                                                \
        DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) = SAC_DESC_RC_MODE_LOCAL;                  \
        SAC_DESC_SET_HIDDEN_DATA (SAC_ND_A_DESC (var_NT), SAC_DESC_RC_MODE_OTHER);       \
    }

/* Check the reference count and if it is one,
 * switch back to the LOCAL mode from the ASYNC. */
/* called from SAC_ND_INC_RC__ASYNC */
#define SAC_ND_TRY_LOCALIZE_RC__ASYNC(var_NT)                                            \
    if (SAC_ND_A_RC__ASYNC (var_NT) == 1) {                                              \
        DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) = SAC_DESC_RC_MODE_LOCAL;                  \
    }

/* Decrement and may free.
 * Call the appropriate macro according to the current mode.
 */
#define SAC_ND_DEC_RC_FREE__DEFAULT(var_NT, rc, freefun)                                 \
    {                                                                                    \
        SAC_RC_PRINT (var_NT);                                                           \
        if (SAC_DESC_HIDDEN_DATA (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_NORC) {    \
            SAC_ND_DEC_RC_FREE__NORC (var_NT, rc, freefun);                              \
        } else {                                                                         \
            if (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_LOCAL) {       \
                SAC_ND_DEC_RC_FREE__C99 (var_NT, rc, freefun);                           \
            } else {                                                                     \
                SAC_ND_DEC_RC_FREE__ASYNC (var_NT, rc, freefun);                         \
            }                                                                            \
        }                                                                                \
    }

/* Set the reference counter.
 * Call the appropriate macro according to the current mode.
 */
#define SAC_ND_SET__RC__DEFAULT(var_NT, rc)                                              \
    {                                                                                    \
        SAC_RC_PRINT (var_NT);                                                           \
        if (SAC_DESC_HIDDEN_DATA (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_NORC) {    \
            SAC_ND_SET__RC__NORC (var_NT, rc);                                           \
        } else {                                                                         \
            if (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_LOCAL) {       \
                SAC_ND_SET__RC__C99 (var_NT, rc);                                        \
            } else if (DESC_RC_MODE (SAC_ND_A_DESC (var_NT))) {                          \
                SAC_ND_SET__RC__ASYNC (var_NT, rc);                                      \
            }                                                                            \
        }                                                                                \
    }

/* Decrement the reference counter.
 * Call the appropriate macro according to the current mode.
 */
#define SAC_ND_DEC_RC__DEFAULT(var_NT, rc)                                               \
    {                                                                                    \
        SAC_RC_PRINT (var_NT);                                                           \
        if (SAC_DESC_HIDDEN_DATA (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_NORC) {    \
            SAC_ND_DEC_RC__NORC (var_NT, rc);                                            \
        } else {                                                                         \
            if (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_LOCAL) {       \
                SAC_ND_DEC_RC__C99 (var_NT, rc);                                         \
            } else {                                                                     \
                SAC_ND_DEC_RC__ASYNC (var_NT, rc);                                       \
            }                                                                            \
        }                                                                                \
    }

/* Increment the reference counter.
 * Call the appropriate macro according to the current mode.
 */
#define SAC_ND_INC_RC__DEFAULT(var_NT, rc)                                               \
    {                                                                                    \
        SAC_RC_PRINT (var_NT);                                                           \
        if (SAC_DESC_HIDDEN_DATA (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_NORC) {    \
            SAC_ND_INC_RC__NORC (var_NT, rc);                                            \
        } else {                                                                         \
            if (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_LOCAL) {       \
                SAC_ND_INC_RC__C99 (var_NT, rc);                                         \
            } else {                                                                     \
                SAC_ND_INC_RC__ASYNC (var_NT, rc);                                       \
            }                                                                            \
        }                                                                                \
    }

/* Return the current value of the reference counter.
 * Call the appropriate macro according to the current mode. */
#define SAC_ND_A_RC__DEFAULT(var_NT)                                                     \
    (SAC_DESC_HIDDEN_DATA (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_NORC              \
       ? SAC_ND_A_RC__NORC (var_NT)                                                      \
       : (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_LOCAL                \
            ? SAC_ND_A_RC__C99 (var_NT)                                                  \
            : SAC_ND_A_RC__ASYNC (var_NT)))

#if 0
/* Return the current value of the reference counter.
 * Call the appropriate macro according to the current mode.
 * In the ASYNC mode check the reference count and if it is one,
 * switch back to the LOCAL mode.
 */
#define SAC_ND_A_RC__DEFAULT(var_NT)                                                     \
    ({                                                                                   \
        int rc;                                                                          \
        SAC_RC_PRINT (var_NT);                                                           \
        if (SAC_DESC_HIDDEN_DATA (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_NORC) {    \
            rc = SAC_ND_A_RC__NORC (var_NT);                                             \
        } else {                                                                         \
            if (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_LOCAL) {       \
                rc = SAC_ND_A_RC__C99 (var_NT);                                          \
            } else {                                                                     \
                rc = SAC_ND_A_RC__ASYNC (var_NT);                                        \
                if (rc == 1) {                                                           \
                    DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) = SAC_DESC_RC_MODE_LOCAL;      \
                }                                                                        \
            }                                                                            \
        }                                                                                \
        (int)rc;                                                                         \
    })
#endif

#define SAC_ND_RC_TO_NORC__NODESC(var_NT) /* noop */

#define SAC_ND_RC_FROM_NORC__NODESC(var_NT) /* noop */

/* Sets the hidden data to NORC, but does not touch the descriptor itself.
 * The descriptor may be in the ASYNC mode */
#define SAC_ND_RC_TO_NORC__DESC(var_NT)                                                  \
    if (SAC_DESC_HIDDEN_DATA (SAC_ND_A_DESC (var_NT)) != SAC_DESC_RC_MODE_NORC) {        \
        SAC_DESC_SET_HIDDEN_DATA (SAC_ND_A_DESC (var_NT), SAC_DESC_RC_MODE_NORC);        \
    }

/* Restore the mode in the hidden data from the descriptor. */
#define SAC_ND_RC_FROM_NORC__DESC(var_NT)                                                \
    SAC_DESC_SET_HIDDEN_DATA (SAC_ND_A_DESC (var_NT),                                    \
                              DESC_RC_MODE (SAC_ND_A_DESC (var_NT)));

/* This method supports the NORC mode. */
#define SAC_HAS_RC_NORC 1

/* THIS DOES NOT CREATE A DESCRIPTOR AS COMMENT SUGGESTS */
/* SAC_ND_RC_GIVE_ASYNC is generated in m4.
 * It *consumes* a single reference from the input descriptor and returns
 * a (possibly new) descriptor that can be accessed asynchronously to
 * the original one.
 *
 * The input descriptor must NOT be in the NORC mode.
 */

#define SAC_ND_RC_GIVE_ASYNC__NODESC(new, array)                                         \
    SAC_ND_A_FIELD (new) = SAC_ND_GETVAR (array, SAC_ND_A_FIELD (array));

#define SAC_ND_RC_GIVE_ASYNC__DESC(new, array)                                           \
    {                                                                                    \
        SAC_ASSERT_RC (DESC_RC_MODE (SAC_ND_A_DESC (array)) != SAC_DESC_RC_MODE_NORC,    \
                       "SAC_RCM_local_pasync_norc_desc::SAC_ND_RC_GIVE_ASYNC__DESC: "    \
                       "called on NORC descriptor!");                                    \
        SAC_ND_A_FIELD (new) = SAC_ND_GETVAR (array, SAC_ND_A_FIELD (array));            \
        if (DESC_RC (SAC_ND_A_DESC (array)) > 1) {                                       \
            SAC_ND_A_DESC_RC_MODE (array) = SAC_DESC_RC_MODE_ASYNC;                      \
            SAC_DESC_SET_HIDDEN_DATA (SAC_ND_A_DESC (array), SAC_DESC_RC_MODE_ASYNC);    \
        }                                                                                \
        SAC_ND_A_DESC (new) = SAC_ND_A_DESC (array);                                     \
    }

#endif /* SAC_RC_METHOD == SAC_RCM_local_async_norc_ptr */

#endif /* defined SAC_BACKEND_C99 */
