/*****************************************************************************
 *
 * file:   sac_message.h
 *
 * description:
 *
 *   This file is part of the SAC standard header file sac.h
 *
 *   It only contains prototypes of functions defined in message.c
 *
 *****************************************************************************/

#ifndef _SAC_MESSAGE_H_
#define _SAC_MESSAGE_H_

#ifndef SAC_C_EXTERN
#define SAC_C_EXTERN extern
#endif /* SAC_C_EXTERN */

SAC_C_EXTERN void SAC_RuntimeError (char *format, ...);
SAC_C_EXTERN void SAC_RuntimeError_Mult (int cnt, ...);
SAC_C_EXTERN void SAC_RuntimeErrorLine (int line, char *format, ...);
SAC_C_EXTERN void SAC_RuntimeWarning (char *format, ...);
SAC_C_EXTERN void SAC_RuntimeWarningMaster (char *format, ...);
SAC_C_EXTERN const char *SAC_PrintShape (SAC_array_descriptor_t desc);
SAC_C_EXTERN void SAC_Print (char *format, ...);

#endif /* _SAC_MESSAGE_H_ */
