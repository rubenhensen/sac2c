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

#endif

/** =========================================================================== */

/**
 * SAC_RCM_local_pasync_norc_desc method
 * The descriptor can be in one of the three modes:
 *  LOCAL = perform updates locally (descriptor not shared).
 *  NORC = do not update the ref. count. (descriptor shared in SPMD)
 *  ASYNC = Hierarchical mode, multiple descriptors with the common parent.
 *          The reference counter in the parent descriptor counts the number of child
 * descriptors, and it must by updated atomically. When the local descriptor is the only
 * child of the parent, we try to get rid of the parent (in
 * SAC_ND_TRY_LOCALIZE_RC__ASYNC).
 */

#if SAC_RC_METHOD == SAC_RCM_local_pasync_norc_desc

#define SAC_ND_INIT__RC__DEFAULT(var_NT, rc) SAC_ND_INIT__RC__C99 (var_NT, rc)

/* Check if the child count in the parent is one and that case switch
 * back to the LOCAL mode. Otherwise do nothing.
 * We deallocate and clean the pointer to the parent because the parent's
 * presence is also indicative for the NORC->LOCAL/ASYNC switching.
 */
#define SAC_ND_TRY_LOCALIZE_RC__ASYNC(var_NT)                                            \
    {                                                                                    \
        SAC_TR_REF_PRINT (("SAC_ND_TRY_LOCALIZE_RC__ASYNC( %s)", NT_STR (var_NT)))       \
        SAC_IF_DEBUG_RC (printf (TO_STR (var_NT) " = %p\n", SAC_ND_A_DESC (var_NT)););   \
                                                                                         \
        SAC_ND_DESC_PARENT_TYPE parent = SAC_ND_A_DESC_PARENT (var_NT);                  \
        if (PARENT_DESC_NCHILD (parent) == 1) {                                          \
            DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) = SAC_DESC_RC_MODE_LOCAL;              \
            free (parent);                                                               \
            DESC_PARENT (SAC_ND_A_DESC (var_NT)) = 0;                                    \
        }                                                                                \
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

/* An attempt is made to try to switch back from the ASYNC into the LOCAL mode. */
#define SAC_ND_DEC_RC__DEFAULT(var_NT, rc)                                               \
    {                                                                                    \
        SAC_RC_PRINT (var_NT);                                                           \
        if (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_LOCAL) {           \
            SAC_ND_DEC_RC__C99 (var_NT, rc);                                             \
        } else if (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_NORC) {     \
            SAC_ND_DEC_RC__NORC (var_NT, rc);                                            \
        } else {                                                                         \
            SAC_ND_DEC_RC__C99 (var_NT, rc);                                             \
            SAC_ND_TRY_LOCALIZE_RC__ASYNC (var_NT);                                      \
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

/* The decrement is perfomed on the local copy, hence it does not have to be atomic.
 * An attempt is made to try to switch back from the ASYNC into the LOCAL mode.
 */
#define SAC_ND_DEC_RC_FREE__ASYNC_RC(var_NT, rc, freefun)                                \
    {                                                                                    \
        SAC_RC_PRINT (var_NT);                                                           \
        if ((SAC_ND_A_RC__C99 (var_NT) -= rc) == 0) {                                    \
            SAC_DEC_FREE_PARENT__ASYNC_RC (var_NT)                                       \
            SAC_TR_REF_PRINT_RC (var_NT)                                                 \
            SAC_ND_FREE (var_NT, freefun)                                                \
        } else {                                                                         \
            SAC_ND_TRY_LOCALIZE_RC__ASYNC (var_NT);                                      \
            SAC_TR_REF_PRINT_RC (var_NT)                                                 \
        }                                                                                \
    }

/* the decrement is perfomed on the parent asynchronous descriptor,
 * hence it must be done with an atomic operation.
 * This is called when the child is going away.
 * Free the parent when it was the last child. */
#define SAC_DEC_FREE_PARENT__ASYNC_RC(var_NT)                                            \
    {                                                                                    \
        SAC_ND_DESC_PARENT_TYPE parent = SAC_ND_A_DESC_PARENT (var_NT);                  \
        if (__sync_sub_and_fetch (&PARENT_DESC_NCHILD (parent), 1) == 0) {               \
            free (parent);                                                               \
        }                                                                                \
    }

#define SAC_ND_A_RC__DEFAULT(var_NT)                                                     \
    (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_LOCAL                     \
       ? SAC_ND_A_RC__C99 (var_NT)                                                       \
       : (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_NORC                 \
            ? SAC_ND_A_RC__NORC (var_NT)                                                 \
            : SAC_ND_A_RC__ASYNC_RC (var_NT)))

#if 0
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
#endif

#define SAC_ND_A_RC__ASYNC_RC(var_NT)                                                    \
    (SAC_ND_A_RC__C99 (var_NT) <= 1 ? SAC_ND_A_PARENT_ASYNC_RC (var_NT)                  \
                                    : SAC_ND_A_RC__C99 (var_NT))

#define SAC_ND_A_PARENT_ASYNC_RC(var_NT)                                                 \
    PARENT_DESC_NCHILD (SAC_ND_A_DESC_PARENT (var_NT))

#if 0
/*
 * While getting the parent count if it is 1 then change mode back to local.
 */
#define SAC_ND_A_PARENT_ASYNC_RC(var_NT)                                                 \
    ({                                                                                   \
        SAC_TR_REF_PRINT (("ND_A_RC__ASYNC_RC( %s)", NT_STR (var_NT)))                   \
        SAC_IF_DEBUG_RC (printf (TO_STR (var_NT) " = %p\n", SAC_ND_A_DESC (var_NT)););   \
                                                                                         \
        SAC_ND_DESC_PARENT_TYPE parent = SAC_ND_A_DESC_PARENT (var_NT);                  \
        int rc;                                                                          \
        if (PARENT_DESC_NCHILD (parent) == 1) {                                          \
            DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) = SAC_DESC_RC_MODE_LOCAL;              \
            free (parent);                                                               \
            DESC_PARENT (SAC_ND_A_DESC (var_NT)) = 0;                                    \
            rc = SAC_ND_A_RC__C99 (var_NT);                                              \
        } else {                                                                         \
            rc = PARENT_DESC_NCHILD (parent);                                            \
        }                                                                                \
        rc;                                                                              \
    })
#endif

/* Increment the number of children in the parent descriptor.
 * Must use the atomic operation. */
#define SAC_RC_PARENT_INC_SYNC(var_NT)                                                   \
    {                                                                                    \
        SAC_IF_DEBUG_RC (printf (TO_STR (var_NT) " = %p\n", SAC_ND_A_DESC (var_NT)););   \
        SAC_TR_REF_PRINT (("RC_PARENT_INC( %s)", NT_STR (var_NT)))                       \
        __sync_add_and_fetch (&PARENT_DESC_NCHILD (                                      \
                                (SAC_ND_DESC_PARENT_TYPE)SAC_ND_A_DESC_PARENT (var_NT)), \
                              1);                                                        \
        SAC_TR_REF_PRINT_RC (var_NT)                                                     \
    }

/* SAC_ND_RC_TO_NORC( var_NT) is generated in m4.
 * It switches the descriptor from the mode LOCAL or ASYNC into NORC.
 */

#define SAC_ND_RC_TO_NORC__NODESC(var_NT) /* noop */

#define SAC_ND_RC_TO_NORC__DESC(var_NT)                                                  \
    SAC_ASSERT_RC (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) != SAC_DESC_RC_MODE_NORC,       \
                   "SAC_ND_RC_TO_NORC__DESC: already in the NORC mode!");                \
    DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) = SAC_DESC_RC_MODE_NORC;

/* SAC_ND_RC_FROM_NORC( var_NT) is generated in m4.
 * It switches the descriptor from mode NORC back to the original LOCAL or ASYNC.
 * The original mode is recognized based on the presence of the PARENT descriptor.
 * Hence, LOCAL-mode descriptors must not keep the parent field non-null!
 */

#define SAC_ND_RC_FROM_NORC__NODESC(var_NT) /* noop */

#define SAC_ND_RC_FROM_NORC__DESC(var_NT)                                                \
    {                                                                                    \
        SAC_ASSERT_RC (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_NORC,   \
                       "SAC_ND_RC_FROM_NORC__DESC: not in the NORC mode!");              \
        if (SAC_ND_A_DESC_PARENT (var_NT) != 0) {                                        \
            DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) = SAC_DESC_RC_MODE_ASYNC;              \
        } else {                                                                         \
            DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) = SAC_DESC_RC_MODE_LOCAL;              \
        }                                                                                \
    }

/* This method supports the NORC mode. */
#define SAC_HAS_RC_NORC 1

/* SAC_ND_RC_GIVE_ASYNC is generated in m4.
 * It *consumes* a single reference from the input descriptor and returns
 * a (possibly new) descriptor that can be accessed asynchronously to
 * the original one.
 *
 * The input descriptor must NOT be in the NORC mode.
 * If the current reference count of the input descriptor is one, we do not
 * create a new descriptor but set the new to the old. This works irrespective
 * of the mode.
 * Otherwise we create a new child descriptor, possibly allocating the parent as well,
 * and return the new child, decrementing the ref.count. of the original descriptor.
 */

#define SAC_ND_RC_GIVE_ASYNC__NODESC(new, array)                                         \
    SAC_ND_A_FIELD (new) = SAC_ND_GETVAR (array, SAC_ND_A_FIELD (array));

#define SAC_ND_RC_GIVE_ASYNC__DESC(new, array)                                           \
    {                                                                                    \
        SAC_ASSERT_RC (DESC_RC_MODE (SAC_ND_A_DESC (array)) != SAC_DESC_RC_MODE_NORC,    \
                       "SAC_RCM_local_pasync_norc_desc::SAC_ND_RC_GIVE_ASYNC__DESC: "    \
                       "called on NORC descriptor!");                                    \
        SAC_ND_A_FIELD (new) = SAC_ND_GETVAR (array, SAC_ND_A_FIELD (array));            \
        if (DESC_RC (SAC_ND_A_DESC (array)) == 1) {                                      \
            SAC_ND_A_DESC (new) = SAC_ND_A_DESC (array);                                 \
        } else {                                                                         \
            SAC_RC_PRINT (array);                                                        \
            if (DESC_RC_MODE (SAC_ND_A_DESC (array)) == SAC_DESC_RC_MODE_LOCAL) {        \
                SAC_ND_A_DESC_RC_MODE (array) = SAC_DESC_RC_MODE_ASYNC;                  \
            }                                                                            \
            if (SAC_ND_A_DESC_PARENT (array) == NULL) {                                  \
                SAC_ND_ALLOC__DESC__PARENT (array, SAC_ND_A_DIM (array));                \
                SAC_DEBUG_RC (printf ("alloced parent at %p in %p\n",                    \
                                      (void *)SAC_ND_A_DESC_PARENT (array),              \
                                      SAC_ND_A_DESC (array));)                           \
                PARENT_DESC_NCHILD (SAC_ND_A_DESC_PARENT (array)) = 2;                   \
            } else {                                                                     \
                SAC_RC_PARENT_INC_SYNC (array);                                          \
            }                                                                            \
            SAC_ND_A_COPY_DESC (new, array);                                             \
            DESC_RC (SAC_ND_A_DESC (new)) = 0;                                           \
            SAC_ND_DEC_RC__DEFAULT (array, 1);                                           \
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
        SAC_DESC_SET_HIDDEN_DATA (SAC_ND_A_DESC (var_NT), SAC_DESC_RC_MODE_LOCAL);       \
    }

/* Check the reference count and if it is one,
 * switch back to the LOCAL mode. */
#define SAC_ND_TRY_LOCALIZE_RC__ASYNC(var_NT)                                            \
    if (SAC_ND_A_RC__ASYNC (var_NT) == 1) {                                              \
        DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) = SAC_DESC_RC_MODE_LOCAL;                  \
        SAC_DESC_SET_HIDDEN_DATA (SAC_ND_A_DESC (var_NT), SAC_DESC_RC_MODE_LOCAL);       \
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

#define SAC_ND_RC_TO_NORC__DESC(var_NT)                                                  \
    if (SAC_DESC_HIDDEN_DATA (SAC_ND_A_DESC (var_NT)) != SAC_DESC_RC_MODE_NORC) {        \
        SAC_DESC_SET_HIDDEN_DATA (SAC_ND_A_DESC (var_NT), SAC_DESC_RC_MODE_NORC);        \
    }

#define SAC_ND_RC_FROM_NORC__DESC(var_NT)                                                \
    SAC_DESC_SET_HIDDEN_DATA (SAC_ND_A_DESC (var_NT),                                    \
                              DESC_RC_MODE (SAC_ND_A_DESC (var_NT)));

/* This method supports the NORC mode. */
#define SAC_HAS_RC_NORC 1

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

#endif

/** =========================================================================== */

#ifndef SAC_HAS_RC_NORC
#define SAC_HAS_RC_NORC 0
#endif

#if !SAC_HAS_RC_NORC
/* Stub SAC_ND_RC_TO_NORC / SAC_ND_RC_FROM_NORC
 * The selected RC method cannot do the NORC mode to save us copying the descriptors
 * in SPMD functions.
 * Define the missing macros as empty, and copy descriptors in SPMD frame code. */

#define SAC_ND_RC_TO_NORC__NODESC(var_NT) /* not used */

#define SAC_ND_RC_TO_NORC__DESC(var_NT) /* not used */

#define SAC_ND_RC_FROM_NORC__NODESC(var_NT) /* not used */

#define SAC_ND_RC_FROM_NORC__DESC(var_NT) /* not used */

#endif

#endif
