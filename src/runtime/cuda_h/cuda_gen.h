

#ifndef DEF_FILE_cuda_gen
#define DEF_FILE_cuda_gen

#define SAC_CUDA_ALLOC_BEGIN(nt, gra1, gra2, gra3)                                       \
    CAT_M4_cuda_gen_1 (SAC_CUDA_ALLOC_BEGIN_, NT_SHP (nt)) (nt, gra1, gra2, gra3)
#define SAC_CUDA_ALLOC_BEGIN_SCL(...) SAC_CUDA_ALLOC_BEGIN__DAO (__VA_ARGS__)

#define SAC_CUDA_ALLOC_BEGIN_AKS(...) SAC_CUDA_ALLOC_BEGIN__DAO (__VA_ARGS__)

#define SAC_CUDA_ALLOC_BEGIN_AKD(...) SAC_CUDA_ALLOC_BEGIN__NO_DAO (__VA_ARGS__)

#define SAC_CUDA_ALLOC_BEGIN_AUD(...) SAC_CUDA_ALLOC_BEGIN__NO_DAO (__VA_ARGS__)

#define SAC_CUDA_ALLOC_END(nt, gra1, gra2, gra3)                                         \
    CAT_M4_cuda_gen_2 (SAC_CUDA_ALLOC_END_, NT_SHP (nt)) (nt, gra1, gra2, gra3)
#define SAC_CUDA_ALLOC_END_SCL(...) SAC_CUDA_ALLOC_END__DAO (__VA_ARGS__)

#define SAC_CUDA_ALLOC_END_AKS(...) SAC_CUDA_ALLOC_END__DAO (__VA_ARGS__)

#define SAC_CUDA_ALLOC_END_AKD(...) SAC_CUDA_ALLOC_END__NO_DAO (__VA_ARGS__)

#define SAC_CUDA_ALLOC_END_AUD(...) SAC_CUDA_ALLOC_END__NO_DAO (__VA_ARGS__)

//----------------------------------------------------------------------

#define SAC_CUDA_ALLOC__DATA(nt, gra1)                                                   \
    CAT_M4_cuda_gen_3 (SAC_CUDA_ALLOC__DATA_, NT_SHP (nt)) (nt, gra1)
#define SAC_CUDA_ALLOC__DATA_SCL(...) SAC_CUDA_ALLOC__DATA__NOOP (__VA_ARGS__)

#define SAC_CUDA_ALLOC__DATA_AKS(...) SAC_CUDA_ALLOC__DATA__AKS (__VA_ARGS__)

#define SAC_CUDA_ALLOC__DATA_AKD(...) SAC_CUDA_ALLOC__DATA__AKD_AUD (__VA_ARGS__)
#define SAC_CUDA_ALLOC__DATA_AUD(...) SAC_CUDA_ALLOC__DATA__AKD_AUD (__VA_ARGS__)
#define SAC_CUDA_ALLOC__DATA____(...) SAC_CUDA_ALLOC__DATA__AKD_AUD (__VA_ARGS__)

//----------------------------------------------------------------------

/*
#define SAC_CUDA_HOST2DEVICE(...) SAC_CUDA_HOST2DEVICE_(__VA_ARGS__)
#define SAC_CUDA_HOST2DEVICE_SCL(...) GEN_SAC_CUDA_HOST2DEVICE__SCL(__VA_ARGS__)

#define SAC_CUDA_HOST2DEVICE_AKS(...) GEN_SAC_CUDA_HOST2DEVICE__AKS_AKD_AUD(__VA_ARGS__)

#define SAC_CUDA_HOST2DEVICE_AKD(...) GEN_SAC_CUDA_HOST2DEVICE__AKS_AKD_AUD(__VA_ARGS__)

#define SAC_CUDA_HOST2DEVICE_AUD(...) GEN_SAC_CUDA_HOST2DEVICE__AKS_AKD_AUD(__VA_ARGS__)


#define GEN_SAC_CUDA_HOST2DEVICE__SCL( arg1, nt )
CAT_M4_cuda_gen_4(GEN_SAC_CUDA_HOST2DEVICE__SCL_, NT_SHP(nt))( arg1, nt ) #define
GEN_SAC_CUDA_HOST2DEVICE__SCL_SCL(...) SAC_CUDA_HOST2DEVICE__SCL(__VA_ARGS__)


#define GEN_SAC_CUDA_HOST2DEVICE__AKS_AKD_AUD( arg1, nt )
CAT_M4_cuda_gen_5(GEN_SAC_CUDA_HOST2DEVICE__AKS_AKD_AUD_, NT_SHP(nt))( arg1, nt ) #define
GEN_SAC_CUDA_HOST2DEVICE__AKS_AKD_AUD_AKS(...)
SAC_CUDA_HOST2DEVICE__AKS_AKD_AUD(__VA_ARGS__)

#define GEN_SAC_CUDA_HOST2DEVICE__AKS_AKD_AUD_AKD(...)
SAC_CUDA_HOST2DEVICE__AKS_AKD_AUD(__VA_ARGS__)

#define GEN_SAC_CUDA_HOST2DEVICE__AKS_AKD_AUD_AUD(...)
SAC_CUDA_HOST2DEVICE__AKS_AKD_AUD(__VA_ARGS__)



#define SAC_CUDA_DEVICE2HOST(...) SAC_CUDA_DEVICE2HOST_(__VA_ARGS__)
#define SAC_CUDA_DEVICE2HOST_SCL(...) GEN_SAC_CUDA_DEVICE2HOST__SCL(__VA_ARGS__)

#define SAC_CUDA_DEVICE2HOST_AKS(...) GEN_SAC_CUDA_DEVICE2HOST__AKS_AKD_AUD(__VA_ARGS__)

#define SAC_CUDA_DEVICE2HOST_AKD(...) GEN_SAC_CUDA_DEVICE2HOST__AKS_AKD_AUD(__VA_ARGS__)

#define SAC_CUDA_DEVICE2HOST_AUD(...) GEN_SAC_CUDA_DEVICE2HOST__AKS_AKD_AUD(__VA_ARGS__)


#define GEN_SAC_CUDA_DEVICE2HOST__SCL( arg1, nt )
CAT_M4_cuda_gen_6(GEN_SAC_CUDA_DEVICE2HOST__SCL_, NT_SHP(nt))( arg1, nt ) #define
GEN_SAC_CUDA_DEVICE2HOST__SCL_SCL(...) SAC_CUDA_DEVICE2HOST__SCL(__VA_ARGS__)


#define GEN_SAC_CUDA_DEVICE2HOST__AKS_AKD_AUD( arg1, nt )
CAT_M4_cuda_gen_7(GEN_SAC_CUDA_DEVICE2HOST__AKS_AKD_AUD_, NT_SHP(nt))( arg1, nt ) #define
GEN_SAC_CUDA_DEVICE2HOST__AKS_AKD_AUD_AKS(...)
SAC_CUDA_DEVICE2HOST__AKS_AKD_AUD(__VA_ARGS__)

#define GEN_SAC_CUDA_DEVICE2HOST__AKS_AKD_AUD_AKD(...)
SAC_CUDA_DEVICE2HOST__AKS_AKD_AUD(__VA_ARGS__)

#define GEN_SAC_CUDA_DEVICE2HOST__AKS_AKD_AUD_AUD(...)
SAC_CUDA_DEVICE2HOST__AKS_AKD_AUD(__VA_ARGS__)

*/

#define SAC_CUDA_MEM_TRANSFER(nt, gra1, gra2, gra3)                                      \
    CAT_M4_cuda_gen_8 (SAC_CUDA_MEM_TRANSFER_, NT_SHP (nt)) (nt, gra1, gra2, gra3)
/*
#define SAC_CUDA_MEM_TRANSFER_SCL(...) GEN_SAC_CUDA_MEM_TRANSFER__SCL(__VA_ARGS__)

*/
#define SAC_CUDA_MEM_TRANSFER_AKS(...)                                                   \
    GEN_SAC_CUDA_MEM_TRANSFER__AKS_AKD_AUD (__VA_ARGS__)

#define SAC_CUDA_MEM_TRANSFER_AKD(...)                                                   \
    GEN_SAC_CUDA_MEM_TRANSFER__AKS_AKD_AUD (__VA_ARGS__)

#define SAC_CUDA_MEM_TRANSFER_AUD(...)                                                   \
    GEN_SAC_CUDA_MEM_TRANSFER__AKS_AKD_AUD (__VA_ARGS__)

/*
#define GEN_SAC_CUDA_MEM_TRANSFER__SCL( arg1, nt , gra1, gra2)
CAT_M4_cuda_gen_9(GEN_SAC_CUDA_MEM_TRANSFER__SCL_, NT_SHP(nt))( arg1, nt , gra1, gra2)
#define GEN_SAC_CUDA_MEM_TRANSFER__SCL_SCL(...) SAC_CUDA_MEM_TRANSFER__SCL(__VA_ARGS__)

*/

#define GEN_SAC_CUDA_MEM_TRANSFER__AKS_AKD_AUD(arg1, nt, gra1, gra2)                     \
    CAT_M4_cuda_gen_10 (GEN_SAC_CUDA_MEM_TRANSFER__AKS_AKD_AUD_,                         \
                        NT_SHP (nt)) (arg1, nt, gra1, gra2)
#define GEN_SAC_CUDA_MEM_TRANSFER__AKS_AKD_AUD_AKS(...)                                  \
    SAC_CUDA_MEM_TRANSFER__AKS_AKD_AUD (__VA_ARGS__)

#define GEN_SAC_CUDA_MEM_TRANSFER__AKS_AKD_AUD_AKD(...)                                  \
    SAC_CUDA_MEM_TRANSFER__AKS_AKD_AUD (__VA_ARGS__)

#define GEN_SAC_CUDA_MEM_TRANSFER__AKS_AKD_AUD_AUD(...)                                  \
    SAC_CUDA_MEM_TRANSFER__AKS_AKD_AUD (__VA_ARGS__)

//----------------------------------------------------------------------

#define SAC_CUDA_PARAM(nt, gra1)                                                         \
    CAT_M4_cuda_gen_11 (SAC_CUDA_PARAM_, NT_SHP (nt)) (nt, gra1)
#define SAC_CUDA_PARAM_SCL(...) SAC_CUDA_PARAM__SCL (__VA_ARGS__)

#define SAC_CUDA_PARAM_AKS(...) SAC_CUDA_PARAM__AKS_AKD (__VA_ARGS__)

#define SAC_CUDA_PARAM_AKD(...) SAC_CUDA_PARAM__AKS_AKD (__VA_ARGS__)

#define SAC_CUDA_ARG(nt, gra1) CAT_M4_cuda_gen_12 (SAC_CUDA_ARG_, NT_SHP (nt)) (nt, gra1)
#define SAC_CUDA_ARG_SCL(...) SAC_CUDA_ARG__SCL (__VA_ARGS__)

#define SAC_CUDA_ARG_AKS(...) SAC_CUDA_ARG__AKS_AKD (__VA_ARGS__)

#define SAC_CUDA_ARG_AKD(...) SAC_CUDA_ARG__AKS_AKD (__VA_ARGS__)

//----------------------------------------------------------------------

#define SAC_CUDA_DEC_RC_FREE(nt, gra1, gra2)                                             \
    CAT_M4_cuda_gen_13 (SAC_CUDA_DEC_RC_FREE_,                                           \
                        CAT_M4_cuda_gen_14 (NT_SHP (nt),                                 \
                                            CAT_M4_cuda_gen_15 (NT_HID (nt),             \
                                                                NT_UNQ (nt)))) (nt,      \
                                                                                gra1,    \
                                                                                gra2)
#define SAC_CUDA_DEC_RC_FREE_SCLNHDUNQ(...) SAC_CUDA_DEC_RC_FREE__UNQ (__VA_ARGS__)
#define SAC_CUDA_DEC_RC_FREE_SCLHIDUNQ(...) SAC_CUDA_DEC_RC_FREE__UNQ (__VA_ARGS__)
#define SAC_CUDA_DEC_RC_FREE_SCL___UNQ(...) SAC_CUDA_DEC_RC_FREE__UNQ (__VA_ARGS__)
#define SAC_CUDA_DEC_RC_FREE_AKSNHDUNQ(...) SAC_CUDA_DEC_RC_FREE__UNQ (__VA_ARGS__)
#define SAC_CUDA_DEC_RC_FREE_AKSHIDUNQ(...) SAC_CUDA_DEC_RC_FREE__UNQ (__VA_ARGS__)
#define SAC_CUDA_DEC_RC_FREE_AKS___UNQ(...) SAC_CUDA_DEC_RC_FREE__UNQ (__VA_ARGS__)
#define SAC_CUDA_DEC_RC_FREE_AKDNHDUNQ(...) SAC_CUDA_DEC_RC_FREE__UNQ (__VA_ARGS__)
#define SAC_CUDA_DEC_RC_FREE_AKDHIDUNQ(...) SAC_CUDA_DEC_RC_FREE__UNQ (__VA_ARGS__)
#define SAC_CUDA_DEC_RC_FREE_AKD___UNQ(...) SAC_CUDA_DEC_RC_FREE__UNQ (__VA_ARGS__)
#define SAC_CUDA_DEC_RC_FREE_AUDNHDUNQ(...) SAC_CUDA_DEC_RC_FREE__UNQ (__VA_ARGS__)
#define SAC_CUDA_DEC_RC_FREE_AUDHIDUNQ(...) SAC_CUDA_DEC_RC_FREE__UNQ (__VA_ARGS__)
#define SAC_CUDA_DEC_RC_FREE_AUD___UNQ(...) SAC_CUDA_DEC_RC_FREE__UNQ (__VA_ARGS__)
#define SAC_CUDA_DEC_RC_FREE____NHDUNQ(...) SAC_CUDA_DEC_RC_FREE__UNQ (__VA_ARGS__)
#define SAC_CUDA_DEC_RC_FREE____HIDUNQ(...) SAC_CUDA_DEC_RC_FREE__UNQ (__VA_ARGS__)
#define SAC_CUDA_DEC_RC_FREE_______UNQ(...) SAC_CUDA_DEC_RC_FREE__UNQ (__VA_ARGS__)

#define SAC_CUDA_DEC_RC_FREE_SCLNHDNUQ(...) SAC_CUDA_DEC_RC_FREE__NOOP (__VA_ARGS__)

#define SAC_CUDA_DEC_RC_FREE_SCLHIDNUQ(...) SAC_CUDA_DEC_RC_FREE__DEFAULT (__VA_ARGS__)
#define SAC_CUDA_DEC_RC_FREE_SCL___NUQ(...) SAC_CUDA_DEC_RC_FREE__DEFAULT (__VA_ARGS__)
#define SAC_CUDA_DEC_RC_FREE_AKSNHDNUQ(...) SAC_CUDA_DEC_RC_FREE__DEFAULT (__VA_ARGS__)
#define SAC_CUDA_DEC_RC_FREE_AKSHIDNUQ(...) SAC_CUDA_DEC_RC_FREE__DEFAULT (__VA_ARGS__)
#define SAC_CUDA_DEC_RC_FREE_AKS___NUQ(...) SAC_CUDA_DEC_RC_FREE__DEFAULT (__VA_ARGS__)
#define SAC_CUDA_DEC_RC_FREE_AKDNHDNUQ(...) SAC_CUDA_DEC_RC_FREE__DEFAULT (__VA_ARGS__)
#define SAC_CUDA_DEC_RC_FREE_AKDHIDNUQ(...) SAC_CUDA_DEC_RC_FREE__DEFAULT (__VA_ARGS__)
#define SAC_CUDA_DEC_RC_FREE_AKD___NUQ(...) SAC_CUDA_DEC_RC_FREE__DEFAULT (__VA_ARGS__)
#define SAC_CUDA_DEC_RC_FREE_AUDNHDNUQ(...) SAC_CUDA_DEC_RC_FREE__DEFAULT (__VA_ARGS__)
#define SAC_CUDA_DEC_RC_FREE_AUDHIDNUQ(...) SAC_CUDA_DEC_RC_FREE__DEFAULT (__VA_ARGS__)
#define SAC_CUDA_DEC_RC_FREE_AUD___NUQ(...) SAC_CUDA_DEC_RC_FREE__DEFAULT (__VA_ARGS__)
#define SAC_CUDA_DEC_RC_FREE____NHDNUQ(...) SAC_CUDA_DEC_RC_FREE__DEFAULT (__VA_ARGS__)
#define SAC_CUDA_DEC_RC_FREE____HIDNUQ(...) SAC_CUDA_DEC_RC_FREE__DEFAULT (__VA_ARGS__)
#define SAC_CUDA_DEC_RC_FREE_______NUQ(...) SAC_CUDA_DEC_RC_FREE__DEFAULT (__VA_ARGS__)

#define CAT_M4_cuda_gen_15(x, y) xCAT_M4_cuda_gen_15 (x, y)
#define xCAT_M4_cuda_gen_15(x, y) x##y

#define CAT_M4_cuda_gen_14(x, y) xCAT_M4_cuda_gen_14 (x, y)
#define xCAT_M4_cuda_gen_14(x, y) x##y

#define CAT_M4_cuda_gen_13(x, y) xCAT_M4_cuda_gen_13 (x, y)
#define xCAT_M4_cuda_gen_13(x, y) x##y

#define CAT_M4_cuda_gen_12(x, y) xCAT_M4_cuda_gen_12 (x, y)
#define xCAT_M4_cuda_gen_12(x, y) x##y

#define CAT_M4_cuda_gen_11(x, y) xCAT_M4_cuda_gen_11 (x, y)
#define xCAT_M4_cuda_gen_11(x, y) x##y

#define CAT_M4_cuda_gen_10(x, y) xCAT_M4_cuda_gen_10 (x, y)
#define xCAT_M4_cuda_gen_10(x, y) x##y

#define CAT_M4_cuda_gen_9(x, y) xCAT_M4_cuda_gen_9 (x, y)
#define xCAT_M4_cuda_gen_9(x, y) x##y

#define CAT_M4_cuda_gen_8(x, y) xCAT_M4_cuda_gen_8 (x, y)
#define xCAT_M4_cuda_gen_8(x, y) x##y

#define CAT_M4_cuda_gen_7(x, y) xCAT_M4_cuda_gen_7 (x, y)
#define xCAT_M4_cuda_gen_7(x, y) x##y

#define CAT_M4_cuda_gen_6(x, y) xCAT_M4_cuda_gen_6 (x, y)
#define xCAT_M4_cuda_gen_6(x, y) x##y

#define CAT_M4_cuda_gen_5(x, y) xCAT_M4_cuda_gen_5 (x, y)
#define xCAT_M4_cuda_gen_5(x, y) x##y

#define CAT_M4_cuda_gen_4(x, y) xCAT_M4_cuda_gen_4 (x, y)
#define xCAT_M4_cuda_gen_4(x, y) x##y

#define CAT_M4_cuda_gen_3(x, y) xCAT_M4_cuda_gen_3 (x, y)
#define xCAT_M4_cuda_gen_3(x, y) x##y

#define CAT_M4_cuda_gen_2(x, y) xCAT_M4_cuda_gen_2 (x, y)
#define xCAT_M4_cuda_gen_2(x, y) x##y

#define CAT_M4_cuda_gen_1(x, y) xCAT_M4_cuda_gen_1 (x, y)
#define xCAT_M4_cuda_gen_1(x, y) x##y

#endif
