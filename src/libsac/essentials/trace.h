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

/*
 *  External declarations of global variables and functions defined in trace.c
 *  as part of libsac.
 */
SAC_C_EXTERN void SAC_TR_Print (char *format, ...);
SAC_C_EXTERN void SAC_TR_OptPrint (bool doPrint, char *format, ...);

SAC_C_EXTERN int SAC_TR_hidden_memcnt;
SAC_C_EXTERN int SAC_TR_array_memcnt;

SAC_C_EXTERN void SAC_TR_IncArrayMemcnt (int size);
SAC_C_EXTERN void SAC_TR_DecArrayMemcnt (int size);
SAC_C_EXTERN void SAC_TR_IncHiddenMemcnt (int size);
SAC_C_EXTERN void SAC_TR_DecHiddenMemcnt (int size);

#endif /* _SAC_TRACE_H */

