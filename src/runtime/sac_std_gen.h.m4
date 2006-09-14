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
