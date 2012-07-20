#ifdef SAC_BACKEND_C99

/**
 * SAC_RCM_local_pasync method
 * The descriptor can be in one of the three modes:
 *  LOCAL = perform updates locally (descriptor not shared).
 *  ASYNC = Hierarchical mode, multiple descriptors with the common parent.
 *          The reference counter in the parent descriptor counts the number of child
 * descriptors, and it must by updated atomically. When the local descriptor is the only
 * child of the parent, we try to get rid of the parent (in
 * SAC_ND_TRY_LOCALIZE_RC__ASYNC). The common inc/dec updates are performed locally.
 *
 *  When running SPMD local on-stack copies are created in each thread.
 */

#if SAC_RC_METHOD == SAC_RCM_local_pasync

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

/* The local counter is set irrespective of the mode. */
#define SAC_ND_SET__RC__DEFAULT(var_NT, rc)                                              \
    {                                                                                    \
        SAC_RC_PRINT (var_NT);                                                           \
        SAC_ND_SET__RC__C99 (var_NT, rc);                                                \
    }

#define SAC_ND_SET__RC__ASYNC_RC(var_NT, rc)                                             \
    {                                                                                    \
        DESC_RC (SAC_ND_A_DESC (var_NT)) = rc;                                           \
        SAC_TR_REF_PRINT_RC (var_NT)                                                     \
    }

/*
 * SAC_ND_INC_RC implementations (referenced by sac_std_gen.h)
 * The local counter is always updated, without any locking.
 */
#define SAC_ND_INC_RC__DEFAULT(var_NT, rc)                                               \
    {                                                                                    \
        SAC_RC_PRINT (var_NT);                                                           \
        SAC_ND_INC_RC__C99 (var_NT, rc);                                                 \
    }

/* The local counter is always updated, without any locking. */
#define SAC_ND_DEC_RC__DEFAULT(var_NT, rc)                                               \
    {                                                                                    \
        SAC_RC_PRINT (var_NT);                                                           \
        SAC_ND_DEC_RC__C99 (var_NT, rc);                                                 \
    }

/*
 * SAC_ND_DEC_RC_FREE implementations (referenced by sac_std_gen.h).
 * The decrement is perfomed on the local copy, hence it does not have to be atomic.
 * An attempt is made to try to switch back from the ASYNC into the LOCAL mode.
 */
#define SAC_ND_DEC_RC_FREE__DEFAULT(var_NT, rc, freefun)                                 \
    {                                                                                    \
        SAC_RC_PRINT (var_NT);                                                           \
        if ((SAC_ND_A_RC__C99 (var_NT) -= rc) == 0) {                                    \
            SAC_TR_REF_PRINT_RC (var_NT)                                                 \
            if (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_ASYNC) {       \
                SAC_DEC_FREE_PARENT__ASYNC_RC (var_NT)                                   \
            }                                                                            \
            SAC_ND_FREE (var_NT, freefun)                                                \
        } else {                                                                         \
            if (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_ASYNC) {       \
                SAC_ND_TRY_LOCALIZE_RC__ASYNC (var_NT);                                  \
            }                                                                            \
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
       : SAC_ND_A_RC__ASYNC_RC (var_NT))

#define SAC_ND_A_RC__ASYNC_RC(var_NT)                                                    \
    (SAC_ND_A_RC__C99 (var_NT) <= 1 ? SAC_ND_A_PARENT_ASYNC_RC (var_NT)                  \
                                    : SAC_ND_A_RC__C99 (var_NT))

#define SAC_ND_A_PARENT_ASYNC_RC(var_NT)                                                 \
    PARENT_DESC_NCHILD (SAC_ND_A_DESC_PARENT (var_NT))

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

/* This method DOES NOT support the NORC mode. */
#define SAC_HAS_RC_NORC 0

/* stub the NORC macros */
#define SAC_ND_RC_TO_NORC__NODESC(var_NT)   /* stub */
#define SAC_ND_RC_TO_NORC__DESC(var_NT)     /* stub */
#define SAC_ND_RC_FROM_NORC__NODESC(var_NT) /* stub */
#define SAC_ND_RC_FROM_NORC__DESC(var_NT)   /* stub */

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

#endif /* of SAC_RC_METHOD == SAC_RCM_local_pasync */

#endif /* defined SAC_BACKEND_C99 */
