/*
 *
 * $Log$
 * Revision 3.2  2002/04/30 09:10:46  dkr
 * no changes done
 *
 * Revision 3.1  2000/11/20 18:02:11  sacbase
 * new release made
 *
 * Revision 2.4  1999/10/25 15:38:08  sbs
 * changed SAC_BC_WRITE and SAC_BC_READ:
 * ( check_bound ? 0 : SAC_RuntimeError(...))    is not correct,
 * since 0 is int and SAC_RuntimeError(...) is void!!!
 * This causes problems on the alpha!!!
 * The new solution is:
 * ( check_bound ? 0 : (SAC_RuntimeError(...), 0))    which is ugly
 * but ANSI compliant and as efficient as the former solution!
 *
 * Revision 2.3  1999/07/06 11:00:57  sbs
 * SAC_DO_BOUNDCHECK changed in SAC_DO_CHECK_BOUNDARY !
 *
 * Revision 2.2  1999/04/12 09:41:44  cg
 * Boundchecking is completely redesigned.
 * Now, any C array access can be checked on demand.
 *
 * Revision 2.1  1999/02/23 12:43:49  sacbase
 * new release made
 *
 * Revision 1.2  1998/05/07 08:17:51  cg
 * SAC header files converted to new naming conventions.
 *
 * Revision 1.1  1998/03/19 16:37:58  cg
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   sac_boundcheck.h
 *
 * description:
 *
 *   This file is part of the SAC standard header file sac.h
 *
 *   It provides macros for selectively checking array bounds upon access.
 *
 *
 *****************************************************************************/

/*
 *  REMARK:
 *
 * The current way of boundcchecking is not yet satisfying.
 *
 * Bounds are not checked in each dimension, but only accesses
 * are determined which exceed the memory allocated for the accessed
 * array. However, doing boundary checking correctly would require
 * much more effort.
 *
 */

#ifndef _SAC_BOUNDCHECK_H
#define _SAC_BOUNDCHECK_H

#if SAC_DO_CHECK_BOUNDARY

#include "sac_message.h"

extern char SAC_BC_format_write[];
extern char SAC_BC_format_read[];

#define SAC_BC_WRITE(nt, pos)                                                            \
    (((pos >= 0) && (pos < SAC_ND_A_SIZE (nt)))                                          \
       ? 0                                                                               \
       : (SAC_RuntimeError (SAC_BC_format_write, #nt, SAC_ND_A_SIZE (nt), pos), 0)),

#define SAC_BC_READ(nt, pos)                                                             \
    (((pos >= 0) && (pos < SAC_ND_A_SIZE (nt)))                                          \
       ? 0                                                                               \
       : (SAC_RuntimeError (SAC_BC_format_read, #nt, SAC_ND_A_SIZE (nt), pos), 0)),

#else /* SAC_DO_CHECK_BOUNDARY */

#define SAC_BC_WRITE(nt, pos)
#define SAC_BC_READ(nt, pos)

#endif /* SAC_DO_CHECK_BOUNDARY */

#endif /* _SAC_BOUNDCHECK_H */
