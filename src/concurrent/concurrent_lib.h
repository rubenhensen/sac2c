/*
 *
 * $Log$
 * Revision 1.1  1999/07/30 12:34:50  jhs
 * Initial revision
 *
 *
 */

#ifndef CONCURRENT_LIB_H

#define CONCURRENT_LIB_H

/*
 *  returns 0 for refcounting-objects and -1 otherwise
 */
#define GET_ZERO_REFCNT(prefix, node) ((prefix##_REFCNT (node) >= 0) ? 0 : -1)

/*
 *  returns 1 for refcounting-objects and -1 otherwise
 */
#define GET_STD_REFCNT(prefix, node) ((prefix##_REFCNT (node) >= 0) ? 1 : -1)

#endif /* CONCURRENT_LIB_H */
