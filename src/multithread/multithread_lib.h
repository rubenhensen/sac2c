/*
 *
 * $Log$
 * Revision 1.4  2000/03/28 09:48:44  jhs
 * Added refcunt-macros
 *
 * Revision 1.3  2000/03/09 18:34:47  jhs
 * Additional features.
 *
 * Revision 1.2  2000/03/02 13:01:02  jhs
 * Added MUTHExchangeApplication,
 * added MUTHExpandFundefName,
 * added DBUG_PRINTS and DBUG_ASSERTS.
 *
 * Revision 1.1  2000/02/21 17:48:22  jhs
 * Initial revision
 *
 *
 */

#ifndef MULTITHREAD_LIB_H

#define MULTITHREAD_LIB_H

/*
 *  returns 0 for refcounting-objects and -1 otherwise
 */
#define GET_ZERO_REFCNT(prefix, node) ((prefix##_REFCNT (node) >= 0) ? 0 : -1)

/*
 *  returns 1 for refcounting-objects and -1 otherwise
 */
#define GET_STD_REFCNT(prefix, node) ((prefix##_REFCNT (node) >= 0) ? 1 : -1)

extern node *MUTHBlocksLastInstruction (node *block);
extern node *MUTHMeltBlocks (node *first_block, node *second_block);
extern node *MUTHMeltBlocksOnCopies (node *first_block, node *second_block);
extern node *MUTHExchangeApplication (node *arg_node, node *new_fundef);
extern node *MUTHExpandFundefName (node *fundef, char *prefix);

#endif /* CONCURRENT_LIB_H */
