/*
 * CAUTION: 
 *
 * std_gen.h  is generated automatically from std_gen.h.m4
 *
 */

/*
 * See ../m4/README
 */

include(`icm.m4')

start_icm_definition(std_gen)


pat(`SAC_ND_A_DESC', `0', `0', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_A_DESC', `SAC_ND_A_DESC__UNDEF', `SCL', `NHD', `*UNQ')
rule(`SAC_ND_A_DESC', `SAC_ND_A_DESC__DEFAULT', `SCL', `HID', `NUQ')
rule(`SAC_ND_A_DESC', `SAC_ND_A_DESC_NESTED__SCL', `SCL', `HNS', `NUQ')
rule(`SAC_ND_A_DESC', `SAC_ND_A_DESC__UNDEF', `SCL', `HID', `UNQ')
rule(`SAC_ND_A_DESC', `SAC_ND_A_DESC__UNDEF', `SCL', `*HID', `UNQ')
rule(`SAC_ND_A_DESC', `SAC_ND_A_DESC__UNDEF', `AKS', `*HID', `UNQ')
rule(`SAC_ND_A_DESC', `SAC_ND_A_DESC__DEFAULT', `*SHP', `*HID', `*UNQ')

dnl Perform SAC_ND_A_DESC or return NULL if there is no desc
pat(`SAC_ND_A_DESC_NULL', `0', `0', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_A_DESC_NULL', `SAC_ND_A_DESC__NULL', `SCL', `NHD', `*UNQ')
rule(`SAC_ND_A_DESC_NULL', `SAC_ND_A_DESC__DEFAULT', `SCL', `HID', `NUQ')
rule(`SAC_ND_A_DESC_NULL', `SAC_ND_A_DESC__NULL', `SCL', `HID', `UNQ')
rule(`SAC_ND_A_DESC_NULL', `SAC_ND_A_DESC__NULL', `SCL', `*HID', `UNQ')
rule(`SAC_ND_A_DESC_NULL', `SAC_ND_A_DESC__NULL', `AKS', `*HID', `UNQ')
rule(`SAC_ND_A_DESC_NULL', `SAC_ND_A_DESC__DEFAULT', `*SHP', `*HID', `*UNQ')

pat(`SAC_ND_A_DESC_NAME', `0', `0', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_A_DESC_NAME', `SAC_ND_A_DESC_NAME__UNDEF', `SCL', `NHD', `*UNQ')
rule(`SAC_ND_A_DESC_NAME', `SAC_ND_A_DESC_NAME__DEFAULT', `SCL', `HID', `NUQ')
rule(`SAC_ND_A_DESC_NAME', `SAC_ND_A_DESC_NAME__UNDEF', `SCL', `HID', `UNQ')
rule(`SAC_ND_A_DESC_NAME', `SAC_ND_A_DESC_NAME__UNDEF', `SCL', `*HID', `UNQ')
rule(`SAC_ND_A_DESC_NAME', `SAC_ND_A_DESC_NAME__UNDEF', `AKS', `*HID', `UNQ')
rule(`SAC_ND_A_DESC_NAME', `SAC_ND_A_DESC_NAME__DEFAULT', `*SHP', `*HID', `*UNQ')

pat(`SAC_ND_A_DESC_DIM', `0', `0', `NT_SHP')
rule(`SAC_ND_A_DESC_DIM', `SAC_ND_A_DESC_DIM__DEFAULT', `AUD')
rule(`SAC_ND_A_DESC_DIM', `SAC_ND_A_DESC_DIM__UNDEF', `*SHP')


pat(`SAC_ND_A_DESC_SIZE', `0', `0', `NT_SHP')
rule(`SAC_ND_A_DESC_SIZE', `SAC_ND_A_DESC_SIZE__DEFAULT', `AKD')
rule(`SAC_ND_A_DESC_SIZE', `SAC_ND_A_DESC_SIZE__DEFAULT', `AUD')
rule(`SAC_ND_A_DESC_SIZE', `SAC_ND_A_DESC_SIZE__UNDEF', `*SHP')


pat(`SAC_ND_A_DESC_SHAPE', `0', `1', `NT_SHP')
rule(`SAC_ND_A_DESC_SHAPE', `SAC_ND_A_DESC_SHAPE__DEFAULT', `AKD')
rule(`SAC_ND_A_DESC_SHAPE', `SAC_ND_A_DESC_SHAPE__DEFAULT', `AUD')
rule(`SAC_ND_A_DESC_SHAPE', `SAC_ND_A_DESC_SHAPE__UNDEF', `*SHP')


pat(`SAC_ND_A_MIRROR_DIM', `0', `0', `NT_SHP')
rule(`SAC_ND_A_MIRROR_DIM', `SAC_ND_A_MIRROR_DIM__SCL', `SCL')
rule(`SAC_ND_A_MIRROR_DIM', `SAC_ND_A_MIRROR_DIM__DEFAULT', `*SHP')


pat(`SAC_ND_A_MIRROR_SIZE', `0', `0', `NT_SHP')
rule(`SAC_ND_A_MIRROR_SIZE', `SAC_ND_A_MIRROR_SIZE__SCL', `SCL')
rule(`SAC_ND_A_MIRROR_SIZE', `SAC_ND_A_MIRROR_SIZE__DEFAULT', `*SHP')


pat(`SAC_ND_A_MIRROR_SHAPE', `0', `1', `NT_SHP')
rule(`SAC_ND_A_MIRROR_SHAPE', `SAC_ND_A_MIRROR_SHAPE__DEFAULT', `AKS')
rule(`SAC_ND_A_MIRROR_SHAPE', `SAC_ND_A_MIRROR_SHAPE__DEFAULT', `AKD')
rule(`SAC_ND_A_MIRROR_SHAPE', `SAC_ND_A_MIRROR_SHAPE__UNDEF', `*SHP')


pat(`SAC_ND_A_FIELD', `0', `0')
rule(`SAC_ND_A_FIELD', `SAC_ND_A_FIELD__DEFAULT')

pat(`SAC_ND_A_FIELD_UNBOX', `0', `0', `NT_SHP')
rule(`SAC_ND_A_FIELD_UNBOX', `SAC_ND_A_FIELD', `SCL')
rule(`SAC_ND_A_FIELD_UNBOX', `SAC_ND_A_FIELD__BOXED', `*SHP')

pat(`SAC_ND_A_RC', `0', `0', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_A_RC', `SAC_ND_A_RC__UNDEF', `*SHP', `*HID', `UNQ')
rule(`SAC_ND_A_RC', `SAC_ND_A_RC__UNDEF', `SCL', `NHD', `NUQ')
rule(`SAC_ND_A_RC', `SAC_ND_A_RC__UNDEF', `*SHP', `*HID', `UNQ')
rule(`SAC_ND_A_RC', `SAC_ND_A_RC__DEFAULT', `*SHP', `*HID', `NUQ')


pat(`SAC_ND_A_DIM', `0' `0',)
rule(`SAC_ND_A_DIM', `SAC_ND_A_DIM__DEFAULT')


pat(`SAC_ND_A_SIZE', `0', `0')
rule(`SAC_ND_A_SIZE', `SAC_ND_A_SIZE__DEFAULT')


pat(`SAC_ND_A_SHAPE', `0', `1', `NT_SHP')
rule(`SAC_ND_A_SHAPE', `SAC_ND_A_SHAPE__SCL', `SCL')
rule(`SAC_ND_A_SHAPE', `SAC_ND_A_SHAPE__AKS_AKD', `AKS')
rule(`SAC_ND_A_SHAPE', `SAC_ND_A_SHAPE__AKS_AKD', `AKD')
rule(`SAC_ND_A_SHAPE', `SAC_ND_A_SHAPE__AUD', `AUD')


pat(`SAC_ND_READ', `0', `1', `NT_SHP', `NT_BIT')
rule(`SAC_ND_READ', `SAC_ND_READ__SCL', `SCL', `*BIT')
rule(`SAC_ND_READ', `SAC_ND_READ__BITARRAY', `*SHP', `YES')
rule(`SAC_ND_READ', `SAC_ND_READ__DEFAULT', `*SHP', `*BIT')


pat(`SAC_ND_WRITE', `0', `1', `NT_SHP', `NT_BIT')
rule(`SAC_ND_WRITE', `SAC_ND_WRITE__SCL', `SCL', `*BIT')
rule(`SAC_ND_WRITE', `SAC_ND_WRITE__BITARRAY', `*SHP', `YES')
rule(`SAC_ND_WRITE', `SAC_ND_WRITE__DEFAULT', `*SHP', `*BIT')


pat(`SAC_ND_WRITE_READ_COPY', `0', `4', `NT_HID')
rule(`SAC_ND_WRITE_READ_COPY', `SAC_ND_WRITE_READ_COPY__NESTED', `HNS')
rule(`SAC_ND_WRITE_READ_COPY', `SAC_ND_WRITE_READ_COPY__DEFAULT', `*HID')

pat(`SAC_ND_WRITE_COPY', `0', `3', `NT_HID')
rule(`SAC_ND_WRITE_COPY', `SAC_ND_WRITE_COPY__HID', `HID')
rule(`SAC_ND_WRITE_COPY', `SAC_ND_WRITE_COPY__NHD', `NHD')
rule(`SAC_ND_WRITE_COPY', `SAC_ND_WRITE_COPY__HNS', `HNS')

pat(`SAC_ND_PRINT_SHAPE', `0', `0', `NT_SHP')
rule(`SAC_ND_PRINT_SHAPE', `SAC_ND_PRINT_SHAPE__SCL', `SCL')
rule(`SAC_ND_PRINT_SHAPE', `SAC_ND_PRINT_SHAPE__SHP', `*SHP')

pat(`SAC_ND_DESC_TYPE', `0', `0')
rule(`SAC_ND_DESC_TYPE', `SAC_ND_DESC_TYPE__DEFAULT')


pat(`SAC_ND_TYPE', `0', `1', `NT_SHP')
rule(`SAC_ND_TYPE', `SAC_ND_TYPE_TAG__SCL', `SCL')
rule(`SAC_ND_TYPE', `SAC_ND_TYPE_TAG__DEFAULT', `*SHP')


pat(`SAC_ND_TYPEDEF', `0', `1', `NT_HID')
rule(`SAC_ND_TYPEDEF', `SAC_ND_TYPEDEF__HID', `HID')
rule(`SAC_ND_TYPEDEF', `SAC_ND_TYPEDEF__DEFAULT', `*HID')


pat(`SAC_ND_DECL__DESC', `0', `1', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_DECL__DESC', `SAC_ND_DECL__DESC__NONE', `SCL', `NHD', `*UNQ')
rule(`SAC_ND_DECL__DESC', `SAC_ND_DECL__DESC__NONE', `SCL', `HID', `UNQ')
rule(`SAC_ND_DECL__DESC', `SAC_ND_DECL__DESC__NONE', `SCL', `*HID', `UNQ')
rule(`SAC_ND_DECL__DESC', `SAC_ND_DECL__DESC__NONE', `AKS', `*HID', `UNQ')
rule(`SAC_ND_DECL__DESC', `SAC_ND_DECL__DESC__DEFAULT', `*SHP', `*HID', `*UNQ')


pat(`SAC_ND_PARAM_in', `0', `1', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_PARAM_in', `SAC_ND_PARAM_in__NODESC', `SCL', `NHD', `*UNQ')
rule(`SAC_ND_PARAM_in', `SAC_ND_PARAM_in__NODESC', `SCL', `HID', `UNQ')
rule(`SAC_ND_PARAM_in', `SAC_ND_PARAM_NESTED_in__NODESC', `SCL', `HNS', `NUQ')
rule(`SAC_ND_PARAM_in', `SAC_ND_PARAM_in__NODESC', `SCL', `*HID', `UNQ')
rule(`SAC_ND_PARAM_in', `SAC_ND_PARAM_in__NODESC', `AKS', `*HID', `UNQ')
rule(`SAC_ND_PARAM_in', `SAC_ND_PARAM_NESTED_in__DESC', `*SHP', `HNS', `*UNQ')
rule(`SAC_ND_PARAM_in', `SAC_ND_PARAM_in__DESC', `*SHP', `*HID', `*UNQ')


pat(`SAC_ND_PARAM_out', `0', `1', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_PARAM_out', `SAC_ND_PARAM_out__NODESC', `SCL', `NHD', `*UNQ')
rule(`SAC_ND_PARAM_out', `SAC_ND_PARAM_out__NODESC', `SCL', `HID', `UNQ')
rule(`SAC_ND_PARAM_out', `SAC_ND_PARAM_NESTED_out__NODESC', `SCL', `HNS', `NUQ')
rule(`SAC_ND_PARAM_out', `SAC_ND_PARAM_out__NODESC', `SCL', `*HID', `UNQ')
rule(`SAC_ND_PARAM_out', `SAC_ND_PARAM_out__NODESC', `AKS', `*HID', `UNQ')
rule(`SAC_ND_PARAM_out', `SAC_ND_PARAM_NESTED_out__DESC', `*SHP', `HNS', `*UNQ')
rule(`SAC_ND_PARAM_out', `SAC_ND_PARAM_out__DESC', `*SHP', `*HID', `*UNQ')


pat(`SAC_ND_ARG_in', `0', `1', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_ARG_in', `SAC_ND_ARG_in__NODESC', `SCL', `NHD', `*UNQ')
rule(`SAC_ND_ARG_in', `SAC_ND_ARG_in__NODESC', `SCL', `HID', `UNQ')
rule(`SAC_ND_ARG_in', `SAC_ND_ARG_in__NODESC', `SCL', `HNS', `NUQ')
rule(`SAC_ND_ARG_in', `SAC_ND_ARG_in__NODESC', `SCL', `*HID', `UNQ')
rule(`SAC_ND_ARG_in', `SAC_ND_ARG_in__NODESC', `AKS', `*HID', `UNQ')
rule(`SAC_ND_ARG_in', `SAC_ND_ARG_in__DESC', `*SHP', `*HID', `*UNQ')


pat(`SAC_ND_ARG_out', `0', `1', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_ARG_out', `SAC_ND_ARG_out__NODESC', `SCL', `NHD', `*UNQ')
rule(`SAC_ND_ARG_out', `SAC_ND_ARG_out__NODESC', `SCL', `HID', `UNQ')
rule(`SAC_ND_ARG_out', `SAC_ND_ARG_out__NODESC', `SCL', `HNS', `NUQ')
rule(`SAC_ND_ARG_out', `SAC_ND_ARG_out__NODESC', `SCL', `*HID', `UNQ')
rule(`SAC_ND_ARG_out', `SAC_ND_ARG_out__NODESC', `AKS', `*HID', `UNQ')
rule(`SAC_ND_ARG_out', `SAC_ND_ARG_NESTED_out__DESC', `*SHP', `HNS', `*UNQ')
rule(`SAC_ND_ARG_out', `SAC_ND_ARG_out__DESC', `*SHP', `*HID', `*UNQ')


pat(`SAC_ND_RET_out', `1', `0', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_RET_out', `SAC_ND_RET_out__NODESC', `SCL', `NHD', `*UNQ')
rule(`SAC_ND_RET_out', `SAC_ND_RET_out__NODESC', `SCL', `HID', `UNQ')
rule(`SAC_ND_RET_out', `SAC_ND_RET_NESTED_out__NODESC', `SCL', `HNS', `NUQ')
rule(`SAC_ND_RET_out', `SAC_ND_RET_out__NODESC', `SCL', `*HID', `UNQ')
rule(`SAC_ND_RET_out', `SAC_ND_RET_out__NODESC', `AKS', `*HID', `UNQ')
rule(`SAC_ND_RET_out', `SAC_ND_RET_NESTED_out__DESC', `*SHP', `HNS', `*UNQ')
rule(`SAC_ND_RET_out', `SAC_ND_RET_out__DESC', `*SHP', `*HID', `*UNQ')


pat(`SAC_ND_DECL_PARAM_inout', `0', `1', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_DECL_PARAM_inout', `SAC_ND_DECL_PARAM_inout__NODESC', `SCL', `NHD', `*UNQ')
rule(`SAC_ND_DECL_PARAM_inout', `SAC_ND_DECL_PARAM_inout__NODESC', `SCL', `HID', `UNQ')
rule(`SAC_ND_DECL_PARAM_inout', `SAC_ND_DECL_PARAM_inout__NODESC', `SCL', `*HID', `UNQ')
rule(`SAC_ND_DECL_PARAM_inout', `SAC_ND_DECL_PARAM_inout__NODESC', `AKS', `*HID', `UNQ')
rule(`SAC_ND_DECL_PARAM_inout', `SAC_ND_DECL_PARAM_inout__DESC', `*SHP', `*HID', `*UNQ')


pat(`SAC_ND_ALLOC', `0', `3', `NT_SHP')
rule(`SAC_ND_ALLOC', `SAC_ND_ALLOC__DAO', `SCL')
rule(`SAC_ND_ALLOC', `SAC_ND_ALLOC__DAO', `AKS')
rule(`SAC_ND_ALLOC', `SAC_ND_ALLOC__NO_DAO', `AKD')
rule(`SAC_ND_ALLOC', `SAC_ND_ALLOC__NO_DAO', `AUD')


pat(`SAC_ND_ALLOC_BEGIN', `0', `3', `NT_SHP')
rule(`SAC_ND_ALLOC_BEGIN', `SAC_ND_ALLOC_BEGIN__DAO', `SCL')
rule(`SAC_ND_ALLOC_BEGIN', `SAC_ND_ALLOC_BEGIN__DAO', `AKS')
rule(`SAC_ND_ALLOC_BEGIN', `SAC_ND_ALLOC_BEGIN__NO_DAO', `AKD')
rule(`SAC_ND_ALLOC_BEGIN', `SAC_ND_ALLOC_BEGIN__NO_DAO', `AUD')


pat(`SAC_ND_ALLOC_END', `0', `3', `NT_SHP')
rule(`SAC_ND_ALLOC_END', `SAC_ND_ALLOC_END__DAO', `SCL')
rule(`SAC_ND_ALLOC_END', `SAC_ND_ALLOC_END__DAO', `AKS')
rule(`SAC_ND_ALLOC_END', `SAC_ND_ALLOC_END__NO_DAO', `AKD')
rule(`SAC_ND_ALLOC_END', `SAC_ND_ALLOC_END__NO_DAO', `AUD')


pat(`SAC_ND_ALLOC_END__NO_DAO', `0', `3', `NT_HID')
rule(`SAC_ND_ALLOC_END__NO_DAO', `SAC_ND_ALLOC_END__NO_DAO__NESTED', `HNS')
rule(`SAC_ND_ALLOC_END__NO_DAO', `SAC_ND_ALLOC_END__NO_DAO__DEFAULT', `*HID')

pat(`SAC_ND_REALLOC_BEGIN', `0', `4', `NT_SHP')
rule(`SAC_ND_REALLOC_BEGIN', `SAC_ND_REALLOC_BEGIN__DAO', `SCL')
rule(`SAC_ND_REALLOC_BEGIN', `SAC_ND_REALLOC_BEGIN__DAO', `AKS')
rule(`SAC_ND_REALLOC_BEGIN', `SAC_ND_REALLOC_BEGIN__NO_DAO', `AKD')
rule(`SAC_ND_REALLOC_BEGIN', `SAC_ND_REALLOC_BEGIN__NO_DAO', `AUD')


pat(`SAC_ND_REALLOC_END', `0', `4', `NT_SHP')
rule(`SAC_ND_REALLOC_END', `SAC_ND_REALLOC_END__DAO', `SCL')
rule(`SAC_ND_REALLOC_END', `SAC_ND_REALLOC_END__DAO', `AKS')
rule(`SAC_ND_REALLOC_END', `SAC_ND_REALLOC_END__NO_DAO', `AKD')
rule(`SAC_ND_REALLOC_END', `SAC_ND_REALLOC_END__NO_DAO', `AUD')


pat(`SAC_ND_ALLOC__DESC', `0', `1', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_ALLOC__DESC', `SAC_ND_ALLOC__DESC__NOOP', `SCL', `NHD', `*UNQ')
rule(`SAC_ND_ALLOC__DESC', `SAC_ND_ALLOC__DESC__NOOP', `SCL', `HID', `UNQ')
rule(`SAC_ND_ALLOC__DESC', `SAC_ND_ALLOC__DESC__NOOP', `SCL', `*HID', `UNQ')
rule(`SAC_ND_ALLOC__DESC', `SAC_ND_ALLOC__DESC__NOOP', `AKS', `*HID', `UNQ')
rule(`SAC_ND_ALLOC__DESC', `SAC_ND_ALLOC__DESC__AUD', `AUD', `*HID', `*UNQ')
rule(`SAC_ND_ALLOC__DESC', `SAC_ND_ALLOC__DESC__FIXED', `*SHP', `*HID', `*UNQ')


pat(`SAC_ND_ALLOC__DATA', `0', `0', `NT_SHP')
rule(`SAC_ND_ALLOC__DATA', `SAC_ND_ALLOC__DATA__NOOP', `SCL')
rule(`SAC_ND_ALLOC__DATA', `SAC_ND_ALLOC__DATA__AKS', `AKS')
rule(`SAC_ND_ALLOC__DATA', `SAC_ND_ALLOC__DATA__AKD_AUD', `*SHP')

pat(`SAC_ND_ALLOC__DATA_BASETYPE', `0', `1', `NT_SHP')
rule(`SAC_ND_ALLOC__DATA_BASETYPE', `SAC_ND_ALLOC__DATA_BASETYPE__NOOP', `SCL')
rule(`SAC_ND_ALLOC__DATA_BASETYPE', `SAC_ND_ALLOC__DATA_BASETYPE__AKS', `AKS')
rule(`SAC_ND_ALLOC__DATA_BASETYPE', `SAC_ND_ALLOC__DATA_BASETYPE__AKD_AUD', `*SHP')

pat(`SAC_ND_ALLOC__DESC_AND_DATA', `0', `2', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_ALLOC__DESC_AND_DATA', `SAC_ND_ALLOC__DESC__NOOP_BASETYPE', `SCL', `NHD', `*UNQ')
rule(`SAC_ND_ALLOC__DESC_AND_DATA', `SAC_ND_ALLOC__DESC__NOOP_BASETYPE', `SCL', `HID', `UNQ')
rule(`SAC_ND_ALLOC__DESC_AND_DATA', `SAC_ND_ALLOC__DESC__NOOP_BASETYPE', `SCL', `*HID', `UNQ')
rule(`SAC_ND_ALLOC__DESC_AND_DATA', `SAC_ND_ALLOC__DESC__NOOP_BASETYPE', `AKS', `*HID', `UNQ')
rule(`SAC_ND_ALLOC__DESC_AND_DATA', `SAC_ND_ALLOC__DESC__FIXED_BASETYPE', `SCL', `HID', `NUQ')
rule(`SAC_ND_ALLOC__DESC_AND_DATA', `SAC_ND_ALLOC__DESC__NOOP_BASETYPE', `SCL', `HNS', `NUQ')
rule(`SAC_ND_ALLOC__DESC_AND_DATA', `SAC_ND_ALLOC__DESC_AND_DATA_NESTED__AKS', `AKS', `HNS', `NUQ')
rule(`SAC_ND_ALLOC__DESC_AND_DATA', `SAC_ND_ALLOC__DESC_AND_DATA__AKS', `AKS', `*HID', `*UNQ')
rule(`SAC_ND_ALLOC__DESC_AND_DATA', `SAC_ND_ALLOC__DESC_AND_DATA__UNDEF', `*SHP', `*HID', `*UNQ')


pat(`SAC_ND_FREE__DESC', `0', `0', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_FREE__DESC', `SAC_ND_FREE__DESC__NOOP', `SCL', `NHD', `*UNQ')
rule(`SAC_ND_FREE__DESC', `SAC_ND_FREE__DESC__NOOP', `SCL', `HID', `UNQ')
rule(`SAC_ND_FREE__DESC', `SAC_ND_FREE__DESC__NOOP', `SCL', `HNS', `NUQ')
rule(`SAC_ND_FREE__DESC', `SAC_ND_FREE__DESC__NOOP', `SCL', `*HID', `UNQ')
rule(`SAC_ND_FREE__DESC', `SAC_ND_FREE__DESC__NOOP', `AKS', `*HID', `UNQ')
rule(`SAC_ND_FREE__DESC', `SAC_ND_FREE__DESC__DEFAULT', `*SHP', `*HID', `*UNQ')


pat(`SAC_ND_FREE__DATA', `0', `1', `NT_SHP', `NT_HID')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__SCL_NHD', `SCL', `NHD')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__SCL_HID', `SCL', `HID')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__SCL_HNS', `SCL', `HNS')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__AKS_NHD', `AKS', `NHD')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__AKS_HID', `AKS', `HID')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__AKS_HNS', `AKS', `HNS')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__AKD_NHD', `AKD', `NHD')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__AKS_HID', `AKD', `HID')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__AKD_NHD', `AUD', `NHD')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__AKS_HID', `AUD', `HID')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__AKS_NHD', `*SHP', `NHD')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__AKS_HID', `*SHP', `HID')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__AKD_HNS', `AKD', `HNS')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__AKD_HNS', `AUD', `HNS')

pat(`SAC_ND_PRF_SECOND', `0', `1', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_PRF_SECOND', `SAC_ND_PRF_SECOND_NODESC', `SCL', `NHD', `*UNQ')
rule(`SAC_ND_PRF_SECOND', `SAC_ND_PRF_SECOND_NODESC', `SCL', `HID', `UNQ')
rule(`SAC_ND_PRF_SECOND', `SAC_ND_PRF_SECOND_NODESC', `SCL', `*HID', `UNQ')
rule(`SAC_ND_PRF_SECOND', `SAC_ND_PRF_SECOND_NODESC', `AKS', `*HID', `UNQ')
rule(`SAC_ND_PRF_SECOND', `SAC_ND_PRF_SECOND_DESC', `*SHP', `*HID', `*UNQ')

dnl SAC_ND_TYPE_CONV
dnl Check before converting types
pat(`SAC_ND_PRF_TYPE_CONV', `1', `1', `NT_SHP')
rule(`SAC_ND_PRF_TYPE_CONV', `SAC_ND_PRF_TYPE_CONV__SCL', `SCL')
rule(`SAC_ND_PRF_TYPE_CONV', `SAC_ND_PRF_TYPE_CONV__AKD', `AKD')
rule(`SAC_ND_PRF_TYPE_CONV', `SAC_ND_PRF_TYPE_CONV__AUD', `AUD')

pat(`SAC_ND_PRF_TYPE_CONV__AUD', `2', `0', `NT_SHP')
rule(`SAC_ND_PRF_TYPE_CONV__AUD', `SAC_ND_PRF_TYPE_CONV__AUD__AUD', `AUD')
rule(`SAC_ND_PRF_TYPE_CONV__AUD', `SAC_ND_PRF_TYPE_CONV__AUD__SHP', `*SHP')

pat(`SAC_ND_PRF_TYPE_CONV__SCL', `2', `0', `NT_SHP')
rule(`SAC_ND_PRF_TYPE_CONV__SCL', `SAC_ND_PRF_TYPE_CONV__SCL__SCL', `SCL')
rule(`SAC_ND_PRF_TYPE_CONV__SCL', `SAC_ND_PRF_TYPE_CONV__SCL__SHP', `*SHP')

dnl SAC_ND_ASSIGN__DATA
dnl
dnl Note: This ICM takes TWO nametuple arguments instead of one. This is
dnl       implemented by expanding it twice. The intermediate macro's are
dnl       immediately called from here, so it's not visible from the outside,
dnl       but this implementation is fairly inefficient in terms of code size.
dnl
dnl There are five cases for the LHS nametuple:
dnl     SCL, NHD
dnl     SCL, HID
dnl     AKS and AKD
dnl     AUD, NHD
dnl     AUD, HID
dnl
dnl These are actually reduced to less target macro's, but we still need
dnl them.
pat(`SAC_ND_ASSIGN__DATA', `0', `2', `NT_SHP', `NT_HID')
rule(`SAC_ND_ASSIGN__DATA', `GEN_SAC_ND_ASSIGN__DATA__SCL_NHD', `SCL', `NHD')
rule(`SAC_ND_ASSIGN__DATA', `GEN_SAC_ND_ASSIGN__DATA__SCL_HID', `SCL', `HID')
rule(`SAC_ND_ASSIGN__DATA', `GEN_SAC_ND_ASSIGN__DATA__AKS', `AKS', `*HID')
rule(`SAC_ND_ASSIGN__DATA', `GEN_SAC_ND_ASSIGN__DATA__AKS', `AKD', `*HID')
rule(`SAC_ND_ASSIGN__DATA', `GEN_SAC_ND_ASSIGN__DATA__AKS', `AKD', `*HID')
rule(`SAC_ND_ASSIGN__DATA', `GEN_SAC_ND_ASSIGN__DATA__AUD_NHD', `AUD', `NHD')
rule(`SAC_ND_ASSIGN__DATA', `GEN_SAC_ND_ASSIGN__DATA__AUD_HID', `AUD', `HID')
rule(`SAC_ND_ASSIGN__DATA', `GEN_SAC_ND_ASSIGN__DATA__AUD_HNS', `AUD', `HNS')

dnl GEN_SAC_ND_ASSIGN__DATA__SCL_NHD
dnl
dnl Left-hand NT is a SCL NHD. There are three cases for the RHS:
dnl     SCL -> do a direct assignment of the scl value
dnl     AKS/AKD, AUD HID -> undef (can't assign)
dnl     AUD, NHD -> copy aud value to scl
pat(`GEN_SAC_ND_ASSIGN__DATA__SCL_NHD', `1', `1', `NT_SHP', `NT_HID')
rule(`GEN_SAC_ND_ASSIGN__DATA__SCL_NHD', `SAC_ND_ASSIGN__DATA__AKS_AKS', `SCL', `*HID')
rule(`GEN_SAC_ND_ASSIGN__DATA__SCL_NHD', `SAC_ND_ASSIGN__DATA__SCL_AUD', `AUD', `NHD')
rule(`GEN_SAC_ND_ASSIGN__DATA__SCL_NHD', `SAC_ND_ASSIGN__DATA__UNDEF', `*SHP', `*HID')

dnl GEN_SAC_ND_ASSIGN__DATA__SCL_HID
dnl
dnl Left-hand NT is a SCL HID. There are four cases for the RHS:
dnl     SCL -> do a direct assignment of the scl value
dnl     AKS/AKD, AUD NHD -> undef (can't assign)
dnl     AUD HID NUQ  -> copy aud value to scl
dnl     AUD HID UNQ  -> copy aud value to scl, free unique aud
pat(`GEN_SAC_ND_ASSIGN__DATA__SCL_HID', `1', `1', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`GEN_SAC_ND_ASSIGN__DATA__SCL_HID', `SAC_ND_ASSIGN__DATA__AKS_AKS', `SCL', `*HID', `*UNQ')
rule(`GEN_SAC_ND_ASSIGN__DATA__SCL_HID', `SAC_ND_ASSIGN__DATA__SCL_AUD', `AUD', `HID', `NUQ')
rule(`GEN_SAC_ND_ASSIGN__DATA__SCL_HID', `SAC_ND_ASSIGN__DATA__SCL_AUD_UNQ', `AUD', `HID', `UNQ')
rule(`GEN_SAC_ND_ASSIGN__DATA__SCL_HID', `SAC_ND_ASSIGN__DATA__UNDEF', `*SHP', `*HID', `*UNQ')

dnl GEN_SAC_ND_ASSIGN__DATA__AKS
dnl
dnl Left-hand NT is a AKS or AKD. There are two cases for the RHS:
dnl     SCL -> undef (can't assign)
dnl     AKS, AKD or AUD -> do a direct assignment
pat(`GEN_SAC_ND_ASSIGN__DATA__AKS', `1', `1', `NT_SHP')
rule(`GEN_SAC_ND_ASSIGN__DATA__AKS', `SAC_ND_ASSIGN__DATA__UNDEF', `SCL')
rule(`GEN_SAC_ND_ASSIGN__DATA__AKS', `SAC_ND_ASSIGN__DATA__AKS_AKS', `*SHP')

dnl GEN_SAC_ND_ASSIGN__DATA__AUD_NHD
dnl
dnl Left-hand NT is a AUD NHD. There are three cases for the RHS:
dnl     SCL NHD -> allocate aud, copy scl
dnl     SCL HID -> undef (can't assign)
dnl     AKS, AKD, AUD -> allocate aud, do assignment
pat(`GEN_SAC_ND_ASSIGN__DATA__AUD_NHD', `1', `1', `NT_SHP', `NT_HID')
rule(`GEN_SAC_ND_ASSIGN__DATA__AUD_NHD', `SAC_ND_ASSIGN__DATA__AUD_SCL_NHD', `SCL', `NHD')
rule(`GEN_SAC_ND_ASSIGN__DATA__AUD_NHD', `SAC_ND_ASSIGN__DATA__UNDEF', `SCL', `HID')
rule(`GEN_SAC_ND_ASSIGN__DATA__AUD_NHD', `SAC_ND_ASSIGN__DATA__AUD_AKS', `*SHP', `*HID')


dnl Left-hand NT is a AUD NHD. There are four cases for the RHS:
dnl     SCL HID NUQ -> allocate aud, copy scl
dnl     SCL HID UNQ -> allocate aud, copy scl, do uniqueness foo
dnl     SCL NHD -> undef (can't assign)
dnl     AKS, AKD, AUD -> allocate aud, do assignment
pat(`GEN_SAC_ND_ASSIGN__DATA__AUD_HID', `1', `1', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`GEN_SAC_ND_ASSIGN__DATA__AUD_HID', `SAC_ND_ASSIGN__DATA__AUD_SCL_NHD', `SCL', `HID', `NUQ')
rule(`GEN_SAC_ND_ASSIGN__DATA__AUD_HID', `SAC_ND_ASSIGN__DATA__AUD_SCL_UNQ', `SCL', `HID', `UNQ')
rule(`GEN_SAC_ND_ASSIGN__DATA__AUD_HID', `SAC_ND_ASSIGN__DATA__UNDEF', `SCL', `NHD', `*UNQ')
rule(`GEN_SAC_ND_ASSIGN__DATA__AUD_HID', `SAC_ND_ASSIGN__DATA__AUD_AKS', `*SHP', `*HID', `*UNQ')

dnl SAC_ND_COPY__DATA
dnl
dnl Note: This ICM takes TWO nametuple arguments instead of one. This is
dnl       implemented by expanding it twice. The intermediate macro's are
dnl       immediately called from here, so it's not visible from the outside,
dnl       but this implementation is fairly inefficient in terms of code size.
dnl
dnl There are two cases for the LHS nametuple:
dnl     SCL
dnl     AKS, AKD, AUD
dnl
dnl These are actually reduced to two target macro's, but we still need
dnl the intermediate ones for the SCL case because it's not a 1:1
dnl mapping from the LHS.
pat(`SAC_ND_COPY__DATA', `0', `2', `NT_SHP')
rule(`SAC_ND_COPY__DATA', `GEN_SAC_ND_COPY__DATA__SCL', `SCL')
rule(`SAC_ND_COPY__DATA', `GEN_SAC_ND_COPY__DATA__ANY', `*SHP')

dnl GEN_SAC_ND_COPY__DATA__SCL
dnl
dnl There are two cases for the RHS:
dnl   SCL -> copy scalar value
dnl   AKS, AKD, AUD -> do a data copy
pat(`GEN_SAC_ND_COPY__DATA__SCL', `1', `1', `NT_SHP')
rule(`GEN_SAC_ND_COPY__DATA__SCL', `SAC_ND_COPY__DATA__SCL_SCL', `SCL')
rule(`GEN_SAC_ND_COPY__DATA__SCL', `SAC_ND_COPY__DATA__SCL_ANY', `*SHP')

dnl GEN_SAC_ND_COPY__DATA__ANY
dnl
dnl There are two cases for the RHS:
dnl   SCL -> copy scalar value
dnl   AKS, AKD, AUD -> do a data copy
pat(`GEN_SAC_ND_COPY__DATA__ANY', `1', `1', `NT_SHP')
rule(`GEN_SAC_ND_COPY__DATA__ANY', `SAC_ND_COPY__DATA__ANY_SCL', `SCL')
rule(`GEN_SAC_ND_COPY__DATA__ANY', `SAC_ND_COPY__DATA__ANY_ANY', `*SHP')

pat(`GEN_SAC_ND_COPY__DESC', `0', `2', `NT_SHP')
rule(`GEN_SAC_ND_COPY__DESC', `SAC_ND_COPY__DESC_SCL', `SCL')
rule(`GEN_SAC_ND_COPY__DESC', `SAC_ND_COPY__DESC_AKS', `AKS')
rule(`GEN_SAC_ND_COPY__DESC', `SAC_ND_COPY__DESC_AKD', `AKD')
rule(`GEN_SAC_ND_COPY__DESC', `SAC_ND_COPY__DESC_AUD', `AUD')

pat(`SAC_ND_SET__RC', `0', `1', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_SET__RC', `SAC_ND_SET__RC__NOOP', `*SHP', `*HID', `UNQ')
rule(`SAC_ND_SET__RC', `SAC_ND_SET__RC__NOOP', `SCL', `NHD', `NUQ')
rule(`SAC_ND_SET__RC', `SAC_ND_SET__RC__NOOP', `SCL', `*HID', `UNQ')
rule(`SAC_ND_SET__RC', `SAC_ND_SET__RC__NOOP', `AKS', `*HID', `UNQ')
rule(`SAC_ND_SET__RC', `SAC_ND_SET__RC__DEFAULT', `*SHP', `*HID', `NUQ')

pat(`SAC_ND_INIT__RC', `0', `1', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_INIT__RC', `SAC_ND_INIT__RC__NOOP', `*SHP', `*HID', `UNQ')
rule(`SAC_ND_INIT__RC', `SAC_ND_INIT__RC__NOOP', `SCL', `NHD', `NUQ')
rule(`SAC_ND_INIT__RC', `SAC_ND_INIT__RC__NOOP', `SCL', `*HID', `UNQ')
rule(`SAC_ND_INIT__RC', `SAC_ND_INIT__RC__NOOP', `SCL', `HNS', `NUQ')
rule(`SAC_ND_INIT__RC', `SAC_ND_INIT__RC__NOOP', `AKS', `*HID', `UNQ')
rule(`SAC_ND_INIT__RC', `SAC_ND_INIT__RC__DEFAULT', `*SHP', `*HID', `NUQ')


pat(`SAC_ND_INC_RC', `0', `1', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_INC_RC', `SAC_ND_INC_RC__NOOP', `*SHP', `*HID', `UNQ')
rule(`SAC_ND_INC_RC', `SAC_ND_INC_RC__NOOP', `SCL', `NHD', `NUQ')
rule(`SAC_ND_INC_RC', `SAC_ND_INC_RC__NOOP', `SCL', `*HID', `UNQ')
rule(`SAC_ND_INC_RC', `SAC_ND_INC_RC__NOOP', `AKS', `*HID', `UNQ')
rule(`SAC_ND_INC_RC', `SAC_ND_INC_RC__DEFAULT', `*SHP', `*HID', `NUQ')


pat(`SAC_ND_DEC_RC', `0', `1', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_DEC_RC', `SAC_ND_DEC_RC__NOOP', `*SHP', `*HID', `UNQ')
rule(`SAC_ND_DEC_RC', `SAC_ND_DEC_RC__NOOP', `SCL', `NHD', `NUQ')
rule(`SAC_ND_DEC_RC', `SAC_ND_DEC_RC__NOOP', `SCL', `*HID', `UNQ')
rule(`SAC_ND_DEC_RC', `SAC_ND_DEC_RC__NOOP', `AKS', `*HID', `UNQ')
rule(`SAC_ND_DEC_RC', `SAC_ND_DEC_RC__DEFAULT', `*SHP', `*HID', `NUQ')


pat(`SAC_ND_DEC_RC_FREE', `0', `2', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_DEC_RC_FREE', `SAC_ND_DEC_RC_FREE__UNQ', `*SHP', `*HID', `UNQ')
rule(`SAC_ND_DEC_RC_FREE', `SAC_ND_DEC_RC_FREE__NOOP', `SCL', `NHD', `NUQ')
rule(`SAC_ND_DEC_RC_FREE', `SAC_ND_DEC_RC_FREE__NOOP', `SCL', `HNS', `NUQ')
rule(`SAC_ND_DEC_RC_FREE', `SAC_ND_DEC_RC_FREE__NOOP', `SCL', `*HID', `UNQ')
rule(`SAC_ND_DEC_RC_FREE', `SAC_ND_DEC_RC_FREE__NOOP', `AKS', `*HID', `UNQ')
rule(`SAC_ND_DEC_RC_FREE', `SAC_ND_DEC_RC_FREE__DEFAULT', `*SHP', `*HID', `NUQ')


pat(`SAC_IS_LASTREF__BLOCK_BEGIN', `0', `0', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_IS_LASTREF__BLOCK_BEGIN', `SAC_IS_LASTREF__BLOCK_BEGIN__UNQ', `*SHP', `*HID', `UNQ')
rule(`SAC_IS_LASTREF__BLOCK_BEGIN', `SAC_IS_LASTREF__BLOCK_BEGIN__SCL_NHD_NUQ', `SCL', `NHD', `NUQ')
rule(`SAC_IS_LASTREF__BLOCK_BEGIN', `SAC_IS_LASTREF__BLOCK_BEGIN__DEFAULT', `*SHP', `*HID', `NUQ')


pat(`SAC_IS_REUSED__BLOCK_BEGIN', `0', `1', `NT_SHP')
rule(`SAC_IS_REUSED__BLOCK_BEGIN', `SAC_IS_REUSED__BLOCK_BEGIN__SCL', `SCL')
rule(`SAC_IS_REUSED__BLOCK_BEGIN', `SAC_IS_REUSED__BLOCK_BEGIN__DEFAULT', `*SHP')

end_icm_definition

