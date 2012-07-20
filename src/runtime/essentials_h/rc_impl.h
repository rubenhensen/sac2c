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
#else /* SAC_DEBUG_RC */
#define SAC_RC_PRINT(var_NT)
#define SAC_IF_DEBUG_RC(a)
#endif /* SAC_DEBUG_RC */

#define SAC_ASSERT_RC(cond, ...)                                                         \
    if (!(cond)) {                                                                       \
        SAC_RuntimeError (__VA_ARGS__);                                                  \
    }

/** =========================================================================== */

/**
 * Functions for the SAC_RCM_async method.
 * They are also used for the other combined rc methods, hence they are always defined.
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
        __sync_sub_and_fetch (&DESC_RC (SAC_ND_A_DESC (var_NT)), rc);                    \
        SAC_TR_REF_PRINT_RC (var_NT);                                                    \
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
        SAC_ND_TRY_LOCALIZE_RC__ASYNC (var_NT);                                          \
        __sync_add_and_fetch (&DESC_RC (SAC_ND_A_DESC (var_NT)), rc);                    \
        SAC_TR_REF_PRINT_RC (var_NT)                                                     \
    }

/* Return the reference count. */
#define SAC_ND_A_RC__ASYNC(var_NT) DESC_RC (SAC_ND_A_DESC (var_NT))

#if 0
#define SAC_ND_A_RC__ASYNC(var_NT)                                                       \
    ({                                                                                   \
        SAC_TR_REF_PRINT (("ND_A_RC( %s)", NT_STR (var_NT)))                             \
        SAC_IF_DEBUG_RC (printf (TO_STR (var_NT) " = %p\n", SAC_ND_A_DESC (var_NT)););   \
        SAC_RC_PRINT (var_NT);                                                           \
        DESC_RC (SAC_ND_A_DESC (var_NT));                                                \
    })
#endif

/* Atomically decrement the reference counter, and free the object if
 * it has reached zero. */
// TODO: freefun ??
#define SAC_ND_DEC_RC_FREE__ASYNC(var_NT, rc, freefun)                                   \
    {                                                                                    \
        SAC_TR_REF_PRINT (                                                               \
          ("ND_DEC_RC_FREE( %s, %d, %s)", NT_STR (var_NT), rc, #freefun))                \
        SAC_IF_DEBUG_RC (printf (TO_STR (var_NT) " = %p\n", SAC_ND_A_DESC (var_NT)););   \
        SAC_RC_PRINT (var_NT);                                                           \
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

#define SAC_ND_TRY_LOCALIZE_RC__ASYNC(var_NT) /* noop */

/* This method does not support the NORC mode.
 * (It could be emulated by switching to the ASYNC mode though) */
#define SAC_HAS_RC_NORC 0

#define SAC_ND_RC_TO_NORC__NODESC(var_NT)   /* stub */
#define SAC_ND_RC_TO_NORC__DESC(var_NT)     /* stub */
#define SAC_ND_RC_FROM_NORC__NODESC(var_NT) /* stub */
#define SAC_ND_RC_FROM_NORC__DESC(var_NT)   /* stub */

#endif /* SAC_RC_METHOD == SAC_RCM_async */

/** =========================================================================== */

/* The method SAC_RCM_local_async_norc_ptr is implemented in rcm_local_async_norc_ptr.h */
/* The method SAC_RCM_local_pasync is implemented in rcm_local_pasync.h */
/* The method SAC_RCM_local_pasync_norc_desc is implemented in
 * rcm_local_pasync_norc_desc.h */
/* I like to keep things separate ;-) */

#endif /* defined SAC_BACKEND_C99 */
