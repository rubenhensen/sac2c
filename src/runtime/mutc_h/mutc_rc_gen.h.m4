/*
 * $Id$
 *
 */

/*
 * CAUTION: 
 *
 * mutc_rc_gen.h  is generated automatically from mutc_rc_gen.h.m4
 *
 */

/*
 * See ../m4/README
 */

include(icm.m4)

start_icm_definition(mutc_rc_gen)

/*
 * Non SCL are pointers there for they are INTs
 */

pat(`SAC_MUTC_RC_BARRIER', `0', `0', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_MUTC_RC_BARRIER', `SAC_NOP', `SCL', `NHD', `*UNQ')
rule(`SAC_MUTC_RC_BARRIER', `SAC_MUTC_RC_BARRIER__DESC', `*SHP', `*HID', `*UNQ')

end_icm_definition
