/*
 *
 * $Log$
 * Revision 3.3  2003/04/29 11:55:05  cg
 * Added function SAC_RuntimeWarning in analogy to SAC_RuntimeError.
 *
 * Revision 3.2  2002/04/30 08:05:18  dkr
 * no changes done
 *
 * Revision 3.1  2000/11/20 18:02:16  sacbase
 * new release made
 *
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

#ifndef _SAC_MESSAGE_H
#define _SAC_MESSAGE_H

extern void SAC_RuntimeError (char *format, ...);
extern void SAC_RuntimeWarning (char *format, ...);
extern void SAC_Print (char *format, ...);

#endif /* _SAC_MESSAGE_H */
