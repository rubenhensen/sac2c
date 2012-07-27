#ifdef SAC_BACKEND_C99

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

#define SAC_DESC_INIT_RC(desc, rc) SAC_DESC_INIT__RC__C99 (desc, rc)

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
/* An attempt is made to try to switch back from the ASYNC into the LOCAL mode. */
#define SAC_ND_INC_RC__DEFAULT(var_NT, rc)                                               \
    {                                                                                    \
        SAC_RC_PRINT (var_NT);                                                           \
        if (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_LOCAL) {           \
            SAC_ND_INC_RC__C99 (var_NT, rc);                                             \
        } else if (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_NORC) {     \
            SAC_ND_INC_RC__NORC (var_NT, rc);                                            \
        } else {                                                                         \
            SAC_ND_TRY_LOCALIZE_RC__ASYNC (var_NT);                                      \
            SAC_ND_INC_RC__C99 (var_NT, rc);                                             \
        }                                                                                \
    }

#define SAC_DESC_INC_RC(desc, rc)                                                        \
    {                                                                                    \
        if (DESC_RC_MODE (desc) == SAC_DESC_RC_MODE_LOCAL) {                             \
            SAC_DESC_INC_RC__C99 (desc, rc);                                             \
        } else if (DESC_RC_MODE (desc) == SAC_DESC_RC_MODE_NORC) {                       \
            SAC_DESC_INC_RC__NORC (desc, rc);                                            \
        } else {                                                                         \
            SAC_DESC_INC_RC__C99 (desc, rc);                                             \
        }                                                                                \
    }

#define SAC_ND_DEC_RC__DEFAULT(var_NT, rc)                                               \
    {                                                                                    \
        SAC_RC_PRINT (var_NT);                                                           \
        SAC_ASSERT_RC (DESC_RC (SAC_ND_A_DESC (var_NT)) > 1,                             \
                       "SAC_ND_DEC_RC called but RC is <= 1");                           \
        if (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_LOCAL) {           \
            SAC_ND_DEC_RC__C99 (var_NT, rc);                                             \
        } else if (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_NORC) {     \
            SAC_ND_DEC_RC__NORC (var_NT, rc);                                            \
        } else {                                                                         \
            SAC_ND_DEC_RC__C99 (var_NT, rc);                                             \
        }                                                                                \
    }

#define SAC_DESC_DEC_RC(desc, rc)                                                        \
    {                                                                                    \
        SAC_ASSERT_RC (DESC_RC (desc) > 1, "SAC_DESC_DEC_RC called but RC is <= 1");     \
        if (DESC_RC_MODE (desc) == SAC_DESC_RC_MODE_LOCAL) {                             \
            SAC_DESC_DEC_RC__C99 (desc, rc);                                             \
        } else if (DESC_RC_MODE (desc) == SAC_DESC_RC_MODE_NORC) {                       \
            SAC_DESC_DEC_RC__NORC (desc, rc);                                            \
        } else {                                                                         \
            SAC_DESC_DEC_RC__C99 (desc, rc);                                             \
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

#define SAC_DESC_DEC_RC_FREE(desc, rc, q_waslast)                                        \
    {                                                                                    \
        if (DESC_RC_MODE (desc) == SAC_DESC_RC_MODE_LOCAL) {                             \
            SAC_DESC_DEC_RC_FREE__C99 (desc, rc, q_waslast);                             \
        } else if (DESC_RC_MODE (desc) == SAC_DESC_RC_MODE_NORC) {                       \
            SAC_DESC_DEC_RC_FREE__NORC (desc, rc, q_waslast);                            \
        } else {                                                                         \
            SAC_DESC_DEC_RC_FREE__ASYNC_RC (desc, rc, q_waslast);                        \
        }                                                                                \
    }

/* The decrement is perfomed on the local copy, hence it does not have to be atomic.
 * An attempt is made to try to switch back from the ASYNC into the LOCAL mode.
 */
#define SAC_ND_DEC_RC_FREE__ASYNC_RC(var_NT, rc, freefun)                                \
    {                                                                                    \
        SAC_RC_PRINT (var_NT);                                                           \
        if ((SAC_ND_A_RC__C99 (var_NT) -= rc) == 0) {                                    \
            SAC_TR_REF_PRINT_RC (var_NT)                                                 \
            SAC_DEC_FREE_PARENT__ASYNC_RC (var_NT, freefun)                              \
        } else {                                                                         \
            SAC_TR_REF_PRINT_RC (var_NT)                                                 \
        }                                                                                \
    }

#define SAC_DESC_DEC_RC_FREE__ASYNC_RC(desc, rc, q_waslast)                              \
    {                                                                                    \
        if ((DESC_RC (desc) -= rc) == 0) {                                               \
            SAC_DESC_DEC_FREE_PARENT__ASYNC_RC (desc, q_waslast);                        \
        } else {                                                                         \
            q_waslast = 0;                                                               \
        }                                                                                \
    }

/* the decrement is perfomed on the parent asynchronous descriptor,
 * hence it must be done with an atomic operation.
 * This is called when the child is going away.
 * Free the parent and data when it was the last child. */
#define SAC_DEC_FREE_PARENT__ASYNC_RC(var_NT, freefun)                                   \
    {                                                                                    \
        SAC_ND_DESC_PARENT_TYPE parent = SAC_ND_A_DESC_PARENT (var_NT);                  \
        if (__sync_sub_and_fetch (&PARENT_DESC_NCHILD (parent), 1) == 0) {               \
            SAC_ND_FREE (var_NT, freefun)                                                \
            free (parent);                                                               \
        }                                                                                \
    }

#define SAC_DESC_DEC_FREE_PARENT__ASYNC_RC(desc, q_waslast)                              \
    {                                                                                    \
        SAC_ND_DESC_PARENT_TYPE parent = (SAC_ND_DESC_PARENT_TYPE)DESC_PARENT (desc);    \
        if (__sync_sub_and_fetch (&PARENT_DESC_NCHILD (parent), 1) == 0) {               \
            free (parent);                                                               \
            SAC_FREE (desc);                                                             \
            q_waslast = 1;                                                               \
        } else {                                                                         \
            q_waslast = 0;                                                               \
        }                                                                                \
    }

#define SAC_ND_A_RC__DEFAULT(var_NT)                                                     \
    (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_LOCAL                     \
       ? SAC_ND_A_RC__C99 (var_NT)                                                       \
       : (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_NORC                 \
            ? SAC_ND_A_RC__NORC (var_NT)                                                 \
            : SAC_ND_A_RC__ASYNC_RC (var_NT)))

#define SAC_DESC_A_RC(desc)                                                              \
    (DESC_RC_MODE (desc) == SAC_DESC_RC_MODE_LOCAL                                       \
       ? SAC_DESC_A_RC__C99 (desc)                                                       \
       : (DESC_RC_MODE (desc) == SAC_DESC_RC_MODE_NORC                                   \
            ? SAC_DESC_A_RC__NORC (desc)                                                 \
            : SAC_DESC_A_RC__ASYNC_RC (desc)))

#define SAC_ND_A_RC__ASYNC_RC(var_NT)                                                    \
    (SAC_ND_A_RC__C99 (var_NT) <= 1 ? SAC_ND_A_PARENT_ASYNC_RC (var_NT)                  \
                                    : SAC_ND_A_RC__C99 (var_NT))

#define SAC_DESC_A_RC__ASYNC_RC(desc)                                                    \
    (SAC_DESC_A_RC__C99 (desc) <= 1 ? SAC_DESC_A_PARENT_ASYNC_RC (desc)                  \
                                    : SAC_DESC_A_RC__C99 (desc))

#define SAC_ND_A_PARENT_ASYNC_RC(var_NT)                                                 \
    PARENT_DESC_NCHILD (SAC_ND_A_DESC_PARENT (var_NT))

#define SAC_DESC_A_PARENT_ASYNC_RC(desc) PARENT_DESC_NCHILD (DESC_PARENT (desc))

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

#define SAC_DESC_RC_PARENT_INC_SYNC(desc)                                                \
    {                                                                                    \
        __sync_add_and_fetch (&PARENT_DESC_NCHILD (                                      \
                                (SAC_ND_DESC_PARENT_TYPE)DESC_PARENT (desc)),            \
                              1);                                                        \
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
#if 0
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
                SAC_IF_DEBUG_RC (printf ("alloced parent at %p in %p\n",                 \
                                         (void *)SAC_ND_A_DESC_PARENT (array),           \
                                         SAC_ND_A_DESC (array));)                        \
                PARENT_DESC_NCHILD (SAC_ND_A_DESC_PARENT (array)) = 2;                   \
            } else {                                                                     \
                SAC_RC_PARENT_INC_SYNC (array);                                          \
            }                                                                            \
            SAC_ND_A_COPY_DESC (new, array);                                             \
            DESC_RC (SAC_ND_A_DESC (new)) = 1;                                           \
            SAC_ND_DEC_RC__DEFAULT (array, 1);                                           \
            SAC_IF_DEBUG_RC (printf ("copy from %p to %p\n",                             \
                                     SAC_ND_GETVAR (array, SAC_ND_A_DESC (array)),       \
                                     SAC_ND_A_DESC (new)););                             \
            SAC_RC_PRINT (array);                                                        \
            SAC_RC_PRINT (new);                                                          \
        }                                                                                \
    }
#endif

#define SAC_DESC_RC_GIVE_ASYNC(newd, arrayd)                                             \
    {                                                                                    \
        SAC_ASSERT_RC (DESC_RC_MODE (arrayd) != SAC_DESC_RC_MODE_NORC,                   \
                       "SAC_RCM_local_pasync_norc_desc::SAC_DESC_RC_GIVE_ASYNC: called " \
                       "on NORC descriptor!");                                           \
        if (DESC_RC (arrayd) == 1) {                                                     \
            newd = arrayd;                                                               \
        } else {                                                                         \
            if (DESC_RC_MODE (arrayd) == SAC_DESC_RC_MODE_LOCAL) {                       \
                DESC_RC_MODE (arrayd) = SAC_DESC_RC_MODE_ASYNC;                          \
            }                                                                            \
            if (DESC_PARENT (arrayd) == 0) {                                             \
                SAC_DESC_ALLOC_PARENT (arrayd, DESC_DIM (arrayd));                       \
                SAC_IF_DEBUG_RC (printf ("alloced parent at %p in %p\n",                 \
                                         (void *)DESC_PARENT (arrayd), arrayd);)         \
                PARENT_DESC_NCHILD (DESC_PARENT (arrayd)) = 2;                           \
            } else {                                                                     \
                SAC_DESC_RC_PARENT_INC_SYNC (arrayd);                                    \
            }                                                                            \
            SAC_DESC_COPY_DESC (newd, arrayd);                                           \
            DESC_RC (newd) = 1;                                                          \
            SAC_DESC_DEC_RC (arrayd, 1);                                                 \
            SAC_IF_DEBUG_RC (printf ("copy from %p to %p\n", arrayd, newd););            \
        }                                                                                \
    }

#endif /* SAC_RC_METHOD == SAC_RCM_local_pasync_norc_desc */

#endif /* defined SAC_BACKEND_C99 */
