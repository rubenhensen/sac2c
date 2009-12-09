/*
 * $Id$
 *
 */

/*
 * CAUTION: 
 *
 * mutc_gen.h  is generated automatically from mutc_gen.h.m4
 *
 */

/*
 * See ../m4/README
 */

include(icm.m4)

start_icm_definition(mutc_gen)

/*
 * Non SCL are pointers there for they are INTs
 */

pat(`SAC_MUTC_ARG_THREAD', `1', `1', `NT_REG', `NT_SCO', `NT_SHP')
rule(`SAC_MUTC_ARG_THREAD', `SAC_MUTC_ND_ARG_INT_GLO', `INT', `GLO', `*SHP')
rule(`SAC_MUTC_ARG_THREAD', `SAC_MUTC_ND_ARG_FLO_GLO', `FLO', `GLO', `SCL')
rule(`SAC_MUTC_ARG_THREAD', `SAC_MUTC_ND_ARG_INT_GLO', `FLO', `GLO', `*SHP')
rule(`SAC_MUTC_ARG_THREAD', `SAC_MUTC_ND_ARG_INT_SHA', `INT', `SHA', `*SHP')
rule(`SAC_MUTC_ARG_THREAD', `SAC_MUTC_ND_ARG_FLO_SHA', `FLO', `SHA', `SCL')
rule(`SAC_MUTC_ARG_THREAD', `SAC_MUTC_ND_ARG_INT_SHA', `FLO', `SHA', `*SHP')

pat(`SAC_MUTC_ARG', `1', `1', `NT_USG')
rule(`SAC_MUTC_ARG', `SAC_MUTC_ARG_FUN', `FAG')
rule(`SAC_MUTC_ARG', `SAC_MUTC_ARG_FUN', `FPA')
rule(`SAC_MUTC_ARG', `SAC_MUTC_ARG_FUN', `FTA')
rule(`SAC_MUTC_ARG', `SAC_MUTC_ARG_THREAD', `*USG')

pat(`SAC_MUTC_ARG_SHARED', `0', `1', `NT_SHP', `NT_HID', `NT_UNQ', `NT_REG', `NT_SHP')
rule(`SAC_MUTC_ARG_SHARED', `SAC_MUTC_ARG_SHARED_NODESC_INT', `SCL', `NHD', `*UNQ', `INT', `*SHP')
rule(`SAC_MUTC_ARG_SHARED', `SAC_MUTC_ARG_SHARED_DESC_FLO', `SCL', `HID', `UNQ', `FLO', `SCL')
rule(`SAC_MUTC_ARG_SHARED', `SAC_MUTC_ARG_SHARED_DESC_INT', `SCL', `HID', `UNQ', `FLO', `*SHP')
rule(`SAC_MUTC_ARG_SHARED', `SAC_MUTC_ARG_SHARED_NODESC_INT', `*SHP', `*HID', `*UNQ', `INT', `*SHP')
rule(`SAC_MUTC_ARG_SHARED', `SAC_MUTC_ARG_SHARED_NODESC_FLO', `*SHP', `*HID', `*UNQ', `FLO', `SCL')
rule(`SAC_MUTC_ARG_SHARED', `SAC_MUTC_ARG_SHARED_NODESC_INT', `*SHP', `*HID', `*UNQ', `FLO', `*SHP')

pat(`SAC_MUTC_PARAM', `2', `0', `NT_USG')
rule(`SAC_MUTC_PARAM', `SAC_MUTC_PARAM_THREAD', `TPA')
rule(`SAC_MUTC_PARAM', `SAC_MUTC_PARAM_THREAD', `TPM')
rule(`SAC_MUTC_PARAM', `SAC_MUTC_PARAM_THREAD', `TPO')
rule(`SAC_MUTC_PARAM', `SAC_MUTC_PARAM_FUN', `*USG')

pat(`SAC_MUTC_PARAM_THREAD', `2', `0', `NT_REG', `NT_SCO', `NT_SHP')
rule(`SAC_MUTC_PARAM_THREAD', `SAC_MUTC_ND_PARAM_INT_GLO', `INT', `GLO', `*SHP')
rule(`SAC_MUTC_PARAM_THREAD', `SAC_MUTC_ND_PARAM_FLO_GLO', `FLO', `GLO', `SCL')
rule(`SAC_MUTC_PARAM_THREAD', `SAC_MUTC_ND_PARAM_INT_GLO', `FLO', `GLO', `*SHP')
rule(`SAC_MUTC_PARAM_THREAD', `SAC_MUTC_ND_PARAM_INT_SHA', `INT', `SHA', `*SHP')
rule(`SAC_MUTC_PARAM_THREAD', `SAC_MUTC_ND_PARAM_FLO_SHA', `FLO', `SHA', `SCL')
rule(`SAC_MUTC_PARAM_THREAD', `SAC_MUTC_ND_PARAM_INT_SHA', `FLO', `SHA', `*SHP')

pat(`SAC_ND_PRF_SYNCIN', `1', `0',  `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_PRF_SYNCIN', `SAC_ND_PRF_SYNCIN_NODESC', `SCL', `NHD', `*UNQ')
rule(`SAC_ND_PRF_SYNCIN', `SAC_ND_PRF_SYNCIN_NODESC', `SCL', `HID', `UNQ')
rule(`SAC_ND_PRF_SYNCIN', `SAC_ND_PRF_SYNCIN_DESC', `*SHP', `*HID', `*UNQ')

pat(`SAC_ND_PRF_SYNCOUT', `1', `0',  `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_PRF_SYNCOUT', `SAC_ND_PRF_SYNCOUT_NODESC', `SCL', `NHD', `*UNQ')
rule(`SAC_ND_PRF_SYNCOUT', `SAC_ND_PRF_SYNCOUT_NODESC', `SCL', `HID', `UNQ')
rule(`SAC_ND_PRF_SYNCOUT', `SAC_ND_PRF_SYNCOUT_DESC', `*SHP', `*HID', `*UNQ')

pat(`SAC_MUTC_LOCAL_ALLOC__DESC', `0', `1', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_MUTC_LOCAL_ALLOC__DESC', `SAC_MUTC_LOCAL_ALLOC__DESC__NOOP', `SCL', `NHD', `*UNQ')
rule(`SAC_MUTC_LOCAL_ALLOC__DESC', `SAC_MUTC_LOCAL_ALLOC__DESC__NOOP', `SCL', `HID', `UNQ')
rule(`SAC_MUTC_LOCAL_ALLOC__DESC', `SAC_MUTC_LOCAL_ALLOC__DESC__AUD', `AUD', `*HID', `*UNQ')
rule(`SAC_MUTC_LOCAL_ALLOC__DESC', `SAC_MUTC_LOCAL_ALLOC__DESC__FIXED', `*SHP', `*HID', `*UNQ')

#define MUTC 1
#if SAC_BACKEND == MUTC

#define SAC_MUTC_C_GETVAR( ...) SAC_ND_GETVAR( __VA_ARGS__)
#undef SAC_ND_GETVAR
pat(`SAC_ND_GETVAR', `0', `1', `NT_USG')
rule(`SAC_ND_GETVAR', `SAC_MUTC_GETFUNPAR', `FPA')
rule(`SAC_ND_GETVAR', `SAC_MUTC_GETFUNPAR', `FPM')
rule(`SAC_ND_GETVAR', `SAC_MUTC_GETTHREADPAR', `FTA')
rule(`SAC_ND_GETVAR', `SAC_MUTC_GETTHREADPAR', `TPA')
rule(`SAC_ND_GETVAR', `SAC_MUTC_GETTHREADPAR', `TPM')
rule(`SAC_ND_GETVAR', `SAC_MUTC_GETVAR', `*USG')

#undef SAC_ND_GETVAR_INOUT
pat(`SAC_ND_GETVAR_INOUT', `0', `1', `NT_USG')
rule(`SAC_ND_GETVAR_INOUT', `SAC_MUTC_GETFUNPAR', `FPA')
rule(`SAC_ND_GETVAR_INOUT', `SAC_MUTC_GETFUNPAR', `FPM')
rule(`SAC_ND_GETVAR_INOUT', `SAC_MUTC_GETFUNPAR', `FPO')
rule(`SAC_ND_GETVAR_INOUT', `SAC_MUTC_GETFUNPAR', `FTA')
rule(`SAC_ND_GETVAR_INOUT', `SAC_MUTC_GETTHREADPAR', `TPA')
rule(`SAC_ND_GETVAR_INOUT', `SAC_MUTC_GETTHREADPAR', `TPM')
rule(`SAC_ND_GETVAR_INOUT', `SAC_MUTC_GETTHREADPAR', `TPO')
rule(`SAC_ND_GETVAR_INOUT', `SAC_MUTC_GETVAR', `*USG')

#endif /* SAC_BACKEND */
#undef MUTC
end_icm_definition
