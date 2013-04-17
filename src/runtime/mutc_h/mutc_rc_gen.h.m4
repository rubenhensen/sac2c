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

#undef SAC_ND_PRF_RESTORERC
pat(`SAC_ND_PRF_RESTORERC', `0', `1', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_PRF_RESTORERC', `SAC_ND_PRF_RESTORERC__NOOP', `SCL', `NHD', `*UNQ')
rule(`SAC_ND_PRF_RESTORERC', `SAC_ND_PRF_RESTORERC__DO', `SCL', `HID', `NUQ')
rule(`SAC_ND_PRF_RESTORERC', `SAC_ND_PRF_RESTORERC__NOOP', `SCL', `HID', `UNQ')
rule(`SAC_ND_PRF_RESTORERC', `SAC_ND_PRF_RESTORERC__NOOP', `SCL', `*HID', `UNQ')
rule(`SAC_ND_PRF_RESTORERC', `SAC_ND_PRF_RESTORERC__NOOP', `AKS', `*HID', `UNQ')
rule(`SAC_ND_PRF_RESTORERC', `SAC_ND_PRF_RESTORERC__DO', `*SHP', `*HID', `*UNQ')

#undef SAC_ND_PRF_2NORC
pat(`SAC_ND_PRF_2NORC', `1', `0', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_PRF_2NORC', `SAC_ND_PRF_2NORC__NOOP', `SCL', `NHD', `*UNQ')
rule(`SAC_ND_PRF_2NORC', `SAC_ND_PRF_2NORC__DO', `SCL', `HID', `NUQ')
rule(`SAC_ND_PRF_2NORC', `SAC_ND_PRF_2NORC__NOOP', `SCL', `HID', `UNQ')
rule(`SAC_ND_PRF_2NORC', `SAC_ND_PRF_2NORC__NOOP', `SCL', `*HID', `UNQ')
rule(`SAC_ND_PRF_2NORC', `SAC_ND_PRF_2NORC__NOOP', `AKS', `*HID', `UNQ')
rule(`SAC_ND_PRF_2NORC', `SAC_ND_PRF_2NORC__DO', `*SHP', `*HID', `*UNQ')

#undef SAC_ND_PRF_2ASYNC
pat(`SAC_ND_PRF_2ASYNC', `1', `0', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_ND_PRF_2ASYNC', `SAC_ND_PRF_2ASYNC__NOOP', `SCL', `NHD', `*UNQ')
rule(`SAC_ND_PRF_2ASYNC', `SAC_ND_PRF_2ASYNC__DO', `SCL', `HID', `NUQ')
rule(`SAC_ND_PRF_2ASYNC', `SAC_ND_PRF_2ASYNC__NOOP', `SCL', `HID', `UNQ')
rule(`SAC_ND_PRF_2ASYNC', `SAC_ND_PRF_2ASYNC__NOOP', `SCL', `*HID', `UNQ')
rule(`SAC_ND_PRF_2ASYNC', `SAC_ND_PRF_2ASYNC__NOOP', `AKS', `*HID', `UNQ')
rule(`SAC_ND_PRF_2ASYNC', `SAC_ND_PRF_2ASYNC__DO', `*SHP', `*HID', `*UNQ')

end_icm_definition
