/*
 * $Id$
 *
 */

/*
 * CAUTION: 
 *
 * mt_gen.h  is generated automatically from mt_gen.h.m4
 *
 */

include(`icm.m4')

#ifndef _SAC_MT_GEN_H_
#define _SAC_MT_GEN_H_

#ifndef SAC_SIMD_COMPILATION

#if SAC_DO_MULTITHREAD

dnl SAC_MT_FRAME_ELEMENT_inout

icm(`SAC_MT_FRAME_ELEMENT_inout',
    `SCL', `*', `*',
    `SAC_MT_FRAME_ELEMENT_inout__NODESC', `10', `3', `0')
icm(`SAC_MT_FRAME_ELEMENT_inout',
    `AKS', `*', `*',
    `SAC_MT_FRAME_ELEMENT_inout__NODESC', `10', `3', `0')
icm(`SAC_MT_FRAME_ELEMENT_inout',
    `AKD', `*', `*',
    `SAC_MT_FRAME_ELEMENT_inout__DESC', `10', `3', `0')
icm(`SAC_MT_FRAME_ELEMENT_inout',
    `AUD', `*', `*',
    `SAC_MT_FRAME_ELEMENT_inout__DESC', `10', `3', `0')

dnl SAC_MT_FRAME_ELEMENT_in

icm(`SAC_MT_FRAME_ELEMENT_in',
    `SCL', `*', `*',
    `SAC_MT_FRAME_ELEMENT_in__NODESC', `10', `3', `0')
icm(`SAC_MT_FRAME_ELEMENT_in',
    `AKS', `*', `*',
    `SAC_MT_FRAME_ELEMENT_in__NODESC', `10', `3', `0')
icm(`SAC_MT_FRAME_ELEMENT_in',
    `AKD', `*', `*',
    `SAC_MT_FRAME_ELEMENT_in__DESC', `10', `3', `0')
icm(`SAC_MT_FRAME_ELEMENT_in',
    `AUD', `*', `*',
    `SAC_MT_FRAME_ELEMENT_in__DESC', `10', `3', `0')

dnl SAC_MT_FRAME_ELEMENT_out

icm(`SAC_MT_FRAME_ELEMENT_out',
    `*', `*', `*',
    `SAC_MT_FRAME_ELEMENT_out__NOOP', `12', `3', `0')

dnl SAC_MT_BARRIER_ELEMENT_inout

icm(`SAC_MT_BARRIER_ELEMENT_inout',
    `SCL', `*', `*',
    `SAC_MT_BARRIER_ELEMENT_out__NODESC', `10', `3', `0')
icm(`SAC_MT_BARRIER_ELEMENT_inout',
    `AKS', `*', `*',
    `SAC_MT_BARRIER_ELEMENT_out__NODESC', `10', `3', `0')
icm(`SAC_MT_BARRIER_ELEMENT_inout',
    `AKD', `*', `*',
    `SAC_MT_BARRIER_ELEMENT_out__DESC', `10', `3', `0')
icm(`SAC_MT_BARRIER_ELEMENT_inout',
    `AUD', `*', `*',
    `SAC_MT_BARRIER_ELEMENT_out__DESC', `10', `3', `0')

dnl SAC_MT_BARRIER_ELEMENT_out

icm(`SAC_MT_BARRIER_ELEMENT_out',
    `SCL', `*', `*',
    `SAC_MT_BARRIER_ELEMENT_out__NODESC', `10', `3', `0')
icm(`SAC_MT_BARRIER_ELEMENT_out',
    `AKS', `*', `*',
    `SAC_MT_BARRIER_ELEMENT_out__NODESC', `10', `3', `0')
icm(`SAC_MT_BARRIER_ELEMENT_out',
    `AKD', `*', `*',
    `SAC_MT_BARRIER_ELEMENT_out__DESC', `10', `3', `0')
icm(`SAC_MT_BARRIER_ELEMENT_out',
    `AUD', `*', `*',
    `SAC_MT_BARRIER_ELEMENT_out__DESC', `10', `3', `0')

dnl SAC_MT_BARRIER_ELEMENT_in

icm(`SAC_MT_BARRIER_ELEMENT_in',
    `*', `*', `*',
    `SAC_MT_BARRIER_ELEMENT_in__NOOP', `10', `3', `0')

dnl SAC_MT_SEND_PARAM_in

icm(`SAC_MT_SEND_PARAM_in',
    `SCL', `*', `*',
    `SAC_MT_SEND_PARAM_in__NODESC', `10', `2', `0')
icm(`SAC_MT_SEND_PARAM_in',
    `AKS', `*', `*',
    `SAC_MT_SEND_PARAM_in__NODESC', `10', `2', `0')
icm(`SAC_MT_SEND_PARAM_in',
    `AKD', `*', `*',
    `SAC_MT_SEND_PARAM_in__DESC', `10', `2', `0')
icm(`SAC_MT_SEND_PARAM_in',
    `AUD', `*', `*',
    `SAC_MT_SEND_PARAM_in__DESC', `10', `2', `0')

dnl SAC_MT_SEND_PARAM_inout

icm(`SAC_MT_SEND_PARAM_inout',
    `SCL', `*', `*',
    `SAC_MT_SEND_PARAM_inout__NODESC', `10', `2', `0')
icm(`SAC_MT_SEND_PARAM_inout',
    `AKS', `*', `*',
    `SAC_MT_SEND_PARAM_inout__NODESC', `10', `2', `0')
icm(`SAC_MT_SEND_PARAM_inout',
    `AKD', `*', `*',
    `SAC_MT_SEND_PARAM_inout__DESC', `10', `2', `0')
icm(`SAC_MT_SEND_PARAM_inout',
    `AUD', `*', `*',
    `SAC_MT_SEND_PARAM_inout__DESC', `10', `2', `0')

dnl SAC_MT_SEND_PARAM_out

icm(`SAC_MT_SEND_PARAM_out',
    `*', `*', `*',
    `SAC_MT_SEND_PARAM_out__NOOP', `10', `2', `0')

dnl SAC_MT_RECEIVE_PARAM_in

icm(`SAC_MT_RECEIVE_PARAM_in',
    `SCL', `NHD', `*',
    `SAC_MT_RECEIVE_PARAM_in__NODESC', `10', `3', `0')
icm(`SAC_MT_RECEIVE_PARAM_in',
    `SCL', `HID', `*',
    `SAC_MT_RECEIVE_PARAM_in__NODESC__FAKERC', `10', `3', `0')
icm(`SAC_MT_RECEIVE_PARAM_in',
    `AKS', `*', `*',
    `SAC_MT_RECEIVE_PARAM_in__NODESC__FAKERC', `10', `3', `0')
icm(`SAC_MT_RECEIVE_PARAM_in',
    `AKD', `*', `*',
    `SAC_MT_RECEIVE_PARAM_in__DESC', `10', `3', `0')
icm(`SAC_MT_RECEIVE_PARAM_in',
    `AUD', `*', `*',
    `SAC_MT_RECEIVE_PARAM_in__DESC', `10', `3', `0')

dnl SAC_MT_RECEIVE_PARAM_inout

icm(`SAC_MT_RECEIVE_PARAM_inout',
    `SCL', `NHD', `*',
    `SAC_MT_RECEIVE_PARAM_inout__NODESC', `10', `3', `0')
icm(`SAC_MT_RECEIVE_PARAM_inout',
    `SCL', `HID', `*',
    `SAC_MT_RECEIVE_PARAM_inout__NODESC__FAKERC', `10', `3', `0')
icm(`SAC_MT_RECEIVE_PARAM_inout',
    `AKS', `*', `*',
    `SAC_MT_RECEIVE_PARAM_inout__NODESC__FAKERC', `10', `3', `0')
icm(`SAC_MT_RECEIVE_PARAM_inout',
    `AKD', `*', `*',
    `SAC_MT_RECEIVE_PARAM_inout__DESC', `10', `3', `0')
icm(`SAC_MT_RECEIVE_PARAM_inout',
    `AUD', `*', `*',
    `SAC_MT_RECEIVE_PARAM_inout__DESC', `10', `3', `0')

dnl SAC_MT_RECEIVE_PARAM_out

icm(`SAC_MT_RECEIVE_PARAM_out',
    `*', `*', `*',
    `SAC_MT_RECEIVE_PARAM_out__NOOP', `10', `3', `0')

#endif  /* SAC_DO_MULTITHREAD */

#endif  /* SAC_SIMD_COMPILATION */

#endif  /* _SAC_MT_GEN_H_ */
