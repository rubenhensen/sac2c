/*
 * $Id: rc_impl_gen.h.m4 18064 2012-07-12 12:13:31Z jsa $
 *
 */

/*
 * CAUTION: 
 *
 * rc_impl_gen.h  is generated automatically from rc_impl_gen.h.m4
 *
 */

/*
 * See ../m4/README
 */

include(`icm.m4')

start_icm_definition(rc_impl_gen)

#ifndef _SAC_RC_IMPL_GEN_H_
#define _SAC_RC_IMPL_GEN_H_


pat(`SAC_ND_RC_TO_NORC', `0', `0', `NT_SHP')
rule(`SAC_ND_RC_TO_NORC', `SAC_ND_RC_TO_NORC__NODESC', `SCL')
rule(`SAC_ND_RC_TO_NORC', `SAC_ND_RC_TO_NORC__DESC', `AKS')
rule(`SAC_ND_RC_TO_NORC', `SAC_ND_RC_TO_NORC__DESC', `AKD')
rule(`SAC_ND_RC_TO_NORC', `SAC_ND_RC_TO_NORC__DESC', `AUD')


pat(`SAC_ND_RC_FROM_NORC', `0', `0', `NT_SHP')
rule(`SAC_ND_RC_FROM_NORC', `SAC_ND_RC_FROM_NORC__NODESC', `SCL')
rule(`SAC_ND_RC_FROM_NORC', `SAC_ND_RC_FROM_NORC__DESC', `AKS')
rule(`SAC_ND_RC_FROM_NORC', `SAC_ND_RC_FROM_NORC__DESC', `AKD')
rule(`SAC_ND_RC_FROM_NORC', `SAC_ND_RC_FROM_NORC__DESC', `AUD')


#endif

end_icm_definition
