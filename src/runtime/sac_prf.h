/*
 *
 * $Log$
 * Revision 3.23  2005/02/16 09:58:13  ktr
 * Typo fixed for Bodo.
 *
 * Revision 3.22  2004/11/17 13:37:15  ktr
 * added SAC_ND_PRF_IS_REUSED( to_NT, from_NT, from2_NT)
 *
 * Revision 3.21  2004/09/21 17:31:01  ktr
 * Added ND_PRF_IDX_SHAPE_SEL, ND_PRF_SHAPE_SEL
 *
 * Revision 3.20  2004/03/09 23:56:15  dkrHH
 * old backend removed
 *
 * Revision 3.19  2003/11/10 20:22:56  dkrHH
 * debug output: NT objs are converted into strings correctly now
 *
 * Revision 3.18  2003/09/29 23:38:34  dkr
 * comment about ICMs for prfs corrected
 *
 * Revision 3.17  2003/09/25 13:44:39  dkr
 * new argument 'copyfun' added to some ICMs.
 * ND_WRITE replaced by ND_WRITE_READ_COPY.
 *
 * Revision 3.16  2003/09/22 11:59:31  dkr
 * SAC_ABS used
 *
 * Revision 3.15  2003/09/22 11:12:41  dkr
 * bug in SAC_ND_PRF_CAT__DATA fixed
 *
 * Revision 3.14  2003/09/20 14:16:38  dkr
 * ND_PRF_CAT__DATA added
 *
 * Revision 3.13  2003/09/19 15:41:07  dkr
 * some more PRFs added
 *
 * Revision 3.12  2003/09/18 14:43:18  dkr
 * support for F_neg added
 *
 * Revision 3.11  2003/09/15 13:00:04  dkr
 * SAC_MIN, SAC_MAX used
 *
 * Revision 3.10  2003/03/09 21:27:09  dkr
 * SAC_ND_PRF_DIM__DATA modified
 *
 * Revision 3.9  2003/03/08 20:56:10  dkr
 * macros for new backend revisited
 *
 * Revision 3.8  2002/08/03 03:16:35  dkr
 * ND_PRF_SEL__DIM icms removed
 *
 * Revision 3.7  2002/08/02 20:48:04  dkr
 * ND_PRF_SEL__DIM_... icms added
 *
 * Revision 3.6  2002/07/31 16:34:20  dkr
 * parameter 'copyfun' added for several ICMs
 *
 * Revision 3.5  2002/07/16 12:52:06  dkr
 * ICMs ND_PRF_IDX_... moved from sac_prf.h to sac_idx.h
 *
 * Revision 3.4  2002/07/15 18:27:18  dkr
 * bug fixed
 *
 * Revision 3.3  2002/07/15 14:46:57  dkr
 * macros SAC_ND_PRF_... moved from sac_std.tagged.h to sac_prf.h
 *
 * Revision 3.1  2000/11/20 18:02:18  sacbase
 * new release made
 *
 * Revision 2.2  1999/04/12 09:37:48  cg
 * All accesses to C arrays are now performed through the new ICMs
 * ND_WRITE_ARRAY and ND_READ_ARRAY. This allows for an integration
 * of cache simulation as well as boundary checking.
 *
 * Revision 2.1  1999/02/23 12:43:57  sacbase
 * new release made
 *
 * Revision 1.3  1998/07/10 08:09:21  cg
 * some bugs fixed, appropriate renaming of macros
 *
 * Revision 1.2  1998/06/05 07:49:45  cg
 * converted to new renaming conventions of local identifiers.
 *
 * Revision 1.1  1998/05/07 08:38:05  cg
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   sac_prf.h
 *
 * description:
 *   This file is part of the SAC standard header file sac.h
 *
 *   It provides definitions of Intermediate Code Macros (ICM)
 *   implemented as real macros.
 *
 *****************************************************************************/

#ifndef _SAC_PRF_H_
#define _SAC_PRF_H_

/******************************************************************************
 *
 * core ICMs for arithmetical operations
 * =====================================
 *
 * ND_UNIOP( op, arg)
 * ND_BINOP( op, arg1, arg2)
 *
 ******************************************************************************/

#define SAC_ND_UNIOP(op, arg) (op (arg))
#define SAC_ND_BINOP(op, arg1, arg2) ((arg1)op (arg2))

/******************************************************************************
 *
 * internal ICMs for arithmetical operations
 * =========================================
 *
 * ND_PRF_UNIOP( op, arg)
 * ND_PRF_NEG( dummy, arg)
 * ND_PRF_ABS( dummy, arg)
 *
 * ND_PRF_BINOP( op, arg1, arg2)
 * ND_PRF_MIN( dummy, arg1, arg2)
 * ND_PRF_MAX( dummy, arg1, arg2)
 *
 ******************************************************************************/

#define SAC_PRF_UNIOP(op, arg) SAC_ND_UNIOP (op, arg)
#define SAC_PRF_NEG(dummy, arg) (-(arg))
#define SAC_PRF_ABS(dummy, arg) SAC_ABS (arg)

#define SAC_PRF_BINOP(op, arg1, arg2) SAC_ND_BINOP (op, arg1, arg2)
#define SAC_PRF_MIN(dummy, arg1, arg2) SAC_MIN (arg1, arg2)
#define SAC_PRF_MAX(dummy, arg1, arg2) SAC_MAX (arg1, arg2)

/******************************************************************************
 *
 * ICMs for primitive functions
 * ============================
 *
 * ND_PRF_DIM__DATA( to_NT, to_sdim, from_NT, from_sdim)
 *
 * ND_PRF_SHAPE__DATA( to_NT, to_sdim, from_NT, from_sdim)
 *
 * ND_PRF_RESHAPE__SHAPE_id( to_NT, to_sdim, shp_NT)
 * ND_PRF_RESHAPE__SHAPE_arr( to_NT, to_sdim, shp_size, ...shp_ANY...)
 *
 * ND_PRF_SEL__SHAPE_id( to_NT, to_sdim, from_NT, from_sdim,
 *                       idx_size, idx_NT)
 * ND_PRF_SEL__SHAPE_arr( to_NT, to_sdim, from_NT, from_sdim,
 *                        idx_size, idxs_ANY)
 * ND_PRF_SEL__DATA_id( to_NT, to_sdim, from_NT, from_sdim,
 *                      idx_size, idx_NT, copyfun)
 * ND_PRF_SEL__DATA_arr( to_NT, to_sdim, from_NT, from_sdim,
 *                       idx_size, idxs_ANY, copyfun)
 *
 * ND_PRF_MODARRAY__DATA_id( to_NT, to_sdim, from_NT, from_sdim,
 *                           idx_size, idx_NT, val_ANY, copyfun)
 * ND_PRF_MODARRAY__DATA_arr( to_NT, to_sdim, from_NT, from_sdim,
 *                            idx_size, ...idxs_ANY..., val_ANY, copyfun)
 *
 * ND_PRF_TAKE__SHAPE( to_NT, to_sdim, from_NT, from_sdim, cnt_ANY)
 * ND_PRF_TAKE__DATA( to_NT, to_sdim, from_NT, from_sdim, cnt_ANY, copyfun)
 *
 * ND_PRF_DROP__SHAPE( to_NT, to_sdim, from_NT, from_sdim, cnt_ANY)
 * ND_PRF_DROP__DATA( to_NT, to_sdim, from_NT, from_sdim, cnt_ANY, copyfun)
 *
 * ND_PRF_CAT__SHAPE( to_NT, to_sdim,
 *                    from1_NT, from1_sdim, from2_NT, from2_sdim)
 * ND_PRF_CAT__DATA( to_NT, to_sdim,
 *                   from1_NT, from1_sdim, from2_NT, from2_sdim, copyfun)
 *
 * ND_PRF_CONV_A__DATA( to_NT, from_NT)
 *
 * ND_PRF_S__DATA( to_NT, op, scl)
 * ND_PRF_A__DATA( to_NT, op, scl)
 *
 * ND_PRF_SxS__DATA( to_NT, op, scl1,     scl2)
 * ND_PRF_SxA__DATA( to_NT, op, scl,      from_NT)
 * ND_PRF_AxS__DATA( to_NT, op, from_NT,  scl)
 * ND_PRF_AxA__DATA( to_NT, op, from1_NT, from2_NT)
 *
 * ND_PRF_SHAPE_IDX_SEL__DATA( to_NT, to_sdim, from_NT, from_sdim, scl)
 * ND_PRF_SHAPE_SEL__DATA( to_NT, to_sdim, from_NT, from_sdim, from2_NT)
 *
 * ND_PRF_IS_REUSED( to_NT, from_NT, from2_NT)
 *
 ******************************************************************************/

#define SAC_ND_PRF_DIM__DATA(to_NT, to_sdim, from_NT, from_sdim)                         \
    SAC_TR_PRF_PRINT (("ND_PRF_DIM__...( %s, %d, %s, %d)\n", NT_STR (to_NT), to_sdim,    \
                       NT_STR (from_NT), from_sdim))                                     \
    SAC_ND_CREATE__SCALAR__DATA (to_NT, SAC_ND_A_DIM (from_NT))

/* ND_PRF_SHAPE__DATA( ...) is a C-ICM */

/* ND_PRF_RESHAPE__SHAPE_id( ...) is a C-ICM */
/* ND_PRF_RESHAPE__SHAPE_arr( ...) is a C-ICM */

/* ND_PRF_SEL__SHAPE_id( ...) is a C-ICM */
/* ND_PRF_SEL__SHAPE_arr( ...) is a C-ICM */
/* ND_PRF_SEL__DATA_id( ...) is a C-ICM */
/* ND_PRF_SEL__DATA_arr( ...) is a C-ICM */

/* ND_PRF_MODARRAY__DATA_id( ...) is a C-ICM */
/* ND_PRF_MODARRAY__DATA_arr( ...) is a C-ICM */

/* ND_PRF_TAKE__SHAPE( ...) is a C-ICM */
/* ND_PRF_TAKE__DATA( ...) is a C-ICM */

/* ND_PRF_DROP__SHAPE( ...) is a C-ICM */
/* ND_PRF_DROP__DATA( ...) is a C-ICM */

/* ND_PRF_CAT__SHAPE( ...) is a C-ICM */

#define SAC_ND_PRF_CAT__DATA(to_NT, to_sdim, from1_NT, from1_sdim, from2_NT, from2_sdim, \
                             copyfun)                                                    \
    {                                                                                    \
        int SAC_i, SAC_off;                                                              \
        SAC_off = SAC_ND_A_SIZE (from1_NT);                                              \
        for (SAC_i = 0; SAC_i < SAC_off; SAC_i++) {                                      \
            SAC_ND_WRITE_READ_COPY (to_NT, SAC_i, from1_NT, SAC_i, copyfun);             \
        }                                                                                \
        for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE (from2_NT); SAC_i++) {                     \
            SAC_ND_WRITE_READ_COPY (to_NT, SAC_off + SAC_i, from2_NT, SAC_i, copyfun);   \
        }                                                                                \
    }

#define SAC_ND_PRF_CONV_A__DATA(to_NT, from_NT)                                          \
    SAC_TR_PRF_PRINT (                                                                   \
      ("ND_PRF_CONV_A__DATA( %s, %s)\n", NT_STR (to_NT), NT_STR (from_NT)));             \
    {                                                                                    \
        int SAC_i;                                                                       \
        for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE (to_NT); SAC_i++) {                        \
            SAC_ND_WRITE_READ_COPY (to_NT, SAC_i, from_NT, SAC_i, );                     \
        }                                                                                \
    }

#define SAC_ND_PRF_S__DATA(to_NT, op_macro, op, scl)                                     \
    SAC_TR_PRF_PRINT (                                                                   \
      ("ND_PRF_S__DATA( %s, %s, %s, %s)\n", NT_STR (to_NT), #op_macro, #op, #scl));      \
    SAC_ND_WRITE_COPY (to_NT, 0, op_macro (op, scl), );

#define SAC_ND_PRF_A__DATA(to_NT, op_macro, op, arg_NT)                                  \
    SAC_TR_PRF_PRINT (("ND_PRF_A__DATA( %s, %s, %s, %s)\n", NT_STR (to_NT), #op_macro,   \
                       #op, NT_STR (arg_NT)));                                           \
    {                                                                                    \
        int SAC_i;                                                                       \
        for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE (to_NT); SAC_i++) {                        \
            SAC_ND_WRITE_COPY (to_NT, SAC_i,                                             \
                               op_macro (op, SAC_ND_READ (arg_NT, SAC_i)), );            \
        }                                                                                \
    }

#define SAC_ND_PRF_SxS__DATA(to_NT, op_macro, op, scl1, scl2)                            \
    SAC_TR_PRF_PRINT (("ND_PRF_SxS__DATA( %s, %s, %s, %s, %s)\n", NT_STR (to_NT),        \
                       #op_macro, #op, #scl1, #scl2));                                   \
    SAC_ND_WRITE_COPY (to_NT, 0, op_macro (op, scl1, scl2), );

#define SAC_ND_PRF_SxA__DATA(to_NT, op_macro, op, scl, from_NT)                          \
    SAC_TR_PRF_PRINT (("ND_PRF_SxA__DATA( %s, %s, %s, %s, %s)\n", NT_STR (to_NT),        \
                       #op_macro, #op, #scl, NT_STR (from_NT)));                         \
    {                                                                                    \
        int SAC_i;                                                                       \
        for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE (to_NT); SAC_i++) {                        \
            SAC_ND_WRITE_COPY (to_NT, SAC_i,                                             \
                               op_macro (op, scl, SAC_ND_READ (from_NT, SAC_i)), );      \
        }                                                                                \
    }

#define SAC_ND_PRF_AxS__DATA(to_NT, op_macro, op, from_NT, scl)                          \
    SAC_TR_PRF_PRINT (("ND_PRF_AxS__DATA( %s, %s, %s, %s, %s)\n", NT_STR (to_NT),        \
                       #op_macro, #op, NT_STR (from_NT), #scl));                         \
    {                                                                                    \
        int SAC_i;                                                                       \
        for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE (to_NT); SAC_i++) {                        \
            SAC_ND_WRITE_COPY (to_NT, SAC_i,                                             \
                               op_macro (op, SAC_ND_READ (from_NT, SAC_i), scl), );      \
        }                                                                                \
    }

#define SAC_ND_PRF_AxA__DATA(to_NT, op_macro, op, from1_NT, from2_NT)                    \
    SAC_TR_PRF_PRINT (("ND_PRF_AxA__DATA( %s, %s, %s, %s, %s)\n", NT_STR (to_NT),        \
                       #op_macro, #op, NT_STR (from1_NT), NT_STR (from2_NT)));           \
    {                                                                                    \
        int SAC_i;                                                                       \
        for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE (to_NT); SAC_i++) {                        \
            SAC_ND_WRITE_COPY (to_NT, SAC_i,                                             \
                               op_macro (op, SAC_ND_READ (from1_NT, SAC_i),              \
                                         SAC_ND_READ (from2_NT, SAC_i)), );              \
        }                                                                                \
    }

#define SAC_ND_PRF_IDX_SHAPE_SEL__DATA(to_NT, to_sdim, from_NT, from_sdim, scl)          \
    SAC_TR_PRF_PRINT (("ND_PRF_IDX_SHAPE_SEL__...( %s, %d, %s, %d, %s)\n",               \
                       NT_STR (to_NT), to_sdim, NT_STR (from_NT), from_sdim, #scl))      \
    SAC_ND_CREATE__SCALAR__DATA (to_NT, SAC_ND_A_SHAPE (from_NT, scl))

#define SAC_ND_PRF_SHAPE_SEL__DATA(to_NT, to_sdim, from_NT, from_sdim, from2_NT)         \
    SAC_TR_PRF_PRINT (("ND_PRF_IDX_SHAPE_SEL__...( %s, %d, %s, %d, %s)\n",               \
                       NT_STR (to_NT), to_sdim, NT_STR (from_NT), from_sdim, from2_NT))  \
    SAC_ND_CREATE__SCALAR__DATA (to_NT,                                                  \
                                 SAC_ND_A_SHAPE (from_NT, SAC_ND_READ (from2_NT, 0)))

#define SAC_ND_PRF_IS_REUSED(to_NT, from_NT, from2_NT)                                   \
    CAT17 (SAC_ND_PRF_IS_REUSED__, NT_SHP (from_NT) BuildArgs3 (to_NT, from_NT, from2_NT))

#define SAC_ND_PRF_IS_REUSED__SCL(to_NT, from_NT, from2_NT) SAC_ND_A_FIELD (to_NT) = 0;

#define SAC_ND_PRF_IS_REUSED__AKS(to_NT, from_NT, from2_NT)                              \
    SAC_ND_A_FIELD (to_NT) = (SAC_ND_A_FIELD (from_NT) == SAC_ND_A_FIELD (from2_NT));

#define SAC_ND_PRF_IS_REUSED__AKD(to_NT, from_NT, from2_NT)                              \
    SAC_ND_PRF_IS_REUSED__AKS (to_NT, from_NT, from2_NT)

#define SAC_ND_PRF_IS_REUSED__AUD(to_NT, from_NT, from2_NT)                              \
    SAC_ND_PRF_IS_REUSED__AKS (to_NT, from_NT, from2_NT)

#endif /* _SAC_PRF_H_ */
