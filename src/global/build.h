/*
 *
 * $Log$
 * Revision 2.1  1999/05/12 14:27:24  cg
 * initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   build.h
 *
 * prefix:
 *
 * description:
 *
 *   This header file contains declarations of variables used for the -V
 *   command line option. The corresponding build.c file is created by
 *   make upon each compilation attempt.
 *
 *
 *****************************************************************************/

#ifndef _build_h

#define _build_h

extern char build_date[];
extern char build_user[];
extern char build_host[];

#endif /* _build_h */
