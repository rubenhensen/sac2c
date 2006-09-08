include(`icm.m4')

dnl SAC_MT_FRAME_ELEMENT_inout

icm(`SAC_MT_FRAME_ELEMENT_inout',
    `SCL', `*', `*',
    `SAC_MT_FRAME_ELEMENT_in__NODESC', `12', `3', `0')
icm(`SAC_MT_FRAME_ELEMENT_inout',
    `AKS', `*', `*',
    `SAC_MT_FRAME_ELEMENT_in__NODESC', `12', `3', `0')
icm(`SAC_MT_FRAME_ELEMENT_inout',
    `AKD', `*', `*',
    `SAC_MT_FRAME_ELEMENT_in__DESC', `12', `3', `0')
icm(`SAC_MT_FRAME_ELEMENT_inout',
    `AUD', `*', `*',
    `SAC_MT_FRAME_ELEMENT_in__DESC', `12', `3', `0')

dnl SAC_MT_FRAME_ELEMENT_in

icm(`SAC_MT_FRAME_ELEMENT_in',
    `SCL', `*', `*',
    `SAC_MT_FRAME_ELEMENT_in__NODESC', `12', `3', `0')
icm(`SAC_MT_FRAME_ELEMENT_in',
    `AKS', `*', `*',
    `SAC_MT_FRAME_ELEMENT_in__NODESC', `12', `3', `0')
icm(`SAC_MT_FRAME_ELEMENT_in',
    `AKD', `*', `*',
    `SAC_MT_FRAME_ELEMENT_in__DESC', `12', `3', `0')
icm(`SAC_MT_FRAME_ELEMENT_in',
    `AUD', `*', `*',
    `SAC_MT_FRAME_ELEMENT_in__DESC', `12', `3', `0')

dnl SAC_MT_FRAME_ELEMENT_out

icm(`SAC_MT_FRAME_ELEMENT_out',
    `*', `*', `*',
    `SAC_MT_FRAME_ELEMENT_out__NOOP', `12', `3', `0')

dnl SAC_MT_BARRIER_ELEMENT_inout

icm(`SAC_MT_BARRIER_ELEMENT_inout',
    `SCL', `*', `*',
    `SAC_MT_BARRIER_ELEMENT_out__NODESC', `12', `3', `0')
icm(`SAC_MT_BARRIER_ELEMENT_inout',
    `AKS', `*', `*',
    `SAC_MT_BARRIER_ELEMENT_out__NODESC', `12', `3', `0')
icm(`SAC_MT_BARRIER_ELEMENT_inout',
    `AKD', `*', `*',
    `SAC_MT_BARRIER_ELEMENT_out__DESC', `12', `3', `0')
icm(`SAC_MT_BARRIER_ELEMENT_inout',
    `AUD', `*', `*',
    `SAC_MT_BARRIER_ELEMENT_out__DESC', `12', `3', `0')

dnl SAC_MT_BARRIER_ELEMENT_out

icm(`SAC_MT_BARRIER_ELEMENT_out',
    `SCL', `*', `*',
    `SAC_MT_BARRIER_ELEMENT_out__NODESC', `12', `3', `0')
icm(`SAC_MT_BARRIER_ELEMENT_out',
    `AKS', `*', `*',
    `SAC_MT_BARRIER_ELEMENT_out__NODESC', `12', `3', `0')
icm(`SAC_MT_BARRIER_ELEMENT_out',
    `AKD', `*', `*',
    `SAC_MT_BARRIER_ELEMENT_out__DESC', `12', `3', `0')
icm(`SAC_MT_BARRIER_ELEMENT_out',
    `AUD', `*', `*',
    `SAC_MT_BARRIER_ELEMENT_out__DESC', `12', `3', `0')

dnl SAC_MT_BARRIER_ELEMENT_in

icm(`SAC_MT_BARRIER_ELEMENT_in',
    `*', `*', `*',
    `SAC_MT_BARRIER_ELEMENT_in__NOOP', `12', `3', `0')

dnl SAC_MT_SEND_PARAM_in

icm(`SAC_MT_SEND_PARAM_in',
    `SCL', `*', `*',
    `SAC_MT_SEND_PARAM_in__NODESC', `12', `2', `0')
icm(`SAC_MT_SEND_PARAM_in',
    `AKS', `*', `*',
    `SAC_MT_SEND_PARAM_in__NODESC', `12', `2', `0')
icm(`SAC_MT_SEND_PARAM_in',
    `AKD', `*', `*',
    `SAC_MT_SEND_PARAM_in__DESC', `12', `2', `0')
icm(`SAC_MT_SEND_PARAM_in',
    `AUD', `*', `*',
    `SAC_MT_SEND_PARAM_in__DESC', `12', `2', `0')

dnl SAC_MT_SEND_PARAM_inout

icm(`SAC_MT_SEND_PARAM_inout',
    `SCL', `*', `*',
    `SAC_MT_SEND_PARAM_in__NODESC', `12', `2', `0')
icm(`SAC_MT_SEND_PARAM_inout',
    `AKS', `*', `*',
    `SAC_MT_SEND_PARAM_in__NODESC', `12', `2', `0')
icm(`SAC_MT_SEND_PARAM_inout',
    `AKD', `*', `*',
    `SAC_MT_SEND_PARAM_in__DESC', `12', `2', `0')
icm(`SAC_MT_SEND_PARAM_inout',
    `AUD', `*', `*',
    `SAC_MT_SEND_PARAM_in__DESC', `12', `2', `0')

dnl SAC_MT_SEND_PARAM_out

icm(`SAC_MT_SEND_PARAM_out',
    `*', `*', `*',
    `SAC_MT_SEND_PARAM_out__NOOP', `12', `2', `0')

dnl SAC_MT_RECEIVE_PARAM_in

icm(`SAC_MT_RECEIVE_PARAM_in',
    `SCL', `NHD', `*',
    `SAC_MT_RECEIVE_PARAM_in__NODESC', `12', `3', `0')
icm(`SAC_MT_RECEIVE_PARAM_in',
    `SCL', `HID', `*',
    `SAC_MT_RECEIVE_PARAM_in__NODESC__FAKERC', `12', `3', `0')
icm(`SAC_MT_RECEIVE_PARAM_in',
    `AKS', `*', `*',
    `SAC_MT_RECEIVE_PARAM_in__NODESC', `12', `3', `0')
icm(`SAC_MT_RECEIVE_PARAM_in',
    `AKD', `*', `*',
    `SAC_MT_RECEIVE_PARAM_in__DESC', `12', `3', `0')
icm(`SAC_MT_RECEIVE_PARAM_in',
    `AUD', `*', `*',
    `SAC_MT_RECEIVE_PARAM_in__DESC', `12', `3', `0')

dnl SAC_MT_RECEIVE_PARAM_inout

icm(`SAC_MT_RECEIVE_PARAM_inout',
    `SCL', `NHD', `*',
    `SAC_MT_RECEIVE_PARAM_in__NODESC', `12', `3', `0')
icm(`SAC_MT_RECEIVE_PARAM_inout',
    `SCL', `HID', `*',
    `SAC_MT_RECEIVE_PARAM_in__NODESC__FAKERC', `12', `3', `0')
icm(`SAC_MT_RECEIVE_PARAM_inout',
    `AKS', `*', `*',
    `SAC_MT_RECEIVE_PARAM_in__NODESC', `12', `3', `0')
icm(`SAC_MT_RECEIVE_PARAM_inout',
    `AKD', `*', `*',
    `SAC_MT_RECEIVE_PARAM_in__DESC', `12', `3', `0')
icm(`SAC_MT_RECEIVE_PARAM_inout',
    `AUD', `*', `*',
    `SAC_MT_RECEIVE_PARAM_in__DESC', `12', `3', `0')

dnl SAC_MT_RECEIVE_PARAM_out

icm(`SAC_MT_RECEIVE_PARAM_out',
    `*', `*', `*',
    `SAC_MT_RECEIVE_PARAM_out__NOOP', `12', `3', `0')

