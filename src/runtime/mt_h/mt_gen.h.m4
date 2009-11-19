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

/*
 * See ../m4/README
 */

include(`icm.m4')

start_icm_definition(mt_gen)

#ifndef _SAC_MT_GEN_H_
#define _SAC_MT_GEN_H_

#ifndef SAC_SIMD_COMPILATION

#if SAC_DO_MULTITHREAD

/*
 * Macros for defining the SPMD frame
 */

pat(`SAC_MT_FRAME_ELEMENT_inout', `3', `0', `NT_SHP')
rule(`SAC_MT_FRAME_ELEMENT_inout', `SAC_MT_FRAME_ELEMENT_inout__NODESC', `SCL')
rule(`SAC_MT_FRAME_ELEMENT_inout', `SAC_MT_FRAME_ELEMENT_inout__NODESC', `AKS')
rule(`SAC_MT_FRAME_ELEMENT_inout', `SAC_MT_FRAME_ELEMENT_inout__DESC', `AKD')
rule(`SAC_MT_FRAME_ELEMENT_inout', `SAC_MT_FRAME_ELEMENT_inout__DESC', `AUD')

pat(`SAC_MT_FRAME_ELEMENT_in', `3', `0', `NT_SHP')
rule(`SAC_MT_FRAME_ELEMENT_in', `SAC_MT_FRAME_ELEMENT_in__NODESC', `SCL')
rule(`SAC_MT_FRAME_ELEMENT_in', `SAC_MT_FRAME_ELEMENT_in__NODESC', `AKS')
rule(`SAC_MT_FRAME_ELEMENT_in', `SAC_MT_FRAME_ELEMENT_in__DESC', `AKD')
rule(`SAC_MT_FRAME_ELEMENT_in', `SAC_MT_FRAME_ELEMENT_in__DESC', `AUD')

pat(`SAC_MT_FRAME_ELEMENT_out', `3', `0')
rule(`SAC_MT_FRAME_ELEMENT_out', `SAC_MT_FRAME_ELEMENT__NOOP')


/*
 * Macros for sending data to the SPMD frame
 */

pat(`SAC_MT_SEND_PARAM_in', `2', `0', `NT_SHP')
rule(`SAC_MT_SEND_PARAM_in', `SAC_MT_SEND_PARAM_in__NODESC', `SCL')
rule(`SAC_MT_SEND_PARAM_in', `SAC_MT_SEND_PARAM_in__NODESC', `AKS')
rule(`SAC_MT_SEND_PARAM_in', `SAC_MT_SEND_PARAM_in__DESC', `AKD')
rule(`SAC_MT_SEND_PARAM_in', `SAC_MT_SEND_PARAM_in__DESC', `AUD')

pat(`SAC_MT_SEND_PARAM_inout', `2', `0', `NT_SHP')
rule(`SAC_MT_SEND_PARAM_inout', `SAC_MT_SEND_PARAM_inout__NODESC', `SCL')
rule(`SAC_MT_SEND_PARAM_inout', `SAC_MT_SEND_PARAM_inout__NODESC', `AKS')
rule(`SAC_MT_SEND_PARAM_inout', `SAC_MT_SEND_PARAM_inout__DESC', `AKD')
rule(`SAC_MT_SEND_PARAM_inout', `SAC_MT_SEND_PARAM_inout__DESC', `AUD')

pat(`SAC_MT_SEND_PARAM_out', `2', `0')
rule(`SAC_MT_SEND_PARAM_out', `SAC_MT_SEND_PARAM__NOOP')


/*
 * Macros for receiving data from the SPMD frame
 */

pat(`SAC_MT_RECEIVE_PARAM_in', `3', `0', `NT_SHP', `NT_HID')
rule(`SAC_MT_RECEIVE_PARAM_in', `SAC_MT_RECEIVE_PARAM_in__NODESC', `SCL', `NHD')
rule(`SAC_MT_RECEIVE_PARAM_in', `SAC_MT_RECEIVE_PARAM_in__NODESC__FAKERC', `SCL', `HID')
rule(`SAC_MT_RECEIVE_PARAM_in', `SAC_MT_RECEIVE_PARAM_in__NODESC__FAKERC', `AKS', `*HID')
rule(`SAC_MT_RECEIVE_PARAM_in', `SAC_MT_RECEIVE_PARAM_in__DESC', `AKD', `*HID')
rule(`SAC_MT_RECEIVE_PARAM_in', `SAC_MT_RECEIVE_PARAM_in__DESC', `AUD', `*HID')

pat(`SAC_MT_RECEIVE_PARAM_inout', `3', `0', `NT_SHP', `NT_HID')
rule(`SAC_MT_RECEIVE_PARAM_inout', `SAC_MT_RECEIVE_PARAM_inout__NODESC', `SCL', `NHD')
rule(`SAC_MT_RECEIVE_PARAM_inout', `SAC_MT_RECEIVE_PARAM_inout__NODESC__FAKERC', `SCL', `HID')
rule(`SAC_MT_RECEIVE_PARAM_inout', `SAC_MT_RECEIVE_PARAM_inout__NODESC__FAKERC', `AKS', `*HID')
rule(`SAC_MT_RECEIVE_PARAM_inout', `SAC_MT_RECEIVE_PARAM_inout__DESC', `AKD', `*HID')
rule(`SAC_MT_RECEIVE_PARAM_inout', `SAC_MT_RECEIVE_PARAM_inout__DESC', `AUD', `*HID')

pat(`SAC_MT_RECEIVE_PARAM_out', `3', `0')
rule(`SAC_MT_RECEIVE_PARAM_out', `SAC_MT_RECEIVE_PARAM__NOOP')


/*
 * Macros for defining the synchronisation barrier
 */

pat(`SAC_MT_BARRIER_ELEMENT_in', `3', `0')
rule(`SAC_MT_BARRIER_ELEMENT_in', `SAC_MT_BARRIER_ELEMENT__NOOP')

pat(`SAC_MT_BARRIER_ELEMENT_inout', `3', `0')
rule(`SAC_MT_BARRIER_ELEMENT_inout', `SAC_MT_BARRIER_ELEMENT__NOOP')

pat(`SAC_MT_BARRIER_ELEMENT_out', `3', `0', `NT_SHP', `NT_HID')
rule(`SAC_MT_BARRIER_ELEMENT_out', `SAC_MT_BARRIER_ELEMENT_out__NODESC', `SCL', `NHD')
rule(`SAC_MT_BARRIER_ELEMENT_out', `SAC_MT_BARRIER_ELEMENT_out__DESC', `SCL', `HID')
rule(`SAC_MT_BARRIER_ELEMENT_out', `SAC_MT_BARRIER_ELEMENT_out__DESC', `AKS', `*HID')
rule(`SAC_MT_BARRIER_ELEMENT_out', `SAC_MT_BARRIER_ELEMENT_out__DESC', `AKD', `*HID')
rule(`SAC_MT_BARRIER_ELEMENT_out', `SAC_MT_BARRIER_ELEMENT_out__DESC', `AUD', `*HID')


/*
 * Macros for sending data to the synchronisation barrier
 */

pat(`SAC_MT_SEND_RESULT_in', `3', `0')
rule(`SAC_MT_SEND_RESULT_in', `SAC_MT_SEND_RESULT__NOOP')

pat(`SAC_MT_SEND_RESULT_inout', `3', `0')
rule(`SAC_MT_SEND_RESULT_inout', `SAC_MT_SEND_RESULT__NOOP')

pat(`SAC_MT_SEND_RESULT_out', `3', `0', `NT_SHP', `NT_HID')
rule(`SAC_MT_SEND_RESULT_out', `SAC_MT_SEND_RESULT_out__NODESC', `SCL', `NHD')
rule(`SAC_MT_SEND_RESULT_out', `SAC_MT_SEND_RESULT_out__DESC', `SCL', `HID')
rule(`SAC_MT_SEND_RESULT_out', `SAC_MT_SEND_RESULT_out__DESC', `AKS', `*HID')
rule(`SAC_MT_SEND_RESULT_out', `SAC_MT_SEND_RESULT_out__DESC', `AKD', `*HID')
rule(`SAC_MT_SEND_RESULT_out', `SAC_MT_SEND_RESULT_out__DESC', `AUD', `*HID')


/*
 * Macros for receiving data from the synchronisation barrier
 */

pat(`SAC_MT_RECEIVE_RESULT_in', `3', `0')
rule(`SAC_MT_RECEIVE_RESULT_in', `SAC_MT_RECEIVE_RESULT__NOOP')

pat(`SAC_MT_RECEIVE_RESULT_inout', `3', `0')
rule(`SAC_MT_RECEIVE_RESULT_inout', `SAC_MT_RECEIVE_RESULT__NOOP')

pat(`SAC_MT_RECEIVE_RESULT_out', `3', `0', `NT_SHP', `NT_HID')
rule(`SAC_MT_RECEIVE_RESULT_out', `SAC_MT_RECEIVE_RESULT_out__NODESC', `SCL', `NHD')
rule(`SAC_MT_RECEIVE_RESULT_out', `SAC_MT_RECEIVE_RESULT_out__DESC', `SCL', `HID')
rule(`SAC_MT_RECEIVE_RESULT_out', `SAC_MT_RECEIVE_RESULT_out__DESC', `AKS', `*HID')
rule(`SAC_MT_RECEIVE_RESULT_out', `SAC_MT_RECEIVE_RESULT_out__DESC', `AKD', `*HID')
rule(`SAC_MT_RECEIVE_RESULT_out', `SAC_MT_RECEIVE_RESULT_out__DESC', `AUD', `*HID')


/*
 * Macros for folding data during barrier synchronisation
 */

pat(`SAC_MT_SYNC_FOLD_in', `2', `4')
rule(`SAC_MT_SYNC_FOLD_in', `SAC_MT_SYNC_FOLD__NOOP')

pat(`SAC_MT_SYNC_FOLD_inout', `2', `4')
rule(`SAC_MT_SYNC_FOLD_inout', `SAC_MT_SYNC_FOLD__NOOP')

pat(`SAC_MT_SYNC_FOLD_out', `2', `4', `NT_SHP', `NT_HID')
rule(`SAC_MT_SYNC_FOLD_out', `SAC_MT_SYNC_FOLD_out__NODESC', `SCL', `NHD')
rule(`SAC_MT_SYNC_FOLD_out', `SAC_MT_SYNC_FOLD_out__DESC', `SCL', `HID')
rule(`SAC_MT_SYNC_FOLD_out', `SAC_MT_SYNC_FOLD_out__DESC', `AKS', `*HID')
rule(`SAC_MT_SYNC_FOLD_out', `SAC_MT_SYNC_FOLD_out__DESC', `AKD', `*HID')
rule(`SAC_MT_SYNC_FOLD_out', `SAC_MT_SYNC_FOLD_out__DESC', `AUD', `*HID')

#endif  /* SAC_DO_MULTITHREAD */

#endif  /* SAC_SIMD_COMPILATION */

#endif  /* _SAC_MT_GEN_H_ */

end_icm_definition
