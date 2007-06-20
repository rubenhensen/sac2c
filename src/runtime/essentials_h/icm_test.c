/*
 * $Id$
 *
 */

/*
 * This file certainly needs some tweaking to work again.
 * Nevertheless, it is left as is as a starting point.
 */

/**
 **  This file can be used to test the expansion of H-ICMs
 **
 **  Apply the CC preprocessor to this file and verify the generated output:
 **    gcc -I$SACHOME/runtime -E icm_test.C > tutu
 **    grep CAT tutu
 **    grep NT tutu
 **
 **  It exists a Makefile target for this purpose (icm_test) !!!
 **/

#ifndef _SHP_
#define _SHP_ SCL /* SCL, AKS, AKD, AUD */
#endif

#ifndef _HID_
#define _HID_ HID /* NHD, HID */
#endif

#ifndef _UNQ_
#define _UNQ_ NUQ /* NUQ, UNQ */
#endif

#define SAC_DO_CHECK 1
#define SAC_DO_CHECK_TYPE 1
#define SAC_DO_CHECK_BOUNDARY 1
#define SAC_DO_CHECK_MALLOC 1
#define SAC_DO_CHECK_ERRNO 1
#define SAC_DO_CHECK_HEAP 1

#define SAC_DO_PHM 1
#define SAC_DO_APS 1
#define SAC_DO_DAO 1
#define SAC_DO_MSCA 1

#define SAC_DO_PROFILE 1
#define SAC_DO_PROFILE_WITH 1
#define SAC_DO_PROFILE_FUN 1
#define SAC_DO_PROFILE_INL 1
#define SAC_DO_PROFILE_LIB 1

#define SAC_DO_TRACE 1
#define SAC_DO_TRACE_REF 1
#define SAC_DO_TRACE_MEM 1
#define SAC_DO_TRACE_PRF 1
#define SAC_DO_TRACE_FUN 1
#define SAC_DO_TRACE_WL 1
#define SAC_DO_TRACE_AA 1
#define SAC_DO_TRACE_MT 1

#define SAC_DO_CACHESIM 0
#define SAC_DO_CACHESIM_ADV 0
#define SAC_DO_CACHESIM_GLOBAL 1
#define SAC_DO_CACHESIM_FILE 0
#define SAC_DO_CACHESIM_PIPE 1
#define SAC_DO_CACHESIM_IMDT 0

#define SAC_DO_MULTITHREAD 1
#define SAC_DO_THREADS_STATIC 1

#define SAC_DO_COMPILE_MODULE 0

#include "sac.h"

#define CHECK(x) < ---#x--->x

CHECK (SAC_ND_A_DESC ((nt, (_SHP_, (_HID_, (_UNQ_, ))))))
CHECK (SAC_ND_A_DESC_DIM ((nt, (_SHP_, (_HID_, (_UNQ_, ))))))
CHECK (SAC_ND_A_DESC_SIZE ((nt, (_SHP_, (_HID_, (_UNQ_, ))))))
CHECK (SAC_ND_A_DESC_SHAPE ((nt, (_SHP_, (_HID_, (_UNQ_, )))), 0))
CHECK (SAC_ND_A_MIRROR_DIM ((nt, (_SHP_, (_HID_, (_UNQ_, ))))))
CHECK (SAC_ND_A_MIRROR_SIZE ((nt, (_SHP_, (_HID_, (_UNQ_, ))))))
CHECK (SAC_ND_A_MIRROR_SHAPE ((nt, (_SHP_, (_HID_, (_UNQ_, )))), 0))
CHECK (SAC_ND_A_FIELD ((nt, (_SHP_, (_HID_, (_UNQ_, ))))))
CHECK (SAC_ND_A_RC ((nt, (_SHP_, (_HID_, (_UNQ_, ))))))
CHECK (SAC_ND_A_DIM ((nt, (_SHP_, (_HID_, (_UNQ_, ))))))
CHECK (SAC_ND_A_SIZE ((nt, (_SHP_, (_HID_, (_UNQ_, ))))))
CHECK (SAC_ND_A_SHAPE ((nt, (_SHP_, (_HID_, (_UNQ_, )))), 0))

CHECK (SAC_ND_READ ((nt, (_SHP_, (_HID_, (_UNQ_, )))), 0))
CHECK (SAC_ND_WRITE ((nt, (_SHP_, (_HID_, (_UNQ_, )))), 0))
CHECK (SAC_ND_WRITE_READ ((to_nt, (_SHP_, (_HID_, (_UNQ_, )))), 0,
                          (from_nt, (_SHP_, (_HID_, (_UNQ_, )))), 1))
CHECK (SAC_ND_WRITE_COPY ((nt, (_SHP_, (_HID_, (_UNQ_, )))), 0, expr, copyfun))
CHECK (SAC_ND_WRITE_READ_COPY ((to_nt, (_SHP_, (_HID_, (_UNQ_, )))), 0,
                               (from_nt, (_SHP_, (_HID_, (_UNQ_, )))), 1, copyfun))

CHECK (SAC_ND_DESC_TYPE ((nt, (_SHP_, (_HID_, (_UNQ_, ))))))
CHECL (SAC_ND_TYPE_NT ((int, (_SHP_, (_HID_, (_UNQ_, ))))))
CHECL (SAC_ND_TYPE ((nt, (_SHP_, (_HID_, (_UNQ_, )))), int))

CHECK (SAC_ND_TYPEDEF ((nt, (_SHP_, (_HID_, (_UNQ_, )))), int))

CHECK (SAC_ND_DECL__DESC ((nt, (_SHP_, (_HID_, (_UNQ_, )))), ))
CHECK (SAC_ND_DECL__DATA ((nt, (_SHP_, (_HID_, (_UNQ_, )))), int, ))

CHECK (SAC_ND_PARAM_ (dummy, int))
CHECK (SAC_ND_PARAM_in ((nt, (_SHP_, (_HID_, (_UNQ_, )))), int))
CHECK (SAC_ND_PARAM_in_nodesc ((nt, (_SHP_, (_HID_, (_UNQ_, )))), int))
CHECK (SAC_ND_PARAM_out ((nt, (_SHP_, (_HID_, (_UNQ_, )))), int))
CHECK (SAC_ND_PARAM_out_nodesc ((nt, (_SHP_, (_HID_, (_UNQ_, )))), int))
CHECK (SAC_ND_PARAM_inout ((nt, (_SHP_, (_HID_, (_UNQ_, )))), int))
CHECK (SAC_ND_PARAM_inout_nodesc ((nt, (_SHP_, (_HID_, (_UNQ_, )))), int))
CHECK (SAC_ND_PARAM_inout_nodesc_bx ((nt, (_SHP_, (_HID_, (_UNQ_, )))), int))
CHECK (SAC_ND_ARG_in ((nt, (_SHP_, (_HID_, (_UNQ_, ))))))
CHECK (SAC_ND_ARG_in_nodesc ((nt, (_SHP_, (_HID_, (_UNQ_, ))))))
CHECK (SAC_ND_ARG_out ((nt, (_SHP_, (_HID_, (_UNQ_, ))))))
CHECK (SAC_ND_ARG_out_nodesc ((nt, (_SHP_, (_HID_, (_UNQ_, ))))))
CHECK (SAC_ND_ARG_inout ((nt, (_SHP_, (_HID_, (_UNQ_, ))))))
CHECK (SAC_ND_ARG_inout_nodesc ((nt, (_SHP_, (_HID_, (_UNQ_, ))))))
CHECK (SAC_ND_ARG_inout_nodesc_bx ((nt, (_SHP_, (_HID_, (_UNQ_, ))))))
CHECK (SAC_ND_RET_out ((nt, (_SHP_, (_HID_, (_UNQ_, )))),
                       (ntp, (_SHP_, (_HID_, (_UNQ_, ))))))
CHECK (SAC_ND_RET_inout ((nt, (_SHP_, (_HID_, (_UNQ_, )))),
                         (ntp, (_SHP_, (_HID_, (_UNQ_, ))))))
CHECK (SAC_ND_DECL_PARAM_inout ((nt, (_SHP_, (_HID_, (_UNQ_, )))), int))

CHECK (SAC_ND_ALLOC ((nt, (_SHP_, (_HID_, (_UNQ_, )))), rc, get_dim, set_shape_icm))
CHECK (SAC_ND_ALLOC_BEGIN ((nt, (_SHP_, (_HID_, (_UNQ_, )))), rc, dim))
CHECK (SAC_ND_ALLOC_END ((nt, (_SHP_, (_HID_, (_UNQ_, )))), rc, dim))
CHECK (SAC_ND_ALLOC__DESC ((nt, (_SHP_, (_HID_, (_UNQ_, )))), dim))
CHECK (SAC_ND_ALLOC__DATA ((nt, (_SHP_, (_HID_, (_UNQ_, ))))))
CHECK (SAC_ND_ALLOC__DESC_AND_DATA ((nt, (_SHP_, (_HID_, (_UNQ_, )))), dim))

CHECK (SAC_ND_FREE ((nt, (_SHP_, (_HID_, (_UNQ_, )))), freefun))
CHECK (SAC_ND_FREE__DESC ((nt, (_SHP_, (_HID_, (_UNQ_, ))))))
CHECK (SAC_ND_FREE__DATA ((nt, (_SHP_, (_HID_, (_UNQ_, )))), freefun))

CHECK (SAC_ND_ASSIGN__DATA ((to_nt, (_SHP_, (_HID_, (_UNQ_, )))),
                            (from_nt, (_SHP_, (_HID_, (_UNQ_, )))), copyfun))
CHECK (SAC_ND_COPY__DATA ((to_nt, (_SHP_, (_HID_, (_UNQ_, )))),
                          (from_nt, (_SHP_, (_HID_, (_UNQ_, )))), copyfun))

CHECK (SAC_ND_CREATE__SCALAR__DATA ((nt, (_SHP_, (_HID_, (_UNQ_, )))), 17))
CHECK (SAC_ND_CREATE__STRING__DATA ((nt, (_SHP_, (_HID_, (_UNQ_, )))), "hallo"))

CHECK (SAC_ND_SET__RC ((nt, (_SHP_, (_HID_, (_UNQ_, )))), rc))
CHECK (SAC_ND_INC_RC ((nt, (_SHP_, (_HID_, (_UNQ_, )))), rc))
CHECK (SAC_ND_DEC_RC ((nt, (_SHP_, (_HID_, (_UNQ_, )))), rc))
CHECK (SAC_ND_DEC_RC_FREE ((nt, (_SHP_, (_HID_, (_UNQ_, )))), rc, freefun))

CHECK (SAC_IS_LASTREF__BLOCK_BEGIN ((to_nt, (_SHP_, (_HID_, (_UNQ_, ))))))
CHECK (SAC_IS_LASTREF__BLOCK_ELSE ((to_nt, (_SHP_, (_HID_, (_UNQ_, ))))))
CHECK (SAC_IS_LASTREF__BLOCK_END ((to_nt, (_SHP_, (_HID_, (_UNQ_, ))))))

CHECK (SAC_IS_REUSED__BLOCK_BEGIN ((to_nt, (_SHP_, (_HID_, (_UNQ_, )))),
                                   (from_nt, (_SHP_, (_HID_, (_UNQ_, ))))))
CHECK (SAC_IS_REUSED__BLOCK_ELSE ((to_nt, (_SHP_, (_HID_, (_UNQ_, )))),
                                  (from_nt, (_SHP_, (_HID_, (_UNQ_, ))))))
CHECK (SAC_IS_REUSED__BLOCK_END ((to_nt, (_SHP_, (_HID_, (_UNQ_, )))),
                                 (from_nt, (_SHP_, (_HID_, (_UNQ_, ))))))

CHECK (SAC_INITGLOBALOBJECT_BEGIN (varname))
CHECK (SAC_INITGLOBALOBJECT_END ())

CHECK (SAC_ND_UNIOP (op, arg))
CHECK (SAC_ND_BINOP (op, arg1, arg2))

CHECK (SAC_PRF_UNIOP (op, arg))
CHECK (SAC_PRF_NEG (dummy, arg))
CHECK (SAC_PRF_ABS (dummy, arg))
CHECK (SAC_PRF_BINOP (op, arg1, arg2))
CHECK (SAC_PRF_MIN (dummy, arg1, arg2))
CHECK (SAC_PRF_MAX (dummy, arg1, arg2))

CHECK (SAC_ND_PRF_DIM__DATA ((to_nt, (_SHP_, (_HID_, (_UNQ_, )))), 1,
                             (from1_nt, (_SHP_, (_HID_, (_UNQ_, )))), 1))
CHECK (SAC_ND_PRF_CAT__DATA ((to_nt, (_SHP_, (_HID_, (_UNQ_, )))), 1,
                             (from1_nt, (_SHP_, (_HID_, (_UNQ_, )))), 1,
                             (from2_nt, (_SHP_, (_HID_, (_UNQ_, )))), 1, copyfun))
CHECK (SAC_ND_PRF_CONV_A__DATA ((to_nt, (_SHP_, (_HID_, (_UNQ_, )))),
                                (from1_nt, (_SHP_, (_HID_, (_UNQ_, ))))))
CHECK (SAC_ND_PRF_S__DATA ((to_nt, (_SHP_, (_HID_, (_UNQ_, )))), op_macro, op, scl))
CHECK (SAC_ND_PRF_A__DATA ((to_nt, (_SHP_, (_HID_, (_UNQ_, )))), op_macro, op,
                           (arg_nt, (_SHP_, (_HID_, (_UNQ_, ))))))
CHECK (SAC_ND_PRF_SxS__DATA ((to_nt, (_SHP_, (_HID_, (_UNQ_, )))), op_macro, op, scl1,
                             scl2))
CHECK (SAC_ND_PRF_SxV__DATA ((to_nt, (_SHP_, (_HID_, (_UNQ_, )))), op_macro, op, scl,
                             (from_nt, (_SHP_, (_HID_, (_UNQ_, ))))))
CHECK (SAC_ND_PRF_VxS__DATA ((to_nt, (_SHP_, (_HID_, (_UNQ_, )))), op_macro, op,
                             (from_nt, (_SHP_, (_HID_, (_UNQ_, )))), scl))
CHECK (SAC_ND_PRF_VxV__DATA ((to_nt, (_SHP_, (_HID_, (_UNQ_, )))), op_macro, op,
                             (from1_nt, (_SHP_, (_HID_, (_UNQ_, )))),
                             (from2_nt, (_SHP_, (_HID_, (_UNQ_, ))))))

CHECK (SAC_MT_SET_BARRIER_RESULT (n, m, basetype, (res_nt, (_SHP_, (_HID_, (_UNQ_, ))))))
CHECK (SAC_MT_GET_BARRIER_RESULT (n, m, basetype, (res_nt, (_SHP_, (_HID_, (_UNQ_, ))))))

CHECK (SAC_MT_DECL_LOCAL_DESC ((nt, (_SHP_, (_HID_, (_UNQ_, )))), dim))
CHECK (SAC_MT_FREE_LOCAL_DESC ((nt, (_SHP_, (_HID_, (_UNQ_, )))), dim))

CHECK (SAC_MT_SPMD_ARG_in (int, (nt, (_SHP_, (_HID_, (_UNQ_, ))))))
CHECK (SAC_MT_SPMD_ARG_out (int, (nt, (_SHP_, (_HID_, (_UNQ_, ))))))
CHECK (SAC_MT_SPMD_ARG_inout (int, (nt, (_SHP_, (_HID_, (_UNQ_, ))))))
CHECK (SAC_MT_SPMD_ARG_shared (int, (nt, (_SHP_, (_HID_, (_UNQ_, ))))))
CHECK (SAC_MT_SPMD_ARG_preset (int, (nt, (_SHP_, (_HID_, (_UNQ_, ))))))

CHECK (SAC_MT_SPMD_PARAM_in (int, (nt, (_SHP_, (_HID_, (_UNQ_, ))))))
CHECK (SAC_MT_SPMD_PARAM_out (int, (nt, (_SHP_, (_HID_, (_UNQ_, ))))))

CHECK (SAC_MT_SPMD_SETARG_in (spmdname, (nt, (_SHP_, (_HID_, (_UNQ_, ))))))
CHECK (SAC_MT_SPMD_SETARG_out (spmdname, (nt, (_SHP_, (_HID_, (_UNQ_, ))))))
CHECK (SAC_MT_SPMD_SETARG_shared (spmdname, (nt, (_SHP_, (_HID_, (_UNQ_, ))))))

CHECK (SAC_MT_SPMD_RET_out ((nt, (_SHP_, (_HID_, (_UNQ_, ))))))
CHECK (SAC_MT_SPMD_RET_shared ((nt, (_SHP_, (_HID_, (_UNQ_, ))))))

CHECK (SAC_MT_SPMD_GET_shared ((nt, (_SHP_, (_HID_, (_UNQ_, ))))))
