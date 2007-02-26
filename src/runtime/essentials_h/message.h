/*
 *
 * $Id$
 *
 */

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

extern void SAC_RuntimeError (char *format, ...);
extern void SAC_RuntimeError_Mult (int cnt, ...);
extern void SAC_RuntimeErrorLine (int line, char *format, ...);
extern void SAC_RuntimeWarning (char *format, ...);
extern const char *SAC_PrintShape (SAC_array_descriptor_t desc);
extern void SAC_Print (char *format, ...);

#endif /* _SAC_MESSAGE_H_ */
