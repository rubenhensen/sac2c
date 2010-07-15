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

#define MUTC 1
#if SAC_BACKEND == MUTC

sl_decl (SAC_inc_rc, void, sl_glparm (int *, desc), sl_glparm (int, rc));
sl_decl (SAC_dec_rc, void, sl_glparm (int *, desc), sl_glparm (int, rc));
sl_decl (SAC_dec_and_get_rc, void, sl_glparm (int *, desc), sl_shparm (int, val));
sl_decl (SAC_get_rc, void, sl_glparm (int *, desc), sl_shparm (int, val));

/*
 * SAC_ND_SET__RC implementations (referenced by sac_std_gen.h)
 */
#undef SAC_ND_SET__RC__DEFAULT
#define SAC_ND_SET__RC__DEFAULT(var_NT, rc)                                              \
    {                                                                                    \
        SAC_TR_REF_PRINT (("ND_SET__RC( %s, %d)", NT_STR (var_NT), rc))                  \
        sl_create (, SAC_mutc_rc_place, 0, 1, 1, , sl__exclusive, SAC_set_rc,            \
                   sl_glarg (int *, , SAC_ND_A_DESC (var_NT)), sl_glarg (int, , rc));    \
        sl_detach ();                                                                    \
        SAC_TR_REF_PRINT_RC (var_NT)                                                     \
    }

/*
 * SAC_ND_INC_RC implementations (referenced by sac_std_gen.h)
 */
#undef SAC_ND_INC_RC__DEFAULT
#define SAC_ND_INC_RC__DEFAULT(var_NT, rc)                                               \
    {                                                                                    \
        SAC_TR_REF_PRINT (("ND_INC_RC( %s, %d)", NT_STR (var_NT), rc))                   \
        sl_create (, SAC_mutc_rc_place, 0, 1, 1, , sl__exclusive, SAC_inc_rc,            \
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
        SAC_TR_REF_PRINT (("ND_DEC_RC( %s, %d)", NT_STR (var_NT), rc))                   \
        sl_create (, SAC_mutc_rc_place, 0, 1, 1, , sl__exclusive, SAC_dec_rc,            \
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
                                                                                         \
        sl_create (, SAC_mutc_rc_place, 0, 1, 1, , sl__exclusive, SAC_dec_and_get_rc,    \
                   sl_glarg (int *, , SAC_ND_A_DESC (var_NT)), sl_sharg (int, val, rc)); \
        sl_sync ();                                                                      \
        if (sl_geta (val) == 0) {                                                        \
            SAC_TR_REF_PRINT_RC (var_NT)                                                 \
            SAC_ND_FREE (var_NT, freefun)                                                \
        } else {                                                                         \
            SAC_TR_REF_PRINT_RC (var_NT)                                                 \
        }                                                                                \
    }

#endif /* SAC_BACKEND */
#undef MUTC
