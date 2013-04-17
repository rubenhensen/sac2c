/*
 * CAUTION: 
 *
 * fp_gen.h  is generated automatically from fp_gen.h.m4
 *
 */

/*
 * See ../m4/README
 */

include(`icm.m4')

start_icm_definition(fp_gen)

#ifndef _SAC_FP_GEN_H_
#define _SAC_FP_GEN_H_

#if SAC_DO_FP


pat(`SAC_FP_SAVE_RESULT', `2', `0', `NT_SHP')
rule(`SAC_FP_SAVE_RESULT', `SAC_FP_SAVE_RESULT__NODESC',  `SCL')
rule(`SAC_FP_SAVE_RESULT', `SAC_FP_SAVE_RESULT__NOOP',   `*SHP')


pat(`SAC_FP_GET_RESULT', `3', `0', `NT_SHP')
rule(`SAC_FP_GET_RESULT', `SAC_FP_GET_RESULT__NODESC',  `SCL')
rule(`SAC_FP_GET_RESULT', `SAC_FP_GET_RESULT__NOOP',   `*SHP')


#endif /* SAC_DO_FP */

#endif /* _SAC_FP_GEN_H_ */

end_icm_definition
