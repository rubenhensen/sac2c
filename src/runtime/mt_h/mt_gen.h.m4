








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

start_icm_definition(mt_gen)

#ifndef _SAC_MT_GEN_H_
#define _SAC_MT_GEN_H_

#ifndef SAC_SIMD_COMPILATION

#if SAC_DO_MULTITHREAD


pat(`SAC_MT_FRAME_ELEMENT_inout', `3', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_MT_FRAME_ELEMENT_inout', `SAC_MT_FRAME_ELEMENT_inout__NODESC', `SCL', `*HID', `*UNQ')
rule(`SAC_MT_FRAME_ELEMENT_inout', `SAC_MT_FRAME_ELEMENT_inout__NODESC', `AKS', `*HID', `*UNQ')
rule(`SAC_MT_FRAME_ELEMENT_inout', `SAC_MT_FRAME_ELEMENT_inout__DESC', `AKD', `*HID', `*UNQ')
rule(`SAC_MT_FRAME_ELEMENT_inout', `SAC_MT_FRAME_ELEMENT_inout__DESC', `AUD', `*HID', `*UNQ')


pat(`SAC_MT_FRAME_ELEMENT_in', `3', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_MT_FRAME_ELEMENT_in', `SAC_MT_FRAME_ELEMENT_in__NODESC', `SCL', `*HID', `*UNQ')
rule(`SAC_MT_FRAME_ELEMENT_in', `SAC_MT_FRAME_ELEMENT_in__NODESC', `AKS', `*HID', `*UNQ')
rule(`SAC_MT_FRAME_ELEMENT_in', `SAC_MT_FRAME_ELEMENT_in__DESC', `AKD', `*HID', `*UNQ')
rule(`SAC_MT_FRAME_ELEMENT_in', `SAC_MT_FRAME_ELEMENT_in__DESC', `AUD', `*HID', `*UNQ')


pat(`SAC_MT_FRAME_ELEMENT_out', `3', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_MT_FRAME_ELEMENT_out', `SAC_MT_FRAME_ELEMENT_out__NOOP', `*SHP', `*HID', `*UNQ')


pat(`SAC_MT_BARRIER_ELEMENT_inout', `3', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_MT_BARRIER_ELEMENT_inout', `SAC_MT_BARRIER_ELEMENT_out__NODESC', `SCL', `*HID', `*UNQ')
rule(`SAC_MT_BARRIER_ELEMENT_inout', `SAC_MT_BARRIER_ELEMENT_out__NODESC', `AKS', `*HID', `*UNQ')
rule(`SAC_MT_BARRIER_ELEMENT_inout', `SAC_MT_BARRIER_ELEMENT_out__DESC', `AKD', `*HID', `*UNQ')
rule(`SAC_MT_BARRIER_ELEMENT_inout', `SAC_MT_BARRIER_ELEMENT_out__DESC', `AUD', `*HID', `*UNQ')


pat(`SAC_MT_BARRIER_ELEMENT_out', `3', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_MT_BARRIER_ELEMENT_out', `SAC_MT_BARRIER_ELEMENT_out__NODESC', `SCL', `*HID', `*UNQ')
rule(`SAC_MT_BARRIER_ELEMENT_out', `SAC_MT_BARRIER_ELEMENT_out__NODESC', `AKS', `*HID', `*UNQ')
rule(`SAC_MT_BARRIER_ELEMENT_out', `SAC_MT_BARRIER_ELEMENT_out__DESC', `AKD', `*HID', `*UNQ')
rule(`SAC_MT_BARRIER_ELEMENT_out', `SAC_MT_BARRIER_ELEMENT_out__DESC', `AUD', `*HID', `*UNQ')


pat(`SAC_MT_BARRIER_ELEMENT_in', `3', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_MT_BARRIER_ELEMENT_in', `SAC_MT_BARRIER_ELEMENT_in__NOOP', `*SHP', `*HID', `*UNQ')


pat(`SAC_MT_SEND_PARAM_in', `2', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_MT_SEND_PARAM_in', `SAC_MT_SEND_PARAM_in__NODESC', `SCL', `*HID', `*UNQ')
rule(`SAC_MT_SEND_PARAM_in', `SAC_MT_SEND_PARAM_in__NODESC', `AKS', `*HID', `*UNQ')
rule(`SAC_MT_SEND_PARAM_in', `SAC_MT_SEND_PARAM_in__DESC', `AKD', `*HID', `*UNQ')
rule(`SAC_MT_SEND_PARAM_in', `SAC_MT_SEND_PARAM_in__DESC', `AUD', `*HID', `*UNQ')


pat(`SAC_MT_SEND_PARAM_inout', `2', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_MT_SEND_PARAM_inout', `SAC_MT_SEND_PARAM_inout__NODESC', `SCL', `*HID', `*UNQ')
rule(`SAC_MT_SEND_PARAM_inout', `SAC_MT_SEND_PARAM_inout__NODESC', `AKS', `*HID', `*UNQ')
rule(`SAC_MT_SEND_PARAM_inout', `SAC_MT_SEND_PARAM_inout__DESC', `AKD', `*HID', `*UNQ')
rule(`SAC_MT_SEND_PARAM_inout', `SAC_MT_SEND_PARAM_inout__DESC', `AUD', `*HID', `*UNQ')


pat(`SAC_MT_SEND_PARAM_out', `2', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_MT_SEND_PARAM_out', `SAC_MT_SEND_PARAM_out__NOOP', `*SHP', `*HID', `*UNQ')


pat(`SAC_MT_RECEIVE_PARAM_in', `3', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_MT_RECEIVE_PARAM_in', `SAC_MT_RECEIVE_PARAM_in__NODESC', `SCL', `NHD', `*UNQ')
rule(`SAC_MT_RECEIVE_PARAM_in', `SAC_MT_RECEIVE_PARAM_in__NODESC__FAKERC', `SCL', `HID', `*UNQ')
rule(`SAC_MT_RECEIVE_PARAM_in', `SAC_MT_RECEIVE_PARAM_in__NODESC__FAKERC', `AKS', `*HID', `*UNQ')
rule(`SAC_MT_RECEIVE_PARAM_in', `SAC_MT_RECEIVE_PARAM_in__DESC', `AKD', `*HID', `*UNQ')
rule(`SAC_MT_RECEIVE_PARAM_in', `SAC_MT_RECEIVE_PARAM_in__DESC', `AUD', `*HID', `*UNQ')


pat(`SAC_MT_RECEIVE_PARAM_inout', `3', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_MT_RECEIVE_PARAM_inout', `SAC_MT_RECEIVE_PARAM_inout__NODESC', `SCL', `NHD', `*UNQ')
rule(`SAC_MT_RECEIVE_PARAM_inout', `SAC_MT_RECEIVE_PARAM_inout__NODESC__FAKERC', `SCL', `HID', `*UNQ')
rule(`SAC_MT_RECEIVE_PARAM_inout', `SAC_MT_RECEIVE_PARAM_inout__NODESC__FAKERC', `AKS', `*HID', `*UNQ')
rule(`SAC_MT_RECEIVE_PARAM_inout', `SAC_MT_RECEIVE_PARAM_inout__DESC', `AKD', `*HID', `*UNQ')
rule(`SAC_MT_RECEIVE_PARAM_inout', `SAC_MT_RECEIVE_PARAM_inout__DESC', `AUD', `*HID', `*UNQ')


pat(`SAC_MT_RECEIVE_PARAM_out', `3', `NT_SHP', `NT_HID', `NT_UNQ')
rule(`SAC_MT_RECEIVE_PARAM_out', `SAC_MT_RECEIVE_PARAM_out__NOOP', `*SHP', `*HID', `*UNQ')

#endif  /* SAC_DO_MULTITHREAD */

#endif  /* SAC_SIMD_COMPILATION */

#endif  /* _SAC_MT_GEN_H_ */

end_icm_definition
