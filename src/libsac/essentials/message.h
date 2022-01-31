/*****************************************************************************
 *
 * file:   message.h
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

#include "runtime/essentials_h/std.h"
#include "fun-attrs.h"

SAC_C_EXTERN void (*SAC_MessageExtensionCallback) (void);
SAC_C_EXTERN void SAC_RuntimeError (const char *format, ...) FUN_ATTR_NORETURN;
SAC_C_EXTERN void SAC_RuntimeError_Mult (int cnt, ...) FUN_ATTR_NORETURN;
SAC_C_EXTERN void SAC_RuntimeErrorLine (int line, const char *format, ...) FUN_ATTR_NORETURN;
SAC_C_EXTERN void SAC_RuntimeWarning (const char *format, ...);
SAC_C_EXTERN void SAC_RuntimeWarningMaster (const char *format, ...);
SAC_C_EXTERN const char *SAC_PrintShape (SAC_array_descriptor_t desc);
SAC_C_EXTERN void SAC_Print (const char *format, ...);

#endif /* _SAC_MESSAGE_H_ */

