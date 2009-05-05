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

include(icm.m4)

start_icm_definition(mutc_gen)


pat(`SAC_MUTC_ND_ARG', `0', `0', `NT_REG', `NT_SCO')
rule(`SAC_MUTC_ND_ARG', `SAC_MUTC_ND_ARG_INT_GLO', `INT', `GLO')
rule(`SAC_MUTC_ND_ARG', `SAC_MUTC_ND_ARG_FLO_GLO', `FLO', `GLO')
rule(`SAC_MUTC_ND_ARG', `SAC_MUTC_ND_ARG_INT_SHA', `INT', `SHA')
rule(`SAC_MUTC_ND_ARG', `SAC_MUTC_ND_ARG_FLO_SHA', `FLO', `SHA')

pat(`SAC_MUTC_ND_PARAM', `0', `0', `NT_REG', `NT_SCO')
rule(`SAC_MUTC_ND_PARAM', `SAC_MUTC_ND_PARAM_INT_GLO', `INT', `GLO')
rule(`SAC_MUTC_ND_PARAM', `SAC_MUTC_ND_PARAM_FLO_GLO', `FLO', `GLO')
rule(`SAC_MUTC_ND_PARAM', `SAC_MUTC_ND_PARAM_INT_SHA', `INT', `SHA')
rule(`SAC_MUTC_ND_PARAM', `SAC_MUTC_ND_PARAM_FLO_SHA', `FLO', `SHA')

pat(`SAC_MUTC_ND_GET_VAR', `0', `0', `NT_USG')
rule(`SAC_MUTC_ND_GET_VAR', `SAC_MUTC_GET_VAR', `PAR')
rule(`SAC_MUTC_ND_GET_VAR', `SAC_ND_A_FIELD__DEFAULT', `DEF')

end_icm_definition