

include(`icm.m4')

start_icm_definition(cuda_gen)

pat(`SAC_CUDA_ALLOC_BEGIN', `0', `3', `NT_SHP')
rule(`SAC_CUDA_ALLOC_BEGIN', `SAC_CUDA_ALLOC_BEGIN__DAO', `SCL')
rule(`SAC_CUDA_ALLOC_BEGIN', `SAC_CUDA_ALLOC_BEGIN__DAO', `AKS')
rule(`SAC_CUDA_ALLOC_BEGIN', `SAC_CUDA_ALLOC_BEGIN__NO_DAO', `AKD')
rule(`SAC_CUDA_ALLOC_BEGIN', `SAC_CUDA_ALLOC_BEGIN__NO_DAO', `AUD')


pat(`SAC_CUDA_ALLOC_END', `0', `3', `NT_SHP')
rule(`SAC_CUDA_ALLOC_END', `SAC_CUDA_ALLOC_END__DAO', `SCL')
rule(`SAC_CUDA_ALLOC_END', `SAC_CUDA_ALLOC_END__DAO', `AKS')
rule(`SAC_CUDA_ALLOC_END', `SAC_CUDA_ALLOC_END__NO_DAO', `AKD')
rule(`SAC_CUDA_ALLOC_END', `SAC_CUDA_ALLOC_END__NO_DAO', `AUD')

//----------------------------------------------------------------------

pat(`SAC_CUDA_ALLOC__DATA', `0', `1', `NT_SHP')
rule(`SAC_CUDA_ALLOC__DATA', `SAC_CUDA_ALLOC__DATA__NOOP', `SCL')
rule(`SAC_CUDA_ALLOC__DATA', `SAC_CUDA_ALLOC__DATA__AKS', `AKS')
rule(`SAC_CUDA_ALLOC__DATA', `SAC_CUDA_ALLOC__DATA__AKD_AUD', `*SHP')

//----------------------------------------------------------------------

/*
pat(`SAC_CUDA_HOST2DEVICE', `2', `NT_SHP')
rule(`SAC_CUDA_HOST2DEVICE', `GEN_SAC_CUDA_HOST2DEVICE__SCL', `SCL')
rule(`SAC_CUDA_HOST2DEVICE', `GEN_SAC_CUDA_HOST2DEVICE__AKS_AKD_AUD', `AKS')
rule(`SAC_CUDA_HOST2DEVICE', `GEN_SAC_CUDA_HOST2DEVICE__AKS_AKD_AUD', `AKD')
rule(`SAC_CUDA_HOST2DEVICE', `GEN_SAC_CUDA_HOST2DEVICE__AKS_AKD_AUD', `AUD')

pat(`GEN_SAC_CUDA_HOST2DEVICE__SCL', `1', `0', `NT_SHP')
rule(`GEN_SAC_CUDA_HOST2DEVICE__SCL', `SAC_CUDA_HOST2DEVICE__SCL', `SCL')

pat(`GEN_SAC_CUDA_HOST2DEVICE__AKS_AKD_AUD', `1', `0', `NT_SHP')
rule(`GEN_SAC_CUDA_HOST2DEVICE__AKS_AKD_AUD', `SAC_CUDA_HOST2DEVICE__AKS_AKD_AUD', `AKS')
rule(`GEN_SAC_CUDA_HOST2DEVICE__AKS_AKD_AUD', `SAC_CUDA_HOST2DEVICE__AKS_AKD_AUD', `AKD')
rule(`GEN_SAC_CUDA_HOST2DEVICE__AKS_AKD_AUD', `SAC_CUDA_HOST2DEVICE__AKS_AKD_AUD', `AUD')


pat(`SAC_CUDA_DEVICE2HOST', `2', `NT_SHP')
rule(`SAC_CUDA_DEVICE2HOST', `GEN_SAC_CUDA_DEVICE2HOST__SCL', `SCL')
rule(`SAC_CUDA_DEVICE2HOST', `GEN_SAC_CUDA_DEVICE2HOST__AKS_AKD_AUD', `AKS')
rule(`SAC_CUDA_DEVICE2HOST', `GEN_SAC_CUDA_DEVICE2HOST__AKS_AKD_AUD', `AKD')
rule(`SAC_CUDA_DEVICE2HOST', `GEN_SAC_CUDA_DEVICE2HOST__AKS_AKD_AUD', `AUD')

pat(`GEN_SAC_CUDA_DEVICE2HOST__SCL', `1', `0', `NT_SHP')
rule(`GEN_SAC_CUDA_DEVICE2HOST__SCL', `SAC_CUDA_DEVICE2HOST__SCL', `SCL')

pat(`GEN_SAC_CUDA_DEVICE2HOST__AKS_AKD_AUD', `1', `0', `NT_SHP')
rule(`GEN_SAC_CUDA_DEVICE2HOST__AKS_AKD_AUD', `SAC_CUDA_DEVICE2HOST__AKS_AKD_AUD', `AKS')
rule(`GEN_SAC_CUDA_DEVICE2HOST__AKS_AKD_AUD', `SAC_CUDA_DEVICE2HOST__AKS_AKD_AUD', `AKD')
rule(`GEN_SAC_CUDA_DEVICE2HOST__AKS_AKD_AUD', `SAC_CUDA_DEVICE2HOST__AKS_AKD_AUD', `AUD')
*/

pat(`SAC_CUDA_MEM_TRANSFER', `0',`3', `NT_SHP')
/*
rule(`SAC_CUDA_MEM_TRANSFER', `GEN_SAC_CUDA_MEM_TRANSFER__SCL', `SCL')
*/
rule(`SAC_CUDA_MEM_TRANSFER', `GEN_SAC_CUDA_MEM_TRANSFER__AKS_AKD_AUD', `AKS')
rule(`SAC_CUDA_MEM_TRANSFER', `GEN_SAC_CUDA_MEM_TRANSFER__AKS_AKD_AUD', `AKD')
rule(`SAC_CUDA_MEM_TRANSFER', `GEN_SAC_CUDA_MEM_TRANSFER__AKS_AKD_AUD', `AUD')

/*
pat(`GEN_SAC_CUDA_MEM_TRANSFER__SCL', `1', `2', `NT_SHP')
rule(`GEN_SAC_CUDA_MEM_TRANSFER__SCL', `SAC_CUDA_MEM_TRANSFER__SCL', `SCL')
*/

pat(`GEN_SAC_CUDA_MEM_TRANSFER__AKS_AKD_AUD', `1', `2', `NT_SHP')
rule(`GEN_SAC_CUDA_MEM_TRANSFER__AKS_AKD_AUD', `SAC_CUDA_MEM_TRANSFER__AKS_AKD_AUD', `AKS')
rule(`GEN_SAC_CUDA_MEM_TRANSFER__AKS_AKD_AUD', `SAC_CUDA_MEM_TRANSFER__AKS_AKD_AUD', `AKD')
rule(`GEN_SAC_CUDA_MEM_TRANSFER__AKS_AKD_AUD', `SAC_CUDA_MEM_TRANSFER__AKS_AKD_AUD', `AUD')



//----------------------------------------------------------------------

pat(`SAC_CUDA_PARAM', `0', `1', `NT_SHP')
rule(`SAC_CUDA_PARAM', `SAC_CUDA_PARAM__SCL', `SCL')
rule(`SAC_CUDA_PARAM', `SAC_CUDA_PARAM__AKS_AKD', `AKS')
rule(`SAC_CUDA_PARAM', `SAC_CUDA_PARAM__AKS_AKD', `AKD')

pat(`SAC_CUDA_ARG', `0', `1', `NT_SHP')
rule(`SAC_CUDA_ARG', `SAC_CUDA_ARG__SCL', `SCL')
rule(`SAC_CUDA_ARG', `SAC_CUDA_ARG__AKS_AKD', `AKS')
rule(`SAC_CUDA_ARG', `SAC_CUDA_ARG__AKS_AKD', `AKD')

//----------------------------------------------------------------------

pat(`SAC_CUDA_DEC_RC_FREE', `0', `2', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_CUDA_DEC_RC_FREE', `SAC_CUDA_DEC_RC_FREE__UNQ', `*SHP', `*HID', `UNQ')
rule(`SAC_CUDA_DEC_RC_FREE', `SAC_CUDA_DEC_RC_FREE__NOOP', `SCL', `NHD', `NUQ')
rule(`SAC_CUDA_DEC_RC_FREE', `SAC_CUDA_DEC_RC_FREE__DEFAULT', `*SHP', `*HID', `NUQ')

end_icm_definition


