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

/*
 * Non SCL are pointers there for they are INTs
 */
pat(`SAC_MUTC_ARG', `1', `1', `NT_REG', `NT_SCO', `NT_SHP')
rule(`SAC_MUTC_ARG', `SAC_MUTC_ND_ARG_INT_GLO', `INT', `GLO', `*SHP')
rule(`SAC_MUTC_ARG', `SAC_MUTC_ND_ARG_FLO_GLO', `FLO', `GLO', `SCL')
rule(`SAC_MUTC_ARG', `SAC_MUTC_ND_ARG_INT_GLO', `FLO', `GLO', `*SHP')
rule(`SAC_MUTC_ARG', `SAC_MUTC_ND_ARG_INT_SHA', `INT', `SHA', `*SHP')
rule(`SAC_MUTC_ARG', `SAC_MUTC_ND_ARG_FLO_SHA', `FLO', `SHA', `SCL')
rule(`SAC_MUTC_ARG', `SAC_MUTC_ND_ARG_INT_SHA', `FLO', `SHA', `*SHP')

pat(`SAC_MUTC_PARAM', `2', `0', `NT_REG', `NT_SCO', `NT_SHP')
rule(`SAC_MUTC_PARAM', `SAC_MUTC_ND_PARAM_INT_GLO', `INT', `GLO', `*SHP')
rule(`SAC_MUTC_PARAM', `SAC_MUTC_ND_PARAM_FLO_GLO', `FLO', `GLO', `SCL')
rule(`SAC_MUTC_PARAM', `SAC_MUTC_ND_PARAM_INT_GLO', `FLO', `GLO', `*SHP')
rule(`SAC_MUTC_PARAM', `SAC_MUTC_ND_PARAM_INT_SHA', `INT', `SHA', `*SHP')
rule(`SAC_MUTC_PARAM', `SAC_MUTC_ND_PARAM_FLO_SHA', `FLO', `SHA', `SCL')
rule(`SAC_MUTC_PARAM', `SAC_MUTC_ND_PARAM_INT_SHA', `FLO', `SHA', `*SHP')

#define MUTC 1
#if SAC_BACKEND == MUTC
#undef MUTC

#define SAC_MUTC_C_GETVAR( ...) SAC_ND_GETVAR( __VA_ARGS__)
#undef SAC_ND_GETVAR
pat(`SAC_ND_GETVAR', `0', `1', `NT_USG')
rule(`SAC_ND_GETVAR', `SAC_MUTC_GETFUNPAR', `FPA')
rule(`SAC_ND_GETVAR', `SAC_MUTC_GETTHREADPAR', `TPA')
rule(`SAC_ND_GETVAR', `SAC_MUTC_GETVAR', `*USG')

#undef SAC_ND_GETVAR_INOUT
pat(`SAC_ND_GETVAR_INOUT', `0', `1', `NT_USG')
rule(`SAC_ND_GETVAR_INOUT', `SAC_MUTC_GETFUNPAR', `FPA')
rule(`SAC_ND_GETVAR_INOUT', `SAC_MUTC_GETFUNPAR', `FPO')
rule(`SAC_ND_GETVAR_INOUT', `SAC_MUTC_GETTHREADPAR', `TPA')
rule(`SAC_ND_GETVAR_INOUT', `SAC_MUTC_GETTHREADPAR', `TPO')
rule(`SAC_ND_GETVAR_INOUT', `SAC_MUTC_GETVAR', `*USG')

#endif /* SAC_BACKEND */

end_icm_definition
