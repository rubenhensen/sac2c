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

/*
 * See ../m4/README
 */

include(icm.m4)

start_icm_definition(mutc_gen)


pat(`SAC_MUTC_ARG2', `2', `0', `NT_REG', `NT_SCO')
rule(`SAC_MUTC_ARG2', `SAC_MUTC_ND_ARG_INT_GLO', `INT', `GLO')
rule(`SAC_MUTC_ARG2', `SAC_MUTC_ND_ARG_FLO_GLO', `FLO', `GLO')
rule(`SAC_MUTC_ARG2', `SAC_MUTC_ND_ARG_INT_SHA', `INT', `SHA')
rule(`SAC_MUTC_ARG2', `SAC_MUTC_ND_ARG_FLO_SHA', `FLO', `SHA')

pat(`SAC_MUTC_PARAM2', `0', `2', `NT_REG', `NT_SCO')
rule(`SAC_MUTC_PARAM2', `SAC_MUTC_ND_PARAM_INT_GLO', `INT', `GLO')
rule(`SAC_MUTC_PARAM2', `SAC_MUTC_ND_PARAM_FLO_GLO', `FLO', `GLO')
rule(`SAC_MUTC_PARAM2', `SAC_MUTC_ND_PARAM_INT_SHA', `INT', `SHA')
rule(`SAC_MUTC_PARAM2', `SAC_MUTC_ND_PARAM_FLO_SHA', `FLO', `SHA')

pat(`SAC_MUTC_ND_GET_VAR', `0', `1', `NT_USG')
rule(`SAC_MUTC_ND_GET_VAR', `SAC_MUTC_GET_PAR', `PAR')
rule(`SAC_MUTC_ND_GET_VAR', `SAC_MUTC_GET_VAR', `*USG')

end_icm_definition