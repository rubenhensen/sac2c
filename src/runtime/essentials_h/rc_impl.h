#ifdef SAC_BACKEND_C99
/**
 * This header implements reference counting schemes in ICMs
 * in the C99 backend. The mutc backend has its own definitions
 * in mutc_rc.h
 */

/******************************************************************************
 *
 * ICMs for refcounting data objects
 * =================================
 *
 * ND_SET__RC( var_NT, rc) :
 *   sets the refcount (in the descriptor) of a data object
 * ND_INC_RC( var_NT, rc) :
 *   increments the refcount of a data object
 * ND_DEC_RC( var_NT, rc) :
 *   decrements the refcount of a data object
 * ND_DEC_RC_FREE( var_NT, rc, freefun) :
 *   decrements the refcount and frees the data object if refcount becomes 0
 *
 ******************************************************************************/

#if SAC_DEBUG_RC
#include <stdio.h>
#define SAC_RC_PRINT(var_NT)                                                             \
    fprintf (stddebug, " ");                                                             \
    printf ("%s:%d " TO_STR (var_NT) " @ %p = {%d} [ %d, %p, %d]\n", __FILE__, __LINE__, \
            SAC_REAL_DESC_POINTER (SAC_ND_A_DESC (var_NT)),                              \
            DESC_RC_MODE (SAC_ND_A_DESC (var_NT)), (int)SAC_ND_A_DESC (var_NT)[0],       \
            (void *)SAC_ND_A_DESC (var_NT)[1], (int)SAC_ND_A_DESC (var_NT)[2]);

#define SAC_IF_DEBUG_RC(a) a
#else
#define SAC_RC_PRINT(var_NT)
#define SAC_IF_DEBUG_RC(a)
#endif

#define SAC_ASSERT_RC(a, b)

/** =========================================================================== */

/**
 * Functions for the SAC_RCM_async method.
 * They are also used for some other combined rc methods, hence they are always defined.
 * We use the following gcc atomic operations:
 *  __sync_sub_and_fetch
 *  __sync_add_and_fetch
 *  __sync_synchronize()
 */

/* Initialize the RC method.
 * The mode is set to LOCAL.
 */
#define SAC_ND_INIT__RC__ASYNC(var_NT, rc)                                               \
    {                                                                                    \
        SAC_RC_PRINT (var_NT);                                                           \
        SAC_ND_INIT__RC__C99 (var_NT, rc)                                                \
    }

/* Atomically decrement the reference counter. */
#define SAC_ND_DEC_RC__ASYNC(var_NT, rc)                                                 \
    {                                                                                    \
        SAC_IF_DEBUG_RC (printf (TO_STR (var_NT) " = %p\n", SAC_ND_A_DESC (var_NT)););   \
        SAC_TR_REF_PRINT (("ND_DEC_RC( %s, %d)", NT_STR (var_NT), rc))                   \
        SAC_RC_PRINT (var_NT);                                                           \
        SAC_ASSERT_RC (DESC_RC (SAC_ND_A_DESC (var_NT)), SAC_ND_A_DESC (var_NT));        \
        __sync_sub_and_fetch (&DESC_RC (SAC_ND_A_DESC (var_NT)), rc);                    \
        SAC_TR_REF_PRINT_RC (var_NT)                                                     \
    }

/* Atomically set the reference counter to the given value. */
#define SAC_ND_SET__RC__ASYNC(var_NT, rc)                                                \
    {                                                                                    \
        SAC_IF_DEBUG_RC (printf (TO_STR (var_NT) " = %p\n", SAC_ND_A_DESC (var_NT)););   \
        SAC_TR_REF_PRINT (("ND_SET__RC( %s, %d)", NT_STR (var_NT), rc))                  \
        SAC_RC_PRINT (var_NT);                                                           \
        DESC_RC (SAC_ND_A_DESC (var_NT)) = rc;                                           \
        SAC_TR_REF_PRINT_RC (var_NT)                                                     \
    }

/* Atomically increment the reference counter. */
#define SAC_ND_INC_RC__ASYNC(var_NT, rc)                                                 \
    {                                                                                    \
        SAC_TR_REF_PRINT (("ND_INC_RC_ASYNC_RC( %s, %d)", NT_STR (var_NT), rc))          \
        SAC_RC_PRINT (var_NT);                                                           \
        __sync_add_and_fetch (&DESC_RC (SAC_ND_A_DESC (var_NT)), rc);                    \
        SAC_TR_REF_PRINT_RC (var_NT)                                                     \
    }

/* Return the reference count. */
#define SAC_ND_A_RC__ASYNC(var_NT)                                                       \
    ({                                                                                   \
        SAC_TR_REF_PRINT (("ND_A_RC( %s)", NT_STR (var_NT)))                             \
        SAC_IF_DEBUG_RC (printf (TO_STR (var_NT) " = %p\n", SAC_ND_A_DESC (var_NT)););   \
        SAC_RC_PRINT (var_NT);                                                           \
        DESC_RC (SAC_ND_A_DESC (var_NT));                                                \
    })

/* Atomically decrement the reference counter, and free the object if
 * it has reached zero. */
// TODO: freefun ??
#define SAC_ND_DEC_RC_FREE__ASYNC(var_NT, rc, freefun)                                   \
    {                                                                                    \
        SAC_TR_REF_PRINT (                                                               \
          ("ND_DEC_RC_FREE( %s, %d, %s)", NT_STR (var_NT), rc, #freefun))                \
        SAC_IF_DEBUG_RC (printf (TO_STR (var_NT) " = %p\n", SAC_ND_A_DESC (var_NT)););   \
        SAC_RC_PRINT (var_NT);                                                           \
        SAC_ASSERT_RC (DESC_RC (SAC_ND_A_DESC (var_NT)), SAC_ND_A_DESC (var_NT));        \
        if (__sync_sub_and_fetch (&DESC_RC (SAC_ND_A_DESC (var_NT)), rc) == 0) {         \
            free (SAC_ND_GETVAR (var_NT, SAC_ND_A_FIELD (var_NT)));                      \
            free (SAC_ND_A_DESC (var_NT));                                               \
        }                                                                                \
    }

/** =========================================================================== */

/**
 * SAC_RCM_async method.
 * Presumes the descriptor is shared.
 * Performs all RC updates using special atomic operations.
 */
#if SAC_RC_METHOD == SAC_RCM_async
#define SAC_ND_INIT__RC__DEFAULT(var_NT, rc) SAC_ND_INIT__RC__ASYNC (var_NT, rc)
#define SAC_ND_DEC_RC_FREE__DEFAULT(var_NT, rc, freefun)                                 \
    SAC_ND_DEC_RC_FREE__ASYNC (var_NT, rc, freefun)
#define SAC_ND_SET__RC__DEFAULT(var_NT, rc) SAC_ND_SET__RC__ASYNC (var_NT, rc)
#define SAC_ND_DEC_RC__DEFAULT(var_NT, rc) SAC_ND_DEC_RC__ASYNC (var_NT, rc)
#define SAC_ND_INC_RC__DEFAULT(var_NT, rc) SAC_ND_INC_RC__ASYNC (var_NT, rc)
#define SAC_ND_A_RC__DEFAULT(var_NT) SAC_ND_A_RC__ASYNC (var_NT)
#endif

/** =========================================================================== */

/**
 * SAC_RCM_local_pasync_norc_desc method
 * The descriptor can be in one of the three modes:
 *  LOCAL = perform updates locally (descriptor not shared).
 *  NORC = do not update the ref. count.
 *
 * There may be an asynchronous parent descriptor. The reference counter
 * in the parent descriptor counts the number of child descriptors, and it
 * must by updated atomically.
 * When the local descriptor is the only child of the parent, we try to get
 * rid of the parent (in SAC_ND_A_PARENT_ASYNC_RC).
 */

#if SAC_RC_METHOD == SAC_RCM_local_pasync_norc_desc

#define SAC_ND_INIT__RC__DEFAULT(var_NT, rc)                                             \
    {                                                                                    \
        SAC_ND_INIT__RC__C99 (var_NT, rc)                                                \
        DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) = SAC_DESC_RC_MODE_LOCAL;                  \
        DESC_PARENT (SAC_ND_A_DESC (var_NT)) = 0;                                        \
    }

#define SAC_ND_SET__RC__DEFAULT(var_NT, rc)                                              \
    {                                                                                    \
        SAC_RC_PRINT (var_NT);                                                           \
        if (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_LOCAL) {           \
            SAC_ND_SET__RC__C99 (var_NT, rc);                                            \
        } else if (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_NORC) {     \
            SAC_ND_SET__RC__NORC (var_NT, rc);                                           \
        } else {                                                                         \
            SAC_ND_SET__RC__C99 (var_NT, rc);                                            \
        }                                                                                \
    }

#define SAC_ND_SET__RC__ASYNC_RC(var_NT, rc)                                             \
    {                                                                                    \
        DESC_RC (SAC_ND_A_DESC (var_NT)) = rc;                                           \
        SAC_TR_REF_PRINT_RC (var_NT)                                                     \
    }

/*
 * SAC_ND_INC_RC implementations (referenced by sac_std_gen.h)
 */
#define SAC_ND_INC_RC__DEFAULT(var_NT, rc)                                               \
    {                                                                                    \
        SAC_RC_PRINT (var_NT);                                                           \
        if (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_LOCAL) {           \
            SAC_ND_INC_RC__C99 (var_NT, rc);                                             \
        } else if (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_NORC) {     \
            SAC_ND_INC_RC__NORC (var_NT, rc);                                            \
        } else {                                                                         \
            SAC_ND_INC_RC__C99 (var_NT, rc);                                             \
        }                                                                                \
    }

#define SAC_ND_DEC_RC__DEFAULT(var_NT, rc)                                               \
    {                                                                                    \
        SAC_RC_PRINT (var_NT);                                                           \
        if (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_LOCAL) {           \
            SAC_ND_DEC_RC__C99 (var_NT, rc);                                             \
        } else if (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_NORC) {     \
            SAC_ND_DEC_RC__NORC (var_NT, rc);                                            \
        } else {                                                                         \
            SAC_ND_DEC_RC__C99 (var_NT, rc);                                             \
        }                                                                                \
    }

/*
 * SAC_ND_DEC_RC_FREE implementations (referenced by sac_std_gen.h)
 */
#define SAC_ND_DEC_RC_FREE__DEFAULT(var_NT, rc, freefun)                                 \
    {                                                                                    \
        SAC_RC_PRINT (var_NT);                                                           \
        if (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_LOCAL) {           \
            SAC_ND_DEC_RC_FREE__C99 (var_NT, rc, freefun);                               \
        } else if (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_NORC) {     \
            SAC_ND_DEC_RC_FREE__NORC (var_NT, rc, freefun);                              \
        } else {                                                                         \
            SAC_ND_DEC_RC_FREE__ASYNC_RC (var_NT, rc, freefun);                          \
        }                                                                                \
    }

/* the decrement is perfomed on the local copy, hence it does not have to
 * be atomic. */
#define SAC_ND_DEC_RC_FREE__ASYNC_RC(var_NT, rc, freefun)                                \
    {                                                                                    \
        SAC_RC_PRINT (var_NT);                                                           \
        if ((SAC_ND_A_RC__C99 (var_NT) -= rc) == 0) {                                    \
            SAC_DEC_FREE_PARENT__ASYNC_RC (var_NT)                                       \
            SAC_TR_REF_PRINT_RC (var_NT)                                                 \
            SAC_ND_FREE (var_NT, freefun)                                                \
        } else {                                                                         \
            SAC_TR_REF_PRINT_RC (var_NT)                                                 \
        }                                                                                \
    }

/* the decrement is perfomed on the parent asynchronous descriptor,
 * hence it must be done with an atomic operation */
#define SAC_DEC_FREE_PARENT__ASYNC_RC(var_NT)                                            \
    {                                                                                    \
        SAC_ND_DESC_PARENT_TYPE parent = SAC_ND_A_DESC_PARENT (var_NT);                  \
        if (__sync_sub_and_fetch (&PARDESC_NCHILD (parent), 1) == 0) {                   \
            free (parent);                                                               \
        }                                                                                \
    }

#define SAC_ND_A_RC__DEFAULT(var_NT)                                                     \
    ({                                                                                   \
        int rc;                                                                          \
        SAC_RC_PRINT (var_NT);                                                           \
        if (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_LOCAL) {           \
            rc = SAC_ND_A_RC__C99 (var_NT);                                              \
        } else if (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_NORC) {     \
            rc = SAC_ND_A_RC__NORC (var_NT);                                             \
        } else {                                                                         \
            rc = SAC_ND_A_RC__ASYNC_RC (var_NT);                                         \
        }                                                                                \
        (int)rc;                                                                         \
    })

#define SAC_ND_A_RC__ASYNC_RC(var_NT)                                                    \
    (SAC_ND_A_RC__C99 (var_NT) <= 1 ? SAC_ND_A_PARENT_ASYNC_RC (var_NT)                  \
                                    : SAC_ND_A_RC__C99 (var_NT))

/*
 * While getting the parent count if it is 1 then change mode back to local.
 */
#define SAC_ND_A_PARENT_ASYNC_RC(var_NT)                                                 \
    ({                                                                                   \
        SAC_TR_REF_PRINT (("ND_A_RC__ASYNC_RC( %s)", NT_STR (var_NT)))                   \
        SAC_IF_DEBUG_RC (printf (TO_STR (var_NT) " = %p\n", SAC_ND_A_DESC (var_NT)););   \
                                                                                         \
        SAC_ND_DESC_PARENT_TYPE parent = SAC_ND_A_DESC_PARENT (var_NT);                  \
        if (PARDESC_NCHILD (parent) == 1) {                                              \
            DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) = SAC_DESC_RC_MODE_LOCAL;              \
        }                                                                                \
        PARDESC_NCHILD (parent);                                                         \
    })

/* Increment the number of children in the parent descriptor.
 * Must use the atomic operation. */
#define SAC_RC_PARENT_INC_SYNC(var_NT)                                                   \
    {                                                                                    \
        SAC_IF_DEBUG_RC (printf (TO_STR (var_NT) " = %p\n", SAC_ND_A_DESC (var_NT)););   \
        SAC_TR_REF_PRINT (("RC_PARENT_INC( %s)", NT_STR (var_NT)))                       \
        __sync_add_and_fetch (&PARDESC_NCHILD (                                          \
                                (SAC_ND_DESC_PARENT_TYPE)SAC_ND_A_DESC_PARENT (var_NT)), \
                              1);                                                        \
        SAC_TR_REF_PRINT_RC (var_NT)                                                     \
    }

#if 0
/*
 * SAC_MUTC_RC_BARRIER implementation (referenced from mutc_rc_gen.h)
 */
#define SAC_RC_BARRIER__DESC(var_NT)                                                     \
    {                                                                                    \
        SAC_IF_DEBUG_RC (printf (TO_STR (var_NT) " = %p\n", SAC_ND_A_DESC (var_NT)););   \
        SAC_TR_REF_PRINT (("RC_BARRIER( %s)", NT_STR (var_NT)))                          \
        __sync_synchronize ();                                                           \
    }
#endif

/*
 * Access the descriptors rc directly in SAC_ND_PRF_RESTORERC and
 * SAC_ND_PRF_2NORC so that we do not perform any special reference
 * counting access.
 */

#define SAC_ND_PRF_RESTORERC__DO(array, rc)                                              \
    SAC_RC_PRINT (array);                                                                \
    SAC_ND_A_DESC_RC_MODE (array) = SAC_ND_A_RC_T_MODE (rc);

#define SAC_ND_PRF_RESTORERC__NOOP(array, rc)

#define SAC_ND_PRF_2NORC__DO(rc, array)                                                  \
    SAC_RC_PRINT (array);                                                                \
    SAC_ND_A_RC_T_MODE (rc) = SAC_ND_A_DESC_RC_MODE (array);                             \
    SAC_ND_A_DESC_RC_MODE (array) = SAC_DESC_RC_MODE_NORC;

/*
 * Need to do the inc in sync not detached as the queue must be
 * flushed before the spawn else order maybe lost.
 *
 * If rc is 1 the ownership of the memory can be passed to the new
 * reference.  The new reference can reuse the parent count and
 * descriptor.  Full reuse analysis is not needed as we are only
 * reusing this aliases
 */

#define SAC_ND_PRF_2ASYNC__DO(new, array)                                                \
    if (DESC_RC_MODE (SAC_ND_A_DESC (array)) != SAC_DESC_RC_MODE_NORC) {                 \
        if (DESC_RC (SAC_ND_A_DESC (array)) == 1) {                                      \
            SAC_ND_A_FIELD (new) = SAC_ND_GETVAR (array, SAC_ND_A_FIELD (array));        \
            SAC_ND_A_DESC (new) = SAC_ND_A_DESC (array);                                 \
        } else {                                                                         \
            SAC_ND_A_FIELD (new) = SAC_ND_GETVAR (array, SAC_ND_A_FIELD (array));        \
            SAC_ND_A_DESC_RC_MODE (array) = SAC_DESC_RC_MODE_ASYNC;                      \
            SAC_RC_PRINT (array);                                                        \
            if (SAC_ND_A_DESC_PARENT (array) == NULL) {                                  \
                SAC_ND_ALLOC__DESC__PARENT (array, SAC_ND_A_DIM (array));                \
                SAC_DEBUG_RC (printf ("alloced parent at %p in %p\n",                    \
                                      (void *)SAC_ND_A_DESC_PARENT (array),              \
                                      SAC_ND_A_DESC (array));)                           \
                PARDESC_NCHILD (SAC_ND_A_DESC_PARENT (array)) = 2;                       \
            } else {                                                                     \
                SAC_RC_PARENT_INC_SYNC (array);                                          \
            }                                                                            \
            SAC_ND_A_COPY_DESC (new, array);                                             \
            DESC_RC (SAC_ND_A_DESC (new)) = 0;                                           \
            SAC_IF_DEBUG_RC (printf ("copy from %p to %p\n",                             \
                                     SAC_ND_GETVAR (array, SAC_ND_A_DESC (array)),       \
                                     SAC_ND_A_DESC (new)););                             \
            SAC_RC_PRINT (array);                                                        \
            SAC_RC_PRINT (new);                                                          \
        }                                                                                \
    }
#endif

/** =========================================================================== */

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

/* Initialize. The mode is LOCAL. */
#define SAC_ND_INIT__RC__DEFAULT(var_NT, rc)                                             \
    {                                                                                    \
        SAC_ND_INIT__RC__C99 (var_NT, rc)                                                \
        DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) = SAC_DESC_RC_MODE_LOCAL;                  \
        SAC_DESC_SET_HIDDEN_DATA (SAC_ND_A_DESC (var_NT), SAC_DESC_RC_MODE_OTHER);       \
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

#if 0
#define SAC_ND_PRF_RESTORERC__DO(array, rc)                                              \
    if (((intptr_t)SAC_ND_A_FIELD (rc)) == 1) {                                          \
        SAC_DESC_SET_HIDDEN_DATA (SAC_ND_A_DESC (array), SAC_DESC_RC_MODE_OTHER);        \
    }
#endif

#if 0
#define SAC_ND_PRF_2NORC__DO(rc, array)                                                  \
    if (SAC_DESC_HIDDEN_DATA (SAC_ND_A_DESC (array)) != SAC_DESC_RC_MODE_NORC) {         \
        SAC_DESC_SET_HIDDEN_DATA (SAC_ND_A_DESC (array), SAC_DESC_RC_MODE_NORC);         \
        SAC_ND_A_FIELD (rc) = (SAC_referencecount_t)1;                                   \
    } else {                                                                             \
        SAC_ND_A_FIELD (rc) = (SAC_referencecount_t)0;                                   \
    }
#endif

#if 0
#define SAC_ND_PRF_2ASYNC__DO(new, array)                                                \
    if (SAC_DESC_HIDDEN_DATA (SAC_ND_A_DESC (array)) != SAC_DESC_RC_MODE_NORC) {         \
        DESC_RC_MODE (SAC_ND_A_DESC (array)) = SAC_DESC_RC_MODE_ASYNC;                   \
    }
#endif

#endif

#endif
