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

pat(`SAC_MUTC_INIT_SUBALLOC_DESC', `0', `0', `NT_SHP')
rule(`SAC_MUTC_INIT_SUBALLOC_DESC', `SAC_NOP', `SCL')
rule(`SAC_MUTC_INIT_SUBALLOC_DESC', `SAC_NOP', `AKS')
rule(`SAC_MUTC_INIT_SUBALLOC_DESC', `SAC_MUTC_INIT_SUBALLOC_DESC_DO', `*SHP')

pat(`SAC_MUTC_ARG_THREAD', `1', `1', `NT_REG', `NT_SCO', `NT_SHP')
rule(`SAC_MUTC_ARG_THREAD', `SAC_MUTC_ND_ARG_INT_GLO', `INT', `GLO', `*SHP')
rule(`SAC_MUTC_ARG_THREAD', `SAC_MUTC_ND_ARG_FLO_GLO', `FLO', `GLO', `SCL')
rule(`SAC_MUTC_ARG_THREAD', `SAC_MUTC_ND_ARG_INT_GLO', `FLO', `GLO', `*SHP')
rule(`SAC_MUTC_ARG_THREAD', `SAC_MUTC_ND_ARG_INT_SHA', `INT', `SHA', `*SHP')
rule(`SAC_MUTC_ARG_THREAD', `SAC_MUTC_ND_ARG_FLO_SHA', `FLO', `SHA', `SCL')
rule(`SAC_MUTC_ARG_THREAD', `SAC_MUTC_ND_ARG_INT_SHA', `FLO', `SHA', `*SHP')

pat(`SAC_MUTC_ARG_THREAD_out', `1', `1', `NT_SCO')
rule(`SAC_MUTC_ARG_THREAD_out', `SAC_MUTC_ND_ARG_INT_GLO', `GLO')
rule(`SAC_MUTC_ARG_THREAD_out', `SAC_MUTC_ND_ARG_INT_SHA', `SHA')

pat(`SAC_MUTC_ARG', `1', `1', `NT_USG')
rule(`SAC_MUTC_ARG', `SAC_MUTC_ARG_FUN', `FAG')
rule(`SAC_MUTC_ARG', `SAC_MUTC_ARG_FUN', `FPA')
rule(`SAC_MUTC_ARG', `SAC_MUTC_ARG_FUN', `FTA')
rule(`SAC_MUTC_ARG', `SAC_MUTC_ARG_THREAD', `*USG')

pat(`SAC_MUTC_ARG_out', `1', `1', `NT_USG')
rule(`SAC_MUTC_ARG_out', `SAC_MUTC_ARG_FUN', `FAG')
rule(`SAC_MUTC_ARG_out', `SAC_MUTC_ARG_FUN', `FPA')
rule(`SAC_MUTC_ARG_out', `SAC_MUTC_ARG_FUN', `FTA')
rule(`SAC_MUTC_ARG_out', `SAC_MUTC_ARG_THREAD_out', `*USG')

pat(`SAC_MUTC_ARG_SHARED', `0', `1', `NT_SHP', `NT_HID', `NT_UNQ', `NT_REG')
rule(`SAC_MUTC_ARG_SHARED', `SAC_MUTC_ARG_SHARED_NODESC_INT', `SCL', `NHD', `*UNQ', `INT')
rule(`SAC_MUTC_ARG_SHARED', `SAC_MUTC_ARG_SHARED_DESC_INT', `SCL', `HID', `NUQ', `INT')
rule(`SAC_MUTC_ARG_SHARED', `SAC_MUTC_ARG_SHARED_NODESC_INT', `SCL', `HID', `UNQ', `INT')
rule(`SAC_MUTC_ARG_SHARED', `SAC_MUTC_ARG_SHARED_NODESC_INT', `SCL', `*HID', `UNQ', `INT')
rule(`SAC_MUTC_ARG_SHARED', `SAC_MUTC_ARG_SHARED_NODESC_INT', `AKS', `*HID', `UNQ', `INT')
rule(`SAC_MUTC_ARG_SHARED', `SAC_MUTC_ARG_SHARED_DESC_INT', `*SHP', `*HID', `*UNQ', `INT')
rule(`SAC_MUTC_ARG_SHARED', `SAC_MUTC_ARG_SHARED_NODESC_FLO', `SCL', `NHD', `*UNQ', `FLO')
rule(`SAC_MUTC_ARG_SHARED', `SAC_MUTC_ARG_SHARED_DESC_INT', `SCL', `HID', `NUQ', `FLO')
rule(`SAC_MUTC_ARG_SHARED', `SAC_MUTC_ARG_SHARED_NODESC_INT', `SCL', `HID', `UNQ', `FLO')
rule(`SAC_MUTC_ARG_SHARED', `SAC_MUTC_ARG_SHARED_NODESC_INT', `SCL', `*HID', `UNQ', `FLO')
rule(`SAC_MUTC_ARG_SHARED', `SAC_MUTC_ARG_SHARED_NODESC_INT', `AKS', `*HID', `UNQ', `FLO')
rule(`SAC_MUTC_ARG_SHARED', `SAC_MUTC_ARG_SHARED_DESC_INT', `*SHP', `*HID', `*UNQ', `FLO')

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
rule(`SAC_ND_PRF_SYNCIN', `SAC_ND_PRF_SYNCIN_NODESC', `SCL', `*HID', `UNQ')
rule(`SAC_ND_PRF_SYNCIN', `SAC_ND_PRF_SYNCIN_NODESC', `AKS', `*HID', `UNQ')
rule(`SAC_ND_PRF_SYNCIN', `SAC_ND_PRF_SYNCIN_DESC', `*SHP', `*HID', `*UNQ')

pat(`SAC_ND_PRF_SYNCOUT', `1', `0',  `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_PRF_SYNCOUT', `SAC_ND_PRF_SYNCOUT_NODESC', `SCL', `NHD', `*UNQ')
rule(`SAC_ND_PRF_SYNCOUT', `SAC_ND_PRF_SYNCOUT_NODESC', `SCL', `HID', `UNQ')
rule(`SAC_ND_PRF_SYNCOUT', `SAC_ND_PRF_SYNCOUT_NODESC', `SCL', `*HID', `UNQ')
rule(`SAC_ND_PRF_SYNCOUT', `SAC_ND_PRF_SYNCOUT_NODESC', `AKS', `*HID', `UNQ')
rule(`SAC_ND_PRF_SYNCOUT', `SAC_ND_PRF_SYNCOUT_DESC', `*SHP', `*HID', `*UNQ')

pat(`SAC_MUTC_SAVE', `0', `0',  `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_MUTC_SAVE', `SAC_MUTC_SAVE_NODESC', `SCL', `NHD', `*UNQ')
rule(`SAC_MUTC_SAVE', `SAC_MUTC_SAVE_DESC', `SCL', `HID', `UNQ')
rule(`SAC_MUTC_SAVE', `SAC_MUTC_SAVE_DESC', `SCL', `*HID', `UNQ')
rule(`SAC_MUTC_SAVE', `SAC_MUTC_SAVE_DESC', `AKS', `*HID', `UNQ')
rule(`SAC_MUTC_SAVE', `SAC_MUTC_SAVE_DESC', `*SHP', `*HID', `*UNQ')

pat(`SAC_MUTC_LOCAL_ALLOC__DESC', `0', `1', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_MUTC_LOCAL_ALLOC__DESC', `SAC_MUTC_LOCAL_ALLOC__DESC__NOOP', `SCL', `NHD', `*UNQ')
rule(`SAC_MUTC_LOCAL_ALLOC__DESC', `SAC_MUTC_LOCAL_ALLOC__DESC__NOOP', `SCL', `HID', `UNQ')
rule(`SAC_MUTC_LOCAL_ALLOC__DESC', `SAC_MUTC_LOCAL_ALLOC__DESC__NOOP', `SCL', `*HID', `UNQ')
rule(`SAC_MUTC_LOCAL_ALLOC__DESC', `SAC_MUTC_LOCAL_ALLOC__DESC__NOOP', `AKS', `*HID', `UNQ')
rule(`SAC_MUTC_LOCAL_ALLOC__DESC', `SAC_MUTC_LOCAL_ALLOC__DESC__AUD', `AUD', `*HID', `*UNQ')
rule(`SAC_MUTC_LOCAL_ALLOC__DESC', `SAC_MUTC_LOCAL_ALLOC__DESC__FIXED', `*SHP', `*HID', `*UNQ')

#define MUTC 1
#ifdef SAC_BACKEND_MUTC

#define SAC_MUTC_C_GETVAR( ...) SAC_ND_GETVAR( __VA_ARGS__)
pat(`SAC_ND_GETVAR', `0', `1', `NT_USG')
rule(`SAC_ND_GETVAR', `SAC_MUTC_GETFUNPAR', `FPA')
rule(`SAC_ND_GETVAR', `SAC_MUTC_GETFUNPAR', `FPM')
rule(`SAC_ND_GETVAR', `SAC_MUTC_GETTHREADPAR', `FTA')
rule(`SAC_ND_GETVAR', `SAC_MUTC_GETTHREADPAR', `TPA')
rule(`SAC_ND_GETVAR', `SAC_MUTC_GETTHREADPAR', `TPM')
rule(`SAC_ND_GETVAR', `SAC_MUTC_GETVAR', `*USG')

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
