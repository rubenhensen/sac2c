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

#define SAC_SL_DETACH() sl_detach ()
#define SAC_MUTC_DEBUG_RC(a)

/*
sl_decl(SAC_set_rc, void, sl_glparm(int*,desc), sl_glparm(int ,rc));
sl_decl(SAC_inc_rc, void, sl_glparm(int*,desc), sl_glparm(int ,rc));
sl_decl(SAC_dec_rc, void, sl_glparm(int*,desc), sl_glparm(int ,rc));
sl_decl(SAC_dec_and_get_rc, void, sl_glparm(int *,desc), sl_shparm(int, val));
*/

sl_decl (SAC_set_rc_w, void, sl_glparm (int *, desc), sl_glparm (int, rc));
sl_decl (SAC_inc_rc_w, void, sl_glparm (int *, desc), sl_glparm (int, rc));
sl_decl (SAC_dec_rc_w, void, sl_glparm (int *, desc), sl_glparm (int, rc));
sl_decl (SAC_dec_and_maybeFree_rc_w, void, sl_glparm (int *, desc), sl_glparm (int, val),
         sl_glparm (void *, data));

sl_decl (SAC_get_rc_w, void, sl_glparm (int *, desc), sl_shparm (int, val));

/*
 * SAC_ND_SET__RC implementations (referenced by sac_std_gen.h)
 */
#undef SAC_ND_SET__RC__DEFAULT
#define SAC_ND_SET__RC__DEFAULT(var_NT, rc)                                              \
    {                                                                                    \
        SAC_MUTC_DEBUG_RC (printf (TO_STR (var_NT) " = %p\n", SAC_ND_A_DESC (var_NT));); \
        SAC_TR_REF_PRINT (("ND_SET__RC( %s, %d)", NT_STR (var_NT), rc))                  \
        sl_create (, SAC_mutc_rc_place_w, , , , , , SAC_set_rc_w,                        \
                   sl_glarg (int *, , SAC_ND_A_DESC (var_NT)), sl_glarg (int, , rc));    \
        SAC_SL_DETACH ();                                                                \
        SAC_TR_REF_PRINT_RC (var_NT)                                                     \
    }

/*
 * SAC_ND_INC_RC implementations (referenced by sac_std_gen.h)
 */
#undef SAC_ND_INC_RC__DEFAULT
#define SAC_ND_INC_RC__DEFAULT(var_NT, rc)                                               \
    {                                                                                    \
        SAC_MUTC_DEBUG_RC (printf (TO_STR (var_NT) " = %p\n", SAC_ND_A_DESC (var_NT));); \
        SAC_TR_REF_PRINT (("ND_INC_RC( %s, %d)", NT_STR (var_NT), rc))                   \
        sl_create (, SAC_mutc_rc_place_w, , , , , , SAC_inc_rc_w,                        \
                   sl_glarg (int *, , SAC_ND_A_DESC (var_NT)), sl_glarg (int, , rc));    \
        sl_detach ();                                                                    \
        SAC_TR_REF_PRINT_RC (var_NT)                                                     \
    }

/*
 * SAC_ND_DEC_RC implementations (referenced by sac_std_gen.h)
 */
#undef SAC_ND_DEC_RC__DEFAULT
#define SAC_ND_DEC_RC__DEFAULT(var_NT, rc)                                               \
    {                                                                                    \
        SAC_MUTC_DEBUG_RC (printf (TO_STR (var_NT) " = %p\n", SAC_ND_A_DESC (var_NT));); \
        SAC_TR_REF_PRINT (("ND_DEC_RC( %s, %d)", NT_STR (var_NT), rc))                   \
        sl_create (, SAC_mutc_rc_place_w, , , , , , SAC_dec_rc_w,                        \
                   sl_glarg (int *, , SAC_ND_A_DESC (var_NT)), sl_glarg (int, , rc));    \
        sl_detach ();                                                                    \
        SAC_TR_REF_PRINT_RC (var_NT)                                                     \
    }

/*
 * SAC_ND_DEC_RC_FREE implementations (referenced by sac_std_gen.h)
 */
#undef SAC_ND_DEC_RC_FREE__DEFAULT
#define SAC_ND_DEC_RC_FREE__DEFAULT(var_NT, rc, freefun)                                 \
    {                                                                                    \
        SAC_TR_REF_PRINT (                                                               \
          ("ND_DEC_RC_FREE( %s, %d, %s)", NT_STR (var_NT), rc, #freefun))                \
        SAC_MUTC_DEBUG_RC (printf (TO_STR (var_NT) " = %p\n", SAC_ND_A_DESC (var_NT));); \
                                                                                         \
        sl_create (, SAC_mutc_rc_place_w, , , , , , SAC_dec_and_maybeFree_rc_w,          \
                   sl_glarg (int *, , SAC_ND_A_DESC (var_NT)), sl_glarg (int, val, rc),  \
                   sl_glarg (void *, ,                                                   \
                             SAC_ND_GETVAR (var_NT, SAC_ND_A_FIELD (var_NT))));          \
        sl_detach ();                                                                    \
    }

#undef SAC_ND_A_RC__DEFAULT
#define SAC_ND_A_RC__DEFAULT(var_NT)                                                     \
    ({                                                                                   \
        SAC_TR_REF_PRINT (("ND_DEC_RC_FREE( %s)", NT_STR (var_NT)))                      \
        SAC_MUTC_DEBUG_RC (printf (TO_STR (var_NT) " = %p\n", SAC_ND_A_DESC (var_NT));); \
                                                                                         \
        sl_create (, SAC_mutc_rc_place_w, , , , , , SAC_get_rc_w,                        \
                   sl_glarg (int *, , SAC_ND_A_DESC (var_NT)), sl_sharg (int, val, 0));  \
        sl_sync ();                                                                      \
        SAC_TR_REF_PRINT_RC (var_NT)                                                     \
        (int)sl_geta (val);                                                              \
    })

#endif /* SAC_BACKEND */
#undef MUTC

#endif
