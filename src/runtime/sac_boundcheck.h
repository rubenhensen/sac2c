/*
 *
 * $Log$
 * Revision 2.1  1999/02/23 12:43:49  sacbase
 * new release made
 *
 * Revision 1.2  1998/05/07 08:17:51  cg
 * SAC header files converted to new naming conventions.
 *
 * Revision 1.1  1998/03/19 16:37:58  cg
 * Initial revision
 *
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
 *   Bound checking is activated by the global switch BOUNDCHECK.
 *
 *****************************************************************************/

/*
 *  REMARK:
 *
 * The current way of boundcchecking is absolutely unsatisfying.
 *
 * First, bounds are not checked in each dimension, but only accesses
 * are determined which exceed the memory allocated for the accessed
 * array.
 *
 * Second, boundchecking is actually not implemented as a selectively
 * activatible macro, but is hardcoded into the psi operation. The macro
 * defined here is only used for generating an error message, but not
 * for the check itself.
 */

#ifndef SAC_BOUNDCHECK_H

#define SAC_BOUNDCHECK_H

#if SAC_DO_BOUNDCHECK

#define SAC_OUT_OF_BOUND(line, prf, size, idx)                                           \
    {                                                                                    \
        SAC_RuntimeError ("%d: access in function %s is out"                             \
                          " of range (size: %d, index:%d)",                              \
                          line, prf, size, idx);                                         \
    }

#else /* SAC_DO_BOUNDCHECK */

#define SAC_OUT_OF_BOUND(line, prf, size, idx)

#endif /* SAC_DO_BOUNDCHECK */

#endif /* SAC_BOUNDCHECK_H */
