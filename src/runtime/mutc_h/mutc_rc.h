/* $Id$ */

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

#if !SAC_MUTC_DISABLE_CONCURRENT_RC

#define MUTC 1
#if SAC_BACKEND == MUTC

#include <limits.h>

#define SAC_SL_DETACH() sl_detach ()
#define SAC_MUTC_DEBUG_RC(a)

#if 0
#define SAC_MUTC_RC_PRINT(var_NT)                                                        \
    printf ("%s:%d " TO_STR (var_NT) " @ %p = [ %d, %p, %d]\n", __FILE__, __LINE__,      \
            SAC_ND_A_DESC (var_NT), (int)SAC_ND_A_DESC (var_NT)[0],                      \
            (void *)SAC_ND_A_DESC (var_NT)[1], (int)SAC_ND_A_DESC (var_NT)[2]);
#else
#define SAC_MUTC_RC_PRINT(var_NT)
#endif

/****************************************************
 * In module mode number of exclusive places will be
 * always 1 as we dont know how many exclusive places
 * this module will be running on.
 *
 * In program mode it will be as many as specified,
 * by default 1.
 *
 * **************************************************/
#if SAC_DO_COMPILE_MODULE == 1
#define SAC_MUTC_GET_RC_PLACE(DESC) SAC_mutc_rc_place
#else /* SAC_DO_COMPILE_MODULE */
#define SAC_MUTC_GET_RC_PLACE(DESC)                                                      \
    ({                                                                                   \
        long int address = (long int)DESC;                                               \
        address = address >> 6;                                                          \
        address = (SAC_MUTC_RC_PLACES - 1) & address;                                    \
        SAC_mutc_rc_place_many[address];                                                 \
    })
#endif /*SAC_DO_COMPILE_MODULE*/

SAC_IF_MUTC_RC_INDIRECT (
  sl_decl (SAC_set_rc_w, void, sl_glparm (int *, desc), sl_glparm (int, rc));
  sl_decl (SAC_inc_rc_w, void, sl_glparm (int *, desc), sl_glparm (int, rc));
  sl_decl (SAC_dec_rc_w, void, sl_glparm (int *, desc), sl_glparm (int, rc));
  sl_decl (SAC_dec_and_maybeFree_rc_w, void, sl_glparm (int *, desc),
           sl_glparm (int, val), sl_glparm (void *, data));
  sl_decl (SAC_get_rc_w, void, sl_glparm (int *, desc), sl_shparm (int, val));
  sl_decl (SAC_rc_barrier_w, void, sl_glparm (int *, desc));
  sl_decl (SAC_dec_and_maybeFree_parent_w, void,
           sl_glparm (SAC_ND_DESC_PARENT_TYPE, parent));
  sl_decl (SAC_get_parent_count_w, void, sl_glparm (SAC_ND_DESC_PARENT_TYPE, parent),
           sl_shparm (SAC_ND_DESC_PARENT_BASETYPE, val));
  sl_decl (SAC_inc_parent_count_w, void, sl_glparm (SAC_ND_DESC_PARENT_TYPE, parent));)

SAC_IF_NOT_MUTC_RC_INDIRECT (
  sl_decl (SAC_set_rc, void, sl_glparm (int *, desc), sl_glparm (int, rc));
  sl_decl (SAC_inc_rc, void, sl_glparm (int *, desc), sl_glparm (int, rc));
  sl_decl (SAC_dec_rc, void, sl_glparm (int *, desc), sl_glparm (int, rc));
  sl_decl (SAC_get_rc, void, sl_glparm (int *, desc), sl_shparm (int, val));
  sl_decl (SAC_dec_and_get_rc, void, sl_glparm (int *, desc), sl_shparm (int, val));
  sl_decl (SAC_dec_and_maybeFree_rc, void, sl_glparm (int *, desc), sl_glparm (int, val),
           sl_glparm (void *, data));
  sl_decl (SAC_dec_and_maybeFree_parent, void,
           sl_glparm (SAC_ND_DESC_PARENT_TYPE, parent));
  sl_decl (SAC_get_parent_count, void, sl_glparm (SAC_ND_DESC_PARENT_TYPE, parent),
           sl_shparm (SAC_ND_DESC_PARENT_BASETYPE, val));
  sl_decl (SAC_inc_parent_count, void, sl_glparm (SAC_ND_DESC_PARENT_TYPE, parent));)

/*
 * SAC_ND_SET__RC implementations (referenced by sac_std_gen.h)
 */
#undef SAC_ND_SET__RC__DEFAULT
#define SAC_ND_SET__RC__DEFAULT(var_NT, rc)                                              \
    {                                                                                    \
        SAC_MUTC_RC_PRINT (var_NT);                                                      \
        if (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_SEQ) {             \
            SAC_ND_SET__RC__C99 (var_NT, rc);                                            \
        } else if (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_NORC) {     \
            SAC_ND_SET__RC__NORC (var_NT, rc);                                           \
        } else {                                                                         \
            SAC_ND_SET__RC__C99 (var_NT, rc);                                            \
        }                                                                                \
    }

#define SAC_ND_SET__RC__NORC(var_NT, rc)                                                 \
    {                                                                                    \
        SAC_TR_REF_PRINT (("ND_SET__RC__NORC( %s, %d)", NT_STR (var_NT), rc))            \
        SAC_TR_REF_PRINT_RC (var_NT)                                                     \
    }

/* deprecated */
#define SAC_ND_SET__RC__ASYNC_RC(var_NT, rc)                                             \
    {                                                                                    \
        SAC_IF_MUTC_RC_INDIRECT (                                                        \
          sl_create (, SAC_mutc_rc_place_w, , , , , , SAC_set_rc_w,                      \
                     sl_glarg (int *, , SAC_ND_A_DESC (var_NT)), sl_glarg (int, , rc));) \
        SAC_IF_NOT_MUTC_RC_INDIRECT (                                                    \
          sl_create (, SAC_MUTC_GET_RC_PLACE (SAC_ND_A_DESC (var_NT)), 0, 1, 1, ,        \
                     sl__exclusive, SAC_set_rc,                                          \
                     sl_glarg (int *, , SAC_ND_A_DESC (var_NT)), sl_glarg (int, , rc));) \
        SAC_SL_DETACH ();                                                                \
        SAC_TR_REF_PRINT_RC (var_NT)                                                     \
    }

/*
 * SAC_ND_INC_RC implementations (referenced by sac_std_gen.h)
 */
#undef SAC_ND_INC_RC__DEFAULT
#define SAC_ND_INC_RC__DEFAULT(var_NT, rc)                                               \
    {                                                                                    \
        SAC_MUTC_RC_PRINT (var_NT);                                                      \
        if (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_SEQ) {             \
            SAC_ND_INC_RC__C99 (var_NT, rc);                                             \
        } else if (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_NORC) {     \
            SAC_ND_INC_RC__NORC (var_NT, rc);                                            \
        } else {                                                                         \
            SAC_ND_INC_RC__C99 (var_NT, rc);                                             \
        }                                                                                \
    }

#define SAC_ND_INC_RC__NORC(var_NT, rc)                                                  \
    {                                                                                    \
    }

/* depricated */
#define SAC_ND_INC_RC__ASYNC_RC(var_NT, rc)                                              \
    {                                                                                    \
        SAC_TR_REF_PRINT (("ND_INC_RC_ASYNC_RC( %s, %d)", NT_STR (var_NT), rc))          \
        SAC_IF_MUTC_RC_INDIRECT (                                                        \
          sl_create (, SAC_mutc_rc_place_w, , , , , , SAC_inc_rc_w,                      \
                     sl_glarg (int *, , SAC_ND_A_DESC (var_NT)), sl_glarg (int, , rc));) \
        SAC_IF_NOT_MUTC_RC_INDIRECT (                                                    \
          sl_create (, SAC_MUTC_GET_RC_PLACE (SAC_ND_A_DESC (var_NT)), 0, 1, 1, ,        \
                     sl__exclusive, SAC_inc_rc,                                          \
                     sl_glarg (int *, , SAC_ND_A_DESC (var_NT)), sl_glarg (int, , rc));) \
        SAC_SL_DETACH ();                                                                \
        SAC_TR_REF_PRINT_RC (var_NT)                                                     \
    }

/*
 * SAC_ND_DEC_RC implementations (referenced by sac_std_gen.h)
 */
#undef SAC_ND_DEC_RC__DEFAULT
#define SAC_ND_DEC_RC__DEFAULT(var_NT, rc)                                               \
    {                                                                                    \
        SAC_MUTC_RC_PRINT (var_NT);                                                      \
        if (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_SEQ) {             \
            SAC_ND_DEC_RC__C99 (var_NT, rc);                                             \
        } else if (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_NORC) {     \
            SAC_ND_DEC_RC__NORC (var_NT, rc);                                            \
        } else {                                                                         \
            SAC_ND_DEC_RC__C99 (var_NT, rc);                                             \
        }                                                                                \
    }

#define SAC_ND_DEC_RC__NORC(var_NT, rc)                                                  \
    {                                                                                    \
        SAC_MUTC_DEBUG_RC (printf (TO_STR (var_NT) " = %p\n", SAC_ND_A_DESC (var_NT));); \
        SAC_TR_REF_PRINT (("ND_DEC_RC__NORC( %s, %d)", NT_STR (var_NT), rc))             \
        SAC_TR_REF_PRINT_RC (var_NT)                                                     \
    }

/* deprecated */
#define SAC_ND_DEC_RC__ASYNC_RC(var_NT, rc)                                              \
    {                                                                                    \
        SAC_MUTC_DEBUG_RC (printf (TO_STR (var_NT) " = %p\n", SAC_ND_A_DESC (var_NT));); \
        SAC_TR_REF_PRINT (("ND_DEC_RC__ASYNC_RC( %s, %d)", NT_STR (var_NT), rc))         \
        SAC_IF_MUTC_RC_INDIRECT (                                                        \
          sl_create (, SAC_mutc_rc_place_w, , , , , , SAC_dec_rc_w,                      \
                     sl_glarg (int *, , SAC_ND_A_DESC (var_NT)), sl_glarg (int, , rc));) \
        SAC_IF_NOT_MUTC_RC_INDIRECT (                                                    \
          sl_create (, SAC_MUTC_GET_RC_PLACE (SAC_ND_A_DESC (var_NT)), 0, 1, 1, ,        \
                     sl__exclusive, SAC_dec_rc,                                          \
                     sl_glarg (int *, , SAC_ND_A_DESC (var_NT)), sl_glarg (int, , rc));) \
        SAC_SL_DETACH ();                                                                \
        SAC_TR_REF_PRINT_RC (var_NT)                                                     \
    }

/*
 * SAC_ND_DEC_RC_FREE implementations (referenced by sac_std_gen.h)
 */
#undef SAC_ND_DEC_RC_FREE__DEFAULT
#define SAC_ND_DEC_RC_FREE__DEFAULT(var_NT, rc, freefun)                                 \
    {                                                                                    \
        SAC_MUTC_RC_PRINT (var_NT);                                                      \
        if (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_SEQ) {             \
            SAC_ND_DEC_RC_FREE__C99 (var_NT, rc, freefun);                               \
        } else if (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_NORC) {     \
            SAC_ND_DEC_RC_FREE__NORC (var_NT, rc, freefun);                              \
        } else {                                                                         \
            SAC_ND_DEC_RC_FREE__ASYNC_RC (var_NT, rc, freefun);                          \
        }                                                                                \
    }

#define SAC_ND_DEC_RC_FREE__NORC(var_NT, rc, freefun)                                    \
    {                                                                                    \
        SAC_MUTC_DEBUG_RC (printf (TO_STR (var_NT) " = %p\n", SAC_ND_A_DESC (var_NT));); \
        SAC_TR_REF_PRINT (("ND_DEC_RC_FREE__NORC( %s, %d)", NT_STR (var_NT), rc))        \
        SAC_TR_REF_PRINT_RC (var_NT)                                                     \
    }

#define SAC_ND_DEC_RC_FREE__ASYNC_RC(var_NT, rc, freefun)                                \
    {                                                                                    \
        SAC_MUTC_RC_PRINT (var_NT);                                                      \
        if ((SAC_ND_A_RC__C99 (var_NT) -= rc) == 0) {                                    \
            SAC_MUTC_DEC_FREE_PARENT__ASYNC_RC (var_NT)                                  \
            SAC_TR_REF_PRINT_RC (var_NT)                                                 \
            SAC_ND_FREE (var_NT, freefun)                                                \
        } else {                                                                         \
            SAC_TR_REF_PRINT_RC (var_NT)                                                 \
        }                                                                                \
    }

#define SAC_MUTC_DEC_FREE_PARENT__ASYNC_RC(var_NT)                                       \
    {                                                                                    \
        SAC_IF_MUTC_RC_INDIRECT (                                                        \
          sl_create (, SAC_mutc_rc_place_w, , , , , , SAC_dec_and_maybeFree_parent_w,    \
                     sl_glarg (SAC_ND_DESC_PARENT_TYPE (var_NT), ,                       \
                               (SAC_ND_DESC_PARENT_TYPE)SAC_ND_A_DESC_PARENT (           \
                                 var_NT)));)                                             \
        SAC_IF_NOT_MUTC_RC_INDIRECT (                                                    \
          sl_create (, SAC_MUTC_GET_RC_PLACE (SAC_ND_A_DESC_PARENT (var_NT)), 0, 1, 1, , \
                     sl__exclusive, SAC_dec_and_maybeFree_parent,                        \
                     sl_glarg (SAC_ND_DESC_PARENT_TYPE, ,                                \
                               (SAC_ND_DESC_PARENT_TYPE)SAC_ND_A_DESC_PARENT (           \
                                 var_NT)));)                                             \
        SAC_SL_DETACH ();                                                                \
    }

#undef SAC_ND_A_RC__DEFAULT
#define SAC_ND_A_RC__DEFAULT(var_NT)                                                     \
    ({                                                                                   \
        int rc;                                                                          \
        SAC_MUTC_RC_PRINT (var_NT);                                                      \
        if (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_SEQ) {             \
            rc = SAC_ND_A_RC__C99 (var_NT);                                              \
        } else if (DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) == SAC_DESC_RC_MODE_NORC) {     \
            rc = SAC_ND_A_RC__NORC (var_NT);                                             \
        } else {                                                                         \
            rc = SAC_ND_A_RC__ASYNC_RC (var_NT);                                         \
        }                                                                                \
        (int)rc;                                                                         \
    })

#define SAC_ND_A_RC__NORC(var_NT)                                                        \
    ({                                                                                   \
        SAC_MUTC_DEBUG_RC (printf (TO_STR (var_NT) " = %p\n", SAC_ND_A_DESC (var_NT));); \
        SAC_TR_REF_PRINT (("ND_A_RC__NORC( %s, %d)", NT_STR (var_NT), rc))               \
        SAC_TR_REF_PRINT_RC (var_NT)                                                     \
        (int)INT_MAX;                                                                    \
    })

#define SAC_ND_A_RC__ASYNC_RC(var_NT)                                                    \
    ({                                                                                   \
        SAC_ND_A_RC__C99 (var_NT) <= 1 ? SAC_ND_A_PARENT_ASYNC_RC (var_NT)               \
                                       : SAC_ND_A_RC__C99 (var_NT);                      \
    })

/*
 * While getting the parent count if it is 1 then change mode back to
 * seq
 */
#define SAC_ND_A_PARENT_ASYNC_RC(var_NT)                                                 \
    ({                                                                                   \
        SAC_TR_REF_PRINT (("ND_A_RC__ASYNC_RC( %s)", NT_STR (var_NT)))                   \
        SAC_MUTC_DEBUG_RC (printf (TO_STR (var_NT) " = %p\n", SAC_ND_A_DESC (var_NT));); \
                                                                                         \
        SAC_IF_MUTC_RC_INDIRECT (                                                        \
          sl_create (, SAC_mutc_rc_place_w, , , , , , SAC_get_parent_count_w,            \
                     sl_glarg (SAC_ND_DESC_PARENT_TYPE, ,                                \
                               (SAC_ND_DESC_PARENT_TYPE)SAC_ND_A_DESC_PARENT (var_NT)),  \
                     sl_sharg (SAC_ND_DESC_PARENT_BASETYPE, val, 0));)                   \
        SAC_IF_NOT_MUTC_RC_INDIRECT (                                                    \
          sl_create (, SAC_MUTC_GET_RC_PLACE (SAC_ND_A_DESC_PARENT (var_NT)), 0, 1, 1, , \
                     sl__exclusive, SAC_get_parent_count,                                \
                     sl_glarg (SAC_ND_DESC_PARENT_TYPE, ,                                \
                               (SAC_ND_DESC_PARENT_TYPE)SAC_ND_A_DESC_PARENT (var_NT)),  \
                     sl_sharg (SAC_ND_DESC_PARENT_BASETYPE, val, 0));)                   \
        sl_sync ();                                                                      \
        if (sl_geta (val) == 1) {                                                        \
            DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) = SAC_DESC_RC_MODE_SEQ;                \
        }                                                                                \
        (int)sl_geta (val);                                                              \
    })

/*
 * SAC_MUTC_RC_BARRIER implementation (referenced from mutc_rc_gen.h)
 */
#define SAC_MUTC_RC_BARRIER__DESC(var_NT)                                                \
    {                                                                                    \
        SAC_MUTC_DEBUG_RC (printf (TO_STR (var_NT) " = %p\n", SAC_ND_A_DESC (var_NT));); \
        SAC_TR_REF_PRINT (("MUTC_RC_BARRIER( %s)", NT_STR (var_NT)))                     \
        SAC_IF_MUTC_RC_INDIRECT (                                                        \
          sl_create (, SAC_mutc_rc_place_w, , , , , , SAC_rc_barrier_w,                  \
                     sl_glarg (int *, , SAC_ND_A_DESC (var_NT)));)                       \
        SAC_IF_NOT_MUTC_RC_INDIRECT (                                                    \
          sl_create (, SAC_MUTC_GET_RC_PLACE (SAC_ND_A_DESC (var_NT)), 0, 1, 1, ,        \
                     sl__exclusive, SAC_rc_barrier,                                      \
                     sl_glarg (int *, , SAC_ND_A_DESC (var_NT)));)                       \
        SAC_MUTC_SYNC ();                                                                \
    }

#define SAC_MUTC_RC_PARENT_INC_SYNC(var_NT)                                              \
    {                                                                                    \
        SAC_MUTC_DEBUG_RC (printf (TO_STR (var_NT) " = %p\n", SAC_ND_A_DESC (var_NT));); \
        SAC_TR_REF_PRINT (("MUTC_RC_PARENT_INC( %s)", NT_STR (var_NT)))                  \
        SAC_IF_MUTC_RC_INDIRECT (                                                        \
          sl_create (, SAC_mutc_rc_place_w, , , , , , SAC_inc_parent_count_w,            \
                     sl_glarg (SAC_ND_DESC_PARENT_TYPE, ,                                \
                               (SAC_ND_DESC_PARENT_TYPE)SAC_ND_A_DESC_PARENT (           \
                                 var_NT)));)                                             \
        SAC_IF_NOT_MUTC_RC_INDIRECT (                                                    \
          sl_create (, SAC_MUTC_GET_RC_PLACE (SAC_ND_A_DESC (var_NT)), 0, 1, 1, ,        \
                     sl__exclusive, SAC_inc_parent_count,                                \
                     sl_glarg (SAC_ND_DESC_PARENT_TYPE, ,                                \
                               (SAC_ND_DESC_PARENT_TYPE)SAC_ND_A_DESC_PARENT (           \
                                 var_NT)));)                                             \
        SAC_MUTC_SYNC ();                                                                \
        SAC_TR_REF_PRINT_RC (var_NT)                                                     \
    }
/*
 * Access the descriptors rc directly in SAC_ND_PRF_RESTORERC and
 * SAC_ND_PRF_2NORC so that we do not perform any special reference
 * counting access.
 */

#define SAC_ND_PRF_RESTORERC__DO(array, rc)                                              \
    SAC_MUTC_RC_PRINT (array);                                                           \
    SAC_ND_A_DESC_RC_MODE (array) = SAC_ND_A_RC_T_MODE (rc);                             \
    DESC_RC (SAC_ND_A_DESC (array)) = SAC_ND_A_RC_T_RC (rc);

#define SAC_ND_PRF_RESTORERC__NOOP(array, rc)

#define SAC_ND_PRF_2NORC__DO(rc, array)                                                  \
    SAC_MUTC_RC_PRINT (array);                                                           \
    SAC_ND_A_FIELD (rc) = alloca (SAC_RC_T_SIZE);                                        \
    SAC_ND_A_RC_T_MODE (rc) = SAC_ND_A_DESC_RC_MODE (array);                             \
    SAC_ND_A_RC_T_RC (rc) = DESC_RC (SAC_ND_A_DESC (array));                             \
    SAC_ND_A_DESC_RC_MODE (array) = SAC_DESC_RC_MODE_NORC;

#define SAC_ND_PRF_2NORC__NOOP(rc, array)

/*
 * Need to do the inc in sync not detached as the queue must be
 * flushed before the spawn else order maybe lost.
 *
 * If rc is 1 the ownership of the memory can be passed to the new
 * reference.  The new reference coan reuse the parent count and
 * descriptor.  Full reuse analysis is not needed as we are only
 * reusing this aliases
 */

#define SAC_ND_PRF_2ASYNC__DO(new, array)                                                \
    if (DESC_RC_MODE (SAC_ND_A_DESC (array)) != SAC_DESC_RC_MODE_NORC) {                 \
        if (DESC_RC (SAC_ND_A_DESC (array)) == 1) {                                      \
            SAC_ND_A_FIELD (new) = SAC_ND_A_FIELD (array);                               \
            SAC_ND_A_DESC (new) = SAC_ND_A_DESC (array);                                 \
        } else {                                                                         \
            SAC_ND_A_FIELD (new) = SAC_ND_A_FIELD (array);                               \
            SAC_ND_A_DESC_RC_MODE (array) = SAC_DESC_RC_MODE_ASYNC;                      \
            SAC_MUTC_RC_PRINT (array);                                                   \
            if (SAC_ND_A_DESC_PARENT (array) == NULL) {                                  \
                SAC_ND_ALLOC__DESC__PARENT (array, SAC_ND_A_DIM (array));                \
                SAC_MUTC_DEBUG_RC (printf ("alloced parent at %p in %p\n",               \
                                           (void *)SAC_ND_A_DESC_PARENT (array),         \
                                           SAC_ND_A_DESC (array));)                      \
                SAC_DESC_PARENT_T_CHILDREN (SAC_ND_A_DESC_PARENT (array)) = 2;           \
            } else {                                                                     \
                SAC_MUTC_RC_PARENT_INC_SYNC (array);                                     \
            }                                                                            \
            SAC_ND_A_COPY_DESC (new, array);                                             \
            DESC_RC (SAC_ND_A_DESC (new));                                               \
            SAC_MUTC_DEBUG_RC (printf ("copy from %p to %p\n", SAC_ND_A_DESC (array),    \
                                       SAC_ND_A_DESC (new)););                           \
            SAC_MUTC_RC_PRINT (array);                                                   \
            SAC_MUTC_RC_PRINT (new);                                                     \
        }                                                                                \
    }

#define SAC_ND_PRF_2ASYNC__NOOP(new, array)

#endif /* SAC_BACKEND */
#undef MUTC

#endif
