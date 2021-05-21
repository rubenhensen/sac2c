/*****************************************************************************
 *
 * file:   trace.h
 *
 * description:
 *
 *   This file is part of the SAC standard header file sac.h
 *
 *   It provides the interface for trace.c
 *
 *****************************************************************************/

#ifndef _SAC_TRACE_H
#define _SAC_TRACE_H

#ifndef SAC_C_EXTERN
#define SAC_C_EXTERN extern
#endif /* SAC_C_EXTERN */

#include "runtime/essentials_h/bool.h"

#define SAC_TR_LIBSAC_PRINT(msg) (SAC_MT_do_trace ? SAC_TR_Print msg : (void)0)

/*
 *  External declarations of functions defined in trace.c as part of libsac.
 */
SAC_C_EXTERN void SAC_TR_Print (const char *format, ...);

SAC_C_EXTERN void SAC_TR_IncArrayMemcnt (int size);
SAC_C_EXTERN void SAC_TR_DecArrayMemcnt (int size);
SAC_C_EXTERN void SAC_TR_IncHiddenMemcnt (int size);
SAC_C_EXTERN void SAC_TR_DecHiddenMemcnt (int size);

#endif /* _SAC_TRACE_H */
