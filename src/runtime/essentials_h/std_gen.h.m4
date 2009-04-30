








/*
 * $Id$
 *
 */

/*
 * CAUTION: 
 *
 * std_gen.h  is generated automatically from std_gen.h.m4
 *
 */

include(`icm.m4')

start_icm_definition(std_gen)


pat(`SAC_ND_A_DESC', `0', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_A_DESC', `SAC_ND_A_DESC__UNDEF', `SCL', `NHD', `*UNQ')
rule(`SAC_ND_A_DESC', `SAC_ND_A_DESC__DEFAULT', `SCL', `HID', `NUQ')
rule(`SAC_ND_A_DESC', `SAC_ND_A_DESC__UNDEF', `SCL', `HID', `UNQ')
rule(`SAC_ND_A_DESC', `SAC_ND_A_DESC__DEFAULT', `*SHP', `*HID', `*UNQ')


pat(`SAC_ND_A_DESC_DIM', `0', `NT_SHP')
rule(`SAC_ND_A_DESC_DIM', `SAC_ND_A_DESC_DIM__DEFAULT', `AUD')
rule(`SAC_ND_A_DESC_DIM', `SAC_ND_A_DESC_DIM__UNDEF', `*SHP')


pat(`SAC_ND_A_DESC_SIZE', `0', `NT_SHP')
rule(`SAC_ND_A_DESC_SIZE', `SAC_ND_A_DESC_SIZE__DEFAULT', `AKD')
rule(`SAC_ND_A_DESC_SIZE', `SAC_ND_A_DESC_SIZE__DEFAULT', `AUD')
rule(`SAC_ND_A_DESC_SIZE', `SAC_ND_A_DESC_SIZE__UNDEF', `*SHP')


pat(`SAC_ND_A_DESC_SHAPE', `0', `NT_SHP')
rule(`SAC_ND_A_DESC_SHAPE', `SAC_ND_A_DESC_SHAPE__DEFAULT', `AKD')
rule(`SAC_ND_A_DESC_SHAPE', `SAC_ND_A_DESC_SHAPE__DEFAULT', `AUD')
rule(`SAC_ND_A_DESC_SHAPE', `SAC_ND_A_DESC_SHAPE__UNDEF', `*SHP')


pat(`SAC_ND_A_MIRROR_DIM', `0', `NT_SHP')
rule(`SAC_ND_A_MIRROR_DIM', `SAC_ND_A_MIRROR_DIM__SCL', `SCL')
rule(`SAC_ND_A_MIRROR_DIM', `SAC_ND_A_MIRROR_DIM__DEFAULT', `*SHP')


pat(`SAC_ND_A_MIRROR_SIZE', `0', `NT_SHP')
rule(`SAC_ND_A_MIRROR_SIZE', `SAC_ND_A_MIRROR_SIZE__SCL', `SCL')
rule(`SAC_ND_A_MIRROR_SIZE', `SAC_ND_A_MIRROR_SIZE__DEFAULT', `*SHP')


pat(`SAC_ND_A_MIRROR_SHAPE', `0', `NT_SHP')
rule(`SAC_ND_A_MIRROR_SHAPE', `SAC_ND_A_MIRROR_SHAPE__DEFAULT', `AKS')
rule(`SAC_ND_A_MIRROR_SHAPE', `SAC_ND_A_MIRROR_SHAPE__DEFAULT', `AKD')
rule(`SAC_ND_A_MIRROR_SHAPE', `SAC_ND_A_MIRROR_SHAPE__UNDEF', `*SHP')


pat(`SAC_ND_A_FIELD', `0')
rule(`SAC_ND_A_FIELD', `SAC_ND_A_FIELD__DEFAULT')


pat(`SAC_ND_A_RC', `0', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_A_RC', `SAC_ND_A_RC__UNDEF', `*SHP', `*HID', `UNQ')
rule(`SAC_ND_A_RC', `SAC_ND_A_RC__UNDEF', `SCL', `NHD', `NUQ')
rule(`SAC_ND_A_RC', `SAC_ND_A_RC__DEFAULT', `*SHP', `*HID', `NUQ')


pat(`SAC_ND_A_DIM', `0')
rule(`SAC_ND_A_DIM', `SAC_ND_A_DIM__DEFAULT')


pat(`SAC_ND_A_SIZE', `0')
rule(`SAC_ND_A_SIZE', `SAC_ND_A_SIZE__DEFAULT')


pat(`SAC_ND_A_SHAPE', `0', `NT_SHP')
rule(`SAC_ND_A_SHAPE', `SAC_ND_A_SHAPE__SCL', `SCL')
rule(`SAC_ND_A_SHAPE', `SAC_ND_A_SHAPE__AKS_AKD', `AKS')
rule(`SAC_ND_A_SHAPE', `SAC_ND_A_SHAPE__AKS_AKD', `AKD')
rule(`SAC_ND_A_SHAPE', `SAC_ND_A_SHAPE__AUD', `AUD')


pat(`SAC_ND_READ', `0', `NT_SHP')
rule(`SAC_ND_READ', `SAC_ND_READ__SCL', `SCL')
rule(`SAC_ND_READ', `SAC_ND_READ__DEFAULT', `*SHP')


pat(`SAC_ND_WRITE', `0', `NT_SHP')
rule(`SAC_ND_WRITE', `SAC_ND_WRITE__SCL', `SCL')
rule(`SAC_ND_WRITE', `SAC_ND_WRITE__DEFAULT', `*SHP')


pat(`SAC_ND_WRITE_COPY', `0', `NT_HID')
rule(`SAC_ND_WRITE_COPY', `SAC_ND_WRITE_COPY__HID', `HID')
rule(`SAC_ND_WRITE_COPY', `SAC_ND_WRITE_COPY__NHD', `NHD')


pat(`SAC_ND_DESC_TYPE', `0')
rule(`SAC_ND_DESC_TYPE', `SAC_ND_DESC_TYPE__DEFAULT')


pat(`SAC_ND_TYPE', `0', `NT_SHP')
rule(`SAC_ND_TYPE', `SAC_ND_TYPE_TAG__SCL', `SCL')
rule(`SAC_ND_TYPE', `SAC_ND_TYPE_TAG__DEFAULT', `*SHP')


pat(`SAC_ND_TYPEDEF', `0', `NT_HID')
rule(`SAC_ND_TYPEDEF', `SAC_ND_TYPEDEF__HID', `HID')
rule(`SAC_ND_TYPEDEF', `SAC_ND_TYPEDEF__DEFAULT', `*HID')


pat(`SAC_ND_DECL__DESC', `0', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_DECL__DESC', `SAC_ND_DECL__DESC__NONE', `SCL', `NHD', `*UNQ')
rule(`SAC_ND_DECL__DESC', `SAC_ND_DECL__DESC__NONE', `SCL', `HID', `UNQ')
rule(`SAC_ND_DECL__DESC', `SAC_ND_DECL__DESC__DEFAULT', `*SHP', `*HID', `*UNQ')


pat(`SAC_ND_PARAM_FLAG_in', `0', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_PARAM_FLAG_in', `SAC_ND_PARAM_in__NODESC', `SCL', `NHD', `*UNQ')
rule(`SAC_ND_PARAM_FLAG_in', `SAC_ND_PARAM_in__NODESC', `SCL', `HID', `UNQ')
rule(`SAC_ND_PARAM_FLAG_in', `SAC_ND_PARAM_in__DESC', `*SHP', `*HID', `*UNQ')


pat(`SAC_ND_PARAM_FLAG_out', `0', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_PARAM_FLAG_out', `SAC_ND_PARAM_out__NODESC', `SCL', `NHD', `*UNQ')
rule(`SAC_ND_PARAM_FLAG_out', `SAC_ND_PARAM_out__NODESC', `SCL', `HID', `UNQ')
rule(`SAC_ND_PARAM_FLAG_out', `SAC_ND_PARAM_out__DESC', `*SHP', `*HID', `*UNQ')


pat(`SAC_ND_ARG_FLAG_in', `0', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_ARG_FLAG_in', `SAC_ND_ARG_FLAG_in__NODESC', `SCL', `NHD', `*UNQ')
rule(`SAC_ND_ARG_FLAG_in', `SAC_ND_ARG_FLAG_in__NODESC', `SCL', `HID', `UNQ')
rule(`SAC_ND_ARG_FLAG_in', `SAC_ND_ARG_FLAG_in__DESC', `*SHP', `*HID', `*UNQ')


pat(`SAC_ND_ARG_FLAG_out', `0', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_ARG_FLAG_out', `SAC_ND_ARG_FLAG_out__NODESC', `SCL', `NHD', `*UNQ')
rule(`SAC_ND_ARG_FLAG_out', `SAC_ND_ARG_FLAG_out__NODESC', `SCL', `HID', `UNQ')
rule(`SAC_ND_ARG_FLAG_out', `SAC_ND_ARG_FLAG_out__DESC', `*SHP', `*HID', `*UNQ')


pat(`SAC_ND_RET_out', `1', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_RET_out', `SAC_ND_RET_out__NODESC', `SCL', `NHD', `*UNQ')
rule(`SAC_ND_RET_out', `SAC_ND_RET_out__NODESC', `SCL', `HID', `UNQ')
rule(`SAC_ND_RET_out', `SAC_ND_RET_out__DESC', `*SHP', `*HID', `*UNQ')


pat(`SAC_ND_DECL_PARAM_FLAG_inout', `0', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_DECL_PARAM_FLAG_inout', `SAC_ND_DECL_PARAM_inout__NODESC', `SCL', `NHD', `*UNQ')
rule(`SAC_ND_DECL_PARAM_FLAG_inout', `SAC_ND_DECL_PARAM_inout__NODESC', `SCL', `HID', `UNQ')
rule(`SAC_ND_DECL_PARAM_FLAG_inout', `SAC_ND_DECL_PARAM_inout__DESC', `*SHP', `*HID', `*UNQ')


pat(`SAC_ND_ALLOC', `0', `NT_SHP')
rule(`SAC_ND_ALLOC', `SAC_ND_ALLOC__DAO', `SCL')
rule(`SAC_ND_ALLOC', `SAC_ND_ALLOC__DAO', `AKS')
rule(`SAC_ND_ALLOC', `SAC_ND_ALLOC__NO_DAO', `AKD')
rule(`SAC_ND_ALLOC', `SAC_ND_ALLOC__NO_DAO', `AUD')


pat(`SAC_ND_ALLOC_BEGIN', `0', `NT_SHP')
rule(`SAC_ND_ALLOC_BEGIN', `SAC_ND_ALLOC_BEGIN__DAO', `SCL')
rule(`SAC_ND_ALLOC_BEGIN', `SAC_ND_ALLOC_BEGIN__DAO', `AKS')
rule(`SAC_ND_ALLOC_BEGIN', `SAC_ND_ALLOC_BEGIN__NO_DAO', `AKD')
rule(`SAC_ND_ALLOC_BEGIN', `SAC_ND_ALLOC_BEGIN__NO_DAO', `AUD')


pat(`SAC_ND_ALLOC_END', `0', `NT_SHP')
rule(`SAC_ND_ALLOC_END', `SAC_ND_ALLOC_END__DAO', `SCL')
rule(`SAC_ND_ALLOC_END', `SAC_ND_ALLOC_END__DAO', `AKS')
rule(`SAC_ND_ALLOC_END', `SAC_ND_ALLOC_END__NO_DAO', `AKD')
rule(`SAC_ND_ALLOC_END', `SAC_ND_ALLOC_END__NO_DAO', `AUD')


pat(`SAC_ND_ALLOC__DESC', `0', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_ALLOC__DESC', `SAC_ND_ALLOC__DESC__NOOP', `SCL', `NHD', `*UNQ')
rule(`SAC_ND_ALLOC__DESC', `SAC_ND_ALLOC__DESC__NOOP', `SCL', `HID', `UNQ')
rule(`SAC_ND_ALLOC__DESC', `SAC_ND_ALLOC__DESC__AUD', `AUD', `*HID', `*UNQ')
rule(`SAC_ND_ALLOC__DESC', `SAC_ND_ALLOC__DESC__FIXED', `*SHP', `*HID', `*UNQ')


pat(`SAC_ND_ALLOC__DATA', `0', `NT_SHP')
rule(`SAC_ND_ALLOC__DATA', `SAC_ND_ALLOC__DATA__NOOP', `SCL')
rule(`SAC_ND_ALLOC__DATA', `SAC_ND_ALLOC__DATA__AKS', `AKS')
rule(`SAC_ND_ALLOC__DATA', `SAC_ND_ALLOC__DATA__AKD_AUD', `*SHP')


pat(`SAC_ND_ALLOC__DESC_AND_DATA', `0', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_ALLOC__DESC_AND_DATA', `SAC_ND_ALLOC__DESC__NOOP', `SCL', `NHD', `*UNQ')
rule(`SAC_ND_ALLOC__DESC_AND_DATA', `SAC_ND_ALLOC__DESC__NOOP', `SCL', `HID', `UNQ')
rule(`SAC_ND_ALLOC__DESC_AND_DATA', `SAC_ND_ALLOC__DESC__FIXED', `SCL', `HID', `NUQ')
rule(`SAC_ND_ALLOC__DESC_AND_DATA', `SAC_ND_ALLOC__DESC_AND_DATA__AKS', `AKS', `*HID', `*UNQ')
rule(`SAC_ND_ALLOC__DESC_AND_DATA', `SAC_ND_ALLOC__DESC_AND_DATA__UNDEF', `*SHP', `*HID', `*UNQ')


pat(`SAC_ND_FREE__DESC', `0', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_FREE__DESC', `SAC_ND_FREE__DESC__NOOP', `SCL', `NHD', `*UNQ')
rule(`SAC_ND_FREE__DESC', `SAC_ND_FREE__DESC__NOOP', `SCL', `HID', `UNQ')
rule(`SAC_ND_FREE__DESC', `SAC_ND_FREE__DESC__DEFAULT', `*SHP', `*HID', `*UNQ')


pat(`SAC_ND_FREE__DATA', `0', `NT_SHP', `NT_HID')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__SCL_NHD', `SCL', `NHD')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__SCL_HID', `SCL', `HID')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__AKS_NHD', `AKS', `NHD')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__AKS_HID', `AKS', `HID')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__AKD_NHD', `AKD', `NHD')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__AKS_HID', `AKD', `HID')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__AKD_NHD', `AUD', `NHD')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__AKS_HID', `AUD', `HID')


pat(`SAC_ND_ASSIGN__DATA', `0', `NT_SHP', `NT_HID')
rule(`SAC_ND_ASSIGN__DATA', `GEN_SAC_ND_ASSIGN__DATA__SCL_NHD', `SCL', `NHD')
rule(`SAC_ND_ASSIGN__DATA', `GEN_SAC_ND_ASSIGN__DATA__SCL_HID', `SCL', `HID')
rule(`SAC_ND_ASSIGN__DATA', `GEN_SAC_ND_ASSIGN__DATA__AKS', `AKS', `*HID')
rule(`SAC_ND_ASSIGN__DATA', `GEN_SAC_ND_ASSIGN__DATA__AKS', `AKD', `*HID')
rule(`SAC_ND_ASSIGN__DATA', `GEN_SAC_ND_ASSIGN__DATA__AKS', `AKD', `*HID')
rule(`SAC_ND_ASSIGN__DATA', `GEN_SAC_ND_ASSIGN__DATA__AUD_NHD', `AUD', `NHD')
rule(`SAC_ND_ASSIGN__DATA', `GEN_SAC_ND_ASSIGN__DATA__AUD_HID', `AUD', `HID')


pat(`GEN_SAC_ND_ASSIGN__DATA__SCL_NHD', `1', `NT_SHP', `NT_HID')
rule(`GEN_SAC_ND_ASSIGN__DATA__SCL_NHD', `SAC_ND_ASSIGN__DATA__AKS_AKS', `SCL', `*HID')
rule(`GEN_SAC_ND_ASSIGN__DATA__SCL_NHD', `SAC_ND_ASSIGN__DATA__SCL_AUD', `AUD', `NHD')
rule(`GEN_SAC_ND_ASSIGN__DATA__SCL_NHD', `SAC_ND_ASSIGN__DATA__UNDEF', `*SHP', `*HID')


pat(`GEN_SAC_ND_ASSIGN__DATA__SCL_HID', `1', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`GEN_SAC_ND_ASSIGN__DATA__SCL_HID', `SAC_ND_ASSIGN__DATA__AKS_AKS', `SCL', `*HID', `*UNQ')
rule(`GEN_SAC_ND_ASSIGN__DATA__SCL_HID', `SAC_ND_ASSIGN__DATA__SCL_AUD', `AUD', `HID', `NUQ')
rule(`GEN_SAC_ND_ASSIGN__DATA__SCL_HID', `SAC_ND_ASSIGN__DATA__SCL_AUD_UNQ', `AUD', `HID', `UNQ')
rule(`GEN_SAC_ND_ASSIGN__DATA__SCL_HID', `SAC_ND_ASSIGN__DATA__UNDEF', `*SHP', `*HID', `*UNQ')


pat(`GEN_SAC_ND_ASSIGN__DATA__AKS', `1', `NT_SHP')
rule(`GEN_SAC_ND_ASSIGN__DATA__AKS', `SAC_ND_ASSIGN__DATA__UNDEF', `SCL')
rule(`GEN_SAC_ND_ASSIGN__DATA__AKS', `SAC_ND_ASSIGN__DATA__AKS_AKS', `*SHP')


pat(`GEN_SAC_ND_ASSIGN__DATA__AUD_NHD', `1', `NT_SHP', `NT_HID')
rule(`GEN_SAC_ND_ASSIGN__DATA__AUD_NHD', `SAC_ND_ASSIGN__DATA__AUD_SCL_NHD', `SCL', `NHD')
rule(`GEN_SAC_ND_ASSIGN__DATA__AUD_NHD', `SAC_ND_ASSIGN__DATA__UNDEF', `SCL', `HID')
rule(`GEN_SAC_ND_ASSIGN__DATA__AUD_NHD', `SAC_ND_ASSIGN__DATA__AUD_AKS', `*SHP', `*HID')



pat(`GEN_SAC_ND_ASSIGN__DATA__AUD_HID', `1', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`GEN_SAC_ND_ASSIGN__DATA__AUD_HID', `SAC_ND_ASSIGN__DATA__AUD_SCL_NHD', `SCL', `HID', `NUQ')
rule(`GEN_SAC_ND_ASSIGN__DATA__AUD_HID', `SAC_ND_ASSIGN__DATA__AUD_SCL_UNQ', `SCL', `HID', `UNQ')
rule(`GEN_SAC_ND_ASSIGN__DATA__AUD_HID', `SAC_ND_ASSIGN__DATA__UNDEF', `SCL', `NHD', `*UNQ')
rule(`GEN_SAC_ND_ASSIGN__DATA__AUD_HID', `SAC_ND_ASSIGN__DATA__AUD_AKS', `*SHP', `*HID', `*UNQ')


pat(`SAC_ND_COPY__DATA', `0', `NT_SHP')
rule(`SAC_ND_COPY__DATA', `GEN_SAC_ND_COPY__DATA__SCL', `SCL')
rule(`SAC_ND_COPY__DATA', `GEN_SAC_ND_COPY__DATA__ANY', `*SHP')


pat(`GEN_SAC_ND_COPY__DATA__SCL', `1', `NT_SHP')
rule(`GEN_SAC_ND_COPY__DATA__SCL', `SAC_ND_COPY__DATA__SCL_SCL', `SCL')
rule(`GEN_SAC_ND_COPY__DATA__SCL', `SAC_ND_COPY__DATA__SCL_ANY', `*SHP')


pat(`GEN_SAC_ND_COPY__DATA__ANY', `1', `NT_SHP')
rule(`GEN_SAC_ND_COPY__DATA__ANY', `SAC_ND_COPY__DATA__ANY_SCL', `SCL')
rule(`GEN_SAC_ND_COPY__DATA__ANY', `SAC_ND_COPY__DATA__ANY_ANY', `*SHP')


pat(`SAC_ND_SET__RC', `0', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_SET__RC', `SAC_ND_SET__RC__NOOP', `*SHP', `*HID', `UNQ')
rule(`SAC_ND_SET__RC', `SAC_ND_SET__RC__NOOP', `SCL', `NHD', `NUQ')
rule(`SAC_ND_SET__RC', `SAC_ND_SET__RC__DEFAULT', `*SHP', `*HID', `NUQ')


pat(`SAC_ND_INC_RC', `0', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_INC_RC', `SAC_ND_INC_RC__NOOP', `*SHP', `*HID', `UNQ')
rule(`SAC_ND_INC_RC', `SAC_ND_INC_RC__NOOP', `SCL', `NHD', `NUQ')
rule(`SAC_ND_INC_RC', `SAC_ND_INC_RC__DEFAULT', `*SHP', `*HID', `NUQ')


pat(`SAC_ND_DEC_RC', `0', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_DEC_RC', `SAC_ND_DEC_RC__NOOP', `*SHP', `*HID', `UNQ')
rule(`SAC_ND_DEC_RC', `SAC_ND_DEC_RC__NOOP', `SCL', `NHD', `NUQ')
rule(`SAC_ND_DEC_RC', `SAC_ND_DEC_RC__DEFAULT', `*SHP', `*HID', `NUQ')


pat(`SAC_ND_DEC_RC_FREE', `0', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_DEC_RC_FREE', `SAC_ND_DEC_RC_FREE__UNQ', `*SHP', `*HID', `UNQ')
rule(`SAC_ND_DEC_RC_FREE', `SAC_ND_DEC_RC_FREE__NOOP', `SCL', `NHD', `NUQ')
rule(`SAC_ND_DEC_RC_FREE', `SAC_ND_DEC_RC_FREE__DEFAULT', `*SHP', `*HID', `NUQ')


pat(`SAC_IS_LASTREF__BLOCK_BEGIN', `0', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_IS_LASTREF__BLOCK_BEGIN', `SAC_IS_LASTREF__BLOCK_BEGIN__UNQ', `*SHP', `*HID', `UNQ')
rule(`SAC_IS_LASTREF__BLOCK_BEGIN', `SAC_IS_LASTREF__BLOCK_BEGIN__SCL_NHD_NUQ', `SCL', `NHD', `NUQ')
rule(`SAC_IS_LASTREF__BLOCK_BEGIN', `SAC_IS_LASTREF__BLOCK_BEGIN__DEFAULT', `*SHP', `*HID', `NUQ')


pat(`SAC_IS_REUSED__BLOCK_BEGIN', `0', `NT_SHP')
rule(`SAC_IS_REUSED__BLOCK_BEGIN', `SAC_IS_REUSED__BLOCK_BEGIN__SCL', `SCL')
rule(`SAC_IS_REUSED__BLOCK_BEGIN', `SAC_IS_REUSED__BLOCK_BEGIN__DEFAULT', `*SHP')

end_icm_definition

