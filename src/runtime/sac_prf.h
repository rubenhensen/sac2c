/*
 *
 * $Log$
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
 * ICMs for primitive functions on scalars
 * =======================================
 *
 * ND_MIN( a1, a2)
 * ND_MAX( a1, a2)
 * ND_ABS( a)
 *
 ******************************************************************************/

#define SAC_ND_MIN(a1, a2) ((a1) < (a2) ? (a1) : (a2))
#define SAC_ND_MAX(a1, a2) ((a1) > (a2) ? (a1) : (a2))
#define SAC_ND_ABS(a) ((a) < 0 ? (-(a)) : (a))

#ifdef TAGGED_ARRAYS

/******************************************************************************
 *
 * ICMs for primitive functions on arrays
 * ======================================
 *
 * ND_PRF_DIM__DATA( to_nt, to_sdim, from_nt, from_sdim)
 *
 * ND_PRF_SHAPE__DATA( to_nt, to_sdim, from_nt, from_sdim)
 *
 * ND_PRF_RESHAPE__SHAPE_id( to_nt, to_sdim, shp_nt)
 * ND_PRF_RESHAPE__SHAPE_arr( to_nt, to_sdim, shp_size, ...shpa_any...)
 *
 * ND_PRF_SEL__SHAPE_id( to_nt, to_sdim, from_nt, from_sdim,
 *                       idx_size, idx_nt)
 * ND_PRF_SEL__DATA_id( to_nt, to_sdim, from_nt, from_sdim,
 *                      idx_size, idx_nt)
 * ND_PRF_SEL__SHAPE_arr( to_nt, to_sdim, from_nt, from_sdim,
 *                        idx_size, idxa_any)
 * ND_PRF_SEL__DATA_arr( to_nt, to_sdim, from_nt, from_sdim,
 *                       idx_size, idxa_any)
 *
 * ND_PRF_MODARRAY__DATA_id( to_nt, to_sdim, from_nt, from_sdim,
 *                           idx_size, idx_nt, val_any)
 * ND_PRF_MODARRAY__DATA_arr( to_nt, to_sdim, from_nt, from_sdim,
 *                            idx_size, ...idxa_any..., val_any)
 *
 * ND_PRF_CONV_A__DATA( to_nt, from_nt)
 *
 * ND_PRF_SxA__DATA( to_nt, op, scl,  a2_nt)
 * ND_PRF_AxS__DATA( to_nt, op, a2_nt, scl)
 * ND_PRF_AxA__DATA( to_nt, op, a1_nt, a2_nt)
 *
 ******************************************************************************/

#define SAC_ND_PRF_DIM__DATA(to_nt, to_sdim, from_nt, from_sdim)                         \
    SAC_TR_PRF_PRINT (                                                                   \
      ("ND_PRF_DIM__...( %s, %s, %s, %s)\n", #to_nt, #to_sdim, #from_nt, #from_sdim))    \
    SAC_ND_CREATE__SCALAR__DATA (to_nt, SAC_ND_A_DIM (from_nt))

/* ND_PRF_SHAPE__DATA( ...) is a C-ICM */

/* ND_PRF_RESHAPE__SHAPE_id( ...) is a C-ICM */
/* ND_PRF_RESHAPE__SHAPE_arr( ...) is a C-ICM */

/* ND_PRF_SEL__SHAPE_id( ...) is a C-ICM */
/* ND_PRF_SEL__DATA_id( ...) is a C-ICM */
/* ND_PRF_SEL__SHAPE_arr( ...) is a C-ICM */
/* ND_PRF_SEL__DATA_arr( ...) is a C-ICM */

/* ND_PRF_MODARRAY__DATA( ...) is a C-ICM */

#define SAC_ND_PRF_CONV_A__DATA(to_nt, from_nt)                                          \
    SAC_TR_PRF_PRINT (("ND_PRF_CONV_A__DATA( %s, %s)\n", #to_nt, #from_nt));             \
    {                                                                                    \
        int SAC_i;                                                                       \
        for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE (to_nt); SAC_i++) {                        \
            SAC_ND_WRITE (to_nt, SAC_i) = SAC_ND_READ (from_nt, SAC_i);                  \
        }                                                                                \
    }

#define SAC_ND_PRF_AxA__DATA(to_nt, op, a1_nt, a2_nt)                                    \
    SAC_TR_PRF_PRINT (                                                                   \
      ("ND_PRF_AxA__DATA( %s, %s, %s, %s)\n", #to_nt, #op, #a1_nt, #a2_nt));             \
    {                                                                                    \
        int SAC_i;                                                                       \
        for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE (to_nt); SAC_i++) {                        \
            SAC_ND_WRITE (to_nt, SAC_i)                                                  \
              = SAC_ND_READ (a1_nt, SAC_i) op SAC_ND_READ (a2_nt, SAC_i);                \
        }                                                                                \
    }

#define SAC_ND_PRF_AxS__DATA(to_nt, op, a1_nt, scl)                                      \
    SAC_TR_PRF_PRINT (                                                                   \
      ("ND_PRF_AxS__DATA( %s, %s, %s, %s)\n", #to_nt, #op, #a1_nt, #scl));               \
    {                                                                                    \
        int SAC_i;                                                                       \
        for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE (to_nt); SAC_i++) {                        \
            SAC_ND_WRITE (to_nt, SAC_i) = SAC_ND_READ (a1_nt, SAC_i) op scl;             \
        }                                                                                \
    }

#define SAC_ND_PRF_SxA__DATA(to_nt, op, scl, a2_nt)                                      \
    SAC_TR_PRF_PRINT (                                                                   \
      ("ND_PRF_SxA__DATA( %s, %s, %s, %s)\n", #to_nt, #op, #scl, #a2_nt));               \
    {                                                                                    \
        int SAC_i;                                                                       \
        for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE (to_nt); SAC_i++) {                        \
            SAC_ND_WRITE (to_nt, SAC_i) = scl op SAC_ND_READ (a2_nt, SAC_i);             \
        }                                                                                \
    }

#else /* TAGGED_ARRAYS */

/*
 * Macros for primitve arithmetic operations:
 * ==========================================
 *
 * ND_BINOP_SxA_A( op, s, a2, res)        : realizes res = (s op a2)
 * ND_BINOP_AxS_A( op, s, a2, res)        : realizes res = (a2 op s)
 * ND_BINOP_AxA_A( op, a1, a2, res)       : realizes res = (a1 op a2)
 */

#define SAC_ND_BINOP_AxA_A(op, a1, a2, res)                                              \
    SAC_TR_PRF_PRINT (("ND_BINOP_AxA_A( %s, %s, %s, %s)\n", #op, #a1, #a2, #res));       \
    {                                                                                    \
        int SAC_i;                                                                       \
        for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE (res); SAC_i++) {                          \
            SAC_ND_WRITE_ARRAY (res, SAC_i)                                              \
              = SAC_ND_READ_ARRAY (a1, SAC_i) op SAC_ND_READ_ARRAY (a2, SAC_i);          \
        }                                                                                \
    }

#define SAC_ND_BINOP_AxS_A(op, a1, s, res)                                               \
    SAC_TR_PRF_PRINT (("ND_BINOP_AxS_A( %s, %s, %s, %s)\n", #op, #a1, #s, #res));        \
    {                                                                                    \
        int SAC_i;                                                                       \
        for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE (res); SAC_i++) {                          \
            SAC_ND_WRITE_ARRAY (res, SAC_i) = SAC_ND_READ_ARRAY (a1, SAC_i) op s;        \
        }                                                                                \
    }

#define SAC_ND_BINOP_SxA_A(op, s, a2, res)                                               \
    SAC_TR_PRF_PRINT (("ND_BINOP_SxA_A( %s, %s, %s, %s)\n", #op, #s, #a2, #res));        \
    {                                                                                    \
        int SAC_i;                                                                       \
        for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE (res); SAC_i++) {                          \
            SAC_ND_WRITE_ARRAY (res, SAC_i) = s op SAC_ND_READ_ARRAY (a2, SAC_i);        \
        }                                                                                \
    }

/*
 * Macros for primitive type conversion functions on arrays
 */

#define SAC_ND_CONV_A(a1, res)                                                           \
    {                                                                                    \
        int SAC_i;                                                                       \
        for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE (res); SAC_i++) {                          \
            SAC_ND_WRITE_ARRAY (res, SAC_i) = SAC_ND_READ_ARRAY (a1, SAC_i);             \
        }                                                                                \
    }

#define SAC_ND_2I_A(a1, res) SAC_ND_CONV_A (a1, res)
#define SAC_ND_2F_A(a1, res) SAC_ND_CONV_A (a1, res)
#define SAC_ND_2D_A(a1, res) SAC_ND_CONV_A (a1, res)

#endif /* TAGGED_ARRAYS */

#endif /* _SAC_PRF_H_ */
