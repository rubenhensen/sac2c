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


pat(`SAC_NT_CBASETYPE', `0', `0', `NT_CBT')
rule(`SAC_NT_CBASETYPE', `SAC_NT_CBASETYPE__INT', `INT')
rule(`SAC_NT_CBASETYPE', `SAC_NT_CBASETYPE__FLO', `FLO')
rule(`SAC_NT_CBASETYPE', `SAC_NT_CBASETYPE__DOU', `DOU')
rule(`SAC_NT_CBASETYPE', `SAC_NT_CBASETYPE__UCH', `UCH')
rule(`SAC_NT_CBASETYPE', `SAC_NT_CBASETYPE__BOO', `BOO')
rule(`SAC_NT_CBASETYPE', `SAC_NT_CBASETYPE__BYT', `BYT')
rule(`SAC_NT_CBASETYPE', `SAC_NT_CBASETYPE__SHO', `SHO')
rule(`SAC_NT_CBASETYPE', `SAC_NT_CBASETYPE__LON', `LON')
rule(`SAC_NT_CBASETYPE', `SAC_NT_CBASETYPE__LLO', `LLO')
rule(`SAC_NT_CBASETYPE', `SAC_NT_CBASETYPE__UBY', `UBY')
rule(`SAC_NT_CBASETYPE', `SAC_NT_CBASETYPE__USH', `USH')
rule(`SAC_NT_CBASETYPE', `SAC_NT_CBASETYPE__UIN', `UIN')
rule(`SAC_NT_CBASETYPE', `SAC_NT_CBASETYPE__ULO', `ULO')
rule(`SAC_NT_CBASETYPE', `SAC_NT_CBASETYPE__ULL', `ULL')
rule(`SAC_NT_CBASETYPE', `SAC_NT_CBASETYPE__UNDEF', `*CBT')


pat(`SAC_NT_PRINT_CBASETYPE', `0', `0', `NT_CBT')
rule(`SAC_NT_PRINT_CBASETYPE', `SAC_NT_PRINT_CBASETYPE__INT', `INT')
rule(`SAC_NT_PRINT_CBASETYPE', `SAC_NT_PRINT_CBASETYPE__FLO', `FLO')
rule(`SAC_NT_PRINT_CBASETYPE', `SAC_NT_PRINT_CBASETYPE__DOU', `DOU')
rule(`SAC_NT_PRINT_CBASETYPE', `SAC_NT_PRINT_CBASETYPE__UCH', `UCH')
rule(`SAC_NT_PRINT_CBASETYPE', `SAC_NT_PRINT_CBASETYPE__BOO', `BOO')
rule(`SAC_NT_PRINT_CBASETYPE', `SAC_NT_PRINT_CBASETYPE__BYT', `BYT')
rule(`SAC_NT_PRINT_CBASETYPE', `SAC_NT_PRINT_CBASETYPE__SHO', `SHO')
rule(`SAC_NT_PRINT_CBASETYPE', `SAC_NT_PRINT_CBASETYPE__LON', `LON')
rule(`SAC_NT_PRINT_CBASETYPE', `SAC_NT_PRINT_CBASETYPE__LLO', `LLO')
rule(`SAC_NT_PRINT_CBASETYPE', `SAC_NT_PRINT_CBASETYPE__UBY', `UBY')
rule(`SAC_NT_PRINT_CBASETYPE', `SAC_NT_PRINT_CBASETYPE__USH', `USH')
rule(`SAC_NT_PRINT_CBASETYPE', `SAC_NT_PRINT_CBASETYPE__UIN', `UIN')
rule(`SAC_NT_PRINT_CBASETYPE', `SAC_NT_PRINT_CBASETYPE__ULO', `ULO')
rule(`SAC_NT_PRINT_CBASETYPE', `SAC_NT_PRINT_CBASETYPE__ULL', `ULL')
rule(`SAC_NT_PRINT_CBASETYPE', `SAC_NT_PRINT_CBASETYPE__UNDEF', `*CBT')


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


dnl Macros for CUDA backend (allocators)
pat(`SAC_ND_A_DESC_CUDA_PINNED', `0', `0', `NT_SHP')
rule(`SAC_ND_A_DESC_CUDA_PINNED', `SAC_ND_A_DESC_CUDA_PINNED__UNDEF', `SCL')
rule(`SAC_ND_A_DESC_CUDA_PINNED', `SAC_ND_A_DESC_CUDA_PINNED__DEFAULT', `*SHP')


dnl Macros for Distmem backend
pat(`SAC_ND_A_DESC_IS_DIST', `0', `0', `NT_DIS')
rule(`SAC_ND_A_DESC_IS_DIST', `SAC_ND_A_DESC_IS_DIST__DEFAULT', `DIS')
rule(`SAC_ND_A_DESC_IS_DIST', `SAC_ND_A_DESC_IS_DIST__DEFAULT', `DSM')
rule(`SAC_ND_A_DESC_IS_DIST', `SAC_ND_A_DESC_IS_DIST__UNDEF', `*DIS')


pat(`SAC_ND_A_DESC_OFFS', `0', `0', `NT_SHP')
rule(`SAC_ND_A_DESC_OFFS', `SAC_ND_A_DESC_OFFS__DEFAULT', `AUD')
rule(`SAC_ND_A_DESC_OFFS', `SAC_ND_A_DESC_OFFS__DEFAULT', `AKS')
rule(`SAC_ND_A_DESC_OFFS', `SAC_ND_A_DESC_OFFS__DEFAULT', `AKD')
rule(`SAC_ND_A_DESC_OFFS', `SAC_ND_A_DESC_OFFS__UNDEF', `*SHP')


pat(`SAC_ND_A_DESC_FIRST_ELEMS', `0', `0', `NT_DIS')
rule(`SAC_ND_A_DESC_FIRST_ELEMS', `SAC_ND_A_DESC_FIRST_ELEMS__DEFAULT', `DIS')
rule(`SAC_ND_A_DESC_FIRST_ELEMS', `SAC_ND_A_DESC_FIRST_ELEMS__UNDEF', `*DIS')


pat(`SAC_ND_A_DESC_PTR', `0', `1', `NT_DIS')
rule(`SAC_ND_A_DESC_PTR', `SAC_ND_A_DESC_PTR__DEFAULT', `DIS')
rule(`SAC_ND_A_DESC_PTR', `SAC_ND_A_DESC_PTR__UNDEF', `*DIS')


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

dnl Macros for CUDA backend (allocators)
pat(`SAC_ND_A_MIRROR_CUDA_PINNED', `0', `0', `NT_SHP')
rule(`SAC_ND_A_MIRROR_CUDA_PINNED', `SAC_ND_A_MIRROR_CUDA_PINNED__SCL', `SCL')
rule(`SAC_ND_A_MIRROR_CUDA_PINNED', `SAC_ND_A_MIRROR_CUDA_PINNED__DEFAULT', `*SHP')

dnl Macros for Distmem backend
pat(`SAC_ND_A_MIRROR_IS_DIST', `0', `0', `NT_DIS')
rule(`SAC_ND_A_MIRROR_IS_DIST', `SAC_ND_A_MIRROR_IS_DIST__DEFAULT', `DIS')
rule(`SAC_ND_A_MIRROR_IS_DIST', `SAC_ND_A_MIRROR_IS_DIST__DEFAULT', `DSM')
rule(`SAC_ND_A_MIRROR_IS_DIST', `SAC_ND_A_MIRROR_IS_DIST__DEFAULT', `DLO')
rule(`SAC_ND_A_MIRROR_IS_DIST', `SAC_ND_A_MIRROR_IS_DIST__UNDEF', `*DIS')


pat(`SAC_ND_A_MIRROR_OFFS', `0', `0', `NT_DIS')
rule(`SAC_ND_A_MIRROR_OFFS', `SAC_ND_A_MIRROR_OFFS__DEFAULT', `DIS')
rule(`SAC_ND_A_MIRROR_OFFS', `SAC_ND_A_MIRROR_OFFS__DEFAULT', `DSM')
rule(`SAC_ND_A_MIRROR_OFFS', `SAC_ND_A_MIRROR_OFFS__DEFAULT', `DCA')
rule(`SAC_ND_A_MIRROR_OFFS', `SAC_ND_A_MIRROR_OFFS__DEFAULT', `DLO')
rule(`SAC_ND_A_MIRROR_OFFS', `SAC_ND_A_MIRROR_OFFS__UNDEF', `*DIS')


pat(`SAC_ND_A_MIRROR_FIRST_ELEMS', `0', `0', `NT_DIS')
rule(`SAC_ND_A_MIRROR_FIRST_ELEMS', `SAC_ND_A_MIRROR_FIRST_ELEMS__DEFAULT', `DIS')
rule(`SAC_ND_A_MIRROR_FIRST_ELEMS', `SAC_ND_A_MIRROR_FIRST_ELEMS__DEFAULT', `DCA')
rule(`SAC_ND_A_MIRROR_FIRST_ELEMS', `SAC_ND_A_MIRROR_FIRST_ELEMS__DEFAULT', `DLO')
rule(`SAC_ND_A_MIRROR_FIRST_ELEMS', `SAC_ND_A_MIRROR_FIRST_ELEMS__UNDEF', `*DIS')


pat(`SAC_ND_A_MIRROR_LOCAL_FROM', `0', `0', `NT_DIS')
rule(`SAC_ND_A_MIRROR_LOCAL_FROM', `SAC_ND_A_MIRROR_LOCAL_FROM__DEFAULT', `DIS')
rule(`SAC_ND_A_MIRROR_LOCAL_FROM', `SAC_ND_A_MIRROR_LOCAL_FROM__UNDEF', `*DIS')


pat(`SAC_ND_A_MIRROR_LOCAL_COUNT', `0', `0', `NT_DIS')
rule(`SAC_ND_A_MIRROR_LOCAL_COUNT', `SAC_ND_A_MIRROR_LOCAL_COUNT__DEFAULT', `DIS')
rule(`SAC_ND_A_MIRROR_LOCAL_COUNT', `SAC_ND_A_MIRROR_LOCAL_COUNT__UNDEF', `*DIS')


pat(`SAC_ND_A_MIRROR_PTR_CACHE', `0', `0', `NT_DIS')
rule(`SAC_ND_A_MIRROR_PTR_CACHE', `SAC_ND_A_MIRROR_PTR_CACHE__DEFAULT', `DIS')
rule(`SAC_ND_A_MIRROR_PTR_CACHE', `SAC_ND_A_MIRROR_PTR_CACHE__UNDEF', `*DIS')


pat(`SAC_ND_A_MIRROR_PTR_CACHE_FROM', `0', `0', `NT_DIS')
rule(`SAC_ND_A_MIRROR_PTR_CACHE_FROM', `SAC_ND_A_MIRROR_PTR_CACHE_FROM__DEFAULT', `DIS')
rule(`SAC_ND_A_MIRROR_PTR_CACHE_FROM', `SAC_ND_A_MIRROR_PTR_CACHE_FROM__UNDEF', `*DIS')


pat(`SAC_ND_A_MIRROR_PTR_CACHE_COUNT', `0', `0', `NT_DIS')
rule(`SAC_ND_A_MIRROR_PTR_CACHE_COUNT', `SAC_ND_A_MIRROR_PTR_CACHE_COUNT__DEFAULT', `DIS')
rule(`SAC_ND_A_MIRROR_PTR_CACHE_COUNT', `SAC_ND_A_MIRROR_PTR_CACHE_COUNT__UNDEF', `*DIS')


pat(`SAC_ND_A_FIELD', `0', `0')
rule(`SAC_ND_A_FIELD', `SAC_ND_A_FIELD__DEFAULT')

pat(`SAC_ND_A_FIELD_UNBOX', `0', `0', `NT_SHP')
rule(`SAC_ND_A_FIELD_UNBOX', `SAC_ND_A_FIELD', `SCL')
rule(`SAC_ND_A_FIELD_UNBOX', `SAC_ND_A_FIELD__BOXED', `*SHP')


pat(`SAC_ND_A_RC', `0', `0', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_A_RC', `SAC_ND_A_RC__UNDEF', `*SHP', `*HID', `UNQ')
rule(`SAC_ND_A_RC', `SAC_ND_A_RC__UNDEF', `SCL', `NHD', `NUQ')
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

pat(`SAC_ND_A_CUDA_PINNED', `0' `0',)
rule(`SAC_ND_A_CUDA_PINNED', `SAC_ND_A_CUDA_PINNED__DEFAULT')

pat(`SAC_ND_A_IS_DIST', `0', `0', `NT_DIS')
rule(`SAC_ND_A_IS_DIST', `SAC_ND_A_IS_DIST__DIS_DSM', `DIS')
rule(`SAC_ND_A_IS_DIST', `SAC_ND_A_IS_DIST__DIS_DSM', `DSM')
rule(`SAC_ND_A_IS_DIST', `SAC_ND_A_IS_DIST__DIS_DSM', `DLO')
rule(`SAC_ND_A_IS_DIST', `SAC_ND_A_IS_DIST__DEFAULT', `*DIS')


pat(`SAC_ND_A_IS_DSM', `0', `0', `NT_DIS')
rule(`SAC_ND_A_IS_DSM', `SAC_ND_A_IS_DSM__DSM', `DSM')
rule(`SAC_ND_A_IS_DSM', `SAC_ND_A_IS_DSM__DEFAULT', `*DIS')


pat(`SAC_ND_A_FIRST_ELEMS', `0', `0', `NT_DIS')
rule(`SAC_ND_A_FIRST_ELEMS', `SAC_ND_A_FIRST_ELEMS__DIS', `DIS')
rule(`SAC_ND_A_FIRST_ELEMS', `SAC_ND_A_FIRST_ELEMS__DIS', `DCA')
rule(`SAC_ND_A_FIRST_ELEMS', `SAC_ND_A_FIRST_ELEMS__DIS', `DLO')
dnl For DSM we refer to the size on purpose!
rule(`SAC_ND_A_FIRST_ELEMS', `SAC_ND_A_SIZE__DEFAULT', `DSM')
rule(`SAC_ND_A_FIRST_ELEMS', `SAC_ND_A_FIRST_ELEMS__DEFAULT', `*DIS')


pat(`SAC_ND_A_LOCAL_FROM', `0', `0', `NT_DIS')
rule(`SAC_ND_A_LOCAL_FROM', `SAC_ND_A_LOCAL_FROM__DIS', `DIS')
rule(`SAC_ND_A_LOCAL_FROM', `SAC_ND_A_LOCAL_FROM__DEFAULT', `*DIS')


pat(`SAC_ND_A_LOCAL_COUNT', `0', `0', `NT_DIS')
rule(`SAC_ND_A_LOCAL_COUNT', `SAC_ND_A_LOCAL_COUNT__DIS', `DIS')
rule(`SAC_ND_A_LOCAL_COUNT', `SAC_ND_A_LOCAL_COUNT__DEFAULT', `*DIS')


pat(`SAC_ND_A_OFFS', `0', `0',  `NT_DIS')
rule(`SAC_ND_A_OFFS', `SAC_ND_A_OFFS__DIS_DSM', `DIS')
rule(`SAC_ND_A_OFFS', `SAC_ND_A_OFFS__DIS_DSM', `DSM')
rule(`SAC_ND_A_OFFS', `SAC_ND_A_OFFS__DIS_DSM', `DCA')
rule(`SAC_ND_A_OFFS', `SAC_ND_A_OFFS__DIS_DSM', `DLO')
rule(`SAC_ND_A_OFFS', `SAC_ND_A_OFFS__DEFAULT', `*DIS')


pat(`SAC_ND_READ', `0', `1', `NT_SHP', `NT_BIT', `NT_DIS')
rule(`SAC_ND_READ', `SAC_ND_READ__SCL', `SCL', `*BIT', `*DIS')
rule(`SAC_ND_READ', `SAC_ND_READ__BITARRAY', `*SHP', `YES', `*DIS')
rule(`SAC_ND_READ', `SAC_ND_READ__DLO', `*SHP', `*BIT', `DLO')
rule(`SAC_ND_READ', `SAC_ND_READ__DIS', `*SHP', `*BIT', `DIS')
rule(`SAC_ND_READ', `SAC_ND_READ__DSM', `*SHP', `*BIT', `DSM')
rule(`SAC_ND_READ', `SAC_ND_READ__DEFAULT', `*SHP', `*BIT', `*DIS')


pat(`SAC_ND_WRITE', `0', `1', `NT_SHP', `NT_BIT', `NT_DIS')
rule(`SAC_ND_WRITE', `SAC_ND_WRITE__SCL', `SCL', `*BIT', `*DIS')
rule(`SAC_ND_WRITE', `SAC_ND_WRITE__BITARRAY', `*SHP', `YES', `*DIS')
rule(`SAC_ND_WRITE', `SAC_ND_WRITE__DIS', `*SHP', `*BIT', `DIS')
rule(`SAC_ND_WRITE', `SAC_ND_WRITE__DCA', `*SHP', `*BIT', `DCA')
rule(`SAC_ND_WRITE', `SAC_ND_WRITE__DSM', `*SHP', `*BIT', `DSM')
rule(`SAC_ND_WRITE', `SAC_ND_WRITE__DEFAULT', `*SHP', `*BIT', `*DIS')


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

pat(`SAC_ND_ALLOC__DESC__FIXED_C99', `0', `1', `NT_DIS')
rule(`SAC_ND_ALLOC__DESC__FIXED_C99', `SAC_ND_ALLOC__DESC__FIXED_C99__DIS', `DIS')
rule(`SAC_ND_ALLOC__DESC__FIXED_C99', `SAC_ND_ALLOC__DESC__FIXED_C99__DEFAULT', `*DIS')

pat(`SAC_ND_ALLOC__DESC__AUD_C99', `0', `1', `NT_DIS')
rule(`SAC_ND_ALLOC__DESC__AUD_C99', `SAC_ND_ALLOC__DESC__AUD_C99__DIS', `DIS')
rule(`SAC_ND_ALLOC__DESC__AUD_C99', `SAC_ND_ALLOC__DESC__AUD_C99__DEFAULT', `*DIS')

pat(`SAC_DESC_ALLOC', `0', `1', `NT_DIS')
rule(`SAC_DESC_ALLOC', `SAC_DESC_ALLOC__DIS', `DIS')
rule(`SAC_DESC_ALLOC', `SAC_DESC_ALLOC__DEFAULT', `*DIS')

pat(`SAC_ND_ALLOC__DATA', `0', `0', `NT_SHP', `NT_DIS')
rule(`SAC_ND_ALLOC__DATA', `SAC_ND_ALLOC__DATA__NOOP', `SCL', `*DIS')
rule(`SAC_ND_ALLOC__DATA', `SAC_ND_ALLOC__DATA__AKS_DIS', `AKS', `DIS')
rule(`SAC_ND_ALLOC__DATA', `SAC_ND_ALLOC__DATA__AKS', `AKS', `*DIS')
rule(`SAC_ND_ALLOC__DATA', `SAC_ND_ALLOC__DATA__AKD_AUD_DIS', `*SHP', `DIS')
rule(`SAC_ND_ALLOC__DATA', `SAC_ND_ALLOC__DATA__AKD_AUD', `*SHP', `*DIS')


pat(`SAC_ND_ALLOC__DATA_BASETYPE', `0', `1', `NT_SHP', `NT_DIS')
rule(`SAC_ND_ALLOC__DATA_BASETYPE', `SAC_ND_ALLOC__DATA_BASETYPE__NOOP', `SCL', `*DIS')
rule(`SAC_ND_ALLOC__DATA_BASETYPE', `SAC_ND_ALLOC__DATA_BASETYPE__AKS_DIS', `AKS', `DIS')
rule(`SAC_ND_ALLOC__DATA_BASETYPE', `SAC_ND_ALLOC__DATA_BASETYPE__AKS', `AKS', `*DIS')
rule(`SAC_ND_ALLOC__DATA_BASETYPE', `SAC_ND_ALLOC__DATA_BASETYPE__AKD_AUD_DIS', `*SHP', `DIS')
rule(`SAC_ND_ALLOC__DATA_BASETYPE', `SAC_ND_ALLOC__DATA_BASETYPE__AKD_AUD', `*SHP', `*DIS')


pat(`SAC_ND_ALLOC__DESC_AND_DATA', `0', `2', `NT_SHP', `NT_HID', `NT_UNQ', `NT_DIS')
rule(`SAC_ND_ALLOC__DESC_AND_DATA', `SAC_ND_ALLOC__DESC__NOOP_BASETYPE', `SCL', `NHD', `*UNQ', `*DIS')
rule(`SAC_ND_ALLOC__DESC_AND_DATA', `SAC_ND_ALLOC__DESC__NOOP_BASETYPE', `SCL', `HID', `UNQ', `*DIS')
rule(`SAC_ND_ALLOC__DESC_AND_DATA', `SAC_ND_ALLOC__DESC__NOOP_BASETYPE', `SCL', `*HID', `UNQ', `*DIS')
rule(`SAC_ND_ALLOC__DESC_AND_DATA', `SAC_ND_ALLOC__DESC__NOOP_BASETYPE', `AKS', `*HID', `UNQ', `*DIS')
rule(`SAC_ND_ALLOC__DESC_AND_DATA', `SAC_ND_ALLOC__DESC__FIXED_BASETYPE', `SCL', `HID', `NUQ', `*DIS')
rule(`SAC_ND_ALLOC__DESC_AND_DATA', `SAC_ND_ALLOC__DESC__NOOP_BASETYPE', `SCL', `HNS', `NUQ', `*DIS')
rule(`SAC_ND_ALLOC__DESC_AND_DATA', `SAC_ND_ALLOC__DESC_AND_DATA_NESTED__AKS', `AKS', `HNS', `NUQ', `*DIS')
rule(`SAC_ND_ALLOC__DESC_AND_DATA', `SAC_ND_ALLOC__DESC_AND_DATA__AKS_DIS', `AKS', `*HID', `*UNQ', `DIS')
rule(`SAC_ND_ALLOC__DESC_AND_DATA', `SAC_ND_ALLOC__DESC_AND_DATA__AKS', `AKS', `*HID', `*UNQ', `*DIS')
rule(`SAC_ND_ALLOC__DESC_AND_DATA', `SAC_ND_ALLOC__DESC_AND_DATA__UNDEF', `*SHP', `*HID', `*UNQ', `*DIS')


pat(`SAC_ND_FREE__DESC', `0', `0', `NT_SHP', `NT_HID', `NT_UNQ', `NT_DIS')
rule(`SAC_ND_FREE__DESC', `SAC_ND_FREE__DESC__NOOP', `SCL', `NHD', `*UNQ', `*DIS')
rule(`SAC_ND_FREE__DESC', `SAC_ND_FREE__DESC__NOOP', `SCL', `HID', `UNQ', `*DIS')
rule(`SAC_ND_FREE__DESC', `SAC_ND_FREE__DESC__NOOP', `SCL', `HNS', `NUQ', `*DIS')
rule(`SAC_ND_FREE__DESC', `SAC_ND_FREE__DESC__NOOP', `SCL', `*HID', `UNQ', `*DIS')
rule(`SAC_ND_FREE__DESC', `SAC_ND_FREE__DESC__NOOP', `AKS', `*HID', `UNQ', `*DIS')
rule(`SAC_ND_FREE__DESC', `SAC_ND_FREE__DESC__DEFAULT', `*SHP', `*HID', `*UNQ', `*DIS')


pat(`SAC_ND_FREE__DATA', `0', `1', `NT_SHP', `NT_HID', `NT_DIS')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__SCL_NHD', `SCL', `NHD', `*DIS')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__SCL_HID', `SCL', `HID', `*DIS')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__SCL_HNS', `SCL', `HNS', `*DIS')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__AKS_NHD_DIS', `AKS', `NHD', `DIS')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__AKS_NHD', `AKS', `NHD', `*DIS')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__AKS_HID', `AKS', `HID', `*DIS')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__AKS_HNS', `AKS', `HNS', `*DIS')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__AKD_AUD_NHD_DIS', `AKD', `NHD', `DIS')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__AKD_NHD', `AKD', `NHD', `*DIS')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__AKS_HID', `AKD', `HID', `*DIS')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__AKD_AUD_NHD_DIS', `AUD', `NHD', `DIS')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__AKD_NHD', `AUD', `NHD', `*DIS')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__AKS_HID', `AUD', `HID', `*DIS')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__AKS_NHD', `*SHP', `NHD', `*DIS')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__AKS_HID', `*SHP', `HID', `*DIS')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__AKD_HNS', `AKD', `HNS', `*DIS')
rule(`SAC_ND_FREE__DATA', `SAC_ND_FREE__DATA__AKD_HNS', `AUD', `HNS', `*DIS')

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
dnl     AKS and AKD DIS (only if distributed memory backend is used)
dnl     AKS and AKD DSM (only if distributed memory backend is used)
dnl     AKS and AKD NDI
dnl     AUD, NHD DIS (only if distributed memory backend is used)
dnl     AUD, NHD DSM (only if distributed memory backend is used)
dnl     AUD, NHD NDI 
dnl     AUD, HID
dnl
dnl These are actually reduced to less target macro's, but we still need
dnl them.
pat(`SAC_ND_ASSIGN__DATA', `0', `2', `NT_SHP', `NT_HID', `NT_DIS')
rule(`SAC_ND_ASSIGN__DATA', `GEN_SAC_ND_ASSIGN__DATA__SCL_NHD', `SCL', `NHD', `*DIS')
rule(`SAC_ND_ASSIGN__DATA', `GEN_SAC_ND_ASSIGN__DATA__SCL_HID', `SCL', `HID', `*DIS')
rule(`SAC_ND_ASSIGN__DATA', `GEN_SAC_ND_ASSIGN__DATA__AKS_AKD_DIS', `AKS', `NHD', `DIS')
rule(`SAC_ND_ASSIGN__DATA', `GEN_SAC_ND_ASSIGN__DATA__AKS_AKD_DSM', `AKS', `NHD', `DSM')
rule(`SAC_ND_ASSIGN__DATA', `GEN_SAC_ND_ASSIGN__DATA__AKS', `AKS', `*HID', `*DIS')
rule(`SAC_ND_ASSIGN__DATA', `GEN_SAC_ND_ASSIGN__DATA__AKS_AKD_DIS', `AKD', `NHD', `DIS')
rule(`SAC_ND_ASSIGN__DATA', `GEN_SAC_ND_ASSIGN__DATA__AKS_AKD_DSM', `AKD', `NHD', `DSM')
rule(`SAC_ND_ASSIGN__DATA', `GEN_SAC_ND_ASSIGN__DATA__AKS', `AKD', `*HID', `*DIS')
rule(`SAC_ND_ASSIGN__DATA', `GEN_SAC_ND_ASSIGN__DATA__AUD_NHD_DIS', `AUD', `NHD', `DIS')
rule(`SAC_ND_ASSIGN__DATA', `GEN_SAC_ND_ASSIGN__DATA__AUD_NHD_DSM', `AUD', `NHD', `DSM')
rule(`SAC_ND_ASSIGN__DATA', `GEN_SAC_ND_ASSIGN__DATA__AUD_NHD', `AUD', `NHD', `*DIS')
rule(`SAC_ND_ASSIGN__DATA', `GEN_SAC_ND_ASSIGN__DATA__AUD_HID', `AUD', `HID', `*DIS')
rule(`SAC_ND_ASSIGN__DATA', `GEN_SAC_ND_ASSIGN__DATA__AUD_HNS', `AUD', `HNS', `*DIS')

dnl GEN_SAC_ND_ASSIGN__DATA__SCL_NHD
dnl
dnl Left-hand NT is a SCL NHD. There are three cases for the RHS:
dnl     SCL -> do a direct assignment of the scl value
dnl     AUD NHD -> copy aud value to scl
dnl     AKS, AKD, AUD HID -> undef (can't assign)
pat(`GEN_SAC_ND_ASSIGN__DATA__SCL_NHD', `1', `1', `NT_SHP', `NT_HID')
rule(`GEN_SAC_ND_ASSIGN__DATA__SCL_NHD', `SAC_ND_ASSIGN__DATA__AKS_AKS', `SCL', `*HID')
rule(`GEN_SAC_ND_ASSIGN__DATA__SCL_NHD', `SAC_ND_ASSIGN__DATA__SCL_AUD', `AUD', `NHD')
rule(`GEN_SAC_ND_ASSIGN__DATA__SCL_NHD', `SAC_ND_ASSIGN__DATA__UNDEF', `*SHP', `*HID')

dnl GEN_SAC_ND_ASSIGN__DATA__SCL_HID
dnl
dnl Left-hand NT is a SCL HID. There are four cases for the RHS:
dnl     SCL -> do a direct assignment of the scl value
dnl     AUD HID NUQ  -> copy aud value to scl
dnl     AUD HID UNQ  -> copy aud value to scl, free unique aud
dnl     AKS, AKD, AUD NHD -> undef (can't assign)
pat(`GEN_SAC_ND_ASSIGN__DATA__SCL_HID', `1', `1', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`GEN_SAC_ND_ASSIGN__DATA__SCL_HID', `SAC_ND_ASSIGN__DATA__AKS_AKS', `SCL', `*HID', `*UNQ')
rule(`GEN_SAC_ND_ASSIGN__DATA__SCL_HID', `SAC_ND_ASSIGN__DATA__SCL_AUD', `AUD', `HID', `NUQ')
rule(`GEN_SAC_ND_ASSIGN__DATA__SCL_HID', `SAC_ND_ASSIGN__DATA__SCL_AUD_UNQ', `AUD', `HID', `UNQ')
rule(`GEN_SAC_ND_ASSIGN__DATA__SCL_HID', `SAC_ND_ASSIGN__DATA__UNDEF', `*SHP', `*HID', `*UNQ')

dnl GEN_SAC_ND_ASSIGN__DATA__AKS
dnl
dnl Left-hand NT is a AKS NDI or AKD NDI. There are four cases for the RHS:
dnl     SCL -> undef (can't assign)
dnl     AKS DIS, AKD DIS or AUD DIS -> action depends on whether rhs is actually distributed
dnl     AKS DSM, AKD DSM or AUD DSM -> action depends on whether rhs is actually allocated in DSM memory
dnl     AKS NDI, AKD NDI or AUD NDI -> do a direct assignment
pat(`GEN_SAC_ND_ASSIGN__DATA__AKS', `1', `1', `NT_SHP', `NT_DIS')
rule(`GEN_SAC_ND_ASSIGN__DATA__AKS', `SAC_ND_ASSIGN__DATA__UNDEF', `SCL', `*DIS')
rule(`GEN_SAC_ND_ASSIGN__DATA__AKS', `SAC_ND_ASSIGN__DATA__TO_NDI', `*SHP', `DIS')
rule(`GEN_SAC_ND_ASSIGN__DATA__AKS', `SAC_ND_ASSIGN__DATA__TO_NDI', `*SHP', `DSM')
rule(`GEN_SAC_ND_ASSIGN__DATA__AKS', `SAC_ND_ASSIGN__DATA__AKS_AKS', `*SHP', `*DIS')

dnl GEN_SAC_ND_ASSIGN__DATA__AKS_AKD_DIS
dnl
dnl Left-hand NT is a AKS DIS or AKD DIS. There are two cases for the RHS:
dnl     SCL -> undef (can't assign)
dnl     AKS, AKD or AUD -> action depends on whether lhs is actually distributed 
dnl                        and rhs are is actually distributed/allocated in DSM memory
pat(`GEN_SAC_ND_ASSIGN__DATA__AKS_AKD_DIS', `1', `1', `NT_SHP', `NT_DIS')
rule(`GEN_SAC_ND_ASSIGN__DATA__AKS_AKD_DIS', `SAC_ND_ASSIGN__DATA__UNDEF', `SCL', `*DIS')
rule(`GEN_SAC_ND_ASSIGN__DATA__AKS_AKD_DIS', `SAC_ND_ASSIGN__DATA__TO_DIS', `*SHP', `*DIS')

dnl GEN_SAC_ND_ASSIGN__DATA__AKS_AKD_DSM
dnl
dnl Left-hand NT is a AKS DSM or AKD DSM. There are two cases for the RHS:
dnl     SCL -> undef (can't assign)
dnl     AKS, AKD or AUD -> action depends on whether lhs is actually allocated in DSM memory 
dnl                        and rhs are is actually distributed/allocated in DSM memory
pat(`GEN_SAC_ND_ASSIGN__DATA__AKS_AKD_DSM', `1', `1', `NT_SHP', `NT_DIS')
rule(`GEN_SAC_ND_ASSIGN__DATA__AKS_AKD_DSM', `SAC_ND_ASSIGN__DATA__UNDEF', `SCL', `*DIS')
rule(`GEN_SAC_ND_ASSIGN__DATA__AKS_AKD_DSM', `SAC_ND_ASSIGN__DATA__TO_DSM', `*SHP', `*DIS')

dnl GEN_SAC_ND_ASSIGN__DATA__AUD_NHD
dnl
dnl Left-hand NT is a AUD NHD NDI. There are five cases for the RHS:
dnl     SCL NHD -> allocate aud, copy scl
dnl     SCL HID -> undef (can't assign)
dnl     AKS DIS, AKD DIS, AUD DIS -> action depends on whether rhs is actually distributed
dnl     AKS DSM, AKD DSM, AUD DSM -> action depends on whether rhs is actually allocated in DSM memory
dnl     AKS NDI, AKD NDI, AUD NDI -> do a direct assignment
pat(`GEN_SAC_ND_ASSIGN__DATA__AUD_NHD', `1', `1', `NT_SHP', `NT_HID', `NT_DIS')
rule(`GEN_SAC_ND_ASSIGN__DATA__AUD_NHD', `SAC_ND_ASSIGN__DATA__AUD_SCL_NHD', `SCL', `NHD', `*DIS')
rule(`GEN_SAC_ND_ASSIGN__DATA__AUD_NHD', `SAC_ND_ASSIGN__DATA__UNDEF', `SCL', `HID', `*DIS')
rule(`GEN_SAC_ND_ASSIGN__DATA__AUD_NHD', `SAC_ND_ASSIGN__DATA__TO_NDI', `*SHP', `*HID', `DIS')
rule(`GEN_SAC_ND_ASSIGN__DATA__AUD_NHD', `SAC_ND_ASSIGN__DATA__TO_NDI', `*SHP', `*HID', `DSM')
rule(`GEN_SAC_ND_ASSIGN__DATA__AUD_NHD', `SAC_ND_ASSIGN__DATA__AUD_AKS', `*SHP', `*HID', `*DIS')

dnl GEN_SAC_ND_ASSIGN__DATA__AUD_NHD_DIS
dnl
dnl Left-hand NT is a AUD NHD DIS. There are three cases for the RHS:
dnl     SCL NHD -> allocate aud, copy scl
dnl     SCL -> undef (can't assign)
dnl     AKS, AKD or AUD -> action depends on whether lhs is actually distributed 
dnl                        and rhs are is actually distributed/allocated in DSM memory
pat(`GEN_SAC_ND_ASSIGN__DATA__AUD_NHD_DIS', `1', `1', `NT_SHP', `NT_HID', `NT_DIS')
rule(`GEN_SAC_ND_ASSIGN__DATA__AUD_NHD_DIS', `SAC_ND_ASSIGN__DATA__AUD_SCL_NHD', `SCL', `NHD', `*DIS')
rule(`GEN_SAC_ND_ASSIGN__DATA__AUD_NHD_DIS', `SAC_ND_ASSIGN__DATA__UNDEF', `SCL', `HID', `*DIS')
rule(`GEN_SAC_ND_ASSIGN__DATA__AUD_NHD_DIS', `SAC_ND_ASSIGN__DATA__TO_DIS', `*SHP', `*HID', `*DIS')

dnl GEN_SAC_ND_ASSIGN__DATA__AUD_NHD_DSM
dnl
dnl Left-hand NT is a AUD NHD DSM. There are three cases for the RHS:
dnl     SCL NHD -> allocate aud, copy scl
dnl     SCL -> undef (can't assign)
dnl     AKS, AKD or AUD -> action depends on whether lhs is actually allocated in DSM memory 
dnl                        and rhs are is actually distributed/allocated in DSM memory
pat(`GEN_SAC_ND_ASSIGN__DATA__AUD_NHD_DSM', `1', `1', `NT_SHP', `NT_HID', `NT_DIS')
rule(`GEN_SAC_ND_ASSIGN__DATA__AUD_NHD_DSM', `SAC_ND_ASSIGN__DATA__AUD_SCL_NHD', `SCL', `NHD', `*DIS')
rule(`GEN_SAC_ND_ASSIGN__DATA__AUD_NHD_DSM', `SAC_ND_ASSIGN__DATA__UNDEF', `SCL', `HID', `*DIS')
rule(`GEN_SAC_ND_ASSIGN__DATA__AUD_NHD_DSM', `SAC_ND_ASSIGN__DATA__TO_DSM', `*SHP', `*HID', `*DIS')

dnl Left-hand NT is a AUD HID. There are four cases for the RHS:
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
pat(`SAC_ND_COPY__DATA', `0', `2', `NT_SHP', `NT_DIS')
rule(`SAC_ND_COPY__DATA', `GEN_SAC_ND_COPY__DATA__SCL', `SCL', `*DIS')
rule(`SAC_ND_COPY__DATA', `GEN_SAC_ND_COPY__DATA__DIS', `*SHP', `DIS')
rule(`SAC_ND_COPY__DATA', `GEN_SAC_ND_COPY__DATA__ANY', `*SHP', `*DIS')

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

dnl GEN_SAC_ND_COPY__DATA__DIS
dnl
dnl There are two cases for the RHS:
dnl   SCL -> copy scalar value
dnl   AKS, AKD, AUD -> do a data copy
pat(`GEN_SAC_ND_COPY__DATA__DIS', `1', `1', `NT_SHP')
rule(`GEN_SAC_ND_COPY__DATA__DIS', `SAC_ND_COPY__DATA__ANY_SCL', `SCL')
rule(`GEN_SAC_ND_COPY__DATA__DIS', `SAC_ND_COPY__DATA__DIS_ANY', `*SHP')

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

pat(`SAC_UPDATE_A_MIRROR_DIM', `0', `1', `NT_SHP')
rule(`SAC_UPDATE_A_MIRROR_DIM', `SAC_UPDATE_A_MIRROR_DIM__AUD', `AUD')
rule(`SAC_UPDATE_A_MIRROR_DIM', `SAC_UPDATE_A_MIRROR_DIM__AKD', `AKD')
rule(`SAC_UPDATE_A_MIRROR_DIM', `SAC_UPDATE_A_MIRROR_DIM__UNDEF', `*SHP')

pat(`SAC_A_DIM_BEFORE_UPDATE_MIRROR', `0', `0', `NT_SHP')
rule(`SAC_A_DIM_BEFORE_UPDATE_MIRROR', `SAC_A_DIM_BEFORE_UPDATE_MIRROR__AUD', `AUD')
rule(`SAC_A_DIM_BEFORE_UPDATE_MIRROR', `SAC_A_DIM_BEFORE_UPDATE_MIRROR__AKD', `AKD')
rule(`SAC_A_DIM_BEFORE_UPDATE_MIRROR', `SAC_A_DIM_BEFORE_UPDATE_MIRROR__UNDEF', `*SHP')

end_icm_definition

