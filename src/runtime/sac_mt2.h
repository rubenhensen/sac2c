/*
 *
 * $Log$
 * Revision 3.2  2002/04/30 08:41:58  dkr
 * no changes done
 *
 * Revision 3.1  2000/11/20 18:02:18  sacbase
 * new release made
 *
 * Revision 1.1  2000/07/21 14:27:48  jhs
 * Initial revision
 *
 */

/******************************************************************************
 *
 * file:   sac_mt2.h
 *
 * prefix: SAC_MT2_
 *
 * description:
 *   This file is part of the SAC standard header file sac.h
 *
 *   It is the major header file of the implementation of the second
 *   generation of multi-threading facilities.
 *
 ******************************************************************************/

#ifndef _SAC_MT_H
#define _SAC_MT_H

#if SAC_DO_MULTITHREAD_2

/***
 ***   Definitions and declarations for the second generation of
 ***   multi-threaded runtime system.
 ***/

#define TID tid_t tid,

#define FLAGS flags_t flags,

#define CALL_MTLIFT TID FLAGS

#define CALL_MTWORKER TID FLAGS

#define CALL_MTMASTER FLAGS

#define CALL_MTREP TID

#else /* SAC_DO_MULTITHRED_2 */

/***
 ***  Definitions and declarations for sequential execution,
 ***  mostly dummies for second generation macros.
 ***/

#endif /* SAC_DO_MULTITHRED_2 */

#endif /* _SAC_MT_H */
