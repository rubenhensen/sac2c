/*
 * CAUTION:
 *
 * distmem_icm_gen.h  is generated automatically from distmem_icm_gen.h.m4
 *
 */

/*
 * See ../m4/README
 */

#ifndef DEF_FILE_distmem_icm_gen
#define DEF_FILE_distmem_icm_gen

#define SAC_DISTMEM_MIRROR_IS_DIST(nt)                                                   \
    CAT_M4_distmem_icm_gen_1 (SAC_DISTMEM_MIRROR_IS_DIST_, NT_SHP (nt)) (nt)
#define SAC_DISTMEM_MIRROR_IS_DIST_SCL(...) SAC_DISTMEM_MIRROR_IS_DIST__SCL (__VA_ARGS__)

#define SAC_DISTMEM_MIRROR_IS_DIST_AKS(...)                                              \
    SAC_DISTMEM_MIRROR_IS_DIST__DEFAULT (__VA_ARGS__)
#define SAC_DISTMEM_MIRROR_IS_DIST_AKD(...)                                              \
    SAC_DISTMEM_MIRROR_IS_DIST__DEFAULT (__VA_ARGS__)
#define SAC_DISTMEM_MIRROR_IS_DIST_AUD(...)                                              \
    SAC_DISTMEM_MIRROR_IS_DIST__DEFAULT (__VA_ARGS__)
#define SAC_DISTMEM_MIRROR_IS_DIST____(...)                                              \
    SAC_DISTMEM_MIRROR_IS_DIST__DEFAULT (__VA_ARGS__)

#define SAC_DISTMEM_MIRROR_OFFS(...) SAC_DISTMEM_MIRROR_OFFS_ (__VA_ARGS__)
#define SAC_DISTMEM_MIRROR_OFFS_(...) SAC_DISTMEM_MIRROR_OFFS__DEFAULT (__VA_ARGS__)

#define SAC_DISTMEM_MIRROR_FIRST_ELEMS(...) SAC_DISTMEM_MIRROR_FIRST_ELEMS_ (__VA_ARGS__)
#define SAC_DISTMEM_MIRROR_FIRST_ELEMS_(...)                                             \
    SAC_DISTMEM_MIRROR_FIRST_ELEMS__DEFAULT (__VA_ARGS__)

/*
#define SAC_DISTMEM_ALLOC__DATA( nt ) CAT_M4_distmem_icm_gen_2(SAC_DISTMEM_ALLOC__DATA_,
NT_SHP(nt))( nt ) #define SAC_DISTMEM_ALLOC__DATA_SCL(...)
SAC_ND_ALLOC__DATA__NOOP(__VA_ARGS__)

#define SAC_DISTMEM_ALLOC__DATA_AKS(...) SAC_DISTMEM_ALLOC__DATA__AKS(__VA_ARGS__)



#define SAC_DISTMEM_ALLOC__DATA_AKD(...) SAC_ND_ALLOC__DATA__AKD_AUD(__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DATA_AUD(...) SAC_ND_ALLOC__DATA__AKD_AUD(__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DATA____(...) SAC_ND_ALLOC__DATA__AKD_AUD(__VA_ARGS__)

*/

#define SAC_DISTMEM_ALLOC__DESC_AND_DATA(nt, gra1, gra2)                                 \
    CAT_M4_distmem_icm_gen_3 (                                                           \
      SAC_DISTMEM_ALLOC__DESC_AND_DATA_,                                                 \
      CAT_M4_distmem_icm_gen_4 (NT_SHP (nt),                                             \
                                CAT_M4_distmem_icm_gen_5 (NT_HID (nt),                   \
                                                          NT_UNQ (nt)))) (nt, gra1,      \
                                                                          gra2)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_SCLNHDNUQ(...)                                  \
    SAC_ND_ALLOC__DESC__NOOP_BASETYPE (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_SCLNHDUNQ(...)                                  \
    SAC_ND_ALLOC__DESC__NOOP_BASETYPE (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_SCLNHD___(...)                                  \
    SAC_ND_ALLOC__DESC__NOOP_BASETYPE (__VA_ARGS__)

#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_SCLHIDUNQ(...)                                  \
    SAC_ND_ALLOC__DESC__NOOP_BASETYPE (__VA_ARGS__)

#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_SCLHNSUNQ(...)                                  \
    SAC_ND_ALLOC__DESC__NOOP_BASETYPE (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_SCL___UNQ(...)                                  \
    SAC_ND_ALLOC__DESC__NOOP_BASETYPE (__VA_ARGS__)

#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_AKSNHDUNQ(...)                                  \
    SAC_ND_ALLOC__DESC__NOOP_BASETYPE (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_AKSHIDUNQ(...)                                  \
    SAC_ND_ALLOC__DESC__NOOP_BASETYPE (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_AKSHNSUNQ(...)                                  \
    SAC_ND_ALLOC__DESC__NOOP_BASETYPE (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_AKS___UNQ(...)                                  \
    SAC_ND_ALLOC__DESC__NOOP_BASETYPE (__VA_ARGS__)

#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_SCLHIDNUQ(...)                                  \
    SAC_ND_ALLOC__DESC__FIXED_BASETYPE (__VA_ARGS__)

#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_SCLHNSNUQ(...)                                  \
    SAC_ND_ALLOC__DESC__NOOP_BASETYPE (__VA_ARGS__)

#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_AKSHNSNUQ(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA_NESTED__AKS (__VA_ARGS__)

#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_AKSNHDNUQ(...)                                  \
    SAC_DISTMEM_ALLOC__DESC_AND_DATA__AKS (__VA_ARGS__)

#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_AKSNHD___(...)                                  \
    SAC_DISTMEM_ALLOC__DESC_AND_DATA__AKS (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_AKSHIDNUQ(...)                                  \
    SAC_DISTMEM_ALLOC__DESC_AND_DATA__AKS (__VA_ARGS__)

#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_AKSHID___(...)                                  \
    SAC_DISTMEM_ALLOC__DESC_AND_DATA__AKS (__VA_ARGS__)

#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_AKSHNS___(...)                                  \
    SAC_DISTMEM_ALLOC__DESC_AND_DATA__AKS (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_AKS___NUQ(...)                                  \
    SAC_DISTMEM_ALLOC__DESC_AND_DATA__AKS (__VA_ARGS__)

#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_AKS______(...)                                  \
    SAC_DISTMEM_ALLOC__DESC_AND_DATA__AKS (__VA_ARGS__)

#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_SCLHID___(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)

#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_SCLHNS___(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_SCL___NUQ(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)

#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_SCL______(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)

#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_AKDNHDNUQ(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_AKDNHDUNQ(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_AKDNHD___(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_AKDHIDNUQ(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_AKDHIDUNQ(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_AKDHID___(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_AKDHNSNUQ(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_AKDHNSUNQ(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_AKDHNS___(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_AKD___NUQ(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_AKD___UNQ(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_AKD______(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_AUDNHDNUQ(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_AUDNHDUNQ(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_AUDNHD___(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_AUDHIDNUQ(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_AUDHIDUNQ(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_AUDHID___(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_AUDHNSNUQ(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_AUDHNSUNQ(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_AUDHNS___(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_AUD___NUQ(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_AUD___UNQ(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_AUD______(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA____NHDNUQ(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA____NHDUNQ(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA____NHD___(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA____HIDNUQ(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA____HIDUNQ(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA____HID___(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA____HNSNUQ(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA____HNSUNQ(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA____HNS___(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_______NUQ(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA_______UNQ(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA__________(...)                                  \
    SAC_ND_ALLOC__DESC_AND_DATA__UNDEF (__VA_ARGS__)

#define SAC_DISTMEM_ALLOC_BEGIN(nt, gra1, gra2, gra3)                                    \
    CAT_M4_distmem_icm_gen_6 (SAC_DISTMEM_ALLOC_BEGIN_, NT_SHP (nt)) (nt, gra1, gra2,    \
                                                                      gra3)
#define SAC_DISTMEM_ALLOC_BEGIN_SCL(...) SAC_ND_ALLOC_BEGIN__DAO (__VA_ARGS__)

#define SAC_DISTMEM_ALLOC_BEGIN_AKS(...) SAC_DISTMEM_ALLOC_BEGIN__DAO (__VA_ARGS__)

#define SAC_DISTMEM_ALLOC_BEGIN_AKD(...) SAC_ND_ALLOC_BEGIN__NO_DAO (__VA_ARGS__)

#define SAC_DISTMEM_ALLOC_BEGIN_AUD(...) SAC_ND_ALLOC_BEGIN__NO_DAO (__VA_ARGS__)

#define SAC_DISTMEM_FREE__DATA(nt, gra1)                                                 \
    CAT_M4_distmem_icm_gen_7 (SAC_DISTMEM_FREE__DATA_,                                   \
                              CAT_M4_distmem_icm_gen_8 (NT_SHP (nt),                     \
                                                        NT_HID (nt))) (nt, gra1)
#define SAC_DISTMEM_FREE__DATA_SCLNHD(...) SAC_ND_FREE__DATA__SCL_NHD (__VA_ARGS__)

#define SAC_DISTMEM_FREE__DATA_SCLHID(...) SAC_ND_FREE__DATA__SCL_HID (__VA_ARGS__)

#define SAC_DISTMEM_FREE__DATA_SCLHNS(...) SAC_ND_FREE__DATA__SCL_HNS (__VA_ARGS__)

#define SAC_DISTMEM_FREE__DATA_AKSNHD(...) SAC_DISTMEM_FREE__DATA__AKS_NHD (__VA_ARGS__)

#define SAC_DISTMEM_FREE__DATA_AKSHID(...) SAC_ND_FREE__DATA__AKS_HID (__VA_ARGS__)

#define SAC_DISTMEM_FREE__DATA_AKSHNS(...) SAC_ND_FREE__DATA__AKS_HNS (__VA_ARGS__)

#define SAC_DISTMEM_FREE__DATA_AKDNHD(...) SAC_ND_FREE__DATA__AKD_NHD (__VA_ARGS__)

#define SAC_DISTMEM_FREE__DATA_AKDHID(...) SAC_ND_FREE__DATA__AKS_HID (__VA_ARGS__)

#define SAC_DISTMEM_FREE__DATA_AUDNHD(...) SAC_ND_FREE__DATA__AKD_NHD (__VA_ARGS__)

#define SAC_DISTMEM_FREE__DATA_AUDHID(...) SAC_ND_FREE__DATA__AKS_HID (__VA_ARGS__)

#define SAC_DISTMEM_FREE__DATA____NHD(...) SAC_ND_FREE__DATA__AKS_NHD (__VA_ARGS__)

#define SAC_DISTMEM_FREE__DATA____HID(...) SAC_ND_FREE__DATA__AKS_HID (__VA_ARGS__)

#define SAC_DISTMEM_FREE__DATA_AKDHNS(...) SAC_ND_FREE__DATA__AKD_HNS (__VA_ARGS__)

#define SAC_DISTMEM_FREE__DATA_AUDHNS(...) SAC_ND_FREE__DATA__AKD_HNS (__VA_ARGS__)

#define SAC_DISTMEM_DEC_RC_FREE(nt, gra1, gra2)                                          \
    CAT_M4_distmem_icm_gen_9 (                                                           \
      SAC_DISTMEM_DEC_RC_FREE_,                                                          \
      CAT_M4_distmem_icm_gen_10 (NT_SHP (nt),                                            \
                                 CAT_M4_distmem_icm_gen_11 (NT_HID (nt),                 \
                                                            NT_UNQ (nt)))) (nt, gra1,    \
                                                                            gra2)
#define SAC_DISTMEM_DEC_RC_FREE_SCLNHDUNQ(...) SAC_ND_DEC_RC_FREE__UNQ (__VA_ARGS__)
#define SAC_DISTMEM_DEC_RC_FREE_SCLHIDUNQ(...) SAC_ND_DEC_RC_FREE__UNQ (__VA_ARGS__)
#define SAC_DISTMEM_DEC_RC_FREE_SCLHNSUNQ(...) SAC_ND_DEC_RC_FREE__UNQ (__VA_ARGS__)
#define SAC_DISTMEM_DEC_RC_FREE_SCL___UNQ(...) SAC_ND_DEC_RC_FREE__UNQ (__VA_ARGS__)
#define SAC_DISTMEM_DEC_RC_FREE_AKSNHDUNQ(...) SAC_ND_DEC_RC_FREE__UNQ (__VA_ARGS__)
#define SAC_DISTMEM_DEC_RC_FREE_AKSHIDUNQ(...) SAC_ND_DEC_RC_FREE__UNQ (__VA_ARGS__)
#define SAC_DISTMEM_DEC_RC_FREE_AKSHNSUNQ(...) SAC_ND_DEC_RC_FREE__UNQ (__VA_ARGS__)
#define SAC_DISTMEM_DEC_RC_FREE_AKS___UNQ(...) SAC_ND_DEC_RC_FREE__UNQ (__VA_ARGS__)
#define SAC_DISTMEM_DEC_RC_FREE_AKDNHDUNQ(...) SAC_ND_DEC_RC_FREE__UNQ (__VA_ARGS__)
#define SAC_DISTMEM_DEC_RC_FREE_AKDHIDUNQ(...) SAC_ND_DEC_RC_FREE__UNQ (__VA_ARGS__)
#define SAC_DISTMEM_DEC_RC_FREE_AKDHNSUNQ(...) SAC_ND_DEC_RC_FREE__UNQ (__VA_ARGS__)
#define SAC_DISTMEM_DEC_RC_FREE_AKD___UNQ(...) SAC_ND_DEC_RC_FREE__UNQ (__VA_ARGS__)
#define SAC_DISTMEM_DEC_RC_FREE_AUDNHDUNQ(...) SAC_ND_DEC_RC_FREE__UNQ (__VA_ARGS__)
#define SAC_DISTMEM_DEC_RC_FREE_AUDHIDUNQ(...) SAC_ND_DEC_RC_FREE__UNQ (__VA_ARGS__)
#define SAC_DISTMEM_DEC_RC_FREE_AUDHNSUNQ(...) SAC_ND_DEC_RC_FREE__UNQ (__VA_ARGS__)
#define SAC_DISTMEM_DEC_RC_FREE_AUD___UNQ(...) SAC_ND_DEC_RC_FREE__UNQ (__VA_ARGS__)
#define SAC_DISTMEM_DEC_RC_FREE____NHDUNQ(...) SAC_ND_DEC_RC_FREE__UNQ (__VA_ARGS__)
#define SAC_DISTMEM_DEC_RC_FREE____HIDUNQ(...) SAC_ND_DEC_RC_FREE__UNQ (__VA_ARGS__)
#define SAC_DISTMEM_DEC_RC_FREE____HNSUNQ(...) SAC_ND_DEC_RC_FREE__UNQ (__VA_ARGS__)
#define SAC_DISTMEM_DEC_RC_FREE_______UNQ(...) SAC_ND_DEC_RC_FREE__UNQ (__VA_ARGS__)

#define SAC_DISTMEM_DEC_RC_FREE_SCLNHDNUQ(...) SAC_ND_DEC_RC_FREE__NOOP (__VA_ARGS__)

#define SAC_DISTMEM_DEC_RC_FREE_SCLHNSNUQ(...) SAC_ND_DEC_RC_FREE__NOOP (__VA_ARGS__)

#define SAC_DISTMEM_DEC_RC_FREE_SCLHIDNUQ(...)                                           \
    SAC_DISTMEM_DEC_RC_FREE__DEFAULT (__VA_ARGS__)

#define SAC_DISTMEM_DEC_RC_FREE_SCL___NUQ(...)                                           \
    SAC_DISTMEM_DEC_RC_FREE__DEFAULT (__VA_ARGS__)
#define SAC_DISTMEM_DEC_RC_FREE_AKSNHDNUQ(...)                                           \
    SAC_DISTMEM_DEC_RC_FREE__DEFAULT (__VA_ARGS__)
#define SAC_DISTMEM_DEC_RC_FREE_AKSHIDNUQ(...)                                           \
    SAC_DISTMEM_DEC_RC_FREE__DEFAULT (__VA_ARGS__)
#define SAC_DISTMEM_DEC_RC_FREE_AKSHNSNUQ(...)                                           \
    SAC_DISTMEM_DEC_RC_FREE__DEFAULT (__VA_ARGS__)
#define SAC_DISTMEM_DEC_RC_FREE_AKS___NUQ(...)                                           \
    SAC_DISTMEM_DEC_RC_FREE__DEFAULT (__VA_ARGS__)
#define SAC_DISTMEM_DEC_RC_FREE_AKDNHDNUQ(...)                                           \
    SAC_DISTMEM_DEC_RC_FREE__DEFAULT (__VA_ARGS__)
#define SAC_DISTMEM_DEC_RC_FREE_AKDHIDNUQ(...)                                           \
    SAC_DISTMEM_DEC_RC_FREE__DEFAULT (__VA_ARGS__)
#define SAC_DISTMEM_DEC_RC_FREE_AKDHNSNUQ(...)                                           \
    SAC_DISTMEM_DEC_RC_FREE__DEFAULT (__VA_ARGS__)
#define SAC_DISTMEM_DEC_RC_FREE_AKD___NUQ(...)                                           \
    SAC_DISTMEM_DEC_RC_FREE__DEFAULT (__VA_ARGS__)
#define SAC_DISTMEM_DEC_RC_FREE_AUDNHDNUQ(...)                                           \
    SAC_DISTMEM_DEC_RC_FREE__DEFAULT (__VA_ARGS__)
#define SAC_DISTMEM_DEC_RC_FREE_AUDHIDNUQ(...)                                           \
    SAC_DISTMEM_DEC_RC_FREE__DEFAULT (__VA_ARGS__)
#define SAC_DISTMEM_DEC_RC_FREE_AUDHNSNUQ(...)                                           \
    SAC_DISTMEM_DEC_RC_FREE__DEFAULT (__VA_ARGS__)
#define SAC_DISTMEM_DEC_RC_FREE_AUD___NUQ(...)                                           \
    SAC_DISTMEM_DEC_RC_FREE__DEFAULT (__VA_ARGS__)
#define SAC_DISTMEM_DEC_RC_FREE____NHDNUQ(...)                                           \
    SAC_DISTMEM_DEC_RC_FREE__DEFAULT (__VA_ARGS__)
#define SAC_DISTMEM_DEC_RC_FREE____HIDNUQ(...)                                           \
    SAC_DISTMEM_DEC_RC_FREE__DEFAULT (__VA_ARGS__)
#define SAC_DISTMEM_DEC_RC_FREE____HNSNUQ(...)                                           \
    SAC_DISTMEM_DEC_RC_FREE__DEFAULT (__VA_ARGS__)
#define SAC_DISTMEM_DEC_RC_FREE_______NUQ(...)                                           \
    SAC_DISTMEM_DEC_RC_FREE__DEFAULT (__VA_ARGS__)

#define CAT_M4_distmem_icm_gen_11(x, y) xCAT_M4_distmem_icm_gen_11 (x, y)
#define xCAT_M4_distmem_icm_gen_11(x, y) x##y

#define CAT_M4_distmem_icm_gen_10(x, y) xCAT_M4_distmem_icm_gen_10 (x, y)
#define xCAT_M4_distmem_icm_gen_10(x, y) x##y

#define CAT_M4_distmem_icm_gen_9(x, y) xCAT_M4_distmem_icm_gen_9 (x, y)
#define xCAT_M4_distmem_icm_gen_9(x, y) x##y

#define CAT_M4_distmem_icm_gen_8(x, y) xCAT_M4_distmem_icm_gen_8 (x, y)
#define xCAT_M4_distmem_icm_gen_8(x, y) x##y

#define CAT_M4_distmem_icm_gen_7(x, y) xCAT_M4_distmem_icm_gen_7 (x, y)
#define xCAT_M4_distmem_icm_gen_7(x, y) x##y

#define CAT_M4_distmem_icm_gen_6(x, y) xCAT_M4_distmem_icm_gen_6 (x, y)
#define xCAT_M4_distmem_icm_gen_6(x, y) x##y

#define CAT_M4_distmem_icm_gen_5(x, y) xCAT_M4_distmem_icm_gen_5 (x, y)
#define xCAT_M4_distmem_icm_gen_5(x, y) x##y

#define CAT_M4_distmem_icm_gen_4(x, y) xCAT_M4_distmem_icm_gen_4 (x, y)
#define xCAT_M4_distmem_icm_gen_4(x, y) x##y

#define CAT_M4_distmem_icm_gen_3(x, y) xCAT_M4_distmem_icm_gen_3 (x, y)
#define xCAT_M4_distmem_icm_gen_3(x, y) x##y

#define CAT_M4_distmem_icm_gen_2(x, y) xCAT_M4_distmem_icm_gen_2 (x, y)
#define xCAT_M4_distmem_icm_gen_2(x, y) x##y

#define CAT_M4_distmem_icm_gen_1(x, y) xCAT_M4_distmem_icm_gen_1 (x, y)
#define xCAT_M4_distmem_icm_gen_1(x, y) x##y

#endif
