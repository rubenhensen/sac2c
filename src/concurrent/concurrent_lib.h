/*
 *
 * $Log$
 * Revision 1.3  1999/08/02 09:48:35  jhs
 * Moved MeltBlocks[OnCopies] from spmd_opt.[ch] to concurrent_lib.[ch].
 *
 * Revision 1.2  1999/07/30 13:45:44  jhs
 * Added initial macros and functions.
 *
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

extern void CONLDisplayMask (char *tag, char *name, DFMmask_t mask);

extern void AssertSimpleBlock (node *block);

extern node *MeltBlocks (node *first_block, node *second_block);
extern node *MeltBlocksOnCopies (node *first_block, node *second_block);

#endif /* CONCURRENT_LIB_H */
