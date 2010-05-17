/*
 *
 * $Id$
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
 * core ICMs for built-in operators
 * =====================================
 *
 *
 ******************************************************************************/

#define SAC_ND_PRF_TOBOOL(arg) (int)(arg)
#define SAC_ND_PRF_TOC(arg) (char)(arg)
#define SAC_ND_PRF_TOB(arg) (byte) (arg)
#define SAC_ND_PRF_TOS(arg) (short)(arg)
#define SAC_ND_PRF_TOI(arg) (int)(arg)
#define SAC_ND_PRF_TOL(arg) (long)(arg)
#define SAC_ND_PRF_TOLL(arg) (longlong) (arg)
#define SAC_ND_PRF_TOUB(arg) (ubyte) (arg)
#define SAC_ND_PRF_TOUS(arg) (ushort) (arg)
#define SAC_ND_PRF_TOUI(arg) (uint) (arg)
#define SAC_ND_PRF_TOUL(arg) (ulong) (arg)
#define SAC_ND_PRF_TOULL(arg) (ulonglong) (arg)
#define SAC_ND_PRF_TOF(arg) (float)(arg)
#define SAC_ND_PRF_TOD(arg) (double)(arg)

#define SAC_ND_PRF_NOT(arg) !(arg)
#define SAC_ND_PRF_AND(arg1, arg2) (arg1) & (arg2)
#define SAC_ND_PRF_OR(arg1, arg2) (arg1) | (arg2)

#define SAC_ND_PRF_ADD(arg1, arg2) (arg1) + (arg2)
#define SAC_ND_PRF_SUB(arg1, arg2) (arg1) - (arg2)
#define SAC_ND_PRF_MUL(arg1, arg2) (arg1) * (arg2)
#define SAC_ND_PRF_DIV(arg1, arg2) (arg1) / (arg2)
#define SAC_ND_PRF_MOD(arg1, arg2) (arg1) % (arg2)

#define SAC_ND_PRF_NEG(arg) -(arg)
#define SAC_ND_PRF_ABS(arg) ((arg) < 0) ? SAC_ND_PRF_NEG (arg) : (arg)
#define SAC_ND_PRF_MIN(arg1, arg2) (arg1) < (arg2) ? (arg1) : (arg2)
#define SAC_ND_PRF_MAX(arg1, arg2) (arg1) > (arg2) ? (arg1) : (arg2)

#define SAC_ND_PRF_EQ(arg1, arg2) (arg1) == (arg2)
#define SAC_ND_PRF_NE(arg1, arg2) (arg1) != (arg2)
#define SAC_ND_PRF_LE(arg1, arg2) (arg1) <= (arg2)
#define SAC_ND_PRF_LT(arg1, arg2) (arg1) < (arg2)
#define SAC_ND_PRF_GE(arg1, arg2) (arg1) >= (arg2)
#define SAC_ND_PRF_GT(arg1, arg2) (arg1) > (arg2)

#define SAC_ND_PRF_MESH(arg1, arg2, arg3) (arg1) ? (arg2) : (arg3)

/******************************************************************************
 *
 * ICMs for primitive functions
 * ============================
 *
 * ND_PRF_DIM_A__DATA( to_NT, to_sdim, from_NT, from_sdim)
 *
 * ND_PRF_SHAPE_A__DATA( to_NT, to_sdim, from_NT, from_sdim)
 *
 * ND_PRF_SIZE_A__DATA( to_NT, to_sdim, from_NT, from_sdim)
 *
 * ND_PRF_RESHAPE_VxA__SHAPE_id( to_NT, to_sdim, shp_NT)
 * ND_PRF_RESHAPE_VxA__SHAPE_arr( to_NT, to_sdim, shp_size, ...shp_ANY...)
 *
 * ND_PRF_SEL_VxA__SHAPE_id( to_NT, to_sdim, from_NT, from_sdim,
 *                       idx_size, idx_NT)
 * ND_PRF_SEL_VxA__SHAPE_arr( to_NT, to_sdim, from_NT, from_sdim,
 *                        idx_size, idxs_ANY)
 * ND_PRF_SEL_VxA__DATA_id( to_NT, to_sdim, from_NT, from_sdim,
 *                      idx_size, idx_NT, copyfun)
 * ND_PRF_SEL_VxA__DATA_arr( to_NT, to_sdim, from_NT, from_sdim,
 *                       idx_size, idxs_ANY, copyfun)
 *
 * ND_PRF_MODARRAY_AxVxS__DATA_id( to_NT, to_sdim, from_NT, from_sdim,
 *                           idx_size, idx_NT, val_ANY, copyfun)
 * ND_PRF_MODARRAY_AxVxS__DATA_arr( to_NT, to_sdim, from_NT, from_sdim,
 *                            idx_size, ...idxs_ANY..., val_ANY, copyfun)
 *
 * ND_PRF_MODARRAY_AxVxA__DATA_id( to_NT, to_sdim, from_NT, from_sdim,
 *                           idx_size, idx_NT, val_ANY, copyfun)
 * ND_PRF_MODARRAY_AxVxA__DATA_arr( to_NT, to_sdim, from_NT, from_sdim,
 *                            idx_size, ...idxs_ANY..., val_ANY, copyfun)
 *
 * ND_PRF_TAKE_SxV__SHAPE( to_NT, to_sdim, from_NT, from_sdim, cnt_ANY)
 * ND_PRF_TAKE_SxV__DATA( to_NT, to_sdim, from_NT, from_sdim, cnt_ANY, copyfun)
 *
 * ND_PRF_DROP_SxV__SHAPE( to_NT, to_sdim, from_NT, from_sdim, cnt_ANY)
 * ND_PRF_DROP_SxV__DATA( to_NT, to_sdim, from_NT, from_sdim, cnt_ANY, copyfun)
 *
 * ND_PRF_CAT_VxV__SHAPE( to_NT, to_sdim,
 *                    from1_NT, from1_sdim, from2_NT, from2_sdim)
 * ND_PRF_CAT_VxV__DATA( to_NT, to_sdim,
 *                   from1_NT, from1_sdim, from2_NT, from2_sdim, copyfun)
 *
 * ND_PRF_S__DATA( to_NT, op, scl)
 * ND_PRF_A__DATA( to_NT, op, scl)
 *
 * ND_PRF_SxS__DATA( to_NT, op, scl1,     scl2)
 * ND_PRF_SxV__DATA( to_NT, op, scl,      from_NT)
 * ND_PRF_VxS__DATA( to_NT, op, from_NT,  scl)
 * ND_PRF_VxV__DATA( to_NT, op, from1_NT, from2_NT)
 *
 * ND_PRF_SHAPE_IDX_SEL__DATA( to_NT, to_sdim, from_NT, from_sdim, scl)
 * ND_PRF_SHAPE_SEL__DATA( to_NT, to_sdim, from_NT, from_sdim, from2_NT)
 *
 * ND_PRF_IS_REUSED( to_NT, from_NT, from2_NT)
 *
 ******************************************************************************/

#define SAC_ND_PRF_DIM_A__DATA(to_NT, to_sdim, from_NT, from_sdim)                       \
    SAC_TR_PRF_PRINT (("ND_PRF_DIM_A__...( %s, %d, %s, %d)\n", NT_STR (to_NT), to_sdim,  \
                       NT_STR (from_NT), from_sdim))                                     \
    SAC_ND_CREATE__SCALAR__DATA (to_NT, SAC_ND_A_DIM (from_NT))

/* ND_PRF_SHAPE_A__DATA( ...) is a C-ICM */

#define SAC_ND_PRF_SIZE_A__DATA(to_NT, to_sdim, from_NT, from_sdim)                      \
    SAC_TR_PRF_PRINT (("ND_PRF_SIZE_A__...( %s, %d, %s, %d)\n", NT_STR (to_NT), to_sdim, \
                       NT_STR (from_NT), from_sdim))                                     \
    SAC_ND_CREATE__SCALAR__DATA (to_NT, SAC_ND_A_SIZE (from_NT))

/* ND_PRF_RESHAPE_VxA__SHAPE_id( ...) is a C-ICM */
/* ND_PRF_RESHAPE_VxA__SHAPE_arr( ...) is a C-ICM */

/* ND_PRF_SEL_VxA__SHAPE_id( ...) is a C-ICM */
/* ND_PRF_SEL_VxA__SHAPE_arr( ...) is a C-ICM */
/* ND_PRF_SEL_VxA__DATA_id( ...) is a C-ICM */
/* ND_PRF_SEL_VxA__DATA_arr( ...) is a C-ICM */

/* ND_PRF_MODARRAY_AxVxS__DATA_id( ...) is a C-ICM */
/* ND_PRF_MODARRAY_AxVxS__DATA_arr( ...) is a C-ICM */

/* ND_PRF_MODARRAY_AxVxA__DATA_id( ...) is a C-ICM */
/* ND_PRF_MODARRAY_AxVxA__DATA_arr( ...) is a C-ICM */

/* ND_PRF_TAKE_SxV__SHAPE( ...) is a C-ICM */
/* ND_PRF_TAKE_SxV__DATA( ...) is a C-ICM */

/* ND_PRF_DROP_SxV__SHAPE( ...) is a C-ICM */
/* ND_PRF_DROP_SxV__DATA( ...) is a C-ICM */

/* ND_PRF_CAT_VxV__SHAPE( ...) is a C-ICM */

#define SAC_ND_PRF_CAT_VxV__DATA(to_NT, to_sdim, from1_NT, from1_sdim, from2_NT,         \
                                 from2_sdim, copyfun)                                    \
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

#define SAC_ND_PRF_S__DATA(to_NT, op_macro, scl)                                         \
    SAC_TR_PRF_PRINT (                                                                   \
      ("ND_PRF_S__DATA( %s, %s, %s)\n", NT_STR (to_NT), #op_macro, #scl));               \
    SAC_ND_WRITE_COPY (to_NT, 0, op_macro (scl), );

#define SAC_ND_PRF_V__DATA(to_NT, op_macro, arg_NT)                                      \
    SAC_TR_PRF_PRINT (                                                                   \
      ("ND_PRF_V__DATA( %s, %s, %s)\n", NT_STR (to_NT), #op_macro, NT_STR (arg_NT)));    \
    {                                                                                    \
        int SAC_i;                                                                       \
        for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE (to_NT); SAC_i++) {                        \
            SAC_ND_WRITE_COPY (to_NT, SAC_i, op_macro (SAC_ND_READ (arg_NT, SAC_i)), );  \
        }                                                                                \
    }

#define SAC_ND_PRF_SxS__DATA(to_NT, op_macro, scl1, scl2)                                \
    SAC_TR_PRF_PRINT (                                                                   \
      ("ND_PRF_SxS__DATA( %s, %s, %s, %s)\n", NT_STR (to_NT), #op_macro, #scl1, #scl2)); \
    SAC_ND_WRITE_COPY (to_NT, 0, op_macro (scl1, scl2), );

#define SAC_ND_PRF_SxV__DATA(to_NT, op_macro, scl, from_NT)                              \
    SAC_TR_PRF_PRINT (("ND_PRF_SxV__DATA( %s, %s, %s, %s)\n", NT_STR (to_NT), #op_macro, \
                       #scl, NT_STR (from_NT)));                                         \
    {                                                                                    \
        int SAC_i;                                                                       \
        for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE (to_NT); SAC_i++) {                        \
            SAC_ND_WRITE_COPY (to_NT, SAC_i,                                             \
                               op_macro (scl, SAC_ND_READ (from_NT, SAC_i)), );          \
        }                                                                                \
    }

#define SAC_ND_PRF_VxS__DATA(to_NT, op_macro, from_NT, scl)                              \
    SAC_TR_PRF_PRINT (("ND_PRF_VxS__DATA( %s, %s, %s, %s)\n", NT_STR (to_NT), #op_macro, \
                       NT_STR (from_NT), #scl));                                         \
    {                                                                                    \
        int SAC_i;                                                                       \
        for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE (to_NT); SAC_i++) {                        \
            SAC_ND_WRITE_COPY (to_NT, SAC_i,                                             \
                               op_macro (SAC_ND_READ (from_NT, SAC_i), scl), );          \
        }                                                                                \
    }

#define SAC_ND_PRF_VxV__DATA(to_NT, op_macro, from1_NT, from2_NT)                        \
    SAC_TR_PRF_PRINT (("ND_PRF_VxV__DATA( %s, %s, %s, %s)\n", NT_STR (to_NT), #op_macro, \
                       NT_STR (from1_NT), NT_STR (from2_NT)));                           \
    {                                                                                    \
        int SAC_i;                                                                       \
        for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE (to_NT); SAC_i++) {                        \
            SAC_ND_WRITE_COPY (to_NT, SAC_i,                                             \
                               op_macro (SAC_ND_READ (from1_NT, SAC_i),                  \
                                         SAC_ND_READ (from2_NT, SAC_i)), );              \
        }                                                                                \
    }

#define SAC_ND_PRF_VxVxV__DATA(to_NT, op_macro, from1_NT, from2_NT, from3_NT)            \
    SAC_TR_PRF_PRINT (("ND_PRF_VxVxV__DATA( %s, %s, %s, %s)\n", NT_STR (to_NT),          \
                       #op_macro, NT_STR (from1_NT), NT_STR (from2_NT),                  \
                       NT_STR (from3_NT)));                                              \
    {                                                                                    \
        int SAC_i;                                                                       \
        for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE (to_NT); SAC_i++) {                        \
            SAC_ND_WRITE_COPY (to_NT, SAC_i,                                             \
                               op_macro (SAC_ND_READ (from1_NT, SAC_i),                  \
                                         SAC_ND_READ (from2_NT, SAC_i),                  \
                                         SAC_ND_READ (from3_NT, SAC_i)), );              \
        }                                                                                \
    }

#define SAC_ND_PRF_IS_REUSED(to_NT, from_NT, from2_NT)                                   \
    CAT17 (SAC_ND_PRF_IS_REUSED__, NT_SHP (from_NT) BuildArgs3 (to_NT, from_NT, from2_NT))

#define SAC_ND_PRF_IS_REUSED__SCL(to_NT, from_NT, from2_NT)                              \
    SAC_ND_GETVAR (to_NT, SAC_ND_A_FIELD (to_NT)) = 0;

#define SAC_ND_PRF_IS_REUSED__AKS(to_NT, from_NT, from2_NT)                              \
    SAC_ND_GETVAR (to_NT, SAC_ND_A_FIELD (to_NT))                                        \
      = (SAC_ND_GETVAR (from_NT, SAC_ND_A_FIELD (from_NT))                               \
         == SAC_ND_GETVAR (from2_NT, SAC_ND_A_FIELD (from2_NT)));

#define SAC_ND_PRF_IS_REUSED__AKD(to_NT, from_NT, from2_NT)                              \
    SAC_ND_PRF_IS_REUSED__AKS (to_NT, from_NT, from2_NT)

#define SAC_ND_PRF_IS_REUSED__AUD(to_NT, from_NT, from2_NT)                              \
    SAC_ND_PRF_IS_REUSED__AKS (to_NT, from_NT, from2_NT)

#define SAC_ND_PRF_SINGLETHREAD__DATA(to_NT, to_sdim)                                    \
    SAC_TR_PRF_PRINT (("ND_PRF_SINGLETHREAD__...( %s, %d)\n", NT_STR (to_NT), to_sdim)); \
    SAC_ND_CREATE__SCALAR__DATA (to_NT, SAC_MT_not_yet_parallel)

#define SAC_ND_PRF_GUARD(scl)                                                            \
    if (!scl)                                                                            \
        SAC_RuntimeError ("Conditions not met at guard");

#define SAC_ND_PRF_TYPE_CONSTRAINT_AKD(to_NT, from_NT, scl)                              \
    if (SAC_ND_A_DIM (from_NT) != scl)                                                   \
        SAC_RuntimeError ("Array does not adhere to type constraint");                   \
    SAC_ND_A_FIELD (to_NT) = 1;

#define SAC_ND_PRF_TYPE_CONSTRAINT_AUDGZ(to_NT, from_NT)                                 \
    if (SAC_ND_A_DIM (from_NT) == 0)                                                     \
        SAC_Runtime_Error ("Array does not adhere to type constraint");                  \
    SAC_ND_A_FIELD (to_NT) = 1;

#define SAC_ND_PRF_SHAPE_MATCHES_DIM(to_NT, from_NT, from2_NT)                           \
    if ((SAC_ND_A_DIM (from_NT) != 1)                                                    \
        || (SAC_ND_A_SHAPE (from_NT, 0) != SAC_ND_A_DIM (from2_NT))) {                   \
        SAC_RuntimeError ("Arrays do not adhere to shape matches "                       \
                          "dim constraint.");                                            \
    }

#define SAC_ND_PRF_NON_NEG_VAL_S(to_NT, from_NT)                                         \
    {                                                                                    \
        if (SAC_ND_READ (from_NT, 0) < 0)                                                \
            SAC_RuntimeError ("Non-negativity constraint violated");                     \
        SAC_ND_A_FIELD (to_NT) = 1;                                                      \
    }

#define SAC_ND_PRF_NON_NEG_VAL_V(to_NT, from_NT)                                         \
    {                                                                                    \
        int SAC_i;                                                                       \
        for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE (from_NT); SAC_i++) {                      \
            if (SAC_ND_READ (from_NT, SAC_i) < 0)                                        \
                SAC_RuntimeError ("Non-negativity constraint violated");                 \
        }                                                                                \
        SAC_ND_A_FIELD (to_NT) = 1;                                                      \
    }

#define SAC_ND_PRF_VAL_LE_VAL(to_NT, from_NT, from2_NT)                                  \
    {                                                                                    \
        int SAC_i;                                                                       \
        for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE (from_NT); SAC_i++) {                      \
            if (SAC_ND_READ (from_NT, SAC_i) > SAC_ND_READ (from2_NT, SAC_i))            \
                SAC_RuntimeError ("Constraint violated");                                \
        }                                                                                \
        SAC_ND_A_FIELD (to_NT) = 1;                                                      \
    }

#define SAC_ND_PRF_SECOND_NODESC(to_NT, second_NT)                                       \
    SAC_ND_A_FIELD (to_NT) = SAC_ND_A_FIELD (second_NT);

#define SAC_ND_PRF_SECOND_DESC(to_NT, second_NT)                                         \
    SAC_ND_PRF_SECOND_NODESC (to_NT, second_NT)                                          \
    SAC_ND_A_DESC (to_NT) = SAC_ND_A_DESC (second_NT);

#define SAC_ND_PRF_TYPE_CONV__SCL__SHP(error, left_NT, right_NT)                         \
    {                                                                                    \
        if (0 != SAC_ND_A_DIM (right_NT)) {                                              \
            SAC_ND_PRF_TYPE_CONV_ERROR (error, right_NT);                                \
        }                                                                                \
        SAC_ND_A_FIELD (left_NT) = SAC_ND_A_FIELD_UNBOX (right_NT);                      \
        SAC_ND_DEC_RC_FREE (right_NT, 1, )                                               \
    }

#define SAC_ND_PRF_TYPE_CONV__SCL__SCL(error, left_NT, right_NT)                         \
    {                                                                                    \
        SAC_ND_A_FIELD (left_NT) = SAC_ND_A_FIELD (right_NT);                            \
    }

#define SAC_ND_PRF_TYPE_CONV__AKD_START(error, left_NT, right_NT)                        \
    {                                                                                    \
        if (SAC_ND_A_DIM (left_NT) != SAC_ND_A_DIM (right_NT)) {                         \
            SAC_ND_PRF_TYPE_CONV_ERROR (error, right_NT);                                \
        }                                                                                \
        SAC_ND_A_FIELD (left_NT) = SAC_ND_A_FIELD (right_NT);                            \
        SAC_ND_A_DESC (left_NT) = SAC_ND_A_DESC (right_NT);                              \
        SAC_ND_A_MIRROR_SIZE (left_NT) = SAC_ND_A_SIZE (right_NT);

#define SAC_ND_PRF_TYPE_CONV__AKD_SHAPE(num, left_NT, right_NT)                          \
    SAC_ND_A_MIRROR_SHAPE (left_NT, num) = SAC_ND_A_SHAPE (right_NT, num);

#define SAC_ND_PRF_TYPE_CONV__AKD_END(error, left_NT, right_NT) }

#define SAC_ND_PRF_TYPE_CONV__AKS_START(error, left_NT, right_NT)                        \
    {                                                                                    \
        if (SAC_ND_A_DIM (left_NT) != SAC_ND_A_DIM (right_NT)) {                         \
            SAC_ND_PRF_TYPE_CONV_ERROR (error, right_NT);                                \
        }                                                                                \
        if (SAC_ND_A_DIM (left_NT) != SAC_ND_A_DIM (right_NT)) {                         \
            SAC_ND_PRF_TYPE_CONV_ERROR (error, right_NT);                                \
        }                                                                                \
        SAC_ND_A_FIELD (left_NT) = SAC_ND_A_FIELD (right_NT);                            \
        SAC_ND_A_DESC (left_NT) = SAC_ND_A_DESC (right_NT);                              \
  if (

#define SAC_ND_PRF_TYPE_CONV__AKS_COND(num, left_NT, right_NT)                           \
    SAC_ND_A_SHAPE (left_NT, num) != SAC_ND_A_SHAPE (right_NT, num) ||

#define SAC_ND_PRF_TYPE_CONV__AKS_END(error, left_NT, right_NT)                          \
  0)                                                                                     \
    {                                                                                    \
        SAC_ND_PRF_TYPE_CONV_ERROR (error, right_NT);                                    \
    }                                                                                    \
    SAC_ND_A_FIELD (left_NT) = SAC_ND_A_FIELD (right_NT);                                \
    SAC_ND_A_DESC (left_NT) = SAC_ND_A_DESC (right_NT);                                  \
    }

#define SAC_ND_PRF_TYPE_CONV__AUD__SHP(error, left_NT, right_NT)                         \
    {                                                                                    \
        if (0 == SAC_ND_A_DIM (right_NT)) {                                              \
            SAC_ND_PRF_TYPE_CONV_ERROR (error, right_NT);                                \
        }                                                                                \
        SAC_ND_A_FIELD (left_NT) = SAC_ND_A_FIELD (right_NT);                            \
        SAC_ND_A_DESC (left_NT) = SAC_ND_A_DESC (right_NT);                              \
        SAC_ND_A_MIRROR_SIZE (left_NT) = SAC_ND_A_SIZE (right_NT);                       \
        SAC_ND_A_MIRROR_DIM (left_NT) = SAC_ND_A_DIM (right_NT);                         \
    }

/* [*] -> [+] */

#define SAC_ND_PRF_TYPE_CONV__AUD__AUD(error, left_NT, right_NT)                         \
    {                                                                                    \
        SAC_ND_A_FIELD (left_NT) = SAC_ND_A_FIELD (right_NT);                            \
        SAC_ND_A_DESC (left_NT) = SAC_ND_A_DESC (right_NT);                              \
        SAC_ND_A_MIRROR_SIZE (left_NT) = SAC_ND_A_SIZE (right_NT);                       \
        SAC_ND_A_MIRROR_DIM (left_NT) = SAC_ND_A_DIM (right_NT);                         \
    }

#define SAC_ND_PRF_TYPE_CONV_ERROR_NAME(name, tuple) #name

#define SAC_ND_PRF_TYPE_CONV_ERROR(error, right_NT)                                      \
    {                                                                                    \
        SAC_Print (" *");                                                                \
        SAC_ND_PRINT_SHAPE (right_NT);                                                   \
        SAC_Print (" ");                                                                 \
        SAC_Print (SAC_ND_PRF_TYPE_CONV_ERROR_NAME right_NT);                            \
        SAC_RuntimeError (error);                                                        \
    }

#endif /* _SAC_PRF_H_ */
