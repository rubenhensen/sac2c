/*
 *
 * $Log$
 * Revision 2.1  1999/02/23 12:43:53  sacbase
 * new release made
 *
 * Revision 1.3  1998/05/07 08:17:51  cg
 * SAC header files converted to new naming conventions.
 *
 * Revision 1.2  1998/03/24 13:52:49  cg
 * function declaration _SAC_PrintHeader moved to sac_profile.h
 *
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

extern void SAC_RuntimeError (char *format, ...);
extern void SAC_Print (char *format, ...);

#endif /* SAC_MESSAGE_H */
