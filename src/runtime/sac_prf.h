/*
 *
 * $Log$
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
 * macros for TAGGED_ARRAYS revisited
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

#ifdef TAGGED_ARRAYS

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
#define SAC_PRF_ABS(dummy, arg) (((arg) < 0) ? (-(arg)) : (arg))

#define SAC_PRF_BINOP(op, arg1, arg2) SAC_ND_BINOP (op, arg1, arg2)
#define SAC_PRF_MIN(dummy, arg1, arg2) SAC_MIN (arg1, arg2)
#define SAC_PRF_MAX(dummy, arg1, arg2) SAC_MAX (arg1, arg2)

/******************************************************************************
 *
 * ICMs for primitive functions
 * ============================
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
 * ND_PRF_SEL__SHAPE_arr( to_nt, to_sdim, from_nt, from_sdim,
 *                        idx_size, idxa_any)
 * ND_PRF_SEL__DATA_id( to_nt, to_sdim, from_nt, from_sdim,
 *                      idx_size, idx_nt, copyfun)
 * ND_PRF_SEL__DATA_arr( to_nt, to_sdim, from_nt, from_sdim,
 *                       idx_size, idxa_any, copyfun)
 *
 * ND_PRF_MODARRAY__DATA_id( to_nt, to_sdim, from_nt, from_sdim,
 *                           idx_size, idx_nt, val_any, copyfun)
 * ND_PRF_MODARRAY__DATA_arr( to_nt, to_sdim, from_nt, from_sdim,
 *                            idx_size, ...idxa_any..., val_any, copyfun)
 *
 * ND_PRF_CONV_A__DATA( to_nt, from_nt)
 *
 * ND_PRF_S__DATA( to_nt, op, scl)
 * ND_PRF_A__DATA( to_nt, op, scl)
 *
 * ND_PRF_SxS__DATA( to_nt, op, scl1,     scl2)
 * ND_PRF_SxA__DATA( to_nt, op, scl,      from_nt)
 * ND_PRF_AxS__DATA( to_nt, op, from_nt,  scl)
 * ND_PRF_AxA__DATA( to_nt, op, from1_nt, from2_nt)
 *
 ******************************************************************************/

#define SAC_ND_PRF_DIM__DATA(to_nt, to_sdim, from_nt, from_sdim)                         \
    SAC_TR_PRF_PRINT (                                                                   \
      ("ND_PRF_DIM__...( %s, %d, %s, %d)\n", #to_nt, to_sdim, #from_nt, from_sdim))      \
    SAC_ND_CREATE__SCALAR__DATA (to_nt, SAC_ND_A_DIM (from_nt))

/* ND_PRF_SHAPE__DATA( ...) is a C-ICM */

/* ND_PRF_RESHAPE__SHAPE_id( ...) is a C-ICM */
/* ND_PRF_RESHAPE__SHAPE_arr( ...) is a C-ICM */

/* ND_PRF_SEL__SHAPE_id( ...) is a C-ICM */
/* ND_PRF_SEL__SHAPE_arr( ...) is a C-ICM */
/* ND_PRF_SEL__DATA_id( ...) is a C-ICM */
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

#define SAC_ND_PRF_S__DATA(to_nt, op_macro, op, scl)                                     \
    SAC_TR_PRF_PRINT (                                                                   \
      ("ND_PRF_S__DATA( %s, %s, %s, %s)\n", #to_nt, #op_macro, #op, #scl));              \
    SAC_ND_WRITE (to_nt, 0) = op_macro (op, scl);

#define SAC_ND_PRF_A__DATA(to_nt, op_macro, op, arg_nt)                                  \
    SAC_TR_PRF_PRINT (                                                                   \
      ("ND_PRF_A__DATA( %s, %s, %s, %s)\n", #to_nt, #op_macro, #op, #arg_nt));           \
    {                                                                                    \
        int SAC_i;                                                                       \
        for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE (to_nt); SAC_i++) {                        \
            SAC_ND_WRITE (to_nt, SAC_i) = op_macro (op, SAC_ND_READ (arg_nt, SAC_i));    \
        }                                                                                \
    }

#define SAC_ND_PRF_SxS__DATA(to_nt, op_macro, op, scl1, scl2)                            \
    SAC_TR_PRF_PRINT (("ND_PRF_SxS__DATA( %s, %s, %s, %s, %s)\n", #to_nt, #op_macro,     \
                       #op, #scl1, #scl2));                                              \
    SAC_ND_WRITE (to_nt, 0) = op_macro (op, scl1, scl2);

#define SAC_ND_PRF_SxA__DATA(to_nt, op_macro, op, scl, from_nt)                          \
    SAC_TR_PRF_PRINT (("ND_PRF_SxA__DATA( %s, %s, %s, %s, %s)\n", #to_nt, #op_macro,     \
                       #op, #scl, #from_nt));                                            \
    {                                                                                    \
        int SAC_i;                                                                       \
        for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE (to_nt); SAC_i++) {                        \
            SAC_ND_WRITE (to_nt, SAC_i)                                                  \
              = op_macro (op, scl, SAC_ND_READ (from_nt, SAC_i));                        \
        }                                                                                \
    }

#define SAC_ND_PRF_AxS__DATA(to_nt, op_macro, op, from_nt, scl)                          \
    SAC_TR_PRF_PRINT (("ND_PRF_AxS__DATA( %s, %s, %s, %s, %s)\n", #to_nt, #op_macro,     \
                       #op, #from_nt, #scl));                                            \
    {                                                                                    \
        int SAC_i;                                                                       \
        for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE (to_nt); SAC_i++) {                        \
            SAC_ND_WRITE (to_nt, SAC_i)                                                  \
              = op_macro (op, SAC_ND_READ (from_nt, SAC_i), scl);                        \
        }                                                                                \
    }

#define SAC_ND_PRF_AxA__DATA(to_nt, op_macro, op, from1_nt, from2_nt)                    \
    SAC_TR_PRF_PRINT (("ND_PRF_AxA__DATA( %s, %s, %s, %s, %s)\n", #to_nt, #op_macro,     \
                       #op, #from1_nt, #from2_nt));                                      \
    {                                                                                    \
        int SAC_i;                                                                       \
        for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE (to_nt); SAC_i++) {                        \
            SAC_ND_WRITE (to_nt, SAC_i) = op_macro (op, SAC_ND_READ (from1_nt, SAC_i),   \
                                                    SAC_ND_READ (from2_nt, SAC_i));      \
        }                                                                                \
    }

#else /* TAGGED_ARRAYS */

#define SAC_ND_ABS(arg) (((arg) < 0) ? (-(arg)) : (arg))

#define SAC_ND_MIN(arg1, arg2) SAC_MIN (arg1, arg2)
#define SAC_ND_MAX(arg1, arg2) SAC_MAX (arg1, arg2)

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
