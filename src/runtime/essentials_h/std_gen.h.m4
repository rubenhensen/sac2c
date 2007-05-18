/*
 * $Id$
 *
 */

/*
 * CAUTION: 
 *
 * std_gen.h  is generated automatically from std_gen.h.m4
 *
 */

include(`icm.m4')

dnl SAC_ND_A_DESC

icm(`SAC_ND_A_DESC',
    `SCL', `NHD', `*',
    `SAC_ND_A_DESC__UNDEF', `2', `0', `0')
icm(`SAC_ND_A_DESC',
    `SCL', `HID', `NUQ',
    `SAC_ND_A_DESC__DEFAULT', `2', `0', `0')
icm(`SAC_ND_A_DESC',
    `SCL', `HID', `UNQ',
    `SAC_ND_A_DESC__UNDEF', `2', `0', `0')
icm(`SAC_ND_A_DESC',
    `*', `*', `*',
    `SAC_ND_A_DESC__DEFAULT', `2', `0', `0')

dnl SAC_ND_A_DESC_DIM

icm(`SAC_ND_A_DESC_DIM',
    `AUD', `*', `*',
    `SAC_ND_A_DESC_DIM__DEFAULT', `9', `0', `0')
icm(`SAC_ND_A_DESC_DIM',
    `*', `*', `*',
    `SAC_ND_A_DESC_DIM__UNDEF', `9', `0', `0')

dnl SAC_ND_A_DESC_SIZE

icm(`SAC_ND_A_DESC_SIZE',
    `AKD', `*', `*',
    `SAC_ND_A_DESC_SIZE__DEFAULT', `9', `0', `0')
icm(`SAC_ND_A_DESC_SIZE',
    `AUD', `*', `*',
    `SAC_ND_A_DESC_SIZE__DEFAULT', `9', `0', `0')
icm(`SAC_ND_A_DESC_SIZE',
    `*', `*', `*',
    `SAC_ND_A_DESC_SIZE__UNDEF', `9', `0', `0')

dnl SAC_ND_A_DESC_SHAPE

icm(`SAC_ND_A_DESC_SHAPE',
    `AKD', `*', `*',
    `SAC_ND_A_DESC_SHAPE__DEFAULT', `9', `0', `1')
icm(`SAC_ND_A_DESC_SHAPE',
    `AUD', `*', `*',
    `SAC_ND_A_DESC_SHAPE__DEFAULT', `9', `0', `1')
icm(`SAC_ND_A_DESC_SHAPE',
    `*', `*', `*',
    `SAC_ND_A_DESC_SHAPE__UNDEF', `9', `0', `1')

dnl SAC_ND_A_MIRROR_DIM

icm(`SAC_ND_A_MIRROR_DIM',
    `SCL', `*', `*',
    `SAC_ND_A_MIRROR_DIM__SCL', `9', `0', `0')
icm(`SAC_ND_A_MIRROR_DIM',
    `*', `*', `*',
    `SAC_ND_A_MIRROR_DIM__DEFAULT', `9', `0', `0')

dnl SAC_ND_A_MIRROR_SIZE

icm(`SAC_ND_A_MIRROR_SIZE',
    `SCL', `*', `*',
    `SAC_ND_A_MIRROR_SIZE__SCL', `9', `0', `0')
icm(`SAC_ND_A_MIRROR_SIZE',
    `*', `*', `*',
    `SAC_ND_A_MIRROR_SIZE__DEFAULT', `9', `0', `0')

dnl SAC_ND_A_MIRROR_SHAPE

icm(`SAC_ND_A_MIRROR_SHAPE',
    `AKS', `*', `*',
    `SAC_ND_A_MIRROR_SHAPE__DEFAULT', `9', `0', `1')
icm(`SAC_ND_A_MIRROR_SHAPE',
    `AKD', `*', `*',
    `SAC_ND_A_MIRROR_SHAPE__DEFAULT', `9', `0', `1')
icm(`SAC_ND_A_MIRROR_SHAPE',
    `*', `*', `*',
    `SAC_ND_A_MIRROR_SHAPE__UNDEF', `9', `0', `1')

dnl SAC_ND_A_FIELD

icm(`SAC_ND_A_FIELD',
    `*', `*', `*',
    `SAC_ND_A_FIELD__DEFAULT', `6', `0', `0')

dnl SAC_ND_A_RC

icm(`SAC_ND_A_RC',
    `*', `*', `UNQ',
    `SAC_ND_A_RC__UNDEF', `6', `0', `0')
icm(`SAC_ND_A_RC',
    `SCL', `NHD', `NUQ',
    `SAC_ND_A_RC__UNDEF', `6', `0', `0')
icm(`SAC_ND_A_RC',
    `*', `*', `NUQ',
    `SAC_ND_A_RC__DEFAULT', `6', `0', `0')

dnl SAC_ND_A_DIM

icm(`SAC_ND_A_DIM',
    `*', `*', `*',
    `SAC_ND_A_DIM__DEFAULT', `6', `0', `0')

dnl SAC_ND_A_SIZE

icm(`SAC_ND_A_SIZE',
    `*', `*', `*',
    `SAC_ND_A_SIZE__DEFAULT', `6', `0', `0')

dnl SAC_ND_A_SHAPE

icm(`SAC_ND_A_SHAPE',
    `SCL', `*', `*',
    `SAC_ND_A_SHAPE__SCL', `6', `0', `1')
icm(`SAC_ND_A_SHAPE',
    `AKS', `*', `*',
    `SAC_ND_A_SHAPE__AKS_AKD', `6', `0', `1')
icm(`SAC_ND_A_SHAPE',
    `AKD', `*', `*',
    `SAC_ND_A_SHAPE__AKS_AKD', `6', `0', `1')
icm(`SAC_ND_A_SHAPE',
    `AUD', `*', `*',
    `SAC_ND_A_SHAPE__AUD', `6', `0', `1')

dnl SAC_ND_READ

icm(`SAC_ND_READ',
    `SCL', `*', `*',
    `SAC_ND_READ__SCL', `13', `0', `1')
icm(`SAC_ND_READ',
    `*', `*', `*',
    `SAC_ND_READ__DEFAULT', `13', `0', `1')

dnl SAC_ND_WRITE

icm(`SAC_ND_WRITE',
    `SCL', `*', `*',
    `SAC_ND_WRITE__SCL', `13', `0', `1')
icm(`SAC_ND_WRITE',
    `*', `*', `*',
    `SAC_ND_WRITE__DEFAULT', `13', `0', `1')

dnl SAC_ND_WRITE_COPY

icm(`SAC_ND_WRITE_COPY',
    `*', `HID', `*',
    `SAC_ND_WRITE_COPY__HID', `16', `0', `3')

icm(`SAC_ND_WRITE_COPY',
    `*', `NHD', `*',
    `SAC_ND_WRITE_COPY__NHD', `16', `0', `3')

dnl SAC_ND_DESC_TYPE

icm(`SAC_ND_DESC_TYPE',
    `*', `*', `*',
    `SAC_ND_DESC_TYPE__DEFAULT', `13', `0', `0')

dnl SAC_ND_TYPE

icm(`SAC_ND_TYPE',
    `SCL', `*', `*',
    `SAC_ND_TYPE_TAG__SCL', `13', `0', `1')
icm(`SAC_ND_TYPE',
    `*', `*', `*',
    `SAC_ND_TYPE_TAG__DEFAULT', `13', `0', `1')

dnl SAC_ND_TYPEDEF

icm(`SAC_ND_TYPEDEF',
    `*', `HID', `*',
    `SAC_ND_TYPEDEF__HID', `16', `0', `1')

icm(`SAC_ND_TYPEDEF',
    `*', `*', `*',
    `SAC_ND_TYPEDEF__DEFAULT', `16', `0', `1')

dnl SAC_ND_DECL__DESC

icm(`SAC_ND_DECL__DESC',
    `SCL', `NHD', `*',
    `SAC_ND_DECL__DESC__NONE', `19', `0', `1')
icm(`SAC_ND_DECL__DESC',
    `SCL', `HID', `UNQ',
    `SAC_ND_DECL__DESC__NONE', `19', `0', `1')
icm(`SAC_ND_DECL__DESC',
    `*', `*', `*',
    `SAC_ND_DECL__DESC__DEFAULT', `19', `0', `1')

dnl SAC_ND_PARAM_in

icm(`SAC_ND_PARAM_in',
    `SCL', `NHD', `*',
    `SAC_ND_PARAM_in__NODESC', `19', `0', `1')
icm(`SAC_ND_PARAM_in',
    `SCL', `HID', `UNQ',
    `SAC_ND_PARAM_in__NODESC', `19', `0', `1')
icm(`SAC_ND_PARAM_in',
    `*', `*', `*',
    `SAC_ND_PARAM_in__DESC', `19', `0', `1')

dnl SAC_ND_PARAM_out

icm(`SAC_ND_PARAM_out',
    `SCL', `NHD', `*',
    `SAC_ND_PARAM_out__NODESC', `19', `0', `1')
icm(`SAC_ND_PARAM_out',
    `SCL', `HID', `UNQ',
    `SAC_ND_PARAM_out__NODESC', `19', `0', `1')
icm(`SAC_ND_PARAM_out',
    `*', `*', `*',
    `SAC_ND_PARAM_out__DESC', `19', `0', `1')

dnl SAC_ND_ARG_in

icm(`SAC_ND_ARG_in',
    `SCL', `NHD', `*',
    `SAC_ND_ARG_in__NODESC', `19', `0', `0')
icm(`SAC_ND_ARG_in',
    `SCL', `HID', `UNQ',
    `SAC_ND_ARG_in__NODESC', `19', `0', `0')
icm(`SAC_ND_ARG_in',
    `*', `*', `*',
    `SAC_ND_ARG_in__DESC', `19', `0', `0')

dnl SAC_ND_ARG_out

icm(`SAC_ND_ARG_out',
    `SCL', `NHD', `*',
    `SAC_ND_ARG_out__NODESC', `19', `0', `0')
icm(`SAC_ND_ARG_out',
    `SCL', `HID', `UNQ',
    `SAC_ND_ARG_out__NODESC', `19', `0', `0')
icm(`SAC_ND_ARG_out',
    `*', `*', `*',
    `SAC_ND_ARG_out__DESC', `19', `0', `0')

dnl SAC_ND_RET_out

icm(`SAC_ND_RET_out',
    `SCL', `NHD', `*',
    `SAC_ND_RET_out__NODESC', `19', `1', `0')
icm(`SAC_ND_RET_out',
    `SCL', `HID', `UNQ',
    `SAC_ND_RET_out__NODESC', `19', `1', `0')
icm(`SAC_ND_RET_out',
    `*', `*', `*',
    `SAC_ND_RET_out__DESC', `19', `1', `0')

dnl SAC_ND_DECL_PARAM_inout

icm(`SAC_ND_DECL_PARAM_inout',
    `SCL', `NHD', `*',
    `SAC_ND_DECL_PARAM_inout__NODESC', `19', `0', `1')
icm(`SAC_ND_DECL_PARAM_inout',
    `SCL', `HID', `UNQ',
    `SAC_ND_DECL_PARAM_inout__NODESC', `19', `0', `1')
icm(`SAC_ND_DECL_PARAM_inout',
    `*', `*', `*',
    `SAC_ND_DECL_PARAM_inout__DESC', `19', `0', `1')

dnl SAC_ND_ALLOC /* not used */

icm(`SAC_ND_ALLOC',
    `SCL', `*', `*',
    `SAC_ND_ALLOC__DAO', `14', `0', `3')
icm(`SAC_ND_ALLOC',
    `AKS', `*', `*',
    `SAC_ND_ALLOC__DAO', `14', `0', `3')
icm(`SAC_ND_ALLOC',
    `AKD', `*', `*',
    `SAC_ND_ALLOC__NO_DAO', `14', `0', `3')
icm(`SAC_ND_ALLOC',
    `AUD', `*', `*',
    `SAC_ND_ALLOC__NO_DAO', `14', `0', `3')

dnl SAC_ND_ALLOC_BEGIN

icm(`SAC_ND_ALLOC_BEGIN',
    `SCL', `*', `*',
    `SAC_ND_ALLOC_BEGIN__DAO', `14', `0', `2')
icm(`SAC_ND_ALLOC_BEGIN',
    `AKS', `*', `*',
    `SAC_ND_ALLOC_BEGIN__DAO', `14', `0', `2')
icm(`SAC_ND_ALLOC_BEGIN',
    `AKD', `*', `*',
    `SAC_ND_ALLOC_BEGIN__NO_DAO', `14', `0', `2')
icm(`SAC_ND_ALLOC_BEGIN',
    `AUD', `*', `*',
    `SAC_ND_ALLOC_BEGIN__NO_DAO', `14', `0', `2')

dnl SAC_ND_ALLOC_END

icm(`SAC_ND_ALLOC_END',
    `SCL', `*', `*',
    `SAC_ND_ALLOC_END__DAO', `14', `0', `2')
icm(`SAC_ND_ALLOC_END',
    `AKS', `*', `*',
    `SAC_ND_ALLOC_END__DAO', `14', `0', `2')
icm(`SAC_ND_ALLOC_END',
    `AKD', `*', `*',
    `SAC_ND_ALLOC_END__NO_DAO', `14', `0', `2')
icm(`SAC_ND_ALLOC_END',
    `AUD', `*', `*',
    `SAC_ND_ALLOC_END__NO_DAO', `14', `0', `2')

dnl SAC_ND_ALLOC__DESC

icm(`SAC_ND_ALLOC__DESC',
    `SCL', `NHD', `*',
    `SAC_ND_ALLOC__DESC__NOOP', `19', `0', `1')
icm(`SAC_ND_ALLOC__DESC',
    `SCL', `HID', `UNQ',
    `SAC_ND_ALLOC__DESC__NOOP', `19', `0', `1')
icm(`SAC_ND_ALLOC__DESC',
    `AUD', `*', `*',
    `SAC_ND_ALLOC__DESC__AUD', `19', `0', `1')
icm(`SAC_ND_ALLOC__DESC',
    `*', `*', `*',
    `SAC_ND_ALLOC__DESC__FIXED', `19', `0', `1')

dnl SAC_ND_ALLOC__DATA

icm(`SAC_ND_ALLOC__DATA',
    `SCL', `*', `*',
    `SAC_ND_ALLOC__DATA__NOOP', `19', `0', `0')
icm(`SAC_ND_ALLOC__DATA',
    `AKS', `*', `*',
    `SAC_ND_ALLOC__DATA__AKS', `19', `0', `0')
icm(`SAC_ND_ALLOC__DATA',
    `*', `*', `*',
    `SAC_ND_ALLOC__DATA__AKD_AUD', `19', `0', `0')

dnl SAC_ND_ALLOC__DESC_AND_DATA

icm(`SAC_ND_ALLOC__DESC_AND_DATA',
    `SCL', `NHD', `*',
    `SAC_ND_ALLOC__DESC__NOOP', `19', `0', `1')
icm(`SAC_ND_ALLOC__DESC_AND_DATA',
    `SCL', `HID', `UNQ',
    `SAC_ND_ALLOC__DESC__NOOP', `19', `0', `1')
icm(`SAC_ND_ALLOC__DESC_AND_DATA',
    `SCL', `HID', `NUQ',
    `SAC_ND_ALLOC__DESC__FIXED', `19', `0', `1')
icm(`SAC_ND_ALLOC__DESC_AND_DATA',
    `AKS', `*', `*',
    `SAC_ND_ALLOC__DESC_AND_DATA__AKS', `19', `0', `1')
icm(`SAC_ND_ALLOC__DESC_AND_DATA',
    `*', `*', `*',
    `SAC_ND_ALLOC__DESC_AND_DATA__UNDEF', `19', `0', `1')

dnl SAC_ND_FREE__DESC

icm(`SAC_ND_FREE__DESC',
    `SCL', `NHD', `*',
    `SAC_ND_FREE__DESC__NOOP', `19', `0', `0')
icm(`SAC_ND_FREE__DESC',
    `SCL', `HID', `UNQ',
    `SAC_ND_FREE__DESC__NOOP', `19', `0', `0')
icm(`SAC_ND_FREE__DESC',
    `*', `*', `*',
    `SAC_ND_FREE__DESC__DEFAULT', `19', `0', `0')

dnl SAC_ND_FREE__DATA

icm(`SAC_ND_FREE__DATA',
    `SCL', `NHD', `*',
    `SAC_ND_FREE__DATA__SCL_NHD', `19', `0', `1')
icm(`SAC_ND_FREE__DATA',
    `SCL', `HID', `*',
    `SAC_ND_FREE__DATA__SCL_HID', `19', `0', `1')
icm(`SAC_ND_FREE__DATA',
    `AKS', `NHD', `*',
    `SAC_ND_FREE__DATA__AKS_NHD', `19', `0', `1')
icm(`SAC_ND_FREE__DATA',
    `AKS', `HID', `*',
    `SAC_ND_FREE__DATA__AKS_HID', `19', `0', `1')
icm(`SAC_ND_FREE__DATA',
    `AKD', `NHD', `*',
    `SAC_ND_FREE__DATA__AKD_NHD', `19', `0', `1')
icm(`SAC_ND_FREE__DATA',
    `AKD', `HID', `*',
    `SAC_ND_FREE__DATA__AKS_HID', `19', `0', `1')
icm(`SAC_ND_FREE__DATA',
    `AUD', `NHD', `*',
    `SAC_ND_FREE__DATA__AKD_NHD', `19', `0', `1')
icm(`SAC_ND_FREE__DATA',
    `AUD', `HID', `*',
    `SAC_ND_FREE__DATA__AKS_HID', `19', `0', `1')

dnl SAC_ND_ASSIGN__DATA
dnl
dnl Note: This ICM takes TWO nametuple arguments instead of one. This is
dnl       implemented by expanding it twice. The intermediate macro's are
dnl       immediately called from here, so it's not visible from the outside,
dnl       but this implementation is fairly inefficient in terms of code size.
dnl
dnl There are five cases for the LHS nametuple:
dnl     SCL, NHD
dnl     SCL, HID
dnl     AKS and AKD
dnl     AUD, NHD
dnl     AUD, HID
dnl
dnl These are actually reduced to less target macro's, but we still need
dnl them.

icm(`SAC_ND_ASSIGN__DATA',
    `SCL', `NHD', `*',
    `GEN_SAC_ND_ASSIGN__DATA__SCL_NHD', `22', `0', `2')
icm(`SAC_ND_ASSIGN__DATA',
    `SCL', `HID', `*',
    `GEN_SAC_ND_ASSIGN__DATA__SCL_HID', `22', `0', `2')
icm(`SAC_ND_ASSIGN__DATA',
    `AKS', `*', `*',
    `GEN_SAC_ND_ASSIGN__DATA__AKS', `22', `0', `2')
icm(`SAC_ND_ASSIGN__DATA',
    `AKD', `*', `*',
    `GEN_SAC_ND_ASSIGN__DATA__AKS', `22', `0', `2')
icm(`SAC_ND_ASSIGN__DATA',
    `AKD', `*', `*',
    `GEN_SAC_ND_ASSIGN__DATA__AKS', `22', `0', `2')
icm(`SAC_ND_ASSIGN__DATA',
    `AUD', `NHD', `*',
    `GEN_SAC_ND_ASSIGN__DATA__AUD_NHD', `22', `0', `2')
icm(`SAC_ND_ASSIGN__DATA',
    `AUD', `HID', `*',
    `GEN_SAC_ND_ASSIGN__DATA__AUD_HID', `22', `0', `2')

dnl GEN_SAC_ND_ASSIGN__DATA__SCL_NHD
dnl
dnl Left-hand NT is a SCL NHD. There are three cases for the RHS:
dnl     SCL -> do a direct assignment of the scl value
dnl     AKS/AKD, AUD HID -> undef (can't assign)
dnl     AUD, NHD -> copy aud value to scl

icm(`GEN_SAC_ND_ASSIGN__DATA__SCL_NHD',
    `SCL', `*', `*',
    `SAC_ND_ASSIGN__DATA__AKS_AKS', `25', `1', `1')
icm(`GEN_SAC_ND_ASSIGN__DATA__SCL_NHD',
    `AUD', `NHD', `*',
    `SAC_ND_ASSIGN__DATA__SCL_AUD', `25', `1', `1')
icm(`GEN_SAC_ND_ASSIGN__DATA__SCL_NHD',
    `*', `*', `*',
    `SAC_ND_ASSIGN__DATA__UNDEF', `25', `1', `1')

dnl GEN_SAC_ND_ASSIGN__DATA__SCL_HID
dnl
dnl Left-hand NT is a SCL HID. There are four cases for the RHS:
dnl     SCL -> do a direct assignment of the scl value
dnl     AKS/AKD, AUD NHD -> undef (can't assign)
dnl     AUD HID NUQ  -> copy aud value to scl
dnl     AUD HID UNQ  -> copy aud value to scl, free unique aud

icm(`GEN_SAC_ND_ASSIGN__DATA__SCL_HID',
    `SCL', `*', `*',
    `SAC_ND_ASSIGN__DATA__AKS_AKS', `25', `1', `1')
icm(`GEN_SAC_ND_ASSIGN__DATA__SCL_HID',
    `AUD', `HID', `NUQ',
    `SAC_ND_ASSIGN__DATA__SCL_AUD', `25', `1', `1')
icm(`GEN_SAC_ND_ASSIGN__DATA__SCL_HID',
    `AUD', `HID', `UNQ',
    `SAC_ND_ASSIGN__DATA__SCL_AUD_UNQ', `25', `1', `1')
icm(`GEN_SAC_ND_ASSIGN__DATA__SCL_HID',
    `*', `*', `*',
    `SAC_ND_ASSIGN__DATA__UNDEF', `25', `1', `1')

dnl GEN_SAC_ND_ASSIGN__DATA__AKS
dnl
dnl Left-hand NT is a AKS or AKD. There are two cases for the RHS:
dnl     SCL -> undef (can't assign)
dnl     AKS, AKD or AUD -> do a direct assignment

icm(`GEN_SAC_ND_ASSIGN__DATA__AKS',
    `SCL', `*', `*',
    `SAC_ND_ASSIGN__DATA__UNDEF', `25', `1', `1')
icm(`GEN_SAC_ND_ASSIGN__DATA__AKS',
    `*', `*', `*',
    `SAC_ND_ASSIGN__DATA__AKS_AKS', `25', `1', `1')

dnl GEN_SAC_ND_ASSIGN__DATA__AUD_NHD
dnl
dnl Left-hand NT is a AUD NHD. There are three cases for the RHS:
dnl     SCL NHD -> allocate aud, copy scl
dnl     SCL HID -> undef (can't assign)
dnl     AKS, AKD, AUD -> allocate aud, do assignment

icm(`GEN_SAC_ND_ASSIGN__DATA__AUD_NHD',
    `SCL', `NHD', `*',
    `SAC_ND_ASSIGN__DATA__AUD_SCL_NHD', `25', `1', `1')
icm(`GEN_SAC_ND_ASSIGN__DATA__AUD_NHD',
    `SCL', `HID', `*',
    `SAC_ND_ASSIGN__DATA__UNDEF', `25', `1', `1')
icm(`GEN_SAC_ND_ASSIGN__DATA__AUD_NHD',
    `*', `*', `*',
    `SAC_ND_ASSIGN__DATA__AUD_AKS', `25', `1', `1')

dnl GEN_SAC_ND_ASSIGN__DATA__AUD_HID

dnl Left-hand NT is a AUD NHD. There are four cases for the RHS:
dnl     SCL HID NUQ -> allocate aud, copy scl
dnl     SCL HID UNQ -> allocate aud, copy scl, do uniqueness foo
dnl     SCL NHD -> undef (can't assign)
dnl     AKS, AKD, AUD -> allocate aud, do assignment

icm(`GEN_SAC_ND_ASSIGN__DATA__AUD_HID',
    `SCL', `HID', `NUQ',
    `SAC_ND_ASSIGN__DATA__AUD_SCL_NHD', `25', `1', `1')
icm(`GEN_SAC_ND_ASSIGN__DATA__AUD_HID',
    `SCL', `HID', `UNQ',
    `SAC_ND_ASSIGN__DATA__AUD_SCL_UNQ', `25', `1', `1')
icm(`GEN_SAC_ND_ASSIGN__DATA__AUD_HID',
    `SCL', `NHD', `*',
    `SAC_ND_ASSIGN__DATA__UNDEF', `25', `1', `1')
icm(`GEN_SAC_ND_ASSIGN__DATA__AUD_HID',
    `*', `*', `*',
    `SAC_ND_ASSIGN__DATA__AUD_AKS', `25', `1', `1')

dnl SAC_ND_COPY__DATA
dnl
dnl Note: This ICM takes TWO nametuple arguments instead of one. This is
dnl       implemented by expanding it twice. The intermediate macro's are
dnl       immediately called from here, so it's not visible from the outside,
dnl       but this implementation is fairly inefficient in terms of code size.
dnl
dnl There are two cases for the LHS nametuple:
dnl     SCL
dnl     AKS, AKD, AUD
dnl
dnl These are actually reduced to two target macro's, but we still need
dnl the intermediate ones for the SCL case because it's not a 1:1
dnl mapping from the LHS.

icm(`SAC_ND_COPY__DATA',
    `SCL', `*', `*',
    `GEN_SAC_ND_COPY__DATA__SCL', `22', `0', `2')
icm(`SAC_ND_COPY__DATA',
    `*', `*', `*',
    `SAC_ND_COPY__DATA__DEFAULT', `22', `0', `2')

dnl GEN_SAC_ND_COPY__DATA__SCL
dnl
dnl There are two cases for the RHS:
dnl   SCL -> copy scalar value
dnl   AKS, AKD, AUD -> do a data copy

icm(`GEN_SAC_ND_COPY__DATA__SCL',
    `SCL', `*', `*',
    `SAC_ND_COPY__DATA__SCL_SCL', `25', `1', `1')
icm(`GEN_SAC_ND_COPY__DATA__SCL',
    `*', `*', `*',
    `SAC_ND_COPY__DATA__DEFAULT', `25', `1', `1')

dnl SAC_ND_SET__RC

icm(`SAC_ND_SET__RC',
    `*', `*', `UNQ',
    `SAC_ND_SET__RC__NOOP', `28', `0', `1')
icm(`SAC_ND_SET__RC',
    `SCL', `NHD', `NUQ',
    `SAC_ND_SET__RC__NOOP', `28', `0', `1')
icm(`SAC_ND_SET__RC',
    `*', `*', `NUQ',
    `SAC_ND_SET__RC__DEFAULT', `28', `0', `1')

dnl SAC_ND_INC_RC

icm(`SAC_ND_INC_RC',
    `*', `*', `UNQ',
    `SAC_ND_INC_RC__NOOP', `28', `0', `1')
icm(`SAC_ND_INC_RC',
    `SCL', `NHD', `NUQ',
    `SAC_ND_INC_RC__NOOP', `28', `0', `1')
icm(`SAC_ND_INC_RC',
    `*', `*', `NUQ',
    `SAC_ND_INC_RC__DEFAULT', `28', `0', `1')

dnl SAC_ND_DEC_RC

icm(`SAC_ND_DEC_RC',
    `*', `*', `UNQ',
    `SAC_ND_DEC_RC__NOOP', `28', `0', `1')
icm(`SAC_ND_DEC_RC',
    `SCL', `NHD', `NUQ',
    `SAC_ND_DEC_RC__NOOP', `28', `0', `1')
icm(`SAC_ND_DEC_RC',
    `*', `*', `NUQ',
    `SAC_ND_DEC_RC__DEFAULT', `28', `0', `1')

dnl SAC_ND_DEC_RC_FREE

icm(`SAC_ND_DEC_RC_FREE',
    `*', `*', `UNQ',
    `SAC_ND_DEC_RC_FREE__UNQ', `28', `0', `2')
icm(`SAC_ND_DEC_RC_FREE',
    `SCL', `NHD', `NUQ',
    `SAC_ND_DEC_RC_FREE__NOOP', `28', `0', `2')
icm(`SAC_ND_DEC_RC_FREE',
    `*', `*', `NUQ',
    `SAC_ND_DEC_RC_FREE__DEFAULT', `28', `0', `2')

dnl SAC_IS_LASTREF__BLOCK_BEGIN

icm(`SAC_IS_LASTREF__BLOCK_BEGIN',
    `*', `*', `UNQ',
    `SAC_IS_LASTREF__BLOCK_BEGIN__UNQ', `28', `0', `0')
icm(`SAC_IS_LASTREF__BLOCK_BEGIN',
    `SCL', `NHD', `NUQ',
    `SAC_IS_LASTREF__BLOCK_BEGIN__SCL_NHD_NUQ', `28', `0', `0')
icm(`SAC_IS_LASTREF__BLOCK_BEGIN',
    `*', `*', `NUQ',
    `SAC_IS_LASTREF__BLOCK_BEGIN__DEFAULT', `28', `0', `0')

dnl SAC_IS_REUSED__BLOCK_BEGIN

icm(`SAC_IS_REUSED__BLOCK_BEGIN',
    `SCL', `*', `*',
    `SAC_IS_REUSED__BLOCK_BEGIN__SCL', `28', `0', `1')
icm(`SAC_IS_REUSED__BLOCK_BEGIN',
    `*', `*', `*',
    `SAC_IS_REUSED__BLOCK_BEGIN__DEFAULT', `28', `0', `1')

