/*
 *
 * $Log$
 * Revision 1.1  1998/03/19 16:54:15  cg
 * Initial revision
 *
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

#ifndef SAC_MESSAGE_H

#define SAC_MESSAGE_H

extern void _SAC_RuntimeError (char *format, ...);
extern void _SAC_Print (char *format, ...);
extern void _SAC_PrintHeader (char *title);

#endif /* SAC_MESSAGE_H */
