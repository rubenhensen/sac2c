/*
 *
 * $Log$
 * Revision 3.1  2000/11/20 18:02:10  sacbase
 * new release made
 *
 * Revision 2.2  2000/06/30 13:20:46  nmw
 * conditional define for typedef bool added
 *
 * Revision 2.1  1999/02/23 12:43:47  sacbase
 * new release made
 *
 * Revision 1.1  1998/03/19 16:37:28  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   sac_bool.h
 *
 * description:
 *
 *   This file is part of the SAC standard header file sac.h
 *
 *   It provides type and macro definitions for the implementation of
 *   the SAC standard data type bool.
 *
 *****************************************************************************/

#ifndef SAC_BOOL_H

#define SAC_BOOL_H

#ifndef _sac_types_h
typedef int bool;
#endif

#define true 1
#define false 0

#endif /* SAC_BOOL_H */
