/*
 *
 * $Log$
 * Revision 3.2  2001/05/08 12:28:02  dkr
 * new macros for RC used
 *
 * Revision 3.1  2000/11/20 18:02:26  sacbase
 * new release made
 *
 * Revision 1.3  1999/08/02 09:48:35  jhs
 * Moved MeltBlocks[OnCopies] from spmd_opt.[ch] to concurrent_lib.[ch].
 *
 * Revision 1.2  1999/07/30 13:45:44  jhs
 * Added initial macros and functions.
 *
 * Revision 1.1  1999/07/30 12:34:50  jhs
 * Initial revision
 *
 */

#ifndef _SAC_CONCURRENT_LIB_H_
#define _SAC_CONCURRENT_LIB_H_

#include "refcount.h"

/*
 *  returns 0 for refcounting-objects and -1 otherwise
 */
#define GET_ZERO_REFCNT(prefix, node)                                                    \
    (RC_IS_ACTIVE (prefix##_REFCNT (node)) ? 0 : RC_INACTIVE)

/*
 *  returns 1 for refcounting-objects and -1 otherwise
 */
#define GET_STD_REFCNT(prefix, node)                                                     \
    (RC_IS_ACTIVE (prefix##_REFCNT (node)) ? 1 : RC_INACTIVE)

extern void CONLDisplayMask (char *tag, char *name, DFMmask_t mask);

extern void AssertSimpleBlock (node *block);

extern node *MeltBlocks (node *first_block, node *second_block);
extern node *MeltBlocksOnCopies (node *first_block, node *second_block);

#endif /* _SAC_CONCURRENT_LIB_H_ */
